#pragma once

#include <stdio.h>
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <list>
#include <map>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <immintrin.h>


/*
Target memory layout:
v table 1                                      v table 2
[ [ ID, Pos, Light, Mesh, ID, Pos, Light, Mesh ] [ ID, Pos, RigBody, Collider, ID, Pos, RigBody, Collider ]
    ^ row 1               ^ row 2                  ^ row 1                     ^ row 2
*/


/**
 * A partition is an allocated buffer with a fixed maximum size, accessible in a linked list
 * It is the data storage used by a table, stored in a static array.
 * All partitions should end up allocated (i can't remember the proper word for here: one after another) but this allows for efficient access when the system malloc function simply won't let you
 */
class Partition {
  static constexpr uint32_t CAPACITY = 256;
public:

  // Let's just use a linked list of free slots as the vector means extra allocations
  //C:  The vector will not be cached, and will only be examined when a new row is being added to the table (which should be a very rare occasion)
  //    Meanwhile, we'd have to navigate the full linked list of free slots every time a read occurs to make sure that we aren't skipping over empty (freshly allocated but still valid) rows.
  //    Allocation function is the best option here.
  /*
    We could strore the data like this:
    [
    (Entiy Id, [Position, ...])
    (-1      , nextFree)
    ]
    so we have both an easy jump list for adding new rows and a sentinel value to check if the row is used when iterating in the system?
      Having a single item describing the next single free slot is limiting, and has some potential problems:
       - forgetting to update the flag when a slot opens before the current freeSlot means that the new value is completely forgotten about, and leaked.
       - allocating a large number of items in one go means updating the single value in a tight loop, making it basically useless; just scan the list once and track all.
       - nextFree introduces a point of contention: what do we do when the Partition is full? there's no free value. Would it be null? how would we tell the rest of the code that uses partition how to deal with null values?
       - A list of free indices solves all of these problems; updating happens in one location. Updating the list with a new set of values in one batch is easily done. When the list is empty, there are no slots, and you don't need specific nullptr handling.
   */
  //void *nextFree;



  // Allocating a new row is the same as recycling a freed existing row, and are very different operations, so we wrap them in one:
  /**
   * Provides an index of a valid entry into the Partition.
   * @returns the index of a usable row, throws if the partition is full.
   *   does a short look like a pointer? that's the index of the row.

Just realized... Pagination... that is why i called them pages...
   */
  uint16_t AllocateRow();

  // We keep the Partition constructor as no-arguments, default implementation to take advantage of some compiler inlining optimizations later on.
  // "It's a secret tool we'll use later."
  Partition() = default;

  explicit Partition(uint16_t id): partitionID(id) {}
  ~Partition();

  // A table may use multiple partitions, so we may need a way to uniquely identify them without traversing the full list every time.
  // The table creating this Partition is in charge of assigning IDs.
  uint16_t partitionID = 0;

  // Link these partitions to adjacent elements, forming a linked list
  Partition* next = nullptr;
  // May remain nullptr if is the first element in the list
  Partition* prev = nullptr;

  // All partitions are buffers of the same number of rows, so we need not note how many elements are present; we can assume that there are always <capacity-used> null/default elements present.

  uint32_t numUsedRows = 0;
  // We can remove rows at will, and reordering the partition at runtime may cause problems, so we offer a list of indices of elements that can be reused in lieu of moving memory around constantly.
  // This allows
  std::vector<uint16_t> freeRows;

  // Columns of a table are typed attributes describing Components. They are important for reading the table data, and we hold a copy of that data here.
  std::vector<void*> columns;

  // Release the row data to the Partition to be reused, leaving an empty place in the list.
  void FreeRow(uint16_t idx);
  // Immediately purge the row data by swapping the contents with the last valid row, reducing the number of used rows.
  void EraseRow(uint16_t idx);
};

/// describe it please
/**
* Describes a single cpp type
* Currently used for specifying the size of a Component
* cause we STILL don't have reflection in cpp (but maybe in cpp36)
 // Columns of a table are typed attributes describing Components.
* this would be the same as my old class
* this is only for the interface you can map them to columns or something else internally

*/
struct Type
{
  public:
    using ID = uint16_t;

    ID id;
    std::string name;
    size_t size;

    bool needsConstructor = false;
    void* defaultValue = nullptr;

    // A secret tool that we'll use later.
    uint32_t secret = 0;

    // We need to be able to default-allocate so that we can store Attributes in a vector
    Type() = default;
    // We also need to be able to move.
    Type(Type&& attr) noexcept;
    ~Type();

    void operator=(Type&& attr) noexcept;
    void operator=(Type& attr);
};

