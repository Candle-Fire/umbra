#pragma once

namespace ShadowEngine::Entities {

    //TODO: this could be converted into a generic container

	class IEntityContainer {
	public:

		virtual void* CreateObject() = 0;


		virtual void DestroyObject(void* object) = 0;
	};

    /**
     * Entity container is a memory manager for a single type of Entity
     * This creates <see cref="EntityContainer::MemoryChunk"/>s that contain a block of memory for ``MAX_OBJECTS_IN_CHUNK`` number of entities, when it gets full it creates a new MemoryChunk.
     * This container is created by the <see cref="EntityManager"/> for each entity type that gets registered.
     * @tparam Type
     */
	template<class Type>
	class EntityContainer : public IEntityContainer {

		/**
		 * This represents a single element of the memory chunks
		 * and is used for accessing the given element as either a pointer to the next free slot or as the Entity
		 */
		union Element
		{
		public:
			Element* next;
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

		class MemoryChunk
		{
		public:
			Element* chunkStart;
			Element* chunkEnd;

			int count;
			static const bool FreeFlag = true;   //TODO: WTF?
			bool metadata[MAX_OBJECTS_IN_CHUNK];

			//Points to the next free element in the pool
			Element* nextFree;

			MemoryChunk() :count(0)
			{
				chunkStart = (Element*)malloc(ALLOC_SIZE);

                // Might not be needed, probably for nicer debugging....
				memset(chunkStart, -1, ALLOC_SIZE);

				chunkEnd = &chunkStart[MAX_OBJECTS_IN_CHUNK];

                //Sets up the free linked list
				for (size_t i = 1; i < MAX_OBJECTS_IN_CHUNK; i++) {
					chunkStart[i - 1].next = &chunkStart[i];
					metadata[i] = FreeFlag;
				}
				chunkStart[MAX_OBJECTS_IN_CHUNK - 1].next = nullptr;
				nextFree = chunkStart;
			}

            /**
             * Allocates a new instance of the stored type.
             * The allocation is just a large enough memory area,
             * calling the constructor on that are if not done.
             * @return pointer to the new allocation, or nullptr if no free space available
             */
			Type* allocate()
			{
				if (nextFree == nullptr)
					return nullptr;
				count++;
				auto res = nextFree;
				nextFree = nextFree->next;

				int i = ((Element*)res - (Element*)chunkStart);
				metadata[i] = !FreeFlag;

				return (Type*)res;
			}

            /**
             * Frees a place that was previously allocated by this
             * @param ptr The pointer to the start of the allocation.
             */
			void free(void* ptr)
			{
                //TODO: In debug we should check if ptr is actually inside our allocation.

				count--;
				auto element = ((Element*)ptr);
				element->next = nextFree;
				nextFree = element;

				int i = ((Element*)ptr - (Element*)chunkStart);
				metadata[i] = FreeFlag;
			}
		};

        using MemoryChunks = std::vector<MemoryChunk*>;

		class EntityContainerIterator
		{
			typename MemoryChunks::iterator m_CurrentChunk;
			typename MemoryChunks::iterator m_EndChunk;

			Element* m_CurrentElement;
			int index;

		public:

            EntityContainerIterator(typename MemoryChunks::iterator begin, typename MemoryChunks::iterator end) :
				m_CurrentChunk(begin),
				m_EndChunk(end),
				index(0)
			{
				if (begin != end)
				{
					assert((*m_CurrentChunk) != nullptr);
					m_CurrentElement = (*m_CurrentChunk)->chunkStart;
				}
				else
				{
					m_CurrentElement = (*std::prev(m_EndChunk))->chunkEnd;
				}

			}

			inline EntityContainerIterator& operator++()
			{
                //TODO: probably a do while.... also could make the in chunk and between chunk steps a single loop

				// move to next object in current chunk
				m_CurrentElement = &m_CurrentElement[1];
				index++;

				while (index < MAX_OBJECTS_IN_CHUNK && ((*m_CurrentChunk)->metadata[index] == MemoryChunk::FreeFlag)) {

					m_CurrentElement = &m_CurrentElement[1];
					index++;
				}


				// if we reached end of list, move to next chunk
				if (m_CurrentElement == (*m_CurrentChunk)->chunkEnd)
				{
					++m_CurrentChunk;
					index = 0;
					if (m_CurrentChunk != m_EndChunk)
					{
						// set object iterator to begin of next chunk list
						assert((*m_CurrentChunk) != nullptr);
						m_CurrentElement = (*m_CurrentChunk)->chunkStart;
					}
				}

				return *this;
			}

			inline Type& operator*() const { return (m_CurrentElement->element); }
			inline Type* operator->() const { return &(m_CurrentElement->element); }

			inline bool operator==(EntityContainerIterator& other)
			{
				//auto o = dynamic_cast<iterator&>(other);
				return ((this->m_CurrentChunk == other.m_CurrentChunk) && (this->m_CurrentElement == other.m_CurrentElement));
			}

			inline bool operator!=(EntityContainerIterator& other)
			{
				//auto o = dynamic_cast<iterator&>(other);
				return ((this->m_CurrentChunk != other.m_CurrentChunk) && (this->m_CurrentElement != other.m_CurrentElement));
			}
		};

		MemoryChunks m_Chunks;

	public:

		EntityContainer()
		{
			m_Chunks.clear();
		}

		void* CreateObject()
		{
			void* slot = nullptr;

			// get next free slot
			for (auto chunk : this->m_Chunks)
			{
				if (chunk->count > MAX_OBJECTS_IN_CHUNK)
					continue;

				slot = chunk->allocate();
				if (slot != nullptr)
				{
					//chunk->objects.push_back((OBJECT_TYPE*)slot);
					break;
				}
                //TODO: if we got here that is impossible...
                // If ``chunk->count > MAX_OBJECTS_IN_CHUNK`` was right but we still got nullptr
                // than we got a misalignment
			}

			// all chunks are full... allocate a new one
			if (slot == nullptr)
			{
				//Allocator* allocator = new Allocator(ALLOC_SIZE, Allocate(ALLOC_SIZE, this->m_AllocatorTag), sizeof(OBJECT_TYPE), alignof(OBJECT_TYPE));
				MemoryChunk* newChunk = new MemoryChunk();

				// put new chunk in front
				this->m_Chunks.push_back(newChunk);

				slot = newChunk->allocate();

				assert(slot != nullptr && "Unable to create new object. Out of memory?!");
				//newChunk->objects.clear();
				//newChunk->objects.push_back((OBJECT_TYPE*)slot);
			}

			return slot;
		}

		void DestroyObject(void* object)
		{
			intptr_t adr = reinterpret_cast<intptr_t>(object);

			for (auto chunk : this->m_Chunks)
			{
				if (((intptr_t)chunk->chunkStart) <= adr && adr < (intptr_t)chunk->chunkEnd)
				{
					// note: no need to call d'tor since it was called already by 'delete'

					//chunk->objects.remove((OBJECT_TYPE*)object);
					chunk->free(object);
					return;
				}
			}

			assert(false && "Failed to delete object. Memory corruption?!");
		}


		inline EntityContainerIterator begin() {
			auto end = this->m_Chunks.end();
			return iterator(this->m_Chunks.begin(), end);
		}

		inline EntityContainerIterator end() { return iterator(this->m_Chunks.end(), this->m_Chunks.end()); }

	};


}