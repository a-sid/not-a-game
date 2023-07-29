#include "entities/resource.h"

#include <gtest/gtest.h>

using namespace NotAGame;
using namespace NotAGame::Utils;

class TestResource : public ::testing::Test {
protected:
  virtual void SetUp() {
    Resource Gold{"gold", "Gold", "gold"};
    Resource Mana{"mana", "Mana", "mana"};
    Registry_.AddObject("gold", std::move(Gold));
    Registry_.AddObject("mana", std::move(Mana));
  }

  ResourceRegistry Registry_;
};

TEST_F(TestResource, Default) {
  Resources R{Registry_};
  for (auto Id : {0, 1}) {
    EXPECT_EQ(R.GetAmountById(Id), 0);
  }
}

TEST_F(TestResource, Values) {
  const auto Amounts = {1, 2};
  Resources R{Registry_, Amounts};
  for (auto Id : {0, 1}) {
    EXPECT_EQ(R.GetAmountById(Id), *(Amounts.begin() + Id));
  }
}

TEST_F(TestResource, Arithmetics) {
  Resources R1{Registry_, {1, 2}};
  Resources R2{Registry_, {3, 5}};
  EXPECT_EQ(R1 - R2, (Resources{Registry_, {-2, -3}}));
  EXPECT_EQ(R1 + R2, (Resources{Registry_, {4, 7}}));
  EXPECT_EQ(R1 * 3, (Resources{Registry_, {3, 6}}));
  EXPECT_EQ(-R1, (Resources{Registry_, {-1, -2}}));
}

TEST_F(TestResource, SelfArithmetics) {
  Resources R1{Registry_, {1, 2}};
  Resources R2{Registry_, {3, 5}};
  R1 -= R2;
  EXPECT_EQ(R1, (Resources{Registry_, {-2, -3}}));
  R1 += R2;
  EXPECT_EQ(R1, (Resources{Registry_, {1, 2}}));
  R1 *= 3;
  EXPECT_EQ(R1, (Resources{Registry_, {3, 6}}));
}

TEST_F(TestResource, Comparisons_1) {
  Resources R1{Registry_, {1, 2}};
  Resources R2{Registry_, {3, 5}};
  Resources R3{Registry_, {3, 1}};
  EXPECT_TRUE(R1 == R1);
  EXPECT_TRUE(R1 != R2);
  EXPECT_TRUE(R1 <= R1);
  EXPECT_FALSE(R1 < R1);
  EXPECT_TRUE(R1 >= R1);
  EXPECT_TRUE(R1 < R2);
  EXPECT_TRUE(R1 <= R2);
  EXPECT_TRUE(R2 >= R1);
  EXPECT_TRUE(R2 > R1);
  EXPECT_FALSE(R1 < R3);
  EXPECT_FALSE(R1 <= R3);
  EXPECT_FALSE(R1 == R3);
  EXPECT_TRUE(R1 != R3);
  EXPECT_FALSE(R3 < R1);
  EXPECT_FALSE(R3 <= R1);
}

TEST_F(TestResource, GetAmount) {
  Resources R{Registry_, {1, 2}};
  EXPECT_EQ(R.GetAmountByName("gold"), 1);
  EXPECT_EQ(R.GetAmountByName("mana"), 2);
  EXPECT_EQ(R.GetAmountById(0), 1);
  EXPECT_EQ(R.GetAmountById(1), 2);
}

TEST_F(TestResource, SetAmount) {
  Resources R{Registry_, {1, 2}};
  R.SetAmountByName("gold", 3);
  EXPECT_EQ(R.GetAmountByName("gold"), 3);
  EXPECT_EQ(R, (Resources{Registry_, {3, 2}}));

  R.SetAmountByName("mana", 4);
  EXPECT_EQ(R.GetAmountByName("mana"), 4);
  EXPECT_EQ(R, (Resources{Registry_, {3, 4}}));
}
