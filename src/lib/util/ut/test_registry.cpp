#include "util/registry.h"

#include <gtest/gtest.h>

using namespace NotAGame;
using namespace NotAGame::Utils;

TEST(Registry, Empty) {
  Registry<int, 1> R;
  EXPECT_EQ(R.size(), 0);
  EXPECT_TRUE(R.empty());
  EXPECT_DEATH(R.GetObjectByKey("a"), "");
  EXPECT_DEATH(R.GetObjectById(0), "");
  EXPECT_EQ(R.TryGetObjectByKey("a"), nullptr);
  EXPECT_EQ(R.TryGetObjectById(0), nullptr);
}

TEST(Registry, AddObject) {
  Registry<std::string, 1> R;
  EXPECT_EQ(R.AddObject("a", "aaa"), 0);
  EXPECT_EQ(R.size(), 1);
  EXPECT_FALSE(R.empty());
  EXPECT_EQ(R.GetObjectByKey("a"), "aaa");
  EXPECT_EQ(R.GetObjectById(0), "aaa");
  EXPECT_DEATH(R.GetObjectByKey("b"), "");

  EXPECT_EQ(R.AddObject("b", "bbb"), 1);
  EXPECT_EQ(R.size(), 2);
  EXPECT_FALSE(R.empty());
  EXPECT_EQ(R.GetObjectByKey("b"), "bbb");
  EXPECT_EQ(R.GetObjectById(0), "aaa");
  EXPECT_EQ(R.GetObjectById(1), "bbb");
  EXPECT_DEATH(R.GetObjectByKey("c"), "");
}
