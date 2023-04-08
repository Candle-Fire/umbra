#pragma once

#include <string>
#include <cstring>

namespace ShadowEngine::Entities {

    //TODO: this could be converted into a generic container

    class INodeContainer {
      public:

        virtual void *CreateObject() = 0;

        virtual void DestroyObject(void *object) = 0;
    };

    /**
     * Node container is a memory manager for a single type of Node
     * This creates <see cref="NodeContainer::MemoryChunk"/>s that contain a block of memory for
     * ``MAX_OBJECTS_IN_CHUNK`` number of nodes, when it gets full it creates a new MemoryChunk.
     * This container is created by the <see cref="NodeManager"/> for each entity type that gets registered.
     * @tparam Type
     */
    template<class Type>
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
            bool metadata[MAX_OBJECTS_IN_CHUNK];

            //Points to the next free element in the pool
            Element *nextFree;

            MemoryChunk() : count(0) {
                chunk_start = (Element *) malloc(ALLOC_SIZE);

                // Might not be needed, probably for nicer debugging....
                std::memset(chunk_start, -1, ALLOC_SIZE);

                chunk_end = &chunk_start[MAX_OBJECTS_IN_CHUNK];

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
            Type *Allocate() {
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
        };

        using MemoryChunks = std::vector<MemoryChunk *>;

        class Iterator {
            typename MemoryChunks::iterator m_current_chunk;
            typename MemoryChunks::iterator m_end_chunk;

            Element *m_current_element;
            int m_current_element_index;
          public:
            Iterator(typename MemoryChunks::iterator begin, typename MemoryChunks::iterator end) :
                m_current_chunk(begin),
                m_end_chunk(end),
                m_current_element_index(0) {

                if (begin != end) {
                    assert((*m_current_chunk) != nullptr);
                    m_current_element = (*m_current_chunk)->chunk_start - 1;
                    this->Next();
                } else {
                    m_current_element = nullptr;
                }
            }

            // Prefix increment
            Iterator &operator++() {
                //step to next element in chunk
                Next();

                return *this;
            }

            void Next() {
                do {
                    m_current_element_index++;
                    m_current_element = (*m_current_chunk)->chunk_start + m_current_element_index;

                    //if we are at the end of the chunk, move to the next chunk
                    if (m_current_element == (*m_current_chunk)->chunk_end) {
                        m_current_chunk++;
                        if (m_current_chunk == m_end_chunk)
                            break;

                        m_current_element_index = 0;
                        m_current_element = (*m_current_chunk)->chunk_start + m_current_element_index;
                    }

                } while (
                    m_current_chunk != m_end_chunk &&
                        (*m_current_chunk)->metadata[m_current_element_index] != MemoryChunk::FreeFlag);
            }

            // Postfix increment
            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            inline Type &operator*() const { return (m_current_element->element); }

            inline Type *operator->() const { return &(m_current_element->element); }

            inline bool operator==(Iterator &other) {
                //auto o = dynamic_cast<iterator&>(other);
                return ((this->m_current_chunk == other.m_current_chunk)
                    && (this->m_current_element == other.m_current_element));
            }

            inline bool operator!=(Iterator &other) {
                //auto o = dynamic_cast<iterator&>(other);
                return ((this->m_current_chunk != other.m_current_chunk)
                    && (this->m_current_element != other.m_current_element));
            }

        };

        MemoryChunks m_chunks;

      public:

        NodeContainer() {
            m_chunks.clear();
        }

        void *CreateObject() {
            void *slot = nullptr;

            // get next free slot
            for (auto chunk : this->m_chunks) {
                if (chunk->count > MAX_OBJECTS_IN_CHUNK)
                    continue;

                slot = chunk->Allocate();
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
                //Allocator* allocator = new Allocator(ALLOC_SIZE, Allocate(ALLOC_SIZE, this->m_AllocatorTag), sizeof(OBJECT_TYPE), alignof(OBJECT_TYPE));
                MemoryChunk *newChunk = new MemoryChunk();

                // put new chunk in front
                this->m_chunks.push_back(newChunk);

                slot = newChunk->Allocate();

                assert(slot != nullptr && "Unable to create new object. Out of memory?!");
                //newChunk->objects.clear();
                //newChunk->objects.push_back((OBJECT_TYPE*)slot);
            }

            return slot;
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

        inline Iterator begin() {
            return Iterator(this->m_chunks.begin(),
                            this->m_chunks.end());
        }

        inline Iterator end() {
            return Iterator(this->m_chunks.end(),
                            this->m_chunks.end());
        }

    };

}