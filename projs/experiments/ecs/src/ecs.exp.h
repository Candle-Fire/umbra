#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <algorithm>
#include <functional>

//#include "../../shadow/shadow-engine/reflection/inc/shadow/SHObject.h"
//#include "../../shadow/shadow-engine/core/inc/shadow/exports.h"

namespace SH {

  class span_dynamic {
    std::byte *p_start;
    std::byte *p_end;
    size_t itemSize;
  public:
    span_dynamic() = default;
    span_dynamic(void *mem, size_t item_size, size_t count) :
        p_start(static_cast<std::byte *>(mem)),
        p_end(p_start + (item_size * count)),
        itemSize(item_size) {
        assert((p_end - p_start) / item_size == count);
    }

  public:
    class iterator {
    public:
      using difference_type = std::ptrdiff_t;
      using value_type = std::byte;
    private:
      std::byte *pos;
      size_t size;

    public:
      iterator() : pos(nullptr), size(0) {};
      iterator(std::byte *p, size_t item_size) : pos(p), size(item_size) {};

      void move(size_t n) {
          pos += (n * size);
      }

      iterator &operator++() {
          move(1);
          return *this;
      }
      iterator operator++(int) {
          iterator tmp(*this);
          operator++();
          return tmp;
      }

      iterator &operator--() {
          move(-1);
          return *this;
      }
      iterator operator--(int) {
          iterator tmp(*this);
          operator--();
          return tmp;
      }

      iterator &operator+=(difference_type n) {
          move(n);
          return *this;
      }
      iterator &operator-=(difference_type n) {
          move(-n);
          return *this;
      }

      iterator operator+(const difference_type &n) const {
          iterator tmp(*this);
          tmp.move(n);
          return tmp;
      }
      iterator operator-(const difference_type &n) const {
          iterator tmp(*this);
          tmp.move(-n);
          return tmp;
      }

      std::byte &operator[](const difference_type &n) const {
          iterator tmp(*this);
          tmp.move(n);
          return *tmp;
      }

      difference_type operator-(const iterator &rhs) const { return pos - rhs.pos; }

      bool operator==(const iterator &rhs) const { return pos == rhs.pos; }
      bool operator!=(const iterator &rhs) const { return pos != rhs.pos; }

      bool operator<(const iterator &rhs) const { return pos < rhs.pos; }
      bool operator<=(const iterator &rhs) const { return pos <= rhs.pos; }
      bool operator>(const iterator &rhs) const { return pos > rhs.pos; }
      bool operator>=(const iterator &rhs) const { return pos >= rhs.pos; }

      std::byte &operator*() const { return *pos; }

      template<class T>
      T &as() { return *(T *) pos; }
      template<class T>
      T *as_ptr() { return (T *) pos; }

      void *ptr() const { return pos; }
    };

    iterator begin() const {
        return {p_start, itemSize};
    }

    iterator end() const {
        return {p_end, itemSize};
    }

    iterator last() const {
        return end()--;
    }
  };

  span_dynamic::iterator operator+(span_dynamic::iterator::difference_type n, span_dynamic::iterator i) {
      span_dynamic::iterator tmp(i);
      tmp.move(n);
      return tmp;
  }
  span_dynamic::iterator operator-(span_dynamic::iterator::difference_type n, span_dynamic::iterator i) {
      span_dynamic::iterator tmp(i);
      tmp.move(-n);
      return tmp;
  }

  bool operator<(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() < rhs; }
  bool operator<(const void *lhr, const span_dynamic::iterator &rhs) { return lhr < rhs.ptr(); }
  bool operator<=(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() <= rhs; }
  bool operator>(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() > rhs; }
  bool operator>(const void *lhr, const span_dynamic::iterator &rhs) { return lhr > rhs.ptr(); }
  bool operator>=(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() >= rhs; }
  bool operator>=(const void *lhr, const span_dynamic::iterator &rhs) { return lhr >= rhs.ptr(); }
}

static_assert(std::input_or_output_iterator<SH::span_dynamic::iterator>);
static_assert(std::random_access_iterator<SH::span_dynamic::iterator>);

static_assert(std::ranges::random_access_range<SH::span_dynamic>);

namespace SH {

  class PoolAllocator {
  public:

