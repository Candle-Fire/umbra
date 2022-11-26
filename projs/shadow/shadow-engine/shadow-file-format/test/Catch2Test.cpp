#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("15 is less than 20", "[numbers]") {
    REQUIRE(15 < 20);
}