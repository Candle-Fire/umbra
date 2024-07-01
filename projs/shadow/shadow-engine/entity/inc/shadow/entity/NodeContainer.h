#pragma once

#include <string>
#include <cstring>
#include <span>
#include <vector>
#include <cstdint>
#include <assert.h>

#include "shadow/SHObject.h"

namespace SH::Entities {

    //TODO: this could be converted into a generic container

    class INodeContainer {
      public:

        virtual void *allocate() = 0;

        virtual void DestroyObject(void *object) = 0;

        virtual std::string getTypeName() = 0;

        virtual int getCount() = 0;
    };

    /**
     * Node container is a memory manager for a single type of Node
     * This creates <see cref="NodeContainer::MemoryChunk"/>s that contain a block of memory for
     * ``MAX_OBJECTS_IN_CHUNK`` number of nodes, when it gets full it creates a new MemoryChunk.
     * This container is created by the <see cref="NodeManager"/> for each entity type that gets registered.
     * @tparam Type
     */
    template<IsSHObject Type>
    class NodeContainer : public INodeContainer {

        /**
         * This represents a single element of the memory chunks
         * and is used for accessing the given element as either a pointer to the next free slot or as the Entity
         */
        union Element {
          public:
            Element *next;
            Type element;
        };

        //TODO: cosntexp
        /**
         * The maximum number of entities in a MemoryChunk
         * This is basically the size of the memory array that gets allocated.
         */
        static const size_t MAX_OBJECTS_IN_CHUNK = 2048;

        //TODO: cosntexp
        /**
         * The size of a single Entity
         */
        static const size_t ELEMENT_SIZE = (sizeof(Element));

        //TODO: cosntexp
        /**
         * The size of the Memory Chunks in bytes
         */
        static const size_t ALLOC_SIZE = ELEMENT_SIZE * MAX_OBJECTS_IN_CHUNK;

      public:

        class MemoryChunk {
          public:
            Element *chunk_start;
            Element *chunk_end;

            int count;
            static const bool FreeFlag = true;   //TODO: WTF?
            static const bool InUseFlag = false;   //TODO: WTF?
            bool metadata[MAX_OBJECTS_IN_CHUNK];

            //Points to the next free element in the pool
            Element *nextFree;

            MemoryChunk() : count(0) {
                chunk_start = (Element *) malloc(ALLOC_SIZE);

                // Might not be needed, probably for nicer debugging....
                std::memset(chunk_start, -1, ALLOC_SIZE);

                chunk_end = &chunk_start[MAX_OBJECTS_IN_CHUNK];

                metadata[0] = FreeFlag;

                //Sets up the free linked list
                for (size_t i = 1; i < MAX_OBJECTS_IN_CHUNK; i++) {
                    chunk_start[i - 1].next = &chunk_start[i];
                    metadata[i] = FreeFlag;
                }
                chunk_start[MAX_OBJECTS_IN_CHUNK - 1].next = nullptr;
                nextFree = chunk_start;
            }

            /**
             * Allocates a new instance of the stored type.
             * The allocation is just a large enough memory area,
             * calling the constructor on that are if not done.
             * @return pointer to the new allocation, or nullptr if no free space available
             */
            Type *allocate() {
                if (nextFree == nullptr)
                    return nullptr;
                count++;
                auto res = nextFree;
                nextFree = nextFree->next;

                int i = ((Element *) res - (Element *) chunk_start);
                metadata[i] = !FreeFlag;

                return (Type *) res;
            }

            /**
             * Frees a place that was previously allocated by this
             * @param ptr The pointer to the start of the allocation.
             */
            void free(void *ptr) {
                //TODO: In debug we should check if ptr is actually inside our allocation.

                count--;
                auto element = ((Element *) ptr);
                element->next = nextFree;
                nextFree = element;

                int i = ((Element *) ptr - (Element *) chunk_start);
                metadata[i] = FreeFlag;
            }

            class Iterator {
                MemoryChunk *chunk;
                int index;
              public:
                Iterator() : chunk(nullptr), index(0) {}

                Iterator(MemoryChunk *chunk, int pos) : chunk(chunk), index(pos) {
                    while (chunk->metadata[index] != InUseFlag && index < MAX_OBJECTS_IN_CHUNK) {
                        index++;
                    }
                }

                // Prefix increment
                Iterator &operator++() {
                    //step to next element in chunk
                    Next();
                    return *this;
                }

