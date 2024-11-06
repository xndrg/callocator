// Example works only in POSIX
// Using mmap POSIX system call like the real chads
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#define UNIMPLEMENTED \
    do { \
	fprintf(stderr, "%s:%d: TODO: %s is not implemented\n", __FILE__, __LINE__, __func__); \
	abort(); \
    } while(0)

#define CHUNK_LIST_CAP 1024

typedef struct {
    uint8_t *ptr;
    size_t size;
} Chunk;

typedef struct {
    Chunk chunks[CHUNK_LIST_CAP];
    size_t count;
} Chunk_List;

uint8_t *heap;
Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {0};
Chunk_List tmp_chunks = {0};

void chunk_list_insert(Chunk_List *list, uint8_t *ptr, size_t size)
{
    assert(list->count < CHUNK_LIST_CAP);
    
    const Chunk chunk = {.ptr = ptr, .size = size};
    
    for (size_t i = 0; i < list->count; ++i) {
	if (ptr < list->chunks[i].ptr) {
	    memmove(&list->chunks[i+1], &list->chunks[i], (list->count - i)*sizeof(Chunk));
	    list->chunks[i] = chunk;
	    goto end;
	}
    }

    // If chunk's to be inserted pointer isn't less than any in the list,
    // It means that this pointer is the greatest and
    // We should insert it to the end of chunk list
    list->chunks[list->count] = chunk;
    
end:    
    list->count += 1;
}

void chunk_list_merge(Chunk_List *dst, const Chunk_List *src)
{
    dst->count = 0;

    for (size_t i = 0; i < src->count; ++i) {
	const Chunk chunk = src->chunks[i];
	
	if (dst->count > 0) {
	    Chunk *top_chunk = &dst->chunks[dst->count - 1];

	    if (top_chunk->ptr + top_chunk->size == chunk.ptr) {
		top_chunk->size += chunk.size;
	    } else {
		chunk_list_insert(dst, chunk.ptr, chunk.size);
	    }
	} else {
	    chunk_list_insert(dst, chunk.ptr, chunk.size);
	}
    }
}

void chunk_list_dump(const Chunk_List *list)
{
    printf("Chunks (%zu):\n", list->count);
    for (size_t i = 0; i < list->count; ++i) {
	printf("  ptr: 0x%p, size: %zu\n",
	       list->chunks[i].ptr,
	       list->chunks[i].size);
    }
}

int chunk_ptr_compar(const void *a, const void *b)
{
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    
    return a_chunk->ptr - b_chunk->ptr;
}

// O(n) now
// TODO: O(log(n)) binary search
int chunk_list_find(const Chunk_List *list, void *ptr)
{
    for (size_t i = 0; i < list->count; ++i) {
	if (list->chunks[i].ptr == ptr) {
	    return (int) i;
	}
    }

    return -1;
}

void chunk_list_remove(Chunk_List *list, size_t index)
{
    assert(index < list->count);
    memmove(&list->chunks[index], &list->chunks[index + 1], (list->count - index)*sizeof(Chunk));

    list->count -= 1;
}

void *heap_alloc(size_t size)
{
    if (size <= 0) return NULL;

    chunk_list_merge(&tmp_chunks, &freed_chunks);
    freed_chunks = tmp_chunks;
    
    for (size_t i = 0; i < freed_chunks.count; ++i) {
	const Chunk chunk = freed_chunks.chunks[i];
	if (chunk.size >= size) {
	    chunk_list_remove(&freed_chunks, i);

	    void *const ptr = chunk.ptr;
	    const size_t tail_size = chunk.size - size;
	    chunk_list_insert(&alloced_chunks, ptr, size);

	    if (tail_size > 0) {
		chunk_list_insert(&freed_chunks, chunk.ptr + size, tail_size);
	    }

	    return ptr;
	}
    }

    return NULL;
}

void heap_free(void *ptr)
{
    if (ptr == NULL) return;
    
    const int index = chunk_list_find(&alloced_chunks, ptr);
    assert(index >= 0);

    chunk_list_insert(&freed_chunks,
		      alloced_chunks.chunks[index].ptr,
		      alloced_chunks.chunks[index].size);
    chunk_list_remove(&alloced_chunks, (size_t) index);
}

int main(void)
{
	
    heap = mmap(NULL, 640000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    assert(heap != NULL);
    printf("Heap address: %p\n", heap);

    freed_chunks = (Chunk_List) {
	.chunks = {
            [0] = {.ptr = heap, .size = sizeof(heap)}
        },
        .count = 1,
    };

    void *p1 = heap_alloc(1);
    void *p2 = heap_alloc(2);

    chunk_list_dump(&alloced_chunks);
    chunk_list_dump(&freed_chunks);
    
    heap_free(p1);
    heap_free(p2);
    heap_alloc(10);

    chunk_list_dump(&alloced_chunks);
    chunk_list_dump(&freed_chunks);
    
    return 0;
}
