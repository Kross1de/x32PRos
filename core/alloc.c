/*
 * Kevin Lange's Slab Allocator
 *
 * Implemented for CS241, Fall 2010, machine problem 7
 * at the University of Illinois, Urbana-Champaign.
 *
 * Overall competition winner for speed.
 * Well ranked in memory usage.
 *
 * Copyright (c) 2010 Kevin Lange.  All rights reserved.
 *
 * Developed by: Kevin Lange <lange7@acm.uiuc.edu>
 *               Dave Majnemer <dmajnem2@acm.uiuc.edu>
 *               Assocation for Computing Machinery
 *               University of Illinois, Urbana-Champaign
 *               http://acm.uiuc.edu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimers.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimers in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the Association for Computing Machinery, the
 *      University of Illinois, nor the names of its contributors may be used
 *      to endorse or promote products derived from this Software without
 *      specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 *
 * ##########
 * # README #
 * ##########
 *
 * About the slab allocator
 * """"""""""""""""""""""""
 *
 * This is a simple implementation of a "slab" allocator. It works by operating
 * on "bins" of items of predefined sizes and a set of pseudo-bins of any size.
 * When a new allocation request is made, the allocator determines if it will
 * fit in an existing bin. If there are no bins of the correct size for a given
 * allocation request, the allocator will make a bin and add it to a(n empty)
 * list of available bins of that size. In this implementation, we use sizes
 * from 4 bytes (32 bit) or 8 bytes (64-bit) to 2KB for bins, fitting a 4K page
 * size. The implementation allows the number of pages in a single bin to be
 * increased, as well as allowing for changing the size of page (though this
 * should, for the most part, remain 4KB under any modern system).
 *
 * Special thanks
 * """"""""""""""
 *
 * I would like to thank Dave Majnemer, who I have credited above as a
 * contributor, for his assistance. Without Dave, klmalloc would be a mash
 * up of bits of forward movement in no discernible pattern. Dave helped
 * me ensure that I could build a proper slab allocator and has consantly
 * derided me for not fixing the bugs and to-do items listed in the last
 * section of this readme.
 *
 * GCC Function Attributes
 * """""""""""""""""""""""
 *
 * A couple of GCC function attributes, designated by the __attribute__
 * directive, are used in this code to streamline optimization.
 * I've chosen to include a brief overview of the particular attributes
 * I am making use of:
 *
 * - malloc:
 *   Tells gcc that a given function is a memory allocator
 *   and that non-NULL values it returns should never be
 *   associated with other chunks of memory. We use this for
 *   alloc, realloc and calloc, as is requested in the gcc
 *   documentation for the attribute.
 *
 * - always_inline:
 *   Tells gcc to always inline the given code, regardless of the
 *   optmization level. Small functions that would be noticeably
 *   slower with the overhead of paramter handling are given
 *   this attribute.
 *
 * - pure:
 *   Tells gcc that a function only uses inputs and its output.
 *
 * Things to work on
 * """""""""""""""""
 *
 * TODO: Try to be more consistent on comment widths...
 * FIXME: Make thread safe! Not necessary for competition, but would be nice.
 * FIXME: Splitting/coalescing is broken. Fix this ASAP!
 *
**/

/*
 * Disable assertions when not in debug mode,
 * as this saves a lot of time.
 */
#if !defined(DEBUG)
#define NDEBUG
#endif

/* Define standard constants for freestanding environment */
#define CHAR_BIT 8
#define INT32_MAX 0x7FFFFFFF

/* Includes */
#include "../include/kernel.h"

/* Definitions */
#if SIZE_MAX == UINT32_MAX
#define NUM_BINS 11UL                               /* Number of bins, total, under 32-bit. */
#define SMALLEST_BIN_LOG 2UL                        /* Logarithm base two of the smallest bin: log_2(sizeof(int32)). */
#else
#define NUM_BINS 10UL                               /* Number of bins, total, under 64-bit. */
#define SMALLEST_BIN_LOG 3UL                        /* Logarithm base two of the smallest bin: log_2(sizeof(int64)). */
#endif

