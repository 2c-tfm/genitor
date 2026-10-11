#ifndef MALLOC_H
#define MALLOC_H


struct chunk;

// defs 
# define SIZE_SZ sizeof(size_t)
# define DBG
# define NFAST 10
# define BINS 0x7f				// large, unsorted and small bins
# define NSMALL_BINS (int)(BINS / 2)		
# define NLARGE_BINS (int)(BINS / 2) 
# define UNSORTED_IDX (BINS - 1)			
# define CHUNK_ALLIGNEMENT (SIZE_SZ * 2)
# define MALLOC_ALIGN (CHUNK_ALLIGNEMENT - 1)
# define MIN_CHUNK_SIZE (sizeof(chunk) - CHUNK_ALLIGNEMENT)
# define MALLOC_HINC 0x10000
# define CHNK_HDR_SZ (SIZE_SZ * 2)
# define SMALLBIN_WIDTH    MALLOC_ALIGN
# define SMALLBIN_CORRECTION (MALLOC_ALIGN > CHUNK_ALLIGNEMENT)
# define MIN_LARGE_SIZE    ((NSMALL_BINS - SMALLBIN_CORRECTION) * SMALLBIN_WIDTH)

//// flags
# define PREV_MASK 0x1
# define MAPPED_MASK 0x2
# define AV_MASK 0x4
# define ALL_MASKS (PREV_MASK | MAPPED_MASK | AV_MASK)


// macros 
# define sizenomask(size)  (size & ~(PREV_MASK | MAPPED_MASK | AV_MASK))
# define chunk2mem(chunk) ((void *)((char *) chunk + (2 * SIZE_SZ)))
# define mem2chunk(p) ((chunk *)((char *) p - (2 * SIZE_SZ)))
# define sizeallign(size) ((size + MALLOC_ALIGN) & ~MALLOC_ALIGN)
# define chunksize(size) (size + (SIZE_SZ * 2))
# define is_mmaped(p) (p->size & MAPPED_MASK)
# define is_prev(p)   (p->size & PREV_MASK)
# define mapped(size)	(size | MAPPED_MASK)
# define prev(size)	((size) | PREV_MASK)
# define nextchunk(p) ((chunk *) (((char *)p + sizenomask(p->size) + CHNK_HDR_SZ)))
# define prevchunk(p) ((chunk *) (((char *)p - sizenomask((p)->prev_size) - CHNK_HDR_SZ)))

# define MAP(size) mmap(0x0, chunksize(size), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)

// safe linking
#define PROTECT_PTR(pos, ptr) (__typeof(ptr)) ( \
    mtunes[TUN_SAFE_LINK] ? \
    ((((size_t) (pos)) >> 12) ^ ((size_t) (ptr))) : \
    ((size_t) (ptr)) \
)
#define REVEAL_PTR(pos, ptr) (__typeof(ptr)) ( \
    mtunes[TUN_SAFE_LINK] ? \
    ((((size_t) (pos)) >> 12) ^ ((size_t) (ptr))) : \
    ((size_t) (ptr)) \
)

//// fastbin macros
# define fastidx(size) (sizenomask(size) / SIZE_SZ)
# define fastbin_range(size) (size < (NFAST * SIZE_SZ))
# define fastbin_head(size) arena->fastbin[fastidx(sizenomask(size))]

// TAKEN FROM GLIBC https://elixir.bootlin.com/glibc/glibc-2.40/source/malloc/malloc.c#L1560
#define smallbin_range(sz)  \
  ((unsigned long) (sizenomask(sz)) < (unsigned long) MIN_LARGE_SIZE)
#define smallidx(sz) \
    (((((unsigned) sizenomask(sz)) >> \
       (SMALLBIN_WIDTH == 16 ? 4 : 3)) + SMALLBIN_CORRECTION) \
      < NSMALL_BINS \
      ? ((((unsigned) sizenomask(sz)) >> \
          (SMALLBIN_WIDTH == 16 ? 4 : 3)) + SMALLBIN_CORRECTION) \
      : (NSMALL_BINS - 1))

# define smallbin_head(size)  arena->bins[smallidx(sizenomask(size))]


#define largeidx_32(sz)                                                \
  (((((unsigned long) (sz)) >> 6) <= 38) ?  56 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)

#define largeidx_32_big(sz)                                            \
  (((((unsigned long) (sz)) >> 6) <= 45) ?  49 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)

#define largeidx_64(sz)                                                \
  (((((unsigned long) (sz)) >> 6) <= 48) ?  48 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)

#define largeidx(sz) \
  (SIZE_SZ == 8 ? largeidx_64 (sz)                                     \
   : MALLOC_ALIGN == 16 ? largeidx_32_big (sz)                     \
   : largeidx_32 (sz))
#define largebin_range(sz)  \
  ((unsigned long) (sz) >= (unsigned long) MIN_LARGE_SIZE)
# define largebin_head(size)  arena->bins[largeidx(sizenomask(size))]


#define bin_idx(sz) \
  ((smallbin_range (sz)) ? smallidx (sz) : largeidx (sz))


// structs 
typedef struct chunk {
    size_t		prev_size;
    size_t		size;
    struct chunk	*fd;
    struct chunk	*bk;
    // no we are not using large bins 
} chunk;

// TOOD : allign this to be divisible by (SIZE_SZ * 2)
typedef struct malloc_state {
    ///// bins /////
    chunk		*fastbin[NFAST];
    chunk		*bins[BINS];
    chunk		*wild;
    size_t		heap_size;
    struct malloc_state	*next;		// next heap in the next thread
} malloc_state;

// func definitions
void *malloc(size_t size);
void free(void *p);
void *calloc(size_t n, size_t size);
void *realloc(void *p, size_t size);
void *reallocarray(void *p, size_t n, size_t size);

// tunables
typedef enum malloc_tunables {
	TUN_MALLOC_HINC,
	TUN_SAFE_LINK,
	TUN_USE_MMAP,
	TUN_BZERO
} malloc_tunables;

#define MLTUNES 4

extern uint64_t mtunes[];

// global vars 
extern bool minit;
extern malloc_state *arena;

#endif