/**
 * I am not sure if this will work, but I think this solves 3 problems with your current solution by itself.
 * A mask, identifying tables with a certain set of components.
 * Allows retrieving all instances of a set of Components very, very quickly.
 * Cannot uniquely identify all tables, as such.
 *  what... A mask... as in a bit filed mask?
 *  Yes, a bitmask.
 * So you would have as many bits s there are componentTypes ?
 *  16 bits will represent a component type. 128 bits per register = 8 columns per registry that we can mask off.
 *  in the VAST MAJORITY of cases, a single table will fit in a single register, and we can just load table signatures into a register and mask them repeatedly to get the list of tables with a set of components.
 *  It's an extremely fast way to fetch all [Position,Mesh] component pairs from all tables without iterating the contents OR the actual Attribute/type data.
 *

|----- CPU register                 ----|
|-16-|-16-|-16-|-16-|-16-|-16-|-16-|-16-|
 Ligh Pos  Mesh
searhing [Pos, Mesh]

you still need to match each offset insied the register..
we need to check all places for Pos and than all places after the Pos for the Mesh
Or I'm missing what you mean

The best part about SIMD is that it doesn't care about what OR normally does.

    for (int i = 0; i < size; i++)
    {
        __m128i temp = _mm_and_si128(src.mask[i], mask.mask[i]);
        __m128i cmp = _mm_cmpeq_epi32(temp, _mm_setzero_si128());
        if ((_mm_movemask_epi8(cmp) == 0xFFFF))
            return false;
    };

    Behold. src.mask is a TableSignature containing [ Pos, Mesh, 0, 0, 0, ...] and mask.mask is our data.
    As long as Pos and Mesh are there SOMEWHERE, SIMD's and followed by cmpeq and then movemask will catch any combination.
 *
 */
class Signature {
  public:
    Signature() : data(nullptr), len(0) {};
    Signature(std::initializer_list<uint16_t> const& attrs);
    Signature(uint16_t* attrs, size_t len);
    ~Signature();

    const bool operator==(const Signature& other) const;

    const bool IsValid() const { return len > 0; }

    const bool IsPresent(uint16_t attr) const;

    void Flip(uint16_t attr);
    void Set(uint16_t attr);
    void Clear(uint16_t attr);

    static const bool CheckAll(Signature const& sig, Signature const& other);
    static const bool CheckAny(Signature const& sig, Signature const& other);

protected:
  // An up-to-1024-bit buffer backed by SSE registers.
  __m128i* data;
  uint8_t  len;
};

using EntityID = uint64_t;
struct RowMeta{
  EntityID id;
};

/**
 * Stores all entites (rows) that have the same types of components (values?)
*/
class Table {

  // All Attributes that this table contains. Not a unique identifier.
  Signature signature;

  // The actual types of the columns
  std::vector<Type> types;

  // A unique ID for the table, but only at runtime.
  uint32_t id;

  // The total number of rows of data across all Partitions in the table.
  uint32_t rows;


  // All partitions in the table.
  std::vector<Partition*> partitions;

  // Partition access accelaration structures:
  // The current partition, containing the entry that will next be added to the list.
  // the next partition with a free slot
  Partition* activePartition = nullptr;
  // The first partition with usable data.
  // what do you mean usable data? is this the start of the linked list?
  // why are we doing a linked list if we have a vector of all the partitions?
  //  Usable means that it is not default initialized.
  // so that it is not empty?
  // A Partition can hold 256 entities.
  //  If we create 800 (3.3 partitions) entities and then do stuff, then load 200 more and unload the first 800 after, we have 2 full Partitions of default-initialized data.
  //  This points to that sixth partition, which may not be the first ACTUAL partition. This effectively allows us to iterate all "actual" entities by following the linked list.
  //  AKA, yes, it skips the empty partitions.
  // Sleep is for the weak
  Partition* firstPartition = nullptr;

  // If we delete a large number of rows at once, we may end up having partitions empty prior to the active partition. This can keep track of the ones we need to re-use.
  std::vector<Partition*> emptyPartitions;

  /**
   * Later this will need a global allocator to use for the table
   * @param types List of component types in this Table
   */
  Table(
    std::vector<Type> types
  ){
    this->signature = Signature(/*types.map(t->t.id)*/);

    /*Calculate a single entity's size*/
    size_t size = 0;
    size += sizeof(RowMeta);
    for(auto & type : types){
      size += type.size;
    }

    activePartition = new Partition();
    partitions.push_back(activePartition);
  }

  // Return type is up for discussion
  void* Allocate();
  void Free(void* ptr);
  // Moves an already existing row from an other table
  void Adopt(void* ptr);

};