#define BIG_BIN (NUM_BINS - 1)                      /* Index for the big bin, (NUM_BINS - 1) */
#define SMALLEST_BIN (1UL << SMALLEST_BIN_LOG)      /* Size of the smallest bin. */

#define PAGE_SIZE 0x1000                            /* Size of a page (in bytes), should be 4KB */
#define PAGE_MASK (PAGE_SIZE - 1)                   /* Block mask, size of a page * number of pages - 1. */
#define SKIP_P INT32_MAX                            /* INT32_MAX is half of UINT32_MAX; this gives us a 50% marker for skip lists. */
#define SKIP_MAX_LEVEL 6                            /* We have a maximum of 6 levels in our skip lists. */

/*
 * Internal functions.
 */
static void * __attribute__ ((malloc)) klmalloc(size_t size);
static void * __attribute__ ((malloc)) klrealloc(void * ptr, size_t size);
static void * __attribute__ ((malloc)) klcalloc(size_t nmemb, size_t size);
static void klfree(void * ptr);

void * __attribute__ ((malloc)) malloc(size_t size) {
    return klmalloc(size);
}

void * __attribute__ ((malloc)) realloc(void * ptr, size_t size) {
    return klrealloc(ptr, size);
}

void * __attribute__ ((malloc)) calloc(size_t nmemb, size_t size) {
    return klcalloc(nmemb, size);
}

void free(void * ptr) {
    klfree(ptr);
}

/*
 * Adjust bin size in bin_size call to proper bounds.
 */
static inline size_t __attribute__ ((always_inline, pure)) klmalloc_adjust_bin(size_t bin)
{
    if (bin <= SMALLEST_BIN_LOG)
    {
        return 0;
    }
    bin -= SMALLEST_BIN_LOG + 1;
    if (bin > BIG_BIN) {
        return BIG_BIN;
    }
    return bin;
}

/*
 * Given a size value, find the correct bin
 * to place the requested allocation in.
 */
static inline size_t __attribute__ ((always_inline, pure)) klmalloc_bin_size(size_t size) {
    size_t bin = sizeof(size) * CHAR_BIT - __builtin_clzl(size);
    bin += !!(size & (size - 1));
    return klmalloc_adjust_bin(bin);
}

/*
 * Bin header - One page of memory.
 * Appears at the front of a bin to point to the
 * previous bin (or NULL if the first), the next bin
 * (or NULL if the last) and the head of the bin, which
 * is a stack of cells of data.
 */
typedef struct _klmalloc_bin_header {
    struct _klmalloc_bin_header * next;  /* Pointer to the next node. */
    void * head;                        /* Head of this bin. */
    size_t size;                        /* Size of this bin, if big; otherwise bin index. */
} klmalloc_bin_header;

/*
 * A big bin header is basically the same as a regular bin header
 * only with a pointer to the previous (physically) instead of
 * a "next" and with a list of forward headers.
 */
typedef struct _klmalloc_big_bin_header {
    struct _klmalloc_big_bin_header * next;
    void * head;
    size_t size;
    struct _klmalloc_big_bin_header * prev;
    struct _klmalloc_big_bin_header * forward[SKIP_MAX_LEVEL+1];
} klmalloc_big_bin_header;

/*
 * List of pages in a bin.
 */
typedef struct _klmalloc_bin_header_head {
    klmalloc_bin_header * first;
} klmalloc_bin_header_head;

/*
 * Array of available bins.
 */
static klmalloc_bin_header_head klmalloc_bin_head[NUM_BINS - 1]; /* Small bins */
static struct _klmalloc_big_bins {
    klmalloc_big_bin_header head;
    int level;
} klmalloc_big_bins;
static klmalloc_big_bin_header * klmalloc_newest_big = NULL;     /* Newest big bin */

/*
 * Remove an entry from a page list.
 * Decouples the element from its
 * position in the list by linking
 * its neighbors to eachother.
 */
