// test.hpp -- tiny C++ 20 test framework
// Copyright (c) 2026 by Nicholas Lane
// SPDX-License-Identifier: MIT

#pragma once

// Standard-library dependencies ------------------------------------------------

#include <exception>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace test {

// Internal failure handling -------------------------------------------------------------

namespace detail {

constexpr std::string_view color_green = "\033[32m";
constexpr std::string_view color_red = "\033[31m";
constexpr std::string_view color_reset = "\033[0m";

// Assertions use a private sentinel exception to leave the current case while
// retaining its call site. run_one catches it before classifying user exceptions.
struct failure : std::runtime_error {
    std::source_location location;

    failure(std::string_view message, std::source_location location)
        : std::runtime_error(std::string(message)), location(location) 
    {}
};

template<class Action>
bool run_one(std::string_view name, Action&& action) 
{
    // Convert one callable into PASS, FAIL, or ERROR. Caught failures and
    // exceptions do not prevent later cases from running.
    try {
        std::forward<Action>(action)();
    } catch (const failure& error) {
        std::cout << color_red << "[ FAIL ] " << color_reset << name << '\n'
                  << "         at " << error.location.file_name() 
                  << ':' << error.location.line() << " -> " << error.what() << '\n';
        return false;
    } catch (const std::exception& error) {
        std::cout << color_red << "[ ERROR] " << color_reset << name 
                  << " (unexpected exception: " << error.what() << ")\n";
        return false;
    } catch (...) {
        std::cout << color_red << "[ ERROR] " << color_reset << name 
                  << " (unknown exception)\n";
        return false;
    }
    std::cout << color_green << "[ PASS ] " << color_reset << name << '\n';
    return true;
}

} // namespace detail

// Public assertions -------------------------------------------------------------

template<class Condition>
void check(Condition&& condition, std::string_view message = "check failed",
           std::source_location location = std::source_location::current()) 
{
    // The default argument captures the caller rather than this function body.
    // A false boolean-like value leaves the current case through failure.
    if (!static_cast<bool>(std::forward<Condition>(condition))) {
        throw detail::failure(message, location);
    }
}

template<class Error, class Action>
void throws(Action&& action, std::string_view message = "expected exception was not thrown",
            std::source_location location = std::source_location::current()) 
{
    try {
        std::forward<Action>(action)();
    } catch (const detail::failure&) {
        // Do not let throws<std::exception> mistake an assertion for success. 
        throw;
    } catch (const Error&) {
        // Normal C++ catch rules apply, so derived error types also satisfy it.
        return; 
    } 
    throw detail::failure(message, location);
}

// Test model and execution -------------------------------------------------------------

// A test owns an ordinary name and callable; no registry or static setup exists.
template<class Action>
struct test {
    std::string name;
    Action action;
};

template<class... Tests>
int run(std::string_view suite_path, Tests&&... tests)
{
    // Extract base filename from path (e.g. .build/tests/compressor -> compressor)
    auto last_slash = suite_path.find_last_of("/\\");
    std::string_view suite_name = (last_slash != std::string_view::npos) 
        ? suite_path.substr(last_slash + 1) 
        : suite_path;

    if (!suite_name.empty()) {
        std::cout << "=== RUNNING: " << suite_name << " ===\n";
    } else {
        std::cout << "--- RUNNING TESTS ---\n";
    }

    int passed = 0;
    int failed = 0;
    
    ((detail::run_one(tests.name, std::forward<Tests>(tests).action) ? ++passed : ++failed),
     ...);
    
    std::cout << "----------------------------------------\n"
              << "RESULTS: " << passed << " passed, " << failed << " failed.\n\n";
    
    return failed != 0;
}

} // namespace test

// Convenience macros -----------------------------------------------------------

// Cases capture surrounding test data by reference; run them before it goes
// out of scope.
#define TEST_CASE(name, ...) ::test::test{name, [&] { __VA_ARGS__; }}
// The assertion expression is evaluated once; stringization supplies its text.
#define TEST(...) ::test::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__)
// The expression runs once inside its case. A variadic type argument permits
// expected exception types whose template spelling itself contains commas.
#define TEST_THROWS_AS(expression, ...)                                                     \
    ::test::throws<__VA_ARGS__>([&] { static_cast<void>(expression); },                     \
                               #expression " did not throw " #__VA_ARGS__)

// Automatically captures argv[0] to derive the test suite name
#define TEST_MAIN(...)                                                              \
    int main(int argc, char* argv[]) {                                              \
        return ::test::run(argc > 0 ? argv[0] : "", __VA_ARGS__);                   \
    }