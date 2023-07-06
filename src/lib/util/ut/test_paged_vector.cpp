#include "util/paged_vector.h"

#include <gtest/gtest.h>

using namespace NotAGame::Utils;

TEST(PagedVector, Empty) {
  PagedVector<int, 2> V;
  EXPECT_EQ(V.size(), 0);
  EXPECT_EQ(V.capacity(), 2);
  EXPECT_TRUE(V.empty());
  EXPECT_DEATH(V[0], "");
  EXPECT_DEATH(V[1], "");
  EXPECT_DEATH(V[2], "");
}

TEST(PagedVector, PushBack) {
  PagedVector<int, 2> V;
  V.push_back(1);
  EXPECT_EQ(V.size(), 1);
  EXPECT_EQ(V.capacity(), 2);
  EXPECT_FALSE(V.empty());
  EXPECT_EQ(V[0], 1);
  EXPECT_DEATH(V[1], "");
  EXPECT_DEATH(V[2], "");

  const auto *Ptr0 = &V[0];
  V.push_back(2);
  EXPECT_EQ(V.size(), 2);
  EXPECT_EQ(V.capacity(), 2);
  EXPECT_FALSE(V.empty());
  EXPECT_EQ(V[0], 1);
  EXPECT_EQ(Ptr0, &V[0]);
  EXPECT_EQ(V[1], 2);
  EXPECT_DEATH(V[2], "");

  const auto *Ptr1 = &V[1];
  V.push_back(3);
  EXPECT_EQ(V.size(), 3);
  EXPECT_EQ(V.capacity(), 4);
  EXPECT_FALSE(V.empty());
  EXPECT_EQ(V[0], 1);
  EXPECT_EQ(Ptr0, &V[0]);
  EXPECT_EQ(V[1], 2);
  EXPECT_EQ(Ptr1, &V[1]);
  EXPECT_EQ(V[2], 3);
  EXPECT_DEATH(V[3], "");
}