static inline void __attribute__ ((always_inline)) klmalloc_list_decouple(klmalloc_bin_header_head *head, klmalloc_bin_header *node) {
    klmalloc_bin_header *next = node->next;
    head->first = next;
    node->next = NULL;
}

/*
 * Insert an entry into a page list.
 * The new entry is placed at the front
 * of the list and the existing border
 * elements are updated to point back
 * to it (our list is doubly linked).
 */
static inline void __attribute__ ((always_inline)) klmalloc_list_insert(klmalloc_bin_header_head *head, klmalloc_bin_header *node) {
    node->next = head->first;
    head->first = node;
}

/*
 * Get the head of a page list.
 * Because redundant function calls
 * are really great, and just in case
 * we change the list implementation.
 */
static inline klmalloc_bin_header * __attribute__ ((always_inline)) klmalloc_list_head(klmalloc_bin_header_head *head) {
    return head->first;
}

/*
 * Generate a random value in an appropriate range.
 * This is a xor-shift RNG.
 */
static uint32 __attribute__ ((pure)) klmalloc_skip_rand() {
    static uint32 x = 123456789;
    static uint32 y = 362436069;
    static uint32 z = 521288629;
    static uint32 w = 88675123;

    uint32 t;

    t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

/*
 * Generate a random level for a skip node
 */
static inline int __attribute__ ((pure, always_inline)) klmalloc_random_level() {
    int level = 0;
    /*
     * Keep trying to check rand() against 50% of its maximum.
     * This provides 50%, 25%, 12.5%, etc. chance for each level.
     */
    while (klmalloc_skip_rand() < SKIP_P && level < SKIP_MAX_LEVEL) {
        ++level;
    }
    return level;
}

/*
 * Find best fit for a given value.
 */
static klmalloc_big_bin_header * klmalloc_skip_list_findbest(size_t search_size) {
    klmalloc_big_bin_header * node = &klmalloc_big_bins.head;
    /*
     * Loop through the skip list until we hit something > our search value.
     */
    int i;
    for (i = klmalloc_big_bins.level; i >= 0; --i) {
        while (node->forward[i] && (node->forward[i]->size < search_size)) {
            node = node->forward[i];
            if (node)
                ASSERT((node->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
        }
    }
    /*
     * This value will either be NULL (we found nothing)
     * or a node (we found a minimum fit).
     */
    node = node->forward[0];
    if (node) {
        ASSERT((uintptr_t)node % PAGE_SIZE == 0);
        ASSERT((node->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
    }
    return node;
}

/*
 * Insert a header into the skip list.
 */
static void klmalloc_skip_list_insert(klmalloc_big_bin_header * value) {
    /*
     * You better be giving me something valid to insert,
     * or I will slit your ****ing throat.
     */
    ASSERT(value != NULL);
    ASSERT(value->head != NULL);
    ASSERT((uintptr_t)value % PAGE_SIZE == 0);
    ASSERT((value->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
    ASSERT(value->size != 0);

    /*
     * Starting from the head node of the bin locator...
     */
    klmalloc_big_bin_header * node = &klmalloc_big_bins.head;
    klmalloc_big_bin_header * update[SKIP_MAX_LEVEL + 1];

    /*
     * Loop through the skiplist to find the right place
     * to insert the node (where ->forward[] > value)
     */
    int i;
    for (i = klmalloc_big_bins.level; i >= 0; --i) {
        while (node->forward[i] && node->forward[i]->size < value->size) {
            node = node->forward[i];
            if (node)
                ASSERT((node->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
        }
        update[i] = node;
    }
    node = node->forward[0];

    /*
     * Make the new skip node and update
     * the forward values.
     */
    if (node != value) {
        int level = klmalloc_random_level();
        /*
         * Get all of the nodes before this.
         */
        if (level > klmalloc_big_bins.level) {
            for (i = klmalloc_big_bins.level + 1; i <= level; ++i) {
                update[i] = &klmalloc_big_bins.head;
            }
            klmalloc_big_bins.level = level;
        }

        /*
         * Make the new node.
         */
        node = value;

        /*
         * Run through and point the preceeding nodes
         * for each level to the new node.
         */
        for (i = 0; i <= level; ++i) {
            node->forward[i] = update[i]->forward[i];
            if (node->forward[i])
                ASSERT((node->forward[i]->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
            update[i]->forward[i] = node;
        }
    }
}

/*
 * Delete a header from the skip list.
 * Be sure you didn't change the size, or we won't be able to find it.
 */
static void klmalloc_skip_list_delete(klmalloc_big_bin_header * value) {
    /*
     * Debug assertions
     */
    ASSERT(value != NULL);
    ASSERT(value->head);

    /*
     * Starting from the bin header, again...
     */
    klmalloc_big_bin_header * node = &klmalloc_big_bins.head;
    klmalloc_big_bin_header * update[SKIP_MAX_LEVEL + 1];

    /*
     * Find the node.
     */
    int i;
    for (i = klmalloc_big_bins.level; i >= 0; --i) {
        while (node->forward[i] && node->forward[i]->size < value->size) {
            node = node->forward[i];
            if (node)
                ASSERT((node->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
        }
        update[i] = node;
    }
    node = node->forward[0];
    while (node != value) {
        node = node->forward[0];
    }

    if (node != value) {
        node = klmalloc_big_bins.head.forward[0];
        while (node->forward[0] && node->forward[0] != value) {
            node = node->forward[0];
        }
        node = node->forward[0];
    }
    /*
     * If we found the node, delete it;
     * otherwise, we do nothing.
     */
    if (node == value) {
        for (i = 0; i <= klmalloc_big_bins.level; ++i) {
            if (update[i]->forward[i] != node) {
                break;
            }
            update[i]->forward[i] = node->forward[i];
            if (update[i]->forward[i]) {
                ASSERT((uintptr_t)(update[i]->forward[i]) % PAGE_SIZE == 0);
                ASSERT((update[i]->forward[i]->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
            }
        }

        while (klmalloc_big_bins.level > 0 && klmalloc_big_bins.head.forward[klmalloc_big_bins.level] == NULL) {
            --klmalloc_big_bins.level;
        }
    }
}

/*
 * Pop an item from a block.
 * Free space is stored as a stack,
 * so we get a free space for a bin
 * by popping a free node from the
 * top of the stack.
 */
static void * klmalloc_stack_pop(klmalloc_bin_header *header) {
    ASSERT(header);
    ASSERT(header->head != NULL);

    /*
     * Remove the current head and point
     * the head to where the old head pointed.
     */
    void *item = header->head;
    size_t **head = header->head;
    size_t *next = *head;
    header->head = next;
    return item;
}

/*
 * Push an item into a block.
 * When we free memory, we need
 * to add the freed cell back
 * into the stack of free spaces
 * for the block.
 */
static void klmalloc_stack_push(klmalloc_bin_header *header, void *ptr) {
    ASSERT(ptr != NULL);
    size_t **item = (size_t **)ptr;
    *item = (size_t *)header->head;
    header->head = item;
}

/*
 * Is this cell stack empty?
 * If the head of the stack points
 * to NULL, we have exhausted the
 * stack, so there is no more free
 * space available in the block.
 */
static inline int __attribute__ ((always_inline)) klmalloc_stack_empty(klmalloc_bin_header *header) {
    return header->head == NULL;
}

/*
 * C standard implementation:
 * If size is zero, we can choose do a number of things.
 * This implementation will return a NULL pointer.
 */
static void * __attribute__ ((malloc)) klmalloc(size_t size) {
    if (__builtin_expect(size == 0, 0))
        return NULL;

    /*
     * Find the appropriate bin for the requested
     * allocation and start looking through that list.
     */
    unsigned int bucket_id = klmalloc_bin_size(size);

    if (bucket_id < BIG_BIN) {
        /*
         * Small bins.
         */
        klmalloc_bin_header * bin_header = klmalloc_list_head(&klmalloc_bin_head[bucket_id]);
        if (!bin_header) {
            /*
             * Grow the heap for the new bin.
             */
            bin_header = (klmalloc_bin_header*)sbrk(PAGE_SIZE);
            ASSERT((uintptr_t)bin_header % PAGE_SIZE == 0);        

            /*
             * Set the head of the stack.
             */
            bin_header->head = (void*)((uintptr_t)bin_header + sizeof(klmalloc_bin_header));
            /*
             * Insert the new bin at the front of
             * the list of bins for this size.
             */
            klmalloc_list_insert(&klmalloc_bin_head[bucket_id], bin_header);
            /*
             * Initialize the stack inside the bin.
             * The stack is initially full, with each
             * entry pointing to the next until the end
             * which points to NULL.
             */
            size_t adj = SMALLEST_BIN_LOG + bucket_id;
            size_t i, available = ((PAGE_SIZE - sizeof(klmalloc_bin_header)) >> adj) - 1;

            size_t **base = bin_header->head;
            for (i = 0; i < available; ++i) {
                /*
                 * Our available memory is made into a stack, with each
                 * piece of memory turned into a pointer to the next
                 * available piece. When we want to get a new piece
                 * of memory from this block, we just pop off a free
                 * spot and give its address.
                 */
                base[i << bucket_id] = (size_t *)&base[(i + 1) << bucket_id];
            }
            base[available << bucket_id] = NULL;
            bin_header->size = bucket_id;
        }
        size_t ** item = klmalloc_stack_pop(bin_header);
        if (klmalloc_stack_empty(bin_header)) {
            klmalloc_list_decouple(&(klmalloc_bin_head[bucket_id]),bin_header);
        }
        return item;
    } else {
        /*
         * Big bins.
         */
        klmalloc_big_bin_header * bin_header = klmalloc_skip_list_findbest(size);
        if (bin_header) {
            /*
             * If we found one, delete it from the skip list
             */
            klmalloc_skip_list_delete(bin_header);
            /*
             * Retreive the head of the block.
             */
            size_t ** item = klmalloc_stack_pop((klmalloc_bin_header *)bin_header);
#if 0
            /*
             * Resize block, if necessary
             */
            ASSERT(bin_header->head == NULL);
            size_t old_size = bin_header->size;
            /*
             * Round the requeste size to our full required size.
             */
            size = ((size + sizeof(klmalloc_big_bin_header)) / PAGE_SIZE + 1) * PAGE_SIZE - sizeof(klmalloc_big_bin_header);
            ASSERT((size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
            if (bin_header->size > size * 2) {
                ASSERT(old_size != size);
                /*
                 * If we have extra space, start splitting.
                 */
                bin_header->size = size;
                ASSERT(sbrk(0) >= bin_header->size + (uintptr_t)bin_header);
                /*
                 * Make a new block at the end of the needed space.
                 */
                klmalloc_big_bin_header * header_new = (klmalloc_big_bin_header *)((uintptr_t)bin_header + sizeof(klmalloc_big_bin_header) + size);
                ASSERT((uintptr_t)header_new % PAGE_SIZE == 0);
                memset(header_new, 0, sizeof(klmalloc_big_bin_header) + sizeof(void *));
                header_new->prev = bin_header;
                if (bin_header->next) {
                    bin_header->next->prev = header_new;
                }
                header_new->next = bin_header->next;
                bin_header->next = header_new;
                if (klmalloc_newest_big == bin_header) {
                    klmalloc_newest_big = header_new;
                }
                header_new->size = old_size - (size + sizeof(klmalloc_big_bin_header));
                ASSERT(((uintptr_t)header_new->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
                kprintf("Splitting %p [now %zx] at %p [%zx] from [%zx,%zx].\n", (void*)bin_header, bin_header->size, (void*)header_new, header_new->size, old_size, size);
                /*
                 * Free the new block.
                 */
                klfree((void *)((uintptr_t)header_new + sizeof(klmalloc_big_bin_header)));
            }
#endif
            return item;
        } else {
            /*
             * Round requested size to a set of pages, plus the header size.
             */
            size_t pages = (size + sizeof(klmalloc_big_bin_header)) / PAGE_SIZE + 1;
            bin_header = (klmalloc_big_bin_header*)sbrk(PAGE_SIZE * pages);
            ASSERT((uintptr_t)bin_header % PAGE_SIZE == 0);
            /*
             * Give the header the remaining space.
             */
            bin_header->size = pages * PAGE_SIZE - sizeof(klmalloc_big_bin_header);
            ASSERT((bin_header->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
            /*
             * Link the block in physical memory.
             */
            bin_header->prev = klmalloc_newest_big;
            if (bin_header->prev) {
                bin_header->prev->next = bin_header;
            }
            klmalloc_newest_big = bin_header;
            bin_header->next = NULL;
            /*
             * Return the head of the block.
             */
            bin_header->head = NULL;
            return (void*)((uintptr_t)bin_header + sizeof(klmalloc_big_bin_header));
        }
    }
}

/*
 * C standard implementation: Do nothing when NULL is passed to free.
 */
static void klfree(void *ptr) {
    if (__builtin_expect(ptr == NULL, 0)) {
        return;
    }

    /*
     * Get our pointer to the head of this block by
     * page aligning it.
     */
    klmalloc_bin_header * header = (klmalloc_bin_header *)((uintptr_t)ptr & (size_t)~PAGE_MASK);
    ASSERT((uintptr_t)header % PAGE_SIZE == 0);

    /*
     * For small bins, the bin number is stored in the size
     * field of the header. For large bins, the actual size
     * available in the bin is stored in this field. It's
     * easy to tell which is which, though.
     */
    size_t bucket_id = header->size;
    if (bucket_id > NUM_BINS) {
        bucket_id = BIG_BIN;
        klmalloc_big_bin_header *bheader = (klmalloc_big_bin_header*)header;
        
        ASSERT(bheader);
        ASSERT(bheader->head == NULL);
        ASSERT((bheader->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
        /*
         * Coalesce forward blocks into us.
         */
#if 0
        if (bheader != klmalloc_newest_big) {
            /*
             * If we are not the newest big bin, there is most definitely
             * something in front of us that we can read.
             */
            ASSERT((bheader->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
            klmalloc_big_bin_header * next = (void *)((uintptr_t)bheader + sizeof(klmalloc_big_bin_header) + bheader->size);
            ASSERT((uintptr_t)next % PAGE_SIZE == 0);
            if (next == bheader->next && next->head) {
                /*
                 * If that something is an available big bin, we can
                 * coalesce it into us to form one larger bin.
                 */
                size_t old_size = bheader->size;
                klmalloc_skip_list_delete(next);
                bheader->size = (size_t)bheader->size + (size_t)sizeof(klmalloc_big_bin_header) + next->size;
                ASSERT((bheader->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);

                if (next == klmalloc_newest_big) {
                    /*
                     * If the guy in front of us was the newest,
                     * we are now the newest (as we are him).
                     */
                    klmalloc_newest_big = bheader;
                } else {
                    if (next->next) {
                        next->next->prev = bheader;
                    }
                }
                kprintf("Coalesced (forwards) %p [%zx] <- %p [%zx] = %zx\n", (void*)bheader, old_size, (void*)next, next->size, bheader->size);
            }
        }
#endif
        /*
         * Coalesce backwards
         */
#if 0
        if (bheader->prev && bheader->prev->head) {
            /*
             * If there is something behind us, it is available, and there is nothing between
             * it and us, we can coalesce ourselves into it to form a big block.
             */
            if ((uintptr_t)bheader->prev + (bheader->prev->size + sizeof(klmalloc_big_bin_header)) == (uintptr_t)bheader) {
                size_t old_size = bheader->prev->size;
                klmalloc_skip_list_delete(bheader->prev);
                bheader->prev->size = (size_t)bheader->prev->size + (size_t)bheader->size + sizeof(klmalloc_big_bin_header);
                ASSERT((bheader->prev->size + sizeof(klmalloc_big_bin_header)) % PAGE_SIZE == 0);
                klmalloc_skip_list_insert(bheader->prev);
                if (klmalloc_newest_big == bheader) {
                    klmalloc_newest_big = bheader->prev;
                } else {
                    if (bheader->next) {
                        bheader->next->prev = bheader->prev;
                    }
                }
                kprintf("Coalesced (backwards) %p [%zx] <- %p [%zx] = %zx\n", (void*)bheader->prev, old_size, (void*)bheader, bheader->size, bheader->size);
                /*
                 * If we coalesced backwards, we are done.
                 */
                return;
            }
        }
#endif
        /*
         * Push new space back into the stack.
         */
        klmalloc_stack_push((klmalloc_bin_header *)bheader, (void *)((uintptr_t)bheader + sizeof(klmalloc_big_bin_header)));
        ASSERT(bheader->head != NULL);
        /*
         * Insert the block into list of available slabs.
         */
        klmalloc_skip_list_insert(bheader);
    } else {
        /*
         * If the stack is empty, we are freeing
         * a block from a previously full bin.
         * Return it to the busy bins list.
         */
        if (klmalloc_stack_empty(header)) {
            klmalloc_list_insert(&klmalloc_bin_head[bucket_id], header);
        }
        /*
         * Push new space back into the stack.
         */
        klmalloc_stack_push(header, ptr);
    }
}

/*
 * C standard implementation: When NULL is passed to realloc,
 * simply malloc the requested size and return a pointer to that.
 */
static void * __attribute__ ((malloc)) klrealloc(void *ptr, size_t size) {
    if (__builtin_expect(ptr == NULL, 0))
        return malloc(size);

    /*
     * C standard implementation: For a size of zero, free the
     * pointer and return NULL, allocating no new memory.
     */
    if (__builtin_expect(size == 0, 0))
    {
        free(ptr);
        return NULL;
    }

    /*
     * Find the bin for the given pointer
     * by aligning it to a page.
     */
    klmalloc_bin_header * header_old = (void *)((uintptr_t)ptr & (size_t)~PAGE_MASK);

    /*
     * (This will only happen for a big bin, mathematically speaking)
     * If we still have room in our bin for the additonal space,
     * we don't need to do anything.
     */
    if (header_old->size >= size) {
        /*
         * TODO: Break apart blocks here, which is far more important
         * than breaking them up on allocations.
         */
        return ptr;
    }

    /*
     * Reallocate more memory.
     */
    void * newptr = klmalloc(size);
    if (__builtin_expect(newptr != NULL, 1)) {
        size_t old_size = header_old->size;
        if (old_size < BIG_BIN) {
            /*
             * If we are copying from a small bin,
             * we need to get the size of the bin
             * from its id.
             */
            old_size = (1UL << (SMALLEST_BIN_LOG + old_size));
        }

        /*
         * Copy the old value into the new value.
         * Be sure to only copy as much as was in
         * the old block.
         */
        memcpy(newptr, ptr, old_size);
        klfree(ptr);
        return newptr;
    }

    /*
     * We failed to allocate more memory,
     * which means we're probably out.
     *
     * Bail and return NULL.
     */
    return NULL;
}

/*
 * Allocate memory and zero it before returning
 * a pointer to the newly allocated memory.
 * 
 * Implemented by way of a simple malloc followed
 * by a memset to 0x00 across the length of the
 * requested memory chunk.
 */
static void * __attribute__ ((malloc)) klcalloc(size_t nmemb, size_t size) {
    void *ptr = klmalloc(nmemb * size);
    if (__builtin_expect(ptr != NULL, 1))
        memset(ptr, 0x00, nmemb * size);
    return ptr;
}
