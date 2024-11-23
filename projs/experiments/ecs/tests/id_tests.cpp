#include "./ecs.exp.h"
#include "catch2/catch.hpp"

struct A {};
struct B {};
struct C {};

TEST_CASE("id TESTS") {
    SECTION("Gives back ID") {
        const auto res = GetTypeId<A>();
        REQUIRE(res.id > 0);
    }
    SECTION("Gives back same ID") {
        const auto res1 = GetTypeId<A>();
        const auto res2 = GetTypeId<A>();
        REQUIRE(res1.id == res2.id);
    }
    SECTION("Gives back different ID") {
        const auto res1 = GetTypeId<A>();
        const auto res2 = GetTypeId<B>();
        REQUIRE(res1.id != res2.id);
    }
}