                Iterator operator++(int) {
                    Iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                void Next() {
                    do {
                        index++;
                    } while (chunk->metadata[index] != InUseFlag && index < MAX_OBJECTS_IN_CHUNK);
                }

                inline Type &operator*() const { return (chunk->chunk_start[index].element); }

                inline Type *operator->() const { return &(chunk->chunk_start[index].element); }

                inline bool operator==(const Iterator &other) const {
                    return ((this->chunk == other.chunk)
                        && (this->index == other.index));
                }

                inline bool operator!=(const Iterator &other) const {
                    return ((this->chunk != other.chunk)
                        || (this->index != other.index));
                }

                int GetIndex() const { return index; }

                MemoryChunk *GetChunk() const { return chunk; }
            };

            inline Iterator begin() {
                return Iterator(this, 0);
            }

            inline Iterator end() {
                return Iterator(this, MAX_OBJECTS_IN_CHUNK);
            }

        };

        using MemoryChunks = std::vector<MemoryChunk *>;

        class Iterator {
            NodeContainer<Type> *container;
            int chunk_index;

            typename MemoryChunk::Iterator element;
          public:
            Iterator(NodeContainer<Type> *cont, int c_index) :
                container(cont),
                chunk_index(c_index) {

                if (chunk_index < container->m_chunks.size()) {
                    element = container->m_chunks[chunk_index]->begin();
                }

                SeekNextValid();
            }

            // Prefix increment
            Iterator &operator++() {
                //step to next element in chunk
                element++;
                SeekNextValid();

                return *this;
            }

            void SeekNextValid() {
                while (!IsEndChunk() && !IsValid()) {
                    Step();
                }
            }

            [[nodiscard]] inline bool IsEndChunk() const {
                return (chunk_index >= container->m_chunks.size());
            }

            [[nodiscard]] inline bool IsValid() const {
                return !(element == container->m_chunks[chunk_index]->end());
            }

            void Step() {
                chunk_index++;
                if (IsEndChunk()) {
                    element = typename MemoryChunk::Iterator();
                } else {
                    element = container->m_chunks[chunk_index]->begin();
                }
            }

            // Postfix increment
            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            inline Type &operator*() const { return (*element); }

            inline Type *operator->() const { return &(*element); }

            inline bool operator==(const Iterator &other) const {
                return ((this->container == other.container)
                    && (this->chunk_index == other.chunk_index)
                    && (this->element == other.element));
            }

            inline bool operator!=(const Iterator &other) const {
                return ((this->container != other.container)
                    || (this->chunk_index != other.chunk_index)
                    || (this->element != other.element));
            }

            NodeContainer<Type> *GetContainer() const { return container; }

            int GetChunkIndex() const { return chunk_index; }

            typename MemoryChunk::Iterator GetElement() const { return element; }

        };

        MemoryChunks m_chunks;

      public:

        NodeContainer() {
            m_chunks.clear();
        }

        void *allocate() {
            void *slot = nullptr;

            // get next free slot
            for (auto chunk : this->m_chunks) {
                if (chunk->count > MAX_OBJECTS_IN_CHUNK)
                    continue;

                slot = chunk->allocate();
                if (slot != nullptr) {
                    //chunk->objects.push_back((OBJECT_TYPE*)slot);
                    break;
                }
                //TODO: if we got here that is impossible...
                // If ``chunk->count > MAX_OBJECTS_IN_CHUNK`` was right but we still got nullptr
                // than we got a misalignment
            }

            // all chunks are full... allocate a new one
            if (slot == nullptr) {
                //Allocator* allocator = new Allocator(ALLOC_SIZE, allocate(ALLOC_SIZE, this->m_AllocatorTag), sizeof(OBJECT_TYPE), alignof(OBJECT_TYPE));
                MemoryChunk *newChunk = new MemoryChunk();

                // put new chunk in front
                this->m_chunks.push_back(newChunk);

                slot = newChunk->allocate();

                assert(slot != nullptr && "Unable to create new object. Out of memory?!");
                //newChunk->objects.clear();
                //newChunk->objects.push_back((OBJECT_TYPE*)slot);
            }

            return slot;
        }

        Type *allocateWithType() {
            return (Type *) allocate();
        }

        void DestroyObject(void *object) {
            intptr_t adr = reinterpret_cast<intptr_t>(object);

            for (auto chunk : this->m_chunks) {
                if (((intptr_t) chunk->chunk_start) <= adr && adr < (intptr_t) chunk->chunk_end) {
                    // note: no need to call d'tor since it was called already by 'delete'

                    //chunk->objects.remove((OBJECT_TYPE*)object);
                    chunk->free(object);
                    return;
                }
            }

            assert(false && "Failed to delete object. Memory corruption?!");
        }

        std::string getTypeName() override {
            return Type::Type();
        }

        int getCount() override {
            int count = 0;

            for (auto chunk : this->m_chunks) {
                count += chunk->count;
            }

            return count;
        }

        inline Iterator begin() { return Iterator(this, 0); }

        inline Iterator end() { return Iterator(this, m_chunks.size()); }

    };

}