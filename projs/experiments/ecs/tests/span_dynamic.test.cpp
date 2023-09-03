#include "catch2/catch.hpp"

#include "ecs.exp.h"

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

TEST_CASE("PoolAllocator tests") {
    SECTION("Basic allocation") {
        SH::PoolAllocator pool(sizeof(TestObject));
        auto obj = static_cast<TestObject *>(pool.allocate());

        REQUIRE(obj != nullptr);
    }

    SECTION("Multiple allocations within a single chunk") {
        SH::PoolAllocator pool(sizeof(TestObject));
        std::vector<TestObject *> objs;

        for (int i = 0; i < 1024; ++i) {
            auto obj = static_cast<TestObject *>(pool.allocate());
            REQUIRE(obj != nullptr);
            objs.push_back(obj);
        }
    }

    SECTION("Multiple allocations across multiple chunks") {
        SH::PoolAllocator pool(sizeof(TestObject));
        std::vector<TestObject *> objs;

        // Assuming that each chunk can hold 1024 items,
        // this will require at least 2 chunks.
        for (int i = 0; i < 2048; ++i) {
            auto obj = static_cast<TestObject *>(pool.allocate());
            REQUIRE(obj != nullptr);
            objs.push_back(obj);
        }
    }

    SECTION("Allocation and deallocation") {
        SH::PoolAllocator pool(sizeof(TestObject));
        auto obj1 = static_cast<TestObject *>(pool.allocate());
        auto obj2 = static_cast<TestObject *>(pool.allocate());

        REQUIRE(obj1 != nullptr);
        REQUIRE(obj2 != nullptr);

        pool.deallocate(obj1);
        pool.deallocate(obj2);

        // After deallocation, new allocations should reuse the freed memory
        auto obj3 = static_cast<TestObject *>(pool.allocate());
        auto obj4 = static_cast<TestObject *>(pool.allocate());

        REQUIRE(obj3 != nullptr);
        REQUIRE(obj4 != nullptr);

        // The addresses should be reused in reverse order
        REQUIRE(obj3 == obj2);
        REQUIRE(obj4 == obj1);
    }
}