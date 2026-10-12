#include <genitor.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

bool		minit = false;
malloc_state	*arena = NULL;
extern char **environ;
// tunables
uint64_t	mtunes[] = {MALLOC_HINC, true, true, false};


static void unsortedbin_merge(void);

static void init_tunables(){
	uint32_t	i = 0;
	// char		*mntunes[] = {"MALLOC_HINC", "MALLOC_SAFE_LINK", "MALLOC_USE_MMAP", 
	// 	"MALLOC_BZERO"};
	uint64_t	mtunes_max[] = {0x40000, 1, 1, 1};
	uint64_t	mtunes_min[] = {0x1000, 1, 1, 1};
	while (i < MLTUNES){
		char *env = NULL; // null for now
		if (env != NULL){
			uint64_t val = atoi(env);
			if (val < mtunes_max[i] && val > mtunes_min[i]){
				mtunes[i] = val;
			}
		}
		i++;
	}
}

static void malloc_init(void){
	init_tunables();
	arena = mmap(NULL, mtunes[TUN_MALLOC_HINC], PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (arena == MAP_FAILED){
		arena = NULL;
		return;
	}
	arena->heap_size = mtunes[TUN_MALLOC_HINC];
	arena->wild = (void *) ((size_t) arena + sizeof(malloc_state));
	arena->wild->size = (mtunes[TUN_MALLOC_HINC] - sizeof(malloc_state) - CHNK_HDR_SZ) | PREV_MASK;
	for (int i = 0; i < NFAST; i++)
		arena->fastbin[i] = NULL;
	for (int i = 0; i < BINS; i++)
		arena->bins[i] = NULL;
	arena->next = NULL;

	minit = true;
}

static void malloc_inc(void){
	size_t new_size = arena->heap_size + mtunes[TUN_MALLOC_HINC];
	void *new_addr = mremap(arena, arena->heap_size, new_size, 0);
	if (new_addr == MAP_FAILED)
		return ;

	// mremap succeeded; base address unchanged (no MREMAP_MAYMOVE)
	arena->heap_size = new_size;
	arena->wild->size = (sizenomask(arena->wild->size) + mtunes[TUN_MALLOC_HINC])
	                    | (arena->wild->size & (PREV_MASK | MAPPED_MASK));
}

// avoiding fragmentation 
void malloc_consolidate(){
    int		i = -1;
    chunk 	*current = NULL;		// iterator
    chunk 	*next = NULL;
    

    while (++i < NFAST){
	if (arena->fastbin[i] == NULL)
	    continue;
	current = arena->fastbin[i];
	arena->fastbin[i] = NULL;		// we don't need that bin anymore
	while (current != NULL){
	    next = REVEAL_PTR(&arena->fastbin[i], current->fd);
	    // inserting to unsorted bins
	    current->fd = arena->bins[UNSORTED_IDX];
	    current->bk = NULL;
	    if (arena->bins[UNSORTED_IDX])
		arena->bins[UNSORTED_IDX]->bk = current;
	    arena->bins[UNSORTED_IDX] = current;
	    // next
	    current = next;
	}
    }
    unsortedbin_merge();
}

static bool unlink_chunk(chunk *victim, chunk **bin_head, bool protchunk){
	(void) protchunk;
	// TODO : implement ptr protections here

	if (victim == *bin_head){
		*bin_head = victim->fd;
		if (victim->fd)
			victim->fd->bk = NULL;
		return (true);
	}

	chunk *bin = *bin_head;
	while (bin != NULL){
		if (victim == bin->fd){
			bin->fd = victim->fd;
			if (victim->fd)
				victim->fd->bk = bin;
			return (true);
		}
		bin = bin->fd;
	}

	return (false);
}

// unlink_all - unlinks from all bins (large, small, unsorted)
static void unlink_all(chunk *victim){
	bool unlink_stat = unlink_chunk(victim, &arena->bins[UNSORTED_IDX], false);
	if (unlink_stat == false){		// we didn't unlink from unsorted
		if (smallbin_range(victim->size))
			unlink_chunk(victim, &arena->bins[smallidx(victim->size)], false);
		else 
			unlink_chunk(victim, &arena->bins[largeidx(victim->size)], false);
	}
}

static void unsortedbin_merge(void){
	chunk *it = arena->bins[UNSORTED_IDX];
	while (it != NULL){
		// merging the backward chunk, forward chunks will take care of themselves
		if (!is_prev(it) && it->prev_size != 0){
			chunk 		*prev_vict = prevchunk(it);
			uint64_t	nsize;		// new merged chunk size
			unlink_all(prev_vict);		// unlinking the chunk from it's original bin
							// to eliminate fragmentation
			nsize = (sizenomask(prev_vict->size) + sizenomask(it->size) + CHNK_HDR_SZ);
			prev_vict->size = nsize | (prev_vict->size & ALL_MASKS);
			if (it->bk != NULL)
				it->bk->fd = prev_vict;	// linking chunk in unsorted list
			else 
				arena->bins[UNSORTED_IDX] = prev_vict;

			if (it->fd != NULL)
				it->fd->bk = prev_vict;

			prev_vict->fd = it->fd;
			prev_vict->bk = it->bk;
			// updating the next chunk prevsize
			if (nextchunk(prev_vict) < arena->wild){
				nextchunk(prev_vict)->prev_size = prev(sizenomask(prev_vict->size));
			}
			it = prev_vict;
		}
		it = it->fd;
	}
}

void *malloc(size_t size){
    chunk	*selected = NULL;
    chunk	*current = NULL;
    size_t	aligned_size;

    if (minit == false){
	malloc_init();
	if (arena == NULL)
		return (NULL);
    }

    size = sizeallign(size);
    if (size < MIN_CHUNK_SIZE)
	size = MIN_CHUNK_SIZE;

    aligned_size = size;

    // checking fast bins
    if (fastbin_range(size) && (arena->fastbin[fastidx(size)] != NULL)) {
	selected = arena->fastbin[fastidx(size)];
	arena->fastbin[fastidx(size)] = REVEAL_PTR(&arena->fastbin[fastidx(size)], selected->fd);
	nextchunk(selected)->size |= PREV_MASK;
	nextchunk(selected)->prev_size = 0;
	if (mtunes[TUN_BZERO] == true){
		bzero(chunk2mem(selected), size);
	}
	return chunk2mem(selected);
    } else if (smallbin_range(size) && (arena->bins[smallidx(size)] != NULL)){
	// extracting from small bin
	selected = arena->bins[smallidx(size)];
	arena->bins[smallidx(size)] = selected->fd;
	if (selected->fd)
		selected->fd->bk = NULL;
	nextchunk(selected)->size |= PREV_MASK;
	nextchunk(selected)->prev_size = 0;
	if (mtunes[TUN_BZERO] == true){
		bzero(chunk2mem(selected), size);
	}
	return chunk2mem(selected);
    } else if (largebin_range(size)){
	// consolidating fastbins to eliminate fragmentation 
	malloc_consolidate();

	// TODO : ADD ranges in large bin checking
	// we check if we have a chunk that can fill the request from
	// largebins
	selected = largebin_head(size);
	if (selected) {
		nextchunk(selected)->size |= PREV_MASK;
		nextchunk(selected)->prev_size = 0;
		if (mtunes[TUN_BZERO] == true){
			bzero(chunk2mem(selected), size);
		}
		return chunk2mem(selected);
	}
    }

    // since we iterated over all bins and didn't find a chunk that
    // can fill the request, malloc will sort unsorted bin and then
    // recheck all bins for the right one
    
    // we make sure that we have something in unsorted bin
    if (arena->bins[UNSORTED_IDX] != NULL) {
	current = arena->bins[UNSORTED_IDX];
	while (current != NULL) {
	    // removing the chunk from the list	
	    // there is no need to modify the fd or bk since we're 
	    // removing everything
	    chunk *next_unsorted;

	    // avoiding deadlocks
	    if (arena->bins[UNSORTED_IDX] != arena->bins[UNSORTED_IDX]->fd)
		    arena->bins[UNSORTED_IDX] = arena->bins[UNSORTED_IDX]->fd;
	    else
		    arena->bins[UNSORTED_IDX] = NULL;

	    if (arena->bins[UNSORTED_IDX])
		    arena->bins[UNSORTED_IDX]->bk = NULL;
	    
	    next_unsorted = arena->bins[UNSORTED_IDX];

	    // add to small bin
	    if (smallbin_range(sizenomask(current->size))) {
	    	chunk *head = smallbin_head(current->size);

		current->fd = head;
		current->bk = NULL;

		if (head != NULL)
			head->bk = current;

		smallbin_head(current->size) = current;
	    } else {
		current->fd = largebin_head(current->size);
		current->bk = NULL;
		if (largebin_head(current->size))
			largebin_head(current->size)->bk = current;
		largebin_head(current->size) = current;
	    }

	    // NEXT!!!
	    current = next_unsorted;
	}
	// now we search in bins again
	return (malloc(aligned_size));
    }

    if (chunksize(size) < sizenomask(arena->wild->size)){
	selected = arena->wild;
	arena->wild = (chunk *) ((size_t) arena->wild + chunksize(size));
	arena->wild->size = ((sizenomask(selected->size) - chunksize(size)) | PREV_MASK);
	selected->size = size | (selected->size & ALL_MASKS);
	if (mtunes[TUN_BZERO] == true){
		bzero(chunk2mem(selected), size);
	}
	return (chunk2mem(selected));
    }
    if (mtunes[TUN_USE_MMAP] == true){
	// mmaping here
	// TODO : add mmap threshold and make inc to be dynamic
	selected = MAP(size);
	if (selected == MAP_FAILED)
		return (NULL);
	selected->size = mapped(chunksize(size));
	return (chunk2mem(selected));
    }
    else {
	malloc_inc();
	return (NULL);
    }
}


static chunk *free_merge_chunk(chunk *selected){
	chunk *merged = NULL;
 
	// backward merge: check if the previous chunk is free (PREV_MASK clear) hh
	if (!is_prev(selected) && selected->prev_size != 0){
		chunk    *prev_vict = prevchunk(selected);
		size_t    nsize;
		unlink_all(prev_vict);
		nsize = sizenomask(prev_vict->size) + sizenomask(selected->size) + CHNK_HDR_SZ;
		prev_vict->size = nsize | (prev_vict->size & ALL_MASKS);
		// re-link the merged chunk at the head of the unsorted bin
		// (the original selected was already inserted before this call)
		// update the unsorted bin head if selected was there
		if (arena->bins[UNSORTED_IDX] == selected)
			arena->bins[UNSORTED_IDX] = prev_vict;
		else if (selected->bk)
			selected->bk->fd = prev_vict;
		prev_vict->fd = selected->fd;
		prev_vict->bk = selected->bk;
		if (selected->fd)
			selected->fd->bk = prev_vict;
		if (nextchunk(prev_vict) < arena->wild)
			nextchunk(prev_vict)->prev_size = sizenomask(prev_vict->size);
		selected = prev_vict;
		merged = selected;
	}
 
	// forward merge: if the next chunk is free (its PREV_MASK is clear in
	// the chunk after it, meaning next is free)
	chunk *nxt = nextchunk(selected);
	if (nxt < arena->wild && !is_prev(nxt)){
		size_t nsize;
		unlink_all(nxt);
		nsize = sizenomask(selected->size) + sizenomask(nxt->size) + CHNK_HDR_SZ;
		selected->size = nsize | (selected->size & ALL_MASKS);
		if (nextchunk(selected) < arena->wild)
			nextchunk(selected)->prev_size = sizenomask(selected->size);
		merged = selected;
	}
 
	return (merged);
}


void free(void *p){
    chunk *vict = NULL;
    chunk *selected ;

    if (p == NULL)
	return;

    selected = mem2chunk(p);
    if (is_mmaped(selected)) {
	munmap(selected, sizenomask(selected->size));
	return;
    }


    // if the next chunk is the wild we consolidate
    if (nextchunk(selected) == arena->wild) {
	size_t new_wild_size = sizenomask(arena->wild->size) + sizenomask(selected->size) + CHNK_HDR_SZ;
	arena->wild = selected;
	arena->wild->size = new_wild_size | (selected->size & ALL_MASKS);
	return;
    }
    if (fastbin_range(sizenomask(selected->size))){
	int idx = fastidx(sizenomask(selected->size));
	selected->fd = PROTECT_PTR(&arena->fastbin[idx], arena->fastbin[idx]);
	arena->fastbin[idx] = selected;
	nextchunk(selected)->size &= ~PREV_MASK;		// removing the mask from the next chunk
    } else if (!is_mmaped(selected)) {
	/* 
	 * if the victim chunk is gonna be inserted to either [large/small]bins
	 * we must merge it with it's free neighbors
	 */
	nextchunk(selected)->size &= ~PREV_MASK;
	nextchunk(selected)->prev_size = sizenomask(selected->size);
	// we have no time to check the size of that chunk so we throw it in unsorted bins
	// at the moment unsorted bins are not circular
	// also there is no safe linking here
	selected->fd = arena->bins[UNSORTED_IDX];
	if (arena->bins[UNSORTED_IDX])
	    arena->bins[UNSORTED_IDX]->bk = selected;
	arena->bins[UNSORTED_IDX] = selected;

	 
	vict = free_merge_chunk(selected);
	if (vict != NULL)
		selected = vict;
    }
}
