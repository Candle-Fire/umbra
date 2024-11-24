#include "./ecs.exp.h"
#include "catch2/catch.hpp"

struct A {};
struct Position {};
struct C {};

TEST_CASE("Archetype Allocation and Deallocation", "[Archetype]") {
  // Create an archetype with a specific type
  Types types_list = { GetTypeId<int>(TypeFlags::None).id };
  Archetype archetype(types_list);

  SECTION("Allocation") {
    size_t allocated_row = archetype.Allocate();
    REQUIRE(allocated_row != -1);
  }

  SECTION("Deallocation") {
    size_t allocated_row = archetype.Allocate();
    archetype.Deallocate(allocated_row);
    REQUIRE(archetype.rows[allocated_row].next != -1);
  }

  SECTION("Free Linked List Correctness After Multiple Allocations and Deallocations") {
    std::vector<size_t> allocated_rows;

    // Allocate multiple rows
    for (int i = 0; i < 5; ++i) {
      size_t row = archetype.Allocate();
      REQUIRE(row != -1);
      allocated_rows.push_back(row);
    }

    // Deallocate in reverse order
    for (int i = 4; i >= 0; --i) {
      archetype.Deallocate(allocated_rows[i]);
    }

    // Allocate again and ensure the free list is correct
    for (int i = 0; i < 5; ++i) {
      size_t row = archetype.Allocate();
      REQUIRE(row == allocated_rows[i]);
    }
  }

  SECTION("Allocation Beyond Capacity") {
    for (size_t i = 0; i < PAGE_SIZE; ++i) {
      size_t row = archetype.Allocate();
      REQUIRE(row != -1);
    }
    // Attempt to allocate beyond capacity
    size_t row = archetype.Allocate();
    REQUIRE(row == -1);
  }

  // SECTION("Deallocation of Invalid Row") {
  //   // Deallocate an invalid row index
  //   REQUIRE_THROWS_AS(archetype.Deallocate(PAGE_SIZE + 1), std::exception);
  // }
}
struct TestComponent {
  int value;
};

TEST_CASE("Archetype Column Manipulation") {
    Types types_list = { GetTypeId<int>(TypeFlags::None).id, GetTypeId<float>(TypeFlags::None).id };
    Archetype archetype(types_list);

    struct TestComponent1 {
        int value;
    };

    struct TestComponent2 {
        float value;
    };

    SECTION("Add and Retrieve Components") {
        size_t row = archetype.Allocate();
        SH::span_dynamic& column1 = archetype.GetColumn(types_list[0]);
        new(column1[row].as_ptr<TestComponent1>()) TestComponent1{42};

        SH::span_dynamic& column2 = archetype.GetColumn(types_list[1]);
        new(column2[row].as_ptr<TestComponent2>()) TestComponent2{3.14f};

        REQUIRE(column1[row].as<TestComponent1>().value == 42);
        REQUIRE(column2[row].as<TestComponent2>().value == 3.14f);
    }

    SECTION("GetColumn with Invalid TypeId") {
        TypeId invalid_type{999, TypeFlags::None, 0};
        REQUIRE_THROWS_AS(archetype.GetColumn(invalid_type), std::out_of_range);
    }
}

TEST_CASE("Archetype State Validation") {
    Types types_list = { GetTypeId<TestComponent>().id };
    Archetype archetype(types_list);

    SECTION("State After Allocation and Deallocation") {
        size_t row1 = archetype.Allocate();
        size_t row2 = archetype.Allocate();
        REQUIRE(row1 != row2);

        archetype.Deallocate(row1);
        REQUIRE(archetype.rows[row1].next == 2);

        size_t row3 = archetype.Allocate();
        REQUIRE(row3 == row1);
    }
}

TEST_CASE("Archetype Memory Consistency") {
    Types types_list = { GetTypeId<TestComponent>().id };
    Archetype archetype(types_list);

    SECTION("Memory Allocation and Deallocation Consistency") {
        size_t row1 = archetype.Allocate();
        SH::span_dynamic& column = archetype.GetColumn(types_list[0]);
        new(column[row1].as_ptr<TestComponent>()) TestComponent{42};

        archetype.Deallocate(row1);
        size_t row2 = archetype.Allocate();
        REQUIRE(row1 == row2);

        new(column[row2].as_ptr<TestComponent>()) TestComponent{84};
        REQUIRE(column[row2].as<TestComponent>().value == 84);
    }
}

struct ComplexComponent {
  std::string value;
  ComplexComponent(const std::string& val) : value(val) {}
};

TEST_CASE("Archetype Copy From for Complex Types") {
  auto type_id       = GetTypeId<ComplexComponent>().id;
  Types types_list_1 = { type_id };
  Archetype archetype_1(types_list_1);

  Types types_list_2 = { type_id };
  Archetype archetype_2(types_list_2);

  archetype_1.Allocate();
  SH::span_dynamic& column = archetype_1.GetColumn(types_list_1[0]);
  new(column[0].as_ptr<ComplexComponent>()) ComplexComponent{"Hello, World!"};

  SECTION("CopyFrom another archetype with complex type") {
      size_t target_row = archetype_2.Allocate();
      archetype_2.CopyFrom(archetype_1, 0, target_row);

      SH::span_dynamic& target_column = archetype_2.GetColumn(types_list_2[0]);
      REQUIRE(target_column[target_row].as<ComplexComponent>().value == "Hello, World!");
  }
}

TEST_CASE("Archetype Destructor and Cleanup") {
    Types types_list = { GetTypeId<int>(TypeFlags::None).id };

    SECTION("Cleanup on Destruction") {
        Archetype* archetype = new Archetype(types_list);
        size_t row = archetype->Allocate();
        SH::span_dynamic& column = archetype->GetColumn(types_list[0]);
        new(column[row].as_ptr<int>()) int{42};

        delete archetype;
        // If there are memory leaks, the test runner will report them.
    }
}

