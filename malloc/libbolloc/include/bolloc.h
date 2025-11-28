#include <stddef.h>

typedef struct block_meta {
  size_t size;
  struct block_meta* next;
  int free;
  int magic;  // For debugging only. TODO: remove this in non-debug mode.
} block_meta;

struct block_meta* request_space(struct block_meta* last, size_t size);
struct block_meta* find_free_block(struct block_meta** last, size_t size);

void* bolloc(size_t size);
void bfree(void* ptr);
void* rebolloc(void* ptr, size_t size);
void* colloc(size_t nelem, size_t elsize);