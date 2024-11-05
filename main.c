#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define UNIMPLEMENTED \
    do { \
	fprintf(stderr, "%s:%d: TODO: %s is not implemented\n", __FILE__, __LINE__, __func__); \
	abort(); \
    } while(0)

#define HEAP_CAP 640000
#define CHUNK_LIST_CAP 1024

typedef struct {
    void *start;
    size_t size;
} Chunk;

typedef struct {
    Chunk chunks[CHUNK_LIST_CAP];
    size_t count;
} Chunk_List;

uint8_t heap[HEAP_CAP] = {0};
static size_t heap_size = 0;
Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {0};

void chunk_list_dump(const Chunk_List *list)
{
    printf("Chunks (%zu):\n", list->count);
    for (size_t i = 0; i < list->count; ++i) {
	printf("  start: 0x%p, size: %zu\n",
	       list->chunks[i].start,
	       list->chunks[i].size);
    }
}


int chunk_list_find(const Chunk_List *list, void *ptr)
{
    UNIMPLEMENTED;
    return -1;
}

void chunk_list_insert(Chunk_List *list, void *ptr, size_t size)
{
    assert(list->count < CHUNK_LIST_CAP);
    
    Chunk chunk = {.start = ptr, .size = size};
    
    for (size_t i = 0; i < list->count; ++i) {
	if (ptr < list->chunks[i].start) {
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

void chunk_list_remove(Chunk_List *list, size_t index)
{
    UNIMPLEMENTED;
}

void *heap_alloc(size_t size)
{
    if (size <= 0) {
	return NULL;
    }
    assert(heap_size + size <= HEAP_CAP);
    
    void *ptr = (heap + heap_size);
    heap_size += size;
    chunk_list_insert(&alloced_chunks, ptr, size);

    return ptr;
}

void heap_free(void *ptr)
{    
    UNIMPLEMENTED;
}

int main(void)
{
    /* for (size_t i = 0; i < 100; ++i) { */
    /* 	heap_alloc(i); */
    /* } */

    /* heap_alloc(4); */
    /* heap_alloc(8); */
    /* heap_alloc(12); */

    Chunk c1 = {.start = (void*)18, .size = 2};
    Chunk c2 = {.start = (void*)1, .size = 1};
    Chunk c3 = {.start = (void*)399, .size = 5};
    Chunk c4 = {.start = (void*)50, .size = 4};
    Chunk c5 = {.start = (void*)49, .size = 3};

    chunk_list_insert(&alloced_chunks, c1.start, c1.size);
    chunk_list_insert(&alloced_chunks, c2.start, c2.size);
    chunk_list_insert(&alloced_chunks, c3.start, c3.size);
    chunk_list_insert(&alloced_chunks, c4.start, c4.size);
    chunk_list_insert(&alloced_chunks, c5.start, c5.size);
    
    chunk_list_dump(&alloced_chunks);

    return 0;
}
