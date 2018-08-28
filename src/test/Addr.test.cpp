#include <gtirb/Addr.hpp>
#include <gtest/gtest.h>
#include <sstream>

using namespace gtirb;

TEST(Unit_Addr, ctor_0) {
  EXPECT_NO_THROW(Addr());
  EXPECT_EQ(Addr(), Addr());
}

TEST(Unit_Addr, ctor_1) {
  EXPECT_NO_THROW(Addr(2112));

  auto Ea = Addr(2112);
  EXPECT_EQ(2112, uint64_t(Ea));
}

TEST(Unit_Addr, fromIntegers) {
  uint64_t U64 = 1;
  Addr U64Addr(U64);
  EXPECT_EQ(uint64_t(U64Addr), U64);

  int64_t S64 = 2;
  Addr S64Addr(S64);
  EXPECT_EQ(uint64_t(S64Addr), S64);
}

TEST(Unit_Addr, toIntegers) {
  Addr Ea(1);
  EXPECT_EQ(static_cast<uint64_t>(Ea), static_cast<uint64_t>(1));
  EXPECT_EQ(static_cast<uint64_t>(Ea), static_cast<int64_t>(1));
}

TEST(Unit_Addr, comparison) {
  Addr Ea1(2112), Ea2(1221), Ea3(1000), Ea4(1000);

  EXPECT_EQ(Ea3, Ea4);
  EXPECT_TRUE(Ea3 == Ea4);
  EXPECT_FALSE(Ea3 != Ea4);

  EXPECT_NE(Ea1, Ea2);
  EXPECT_TRUE(Ea1 != Ea2);
  EXPECT_FALSE(Ea1 == Ea2);

  EXPECT_LT(Ea2, Ea1);
  EXPECT_TRUE(Ea2 < Ea1);
  EXPECT_FALSE(Ea2 > Ea1);
  
  EXPECT_GT(Ea1, Ea2);
  EXPECT_TRUE(Ea1 > Ea2);
  EXPECT_FALSE(Ea1 < Ea2);

  EXPECT_LE(Ea3, Ea4);
  EXPECT_TRUE(Ea3 <= Ea4);
  EXPECT_FALSE(Ea3 > Ea4);

  EXPECT_GE(Ea3, Ea4);
  EXPECT_TRUE(Ea3 >= Ea4);
  EXPECT_FALSE(Ea3 < Ea4);
}

TEST(Unit_Addr, arithmetic) {
  Addr Ea(10);

  EXPECT_EQ(Ea + 5, Addr(15));
  EXPECT_EQ(Ea - Addr(5), 5);
  EXPECT_EQ(Ea - 5, Addr(5));
}

TEST(Unit_Addr, ostream) {
  std::ostringstream Os;

  Os << 123 << " 0x" << std::hex << static_cast<uint64_t>(Addr(456)) << std::dec
     << " 789";
  EXPECT_EQ(Os.str(), "123 0x1c8 789");
}
