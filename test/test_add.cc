#include "gtest/gtest.h"

int add(int a, int b) {
    return a + b;
}
 
TEST(add, zero) {
    EXPECT_EQ(0, add(0, 0));
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}