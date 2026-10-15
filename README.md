# test

`test` is a tiny C++ test helper for hand-written test programs: one header, one main, and ordinary C++ control flow.

## USE

```cpp
// test_example.cpp
#include "test.hpp"

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
```

```text
./test_example
PASS  basic arithmetic
PASS  exceptions are caught
PASS  vector operations

3 passed, 0 failed
```

## CMake and CTest

CTest can run a test executable as one test. The executable runs all of its
`TEST_CASE`s, and its exit code tells CTest whether the run passed. Add a
`CMakeLists.txt` like this:

```cmake
cmake_minimum_required(VERSION 3.20)
project(test_example LANGUAGES CXX)

include(CTest)

if(BUILD_TESTING)
    add_executable(basic_tests examples/basic.cpp)
    target_compile_features(basic_tests PRIVATE cxx_std_20)
    add_test(NAME basic COMMAND basic_tests)
endif()
```

Configure, build, and run the test with:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest reports one entry for the executable, even though the helper runs each
case inside it:

```text
Internal ctest changing into directory: .../build
Test project .../build
    Start 1: basic
1/1 Test #1: basic ............................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1
```

CTest hides the executable's output when it passes. To see the helper's
per-case output, run CTest in verbose mode:

```sh
ctest --test-dir build -V
```

When the executable fails, `--output-on-failure` displays its output. To have
CTest list or filter individual cases separately, the executable would need
to support selecting a case by name; by default, every invocation runs all
cases.
