#include "span_dynamic.h"
#include "catch2/catch.hpp"

struct TestObject {
  int32_t a = 1;
  int32_t b = 2;
  int32_t c = 3;
  int32_t d = 4;
};

TEST_CASE("span_dynamic tests") {
    SECTION("Initialization and iteration") {
        int arr[5] = {1, 2, 3, 4, 5};
        SH::span_dynamic const span(arr, sizeof(int), 5);

        int i = 1;
        for (auto it = span.begin(); it != span.end(); ++it) {
            REQUIRE(it.as<int>() == i++);
        }
    }

    SECTION("Random access") {
        int arr[5] = {1, 2, 3, 4, 5};
        SH::span_dynamic const span(arr, sizeof(int), 5);

        auto it = span.begin();
        it += 2;
        REQUIRE(it.as<int>() == 3);
    }
}