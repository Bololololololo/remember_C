#include "bolloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define META_SIZE sizeof(struct block_meta)
block_meta* global_base = NULL;

struct block_meta* find_free_block(struct block_meta** last, size_t size) {
  struct block_meta* current = global_base;
  while (current && !(current->free && current->size >= size)) {
    *last = current;
    current = current->next;
  }
  return current;
}

struct block_meta* request_space(struct block_meta* last, size_t size) {
  struct block_meta* block;
  block = (block_meta*)sbrk(0);
  void* request = sbrk(size + META_SIZE);
  assert((void*)block == request);  // Not thread safe.
  if (request == (void*)-1) {
    return NULL;  // sbrk failed.
  }

  if (last) {  // NULL on first request.
    last->next = block;
  }
  block->size = size;
  block->next = NULL;
  block->free = 0;
  block->magic = 0x12345678;
  return block;
}

void* bolloc(size_t size) {
  struct block_meta* block;
  // TODO: align size?

  if (size == 0) {
    return NULL;
  }

  if (global_base != NULL) {  // First call.
    block = request_space(NULL, size);
    if (!block) {
      return NULL;
    }
    global_base = block;
  } else {
    struct block_meta* last = global_base;
    block = find_free_block(&last, size);
    if (!block) {  // Failed to find free block.
      block = request_space(last, size);
      if (!block) {
        return NULL;
      }
    } else {  // Found (free) block
      // TODO: consider splitting block here.
      block->free = 0;
      block->magic = 0x77777777;
    }
  }

  return (block + 1);
}

static struct block_meta* get_block_ptr(void* ptr) {
  return (struct block_meta*)ptr - 1;
}

void bfree(void* ptr) {
  if (!ptr) {
    return;
  }

  // TODO: consider merging blocks once splitting blocks is implemented.
  struct block_meta* block_ptr = get_block_ptr(ptr);
  assert(block_ptr->free == 0);
  assert(block_ptr->magic == 0x77777777 || block_ptr->magic == 0x12345678);
  block_ptr->free = 1;
  block_ptr->magic = 0x55555555;
}

void* rebolloc(void* ptr, size_t size) {
  if (ptr == NULL) {
    // NULL ptr. realloc should act like malloc.
    return bolloc(size);
  }

  if (size <= 0) {
    return NULL;
  }

  if (get_block_ptr(ptr)->size >= size) {
    return ptr;  // We have enough space. Just return the same ptr.
  }

  void* new_ptr = bolloc(size);
  if (!new_ptr) {
    return NULL;  // bolloc failed.
  }

  memcpy(new_ptr, ptr, get_block_ptr(ptr)->size);
  bfree(ptr);
  return new_ptr;
}

void* colloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize;  // TODO: check for overflow.
  void* ptr = bolloc(size);
  memset(ptr, 0, size);
  return ptr;
}