// test_example.cpp
#include "../test.hpp"

#include <vector>

TEST_MAIN(
    TEST_CASE("basic arithmetic",
        TEST(1 + 1 == 2);
        TEST(5 * 5 == 25);
    ),
    
    TEST_CASE("exceptions are caught",
        TEST_THROWS_AS(throw std::runtime_error("error"), std::runtime_error);
    ),

    TEST_CASE("vector operations",
        std::vector<int> v;
        TEST(v.empty());
        v.push_back(42);
        TEST(v.size() == 1);
    )
)