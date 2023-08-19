
#include <cstdint>
#include <iostream>

#include "NodeContainer.h"

#if false
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

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
            REQUIRE(begin.GetContainer() == &container);

            THEN("begin is at the end of the chunks") {
                REQUIRE(begin.GetChunkIndex() == container.m_chunks.size());
            }

            THEN("begin is at the end of the chunk") {
                REQUIRE(begin.GetElement() == NodeContainer<TestClass>::MemoryChunk::Iterator());
            }

            auto end = container.end();
            REQUIRE(end.GetContainer() == &container);

            THEN("end is at the end of the chunks") {
                REQUIRE(begin.GetChunkIndex() == container.m_chunks.size());
            }

            THEN("end is at the end of the chunk") {
                REQUIRE(begin.GetElement() == NodeContainer<TestClass>::MemoryChunk::Iterator());
            }

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

    GIVEN("container with X allocations") {

        int count = GENERATE(1, 10, 2024, 4 * 2024);
        CAPTURE(count);

        for (int i = 0; i < count; ++i) {
            TestClass *ptr = container.allocateWithType();
        }

        WHEN("iterating") {

            auto begin = container.begin();
            REQUIRE(begin.GetContainer() == &container);

            THEN("begin is at the first of the chunks") {
                REQUIRE(begin.GetChunkIndex() == 0);
            }

            THEN("begin is at the first of the chunk") {
                REQUIRE(begin.GetElement() == container.m_chunks[0]->begin());
            }

            auto end = container.end();
            REQUIRE(end.GetContainer() == &container);

            THEN("end is at the end of the chunks") {
                REQUIRE(end.GetChunkIndex() == container.m_chunks.size());
            }

            THEN("end is at the end of the chunk") {
                REQUIRE(end.GetElement() == NodeContainer<TestClass>::MemoryChunk::Iterator());
            }

            THEN("begin is not equal to end") {
                REQUIRE(begin != end);
            }

            THEN("should iterate X times") {

                int iterCount = 0;
                for (auto &a : container) {
                    iterCount++;
                }

                REQUIRE(iterCount == count);
            }
        }
    }

    GIVEN("container with X allocations and all de-allocated") {

        int count = GENERATE(1, 10, 2024, 4 * 2024);
        CAPTURE(count);

        std::vector<TestClass *> ptrs;

        for (int i = 0; i < count; ++i) {
            TestClass *ptr = container.allocateWithType();
            ptrs.push_back(ptr);
        }

        for (int i = 0; i < count; ++i) {
            container.DestroyObject(ptrs[i]);
        }

        WHEN("iterating") {

            auto begin = container.begin();
            REQUIRE(begin.GetContainer() == &container);

            THEN("begin is at the end of the chunks") {
                REQUIRE(begin.GetChunkIndex() == container.m_chunks.size());
            }

            THEN("begin is at the end of the chunk") {
                REQUIRE(begin.GetElement() == NodeContainer<TestClass>::MemoryChunk::Iterator());
            }

            auto end = container.end();
            REQUIRE(end.GetContainer() == &container);

            THEN("end is at the end of the chunks") {
                REQUIRE(end.GetChunkIndex() == container.m_chunks.size());
            }

            THEN("end is at the end of the chunk") {
                REQUIRE(end.GetElement() == NodeContainer<TestClass>::MemoryChunk::Iterator());
            }

            THEN("begin is not equal to end") {
                REQUIRE(begin == end);
            }

            THEN("should iterate X times") {

                int iterCount = 0;
                for (auto &a : container) {
                    iterCount++;
                }

                REQUIRE(iterCount == 0);
            }
        }
    }

}

TEST_CASE("Node Container Benchmarks", "[NodeContainer]") {
    NodeContainer<TestClass> container;

    BENCHMARK("Allocate 1000") {
                                   for (int i = 0; i < 1000; i++) {
                                       container.allocateWithType();
                                   }
                               };

    BENCHMARK("Allocate 1000000") {
                                      for (int i = 0; i < 1000000; i++) {
                                          container.allocateWithType();
                                      }
                                  };

}

#endif