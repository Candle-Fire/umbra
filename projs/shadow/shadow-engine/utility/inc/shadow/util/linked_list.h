#pragma once
#include <concepts>

#include "spdlog/fmt/bundled/compile.h"

template<typename T>
concept LinkedListNode = requires(T val)
{
    std::same_as<decltype(T::next), T*>;
};

template<LinkedListNode T>
class linked_list
{
    T *head;
    T *tail;

public:
    explicit linked_list(T* first)
    {
        head = first;
        auto current = first;
        while (current->next != nullptr)
        {
            current = current->next;
        }
        tail = current;
    }

    void concat(const linked_list& other)
    {
        other.head->next = this->head;
        this->head = other.head;
    }

    class iterator
    {
        T* current;
        public:
        explicit iterator(T* current) : current(current)
        {}

        iterator& operator++()
        {
            current = current->next;
            return *this;
        }

        iterator operator++(int)
        {
            current = current->next;
            return *this;
        }

        T* operator *(){ return current; }
    };

    iterator begin()
    {
        return iterator(head);
    }
    iterator end()
    {
        return iterator(tail);
    }
};
class dummy{ public: dummy *next; };
static_assert(std::ranges::forward_range<linked_list<dummy>>, "forward range");
static_assert(std::forward_iterator<linked_list<dummy>::iterator>, "forward range");
