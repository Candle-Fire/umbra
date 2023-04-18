
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <cstdint>

#include "NodeContainer.h"

uint32_t factorial( uint32_t number ) {
    return number <= 1 ? number : factorial(number-1) * number;
}

using ShadowEngine::Entities::NodeContainer;

class TestClass{

};

void a(){
    NodeContainer<TestClass> container;
}

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( factorial( 1) == 1 );
    REQUIRE( factorial( 2) == 2 );
    REQUIRE( factorial( 3) == 6 );
    REQUIRE( factorial(10) == 3'628'800 );
}