#include "./ecs.exp.h"
#include "catch2/catch.hpp"

struct A : Component<>
{
  int a;
  float b;
  A(int a, float b) : a(a), b(b) {}
  void Print() const override {};
};
struct B : Component<>
{
  std::string value;
  explicit B(const char * str): value(str) {};
  void Print() const override {}
};
struct C : Component<>
{
  void Print() const override {}
};

TEST_CASE("Archetype Allocation and De-allocation", "[Archetype]")
{
  // Create an archetype with a specific type
  Types types_list = {GetNodeType<A>()};
  Archetype archetype(types_list);

  SECTION("Allocation")
  {
    size_t allocated_row = archetype.Allocate();
    REQUIRE(allocated_row != -1);
  }

  SECTION("Deallocation")
  {
    size_t allocated_row = archetype.Allocate();
    archetype.Deallocate(allocated_row);
    REQUIRE(archetype.rows[allocated_row].next != -1);
  }

  SECTION("Free Linked List Correctness After Multiple Allocations and De-allocations")
  {
    std::vector<size_t> allocated_rows;

    // Allocate multiple rows
    for (int i = 0; i < 5; ++i)
    {
      size_t row = archetype.Allocate();
      REQUIRE(row != -1);
      allocated_rows.push_back(row);
    }

    // Deallocate in reverse order
    for (int i = 4; i >= 0; --i)
    {
      archetype.Deallocate(allocated_rows[i]);
    }

    // Allocate again and ensure the free list is correct
    for (int i = 0; i < 5; ++i)
    {
      size_t row = archetype.Allocate();
      REQUIRE(row == allocated_rows[i]);
    }
  }

  SECTION("Allocation Beyond Capacity")
  {
    for (size_t i = 0; i < PAGE_SIZE; ++i)
    {
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


TEST_CASE("Archetype Column Manipulation", "[Archetype]")
{
  const Types types_list = {GetNodeType<A>(), GetNodeType<B>()};
  Archetype archetype(types_list);

  SECTION("Components Constructable in column")
  {
    const size_t row                = archetype.Allocate();
    const SH::span_dynamic &column1 = archetype.GetColumn(types_list[0]);
    new (column1[row].as_ptr<A>()) A{42, 10.3};

    SH::span_dynamic &column2 = archetype.GetColumn(types_list[1]);
    new (column2[row].as_ptr<B>()) B{"asd"};

    REQUIRE(column1[row].as<A>().a == 42);
    REQUIRE(column2[row].as<B>().value == "asd");
  }

  SECTION("GetColumn with Invalid TypeId")
  {
    NodeType invalid_type{999, TypeFlags::None, 0};
    REQUIRE_THROWS_AS(archetype.GetColumn(invalid_type), std::out_of_range);
  }
}

TEST_CASE("Archetype State Validation", "[Archetype]")
{
  const Types types_list = {GetNodeType<A>()};
  Archetype archetype(types_list);

  SECTION("State After Allocation and Deallocation")
  {
    size_t row1 = archetype.Allocate();
    size_t row2 = archetype.Allocate();
    REQUIRE(row1 != row2);

    archetype.Deallocate(row1);
    REQUIRE(archetype.rows[row1].next == 2);

    size_t row3 = archetype.Allocate();
    REQUIRE(row3 == row1);
  }
}

TEST_CASE("Archetype Memory Consistency", "[Archetype]")
{
  const Types types_list = {GetNodeType<A>()};
  Archetype archetype(types_list);

  SECTION("Memory Allocation and De-allocation Consistency")
  {
    size_t row1              = archetype.Allocate();
    SH::span_dynamic &column = archetype.GetColumn(types_list[0]);
    new (column[row1].as_ptr<A>()) A{42, 10.3};

    archetype.Deallocate(row1);
    size_t row2 = archetype.Allocate();
    REQUIRE(row1 == row2);

    new (column[row2].as_ptr<A>()) A{84, 22.4f};
    REQUIRE(column[row2].as<A>().a == 84);
    REQUIRE(column[row2].as<A>().b == 22.4f);
  }
}

TEST_CASE("Archetype Copy From for Complex Types")
{
  auto type_id       = GetNodeType<B>();
  Types types_list_1 = {type_id};
  Archetype archetype_1(types_list_1);

  Types types_list_2 = {type_id};
  Archetype archetype_2(types_list_2);

  archetype_1.Allocate();
  SH::span_dynamic &column = archetype_1.GetColumn(types_list_1[0]);
  new (column[0].as_ptr<B>()) B{"Hello, World!"};

  SECTION("CopyFrom another archetype with complex type")
  {
    size_t target_row = archetype_2.Allocate();
    archetype_2.CopyFrom(archetype_1, 0, target_row);

    SH::span_dynamic &target_column = archetype_2.GetColumn(types_list_2[0]);
    REQUIRE(target_column[target_row].as<B>().value == "Hello, World!");
  }
}

TEST_CASE("Archetype Destructor and Cleanup")
{
  const Types types_list = {GetNodeType<A>()};

  SECTION("Cleanup on Destruction")
  {
    Archetype *archetype     = new Archetype(types_list);
    const size_t row               = archetype->Allocate();
    SH::span_dynamic &column = archetype->GetColumn(types_list[0]);
    new (column[row].as_ptr<int>()) int{42};

    delete archetype;
    // If there are memory leaks, the test runner will report them. In theory
  }
}
