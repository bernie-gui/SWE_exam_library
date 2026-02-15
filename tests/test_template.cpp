#include "catch_lib.hpp"  // Include the Catch2 header

// ==================================================================================
// CATCH2 BASICS FOR BEGINNERS
// ==================================================================================
// 
// 1. TEST_CASE( "Name", "[tags]" )
//    - Defines a test case with a descriptive name and tags.
//    - Tags allow running specific tests: ./test_exec "[math]"
//
// 2. REQUIRE( condition )
//    - Fatal assertion. If it fails, the test stops immediately.
//    - Use for critical checks (e.g., pointer validity).
//
// 3. CHECK( condition )
//    - Non-fatal assertion. If it fails, the test continues.
//    - Use when you want to see all failures in a test case.
//
// 4. SECTION( "Description" )
//    - Sub-tests within a case. 
//    - Catch2 runs the TEST_CASE from the start for EACH section.
//    - Useful for sharing setup code (fixtures).
// ==================================================================================

// Example 1: Math Test (Tag: [math])
TEST_CASE("Basic Math Operations", "[math]") {
    int a = 10;
    int b = 20;

    SECTION("Addition works") {
        REQUIRE(a + b == 30);
    }

    SECTION("Multiplication works") {
        CHECK(a * b == 200);
    }
}

// Example 2: String Test (Tag: [strings])
TEST_CASE("String Manipulations", "[strings]") {
    std::string s = "hello";

    SECTION("Length check") {
        REQUIRE(s.length() == 5);
    }

    SECTION("Concatenation") {
        s += " world";
        CHECK(s == "hello world");
    }
}

// Example 3: Edge Cases
TEST_CASE("Edge Cases", "[edge]") {
    // Check behavior with zero or empty inputs
    SECTION("Division by zero check (conceptual)") {
        int divisor = 1; // Change to 0 to simulate failure if needed
        REQUIRE(divisor != 0);
        CHECK(10 / divisor == 10);
    }
}
