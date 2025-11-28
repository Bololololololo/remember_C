#include <gtest/gtest.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

// Include your header
extern "C" {
#include "bolloc.h"
}

// Test fixture for memory allocator tests
class BollocTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Reset global state before each test
    // Note: In a real scenario, you'd need a way to reset global_base
    // For testing purposes, you might need to add a reset function
  }

  void TearDown() override {
    // Clean up any allocated memory
  }
};

// // ============================================================================
// Basic Allocation Tests
// ============================================================================

TEST_F(BollocTest, BasicAllocation) {
  void* ptr = bolloc(100);
  ASSERT_NE(ptr, nullptr);

  // Verify we can write to the allocated memory
  memset(ptr, 'A', 100);
  EXPECT_EQ(((char*)ptr)[0], 'A');
  EXPECT_EQ(((char*)ptr)[99], 'A');

  bfree(ptr);
  EXPECT_EQ(ptr, nullptr);
}

TEST_F(BollocTest, ZeroSizeAllocation) {
  void* ptr = bolloc(0);
  EXPECT_EQ(ptr, nullptr);
}

TEST_F(BollocTest, NegativeSizeAllocation) {
  void* ptr = bolloc(-1);
  EXPECT_EQ(ptr, nullptr);
}

TEST_F(BollocTest, MultipleAllocations) {
  void* ptr1 = bolloc(50);
  void* ptr2 = bolloc(100);
  void* ptr3 = bolloc(200);

  ASSERT_NE(ptr1, nullptr);
  ASSERT_NE(ptr2, nullptr);
  ASSERT_NE(ptr3, nullptr);

  // Verify all pointers are different
  EXPECT_NE(ptr1, ptr2);
  EXPECT_NE(ptr2, ptr3);
  EXPECT_NE(ptr1, ptr3);

  // Verify we can write to each allocation independently
  memset(ptr1, 'A', 50);
  memset(ptr2, 'B', 100);
  memset(ptr3, 'C', 200);

  EXPECT_EQ(((char*)ptr1)[0], 'A');
  EXPECT_EQ(((char*)ptr2)[0], 'B');
  EXPECT_EQ(((char*)ptr3)[0], 'C');

  bfree(ptr1);
  bfree(ptr2);
  bfree(ptr3);
}

TEST_F(BollocTest, LargeAllocation) {
  void* ptr = bolloc(1024 * 1024);  // 1 MB
  ASSERT_NE(ptr, nullptr);

  // Verify we can write to the allocated memory
  memset(ptr, 0xFF, 1024 * 1024);
  EXPECT_EQ(((unsigned char*)ptr)[0], 0xFF);
  EXPECT_EQ(((unsigned char*)ptr)[1024 * 1024 - 1], 0xFF);

  bfree(ptr);
}

// ============================================================================
// Free Tests
// ============================================================================

TEST_F(BollocTest, FreeNullPointer) {
  // Should not crash
  bfree(nullptr);
  SUCCEED();
}

TEST_F(BollocTest, FreeAndReuseBlock) {
  void* ptr1 = bolloc(100);
  ASSERT_NE(ptr1, nullptr);

  bfree(ptr1);

  // Allocate again with same size - should reuse the freed block
  void* ptr2 = bolloc(100);
  ASSERT_NE(ptr2, nullptr);

  // Depending on implementation, ptr2 might equal ptr1 (block reuse)
  // We just verify it's a valid allocation
  memset(ptr2, 'B', 100);
  EXPECT_EQ(((char*)ptr2)[0], 'B');

  bfree(ptr2);
}

TEST_F(BollocTest, FreeSmallerBlockReuse) {
  void* ptr1 = bolloc(200);
  ASSERT_NE(ptr1, nullptr);

  bfree(ptr1);

  // Allocate smaller size - should reuse the freed block
  void* ptr2 = bolloc(100);
  ASSERT_NE(ptr2, nullptr);

  bfree(ptr2);
}

TEST_F(BollocTest, FreeMultipleBlocks) {
  void* ptr1 = bolloc(50);
  void* ptr2 = bolloc(100);
  void* ptr3 = bolloc(150);

  ASSERT_NE(ptr1, nullptr);
  ASSERT_NE(ptr2, nullptr);
  ASSERT_NE(ptr3, nullptr);

  // Free in different order
  bfree(ptr2);
  bfree(ptr1);
  bfree(ptr3);

  SUCCEED();
}

// ============================================================================
// Rebolloc Tests
// ============================================================================

TEST_F(BollocTest, RebollocNullPointer) {
  void* ptr = rebolloc(nullptr, 100);
  ASSERT_NE(ptr, nullptr);

  memset(ptr, 'A', 100);
  EXPECT_EQ(((char*)ptr)[0], 'A');

  bfree(ptr);
}

TEST_F(BollocTest, RebollocZeroSize) {
  void* ptr1 = bolloc(100);
  ASSERT_NE(ptr1, nullptr);

  void* ptr2 = rebolloc(ptr1, 0);
  EXPECT_EQ(ptr2, nullptr);
}

TEST_F(BollocTest, RebollocSameSize) {
  void* ptr1 = bolloc(100);
  ASSERT_NE(ptr1, nullptr);

  memset(ptr1, 'A', 100);

  void* ptr2 = rebolloc(ptr1, 100);
  ASSERT_NE(ptr2, nullptr);

  // Should return same pointer when size is equal
  EXPECT_EQ(ptr1, ptr2);
  EXPECT_EQ(((char*)ptr2)[0], 'A');

  bfree(ptr2);
}

