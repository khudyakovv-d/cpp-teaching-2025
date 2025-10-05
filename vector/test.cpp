#include <iostream>
#include "vector.h"
#include "gtest/gtest.h"

TEST(vector_test, vector_creation_on_stack) {
    Vector v = Vector();
    int size = v.size();
    ASSERT_EQ(size, 0);
}

TEST(vector_test, vector_copy_operator) {
    Vector v1 = Vector(10);
    Vector v2 = Vector();
    v2 = v1;
    ASSERT_EQ(v1.size(), v2.size());
}

// Параметризованный тест по int (размер)
class VectorSizeTest : public ::testing::TestWithParam<std::pair<int, int>> {};

// Один простой тест — проверяем, что size() == параметр
TEST_P(VectorSizeTest, SizeMatchesParam) {
    const std::pair<int, int> p = GetParam();
    Vector v(p.first);
    EXPECT_EQ(v.size(), p.second);
}

// Набор значений параметров
INSTANTIATE_TEST_SUITE_P(
    Sizes,               // префикс имени инстанса (для фильтрации)
    VectorSizeTest,      // имя фикстуры
    ::testing::Values(std::pair<int, int>(0,0), 
                      std::pair<int, int>(1,1),
                      std::pair<int, int>(2,2),
                      std::pair<int, int>(5,5)) // параметры
);

TEST(vector_test, foo_const_test) {
    Vector v = Vector();
    Vector foo = v.foo();
    foo[5] = 10;
    ASSERT_EQ(foo[5], 10);
}

// TEST(vector_test, foo_const_test_2) {
//     Vector v = Vector();
//     const Vector foo = v.foo();
//     foo[5] = 10;
//     ASSERT_EQ(foo[5], 10);
// }

TEST(vector_test, vector_sum) {
    Vector v1 = Vector(10);
    Vector v2 = Vector(10);
    v1[5] = 10;
    v2[8] = 12;
    
    Vector res = v1 + v2;
    ASSERT_EQ(res[5], 10);
    ASSERT_EQ(res[8], 12);
}

TEST(vector_test, vector_sum_exception) {
    Vector v1 = Vector(10);
    Vector v2 = Vector(11);
    EXPECT_THROW(v1 + v2, std::length_error);
}

