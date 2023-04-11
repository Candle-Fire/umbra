
#include <cstdint>
#include <iostream>

#include "NodeContainer.h"

#include <catch2/catch.hpp>

uint32_t factorial(uint32_t number) {
    return number <= 1 ? number : factorial(number - 1) * number;
}

using ShadowEngine::Entities::NodeContainer;

class TestClass {

};

std::ostream &operator<<(std::ostream &os, NodeContainer<TestClass>::MemoryChunk::Iterator const &value) {
    os << "{chunk: " << value.GetChunk() << " index: " << value.GetIndex() << "}";
    return os;
}

SCENARIO("Node Container Chunk", "[NodeContainer]") {

    NodeContainer<TestClass>::MemoryChunk chunk;

    GIVEN("empty container") {
        REQUIRE(chunk.count == 0);

        WHEN("iterating") {

            auto begin = chunk.begin();
            REQUIRE(begin.GetChunk() == &chunk);

            auto end = chunk.end();
            REQUIRE(end.GetChunk() == &chunk);

            THEN("begin is the end index") {
                REQUIRE(begin.GetIndex() == 2048);
            }

            THEN("end is the end index") {
                REQUIRE(end.GetIndex() == 2048);
            }

            THEN("begin is equal to end") {
                REQUIRE(begin == end);
            }

            int count = 0;
            for (auto &a : chunk) {
                count++;
            }

            THEN("should not iterate") {
                REQUIRE(count == 0);
            }
        }
    }

    GIVEN("container with X allocations") {

        int count = GENERATE(1, 10, 50);
        CAPTURE(count);

        for (int i = 0; i < count; ++i) {
            TestClass *ptr = chunk.allocate();
        }

        WHEN("iterating") {

            auto begin = chunk.begin();
            REQUIRE(begin.GetChunk() == &chunk);

            auto end = chunk.end();
            REQUIRE(end.GetChunk() == &chunk);

            THEN("begin is the end index") {
                REQUIRE(begin.GetIndex() == 0);
            }

            THEN("end is the end index") {
                REQUIRE(end.GetIndex() == 2048);
            }

            THEN("begin is equal to end") {
                //std::cout << begin << end;
                REQUIRE(begin != end);
            }

            int iterCount = 0;
            for (auto &a : chunk) {
                iterCount++;
            }

            THEN("should iterate X times") {
                REQUIRE(iterCount == count);
            }
        }
    }

    GIVEN("container with 10 allocations and first de-allocated") {

        TestClass *first = chunk.allocate();

        for (int i = 0; i < 10; ++i) {
            TestClass *ptr = chunk.allocate();
        }

        chunk.free(first);

        WHEN("iterating") {

            auto begin = chunk.begin();
            REQUIRE(begin.GetChunk() == &chunk);

            auto end = chunk.end();
            REQUIRE(end.GetChunk() == &chunk);

            THEN("begin is the end index") {
                REQUIRE(begin.GetIndex() == 1);
            }

            THEN("end is the end index") {
                REQUIRE(end.GetIndex() == 2048);
            }

            THEN("begin is equal to end") {
                REQUIRE(begin != end);
            }

            int iterCount = 0;
            for (auto &a : chunk) {
                iterCount++;
            }

            THEN("should iterate X times") {
                REQUIRE(iterCount == 10);
            }
        }
    }

}

SCENARIO("Node Container", "[NodeContainer]") {

    NodeContainer<TestClass> container;

    GIVEN("empty container") {

        WHEN("iterating") {

            auto begin = container.begin();
            //REQUIRE(begin.GetChunk() == &chunk);

            auto end = container.end();
            //REQUIRE(end.GetChunk() == &chunk);

            THEN("begin is equal to end") {
                REQUIRE(begin == end);
            }

            int count = 0;
            for (auto &a : container) {
                count++;
            }

            THEN("should not iterate") {
                REQUIRE(count == 0);
            }
        }
    }

}