    struct Item {
      Item *next;
    };

    explicit PoolAllocator(size_t item_size) : item_size(std::max(item_size, sizeof(Item))) {
        chunks.push_back(new Chunk(item_size));
    }

    class Chunk {
      span_dynamic memory; //= span_dynamic(nullptr, 0, 0);
      Item *next_free = nullptr;
    public:
      explicit Chunk(size_t item_size) {
          void *m = malloc(item_size * 1024);
          memory = span_dynamic(m, item_size, 1024);

          next_free = memory.begin().as_ptr<Item>();
          for (auto it = memory.begin(); it != memory.end(); it++) {
              auto next = it + 1;
              it.as_ptr<Item>()->next = next.as_ptr<Item>();
          }
          memory.last().as_ptr<Item>()->next = nullptr;
      }

      bool HasSpace() { return next_free != nullptr; }

      void *allocate() {
          assert(next_free != nullptr);
          auto ptr = next_free;
          next_free = next_free->next;
          return ptr;
      }

      void deallocate(void *p) {
          assert(contains(p));
          auto item = static_cast<Item *>(p);
          item->next = next_free;
          next_free = item;
      }

      bool contains(void *p) {
          return p >= memory.begin() && p < memory.end();
      }
    };

  public:

    virtual void *allocate() {
        auto chunk_it = std::ranges::find_if(chunks, [](auto chunk) { return chunk->HasSpace(); });

        // If no chunk has space, create a new one
        if (chunk_it == chunks.end()) {
            chunks.push_back(new Chunk(item_size));
            chunk_it = std::prev(chunks.end());
        }

        return (*chunk_it)->allocate();
    };

    virtual void deallocate(void *p) {
        auto chunk_it = std::ranges::find_if(chunks, [p](auto chunk) { return chunk->contains(p); });
        if (chunk_it == chunks.end())
            throw std::invalid_argument("WTF man common use pointer correctly");

        (*chunk_it)->deallocate(p);
    };
  private:
    std::vector<Chunk *> chunks;
    size_t item_size;
  };

}

/*
 * Archetype : (T1, T2, T3, T4)
 * | self (T1)  | comp 1 (T2) | comp 2 (T3) | comp 3 (T4) |
 * | T1: 1      | T2: 1       | T3: 1       | T4: 1       |
 * | T1: 2      | T2: 2       | T3: 2       | T4: 2       |
 * | T1: 3      | T2: 3       | T3: 3       | T4: 3       |
 *
 * Archetype : (T1, T2, T3)
 * | self (T1)  | comp 1 (T2) | comp 2 (T3) |
 * | T1: 4      | T2: 4       | T3: 4       |
 * | T1: 5      | T2: 5       | T3: 5       |
 *
 *
 */
class Object {

};

class Entity {

};
template<class T>
concept entity = std::is_base_of_v<Entity, T>;

union Id {
  std::byte bytes[sizeof(uint64_t)];
  uint64_t id;
};

using TypeId = Id;

template<>
struct std::hash<TypeId> {
  std::size_t operator()(const TypeId &s) const noexcept {
      return std::hash<uint64_t>{}(s.id);
  };
};

TypeId next_id;

template<class T>
TypeId getTypeId() {
    static TypeId id = next_id;
    id.id++;
    return id;
}

class Archetype {
  using Id = uint32_t;
  static Id next_id;

  std::vector<TypeId> types;
  Id id;
public:
  Archetype() = default;
  Archetype(std::initializer_list<TypeId> types) : types(types), id(next_id++) {

  }
};

class EntityManager {

  std::unordered_map<TypeId, SH::PoolAllocator> pools = {};

  template<entity Ent>
  SH::PoolAllocator &GetPool(TypeId type) {
      if (!pools.contains(type)) {
          pools.insert({type, SH::PoolAllocator(sizeof(Ent))});
      }
      return pools.at(type);
  }

  template<entity Ent>
  Ent *AddChild(Entity &parent) {
      // Allocate the space for the entity
      SH::PoolAllocator &pool = GetPool<Ent>(getTypeId<Ent>());
      void *pos = pool.allocate();

      // Create it
      Ent *entity = new(pos)Ent();

      // Add it to the parent
      return entity;
  }

};

Archetype::Id Archetype::next_id = 0;