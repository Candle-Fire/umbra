#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <algorithm>
#include <functional>
#include <numeric>

namespace SH
{
  /**
  * A view of a given memory area in item_size steps
  */
  class span_dynamic
  {
    std::byte* p_start;
    std::byte* p_end;
    size_t item_size;

  public:
    span_dynamic() = default;

    span_dynamic(void* mem, const size_t item_size, const size_t count) :
      p_start(static_cast<std::byte*>(mem)),
      p_end(p_start + (item_size * count)),
      item_size(item_size)
    {
      assert((p_end - p_start) / item_size == count);
    }

    class iterator
    {
    public:
      using difference_type = std::ptrdiff_t;
      using value_type      = std::byte;

    private:
      std::byte* pos;
      size_t size;

    public:
      iterator() : pos(nullptr), size(0)
      {
      };

      iterator(std::byte* p, const size_t item_size) : pos(p), size(item_size)
      {
      };

      void Move(size_t n)
      {
        pos += (n * size);
      }

      iterator& operator++()
      {
        Move(1);
        return *this;
      }

      iterator operator++(int)
      {
        const iterator tmp(*this);
        operator++();
        return tmp;
      }

      iterator& operator--()
      {
        Move(-1);
        return *this;
      }

      iterator operator--(int)
      {
        const iterator tmp(*this);
        operator--();
        return tmp;
      }

      iterator& operator+=(const difference_type n)
      {
        Move(n);
        return *this;
      }

      iterator& operator-=(const difference_type n)
      {
        Move(-n);
        return *this;
      }

      iterator operator+(const difference_type& n) const
      {
        iterator tmp(*this);
        tmp.Move(n);
        return tmp;
      }

      iterator operator-(const difference_type& n) const
      {
        iterator tmp(*this);
        tmp.Move(-n);
        return tmp;
      }

      std::byte& operator[](const difference_type& n) const
      {
        iterator tmp(*this);
        tmp.Move(n);
        return *tmp;
      }

      difference_type operator-(const iterator& rhs) const { return pos - rhs.pos; }

      bool operator==(const iterator& rhs) const { return pos == rhs.pos; }
      bool operator!=(const iterator& rhs) const { return pos != rhs.pos; }

      bool operator<(const iterator& rhs) const { return pos < rhs.pos; }
      bool operator<=(const iterator& rhs) const { return pos <= rhs.pos; }
      bool operator>(const iterator& rhs) const { return pos > rhs.pos; }
      bool operator>=(const iterator& rhs) const { return pos >= rhs.pos; }

      std::byte& operator*() const { return *pos; }

      template<class T>
      T &as() { return *(T *) pos; }
      template<class T>
      T *as_ptr() { return (T *) pos; }

      [[nodiscard]] void* ptr() const { return pos; }
    };

    [[nodiscard]] iterator begin() const
    {
      return {p_start, item_size};
    }

    iterator end() const
    {
      return {p_end, item_size};
    }

    iterator last() const
    {
      return end()--;
    }

    iterator operator[](size_t n) const {
      return iterator(p_start + n * item_size, item_size);
    }

    size_t element_size() const { return item_size; }
  };

  inline span_dynamic::iterator operator+(span_dynamic::iterator::difference_type n, span_dynamic::iterator i)
  {
    span_dynamic::iterator tmp(i);
    tmp.Move(n);
    return tmp;
  }

  inline span_dynamic::iterator operator-(span_dynamic::iterator::difference_type n, span_dynamic::iterator i)
  {
    span_dynamic::iterator tmp(i);
    tmp.Move(-n);
    return tmp;
  }

  inline bool operator<(const span_dynamic::iterator& lhr, const void* rhs) { return lhr.ptr() < rhs; }
  inline bool operator<(const void* lhr, const span_dynamic::iterator& rhs) { return lhr < rhs.ptr(); }
  inline bool operator<=(const span_dynamic::iterator& lhr, const void* rhs) { return lhr.ptr() <= rhs; }
  inline bool operator>(const span_dynamic::iterator& lhr, const void* rhs) { return lhr.ptr() > rhs; }
  inline bool operator>(const void* lhr, const span_dynamic::iterator& rhs) { return lhr > rhs.ptr(); }
  inline bool operator>=(const span_dynamic::iterator& lhr, const void* rhs) { return lhr.ptr() >= rhs; }
  inline bool operator>=(const void* lhr, const span_dynamic::iterator& rhs) { return lhr >= rhs.ptr(); }
}

static_assert(std::input_or_output_iterator<SH::span_dynamic::iterator>);
static_assert(std::random_access_iterator<SH::span_dynamic::iterator>);

static_assert(std::ranges::random_access_range<SH::span_dynamic>);