TEST_F(BollocTest, RebollocSmallerSize) {
  void* ptr1 = bolloc(200);
  ASSERT_NE(ptr1, nullptr);

  memset(ptr1, 'A', 200);

  // Realloc to smaller size - should return same pointer
  void* ptr2 = rebolloc(ptr1, 100);
  ASSERT_NE(ptr2, nullptr);

  EXPECT_EQ(ptr1, ptr2);
  EXPECT_EQ(((char*)ptr2)[0], 'A');

  bfree(ptr2);
}

TEST_F(BollocTest, RebollocLargerSize) {
  void* ptr1 = bolloc(100);
  ASSERT_NE(ptr1, nullptr);

  // Write pattern to original allocation
  for (int i = 0; i < 100; i++) {
    ((char*)ptr1)[i] = 'A' + (i % 26);
  }

  // Realloc to larger size
  void* ptr2 = rebolloc(ptr1, 200);
  ASSERT_NE(ptr2, nullptr);

  // Verify original data was copied
  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(((char*)ptr2)[i], 'A' + (i % 26));
  }

  // Verify we can write to new space
  memset(((char*)ptr2) + 100, 'Z', 100);
  EXPECT_EQ(((char*)ptr2)[150], 'Z');

  bfree(ptr2);
}

TEST_F(BollocTest, RebollocMultipleTimes) {
  void* ptr = bolloc(50);
  ASSERT_NE(ptr, nullptr);

  memset(ptr, 'A', 50);

  ptr = rebolloc(ptr, 100);
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(((char*)ptr)[0], 'A');

  ptr = rebolloc(ptr, 200);
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(((char*)ptr)[0], 'A');

  ptr = rebolloc(ptr, 50);
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(((char*)ptr)[0], 'A');

  bfree(ptr);
}

// ============================================================================
// Colloc Tests
// ============================================================================

TEST_F(BollocTest, CollocBasic) {
  int* arr = (int*)colloc(10, sizeof(int));
  ASSERT_NE(arr, nullptr);

  // Verify all elements are zero-initialized
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(arr[i], 0);
  }

  // Verify we can write to the array
  for (int i = 0; i < 10; i++) {
    arr[i] = i * 2;
  }

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(arr[i], i * 2);
  }

  bfree(arr);
}

TEST_F(BollocTest, CollocZeroElements) {
  void* ptr = colloc(0, sizeof(int));
  // Behavior may vary - either NULL or valid pointer
  // Just verify it doesn't crash
  if (ptr) {
    bfree(ptr);
  }
}

TEST_F(BollocTest, CollocZeroElementSize) {
  void* ptr = colloc(10, 0);
  // Should return a pointer for 0 total bytes
  if (ptr) {
    bfree(ptr);
  }
}

TEST_F(BollocTest, CollocLargeArray) {
  char* arr = (char*)colloc(1000, sizeof(char));
  ASSERT_NE(arr, nullptr);

  // Verify all elements are zero-initialized
  for (int i = 0; i < 1000; i++) {
    EXPECT_EQ(arr[i], 0);
  }

  bfree(arr);
}

// ============================================================================
// Block Metadata Tests
// ============================================================================

TEST_F(BollocTest, BlockMetadataIntegrity) {
  void* ptr = bolloc(100);
  ASSERT_NE(ptr, nullptr);

  // Get block metadata
  struct block_meta* block = (struct block_meta*)ptr - 1;

  // Verify metadata
  EXPECT_EQ(block->size, 100);
  EXPECT_EQ(block->free, 0);
  EXPECT_EQ(block->magic, 0x12345678);

  bfree(ptr);

  // After free, verify metadata changed
  EXPECT_EQ(block->free, 1);
  EXPECT_EQ(block->magic, 0x55555555);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(BollocTest, AllocFreePattern) {
  const int iterations = 100;

  for (int i = 0; i < iterations; i++) {
    void* ptr = bolloc(100 + i);
    ASSERT_NE(ptr, nullptr);
    memset(ptr, i % 256, 100 + i);
    bfree(ptr);
  }

  SUCCEED();
}

TEST_F(BollocTest, MixedOperations) {
  void* ptrs[10];

  // Allocate multiple blocks
  for (int i = 0; i < 10; i++) {
    ptrs[i] = bolloc((i + 1) * 50);
    ASSERT_NE(ptrs[i], nullptr);
    memset(ptrs[i], 'A' + i, (i + 1) * 50);
  }

  // Free some blocks
  bfree(ptrs[3]);
  bfree(ptrs[7]);
  bfree(ptrs[1]);

  // Allocate new blocks (should reuse freed space)
  void* new1 = bolloc(50);
  void* new2 = bolloc(150);
  ASSERT_NE(new1, nullptr);
  ASSERT_NE(new2, nullptr);

  // Reallocate some blocks
  ptrs[0] = rebolloc(ptrs[0], 200);
  ASSERT_NE(ptrs[0], nullptr);

  // Free remaining blocks
  bfree(ptrs[0]);
  bfree(ptrs[2]);
  bfree(ptrs[4]);
  bfree(ptrs[5]);
  bfree(ptrs[6]);
  bfree(ptrs[8]);
  bfree(ptrs[9]);
  bfree(new1);
  bfree(new2);

  SUCCEED();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}