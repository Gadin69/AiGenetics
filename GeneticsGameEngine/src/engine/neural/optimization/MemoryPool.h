#pragma once

#include <vector>
#include <cstdint>
#include <memory>

namespace Engine {
namespace Neural {

// Memory pool for efficient neuron and synapse allocation
// Prevents fragmentation and improves cache locality
template<typename T>
class MemoryPool {
public:
    MemoryPool(size_t initialCapacity = 1024);
    ~MemoryPool();
    
    // Allocate an object from the pool
    T* Allocate();
    
    // Return an object to the pool (doesn't actually free memory)
    void Deallocate(T* object);
    
    // Clear all allocated objects
    void Clear();
    
    // Get statistics
    size_t GetCapacity() const { return m_capacity; }
    size_t GetUsedCount() const { return m_usedCount; }
    size_t GetFreeCount() const { return m_capacity - m_usedCount; }
    
    // Reserve more capacity
    void Reserve(size_t additionalCapacity);

private:
    struct Block {
        T* data;
        size_t size;
        
        Block(size_t blockSize)
            : data(new T[blockSize])
            , size(blockSize)
        {}
        
        ~Block() {
            delete[] data;
        }
    };
    
    std::vector<std::unique_ptr<Block>> m_blocks;
    size_t m_capacity;
    size_t m_usedCount;
    size_t m_currentBlockIndex;
    size_t m_currentOffset;
    
    static constexpr size_t DefaultBlockSize = 1024;
    
    void AllocateNewBlock();
};

// Template implementations

template<typename T>
MemoryPool<T>::MemoryPool(size_t initialCapacity)
    : m_capacity(0)
    , m_usedCount(0)
    , m_currentBlockIndex(0)
    , m_currentOffset(0)
{
    if (initialCapacity > 0) {
        size_t numBlocks = (initialCapacity + DefaultBlockSize - 1) / DefaultBlockSize;
        for (size_t i = 0; i < numBlocks; ++i) {
            m_blocks.push_back(std::make_unique<Block>(DefaultBlockSize));
            m_capacity += DefaultBlockSize;
        }
    }
}

template<typename T>
MemoryPool<T>::~MemoryPool()
{
    Clear();
}

template<typename T>
T* MemoryPool<T>::Allocate()
{
    if (m_usedCount >= m_capacity) {
        AllocateNewBlock();
    }
    
    if (m_blocks.empty()) {
        return nullptr;
    }
    
    T* object = &m_blocks[m_currentBlockIndex]->data[m_currentOffset];
    m_usedCount++;
    m_currentOffset++;
    
    if (m_currentOffset >= m_blocks[m_currentBlockIndex]->size) {
        m_currentBlockIndex++;
        m_currentOffset = 0;
    }
    
    return object;
}

template<typename T>
void MemoryPool<T>::Deallocate(T* object)
{
    // Pool doesn't actually free individual objects
    // Just decrement usage count
    if (m_usedCount > 0) {
        m_usedCount--;
    }
}

template<typename T>
void MemoryPool<T>::Clear()
{
    m_blocks.clear();
    m_capacity = 0;
    m_usedCount = 0;
    m_currentBlockIndex = 0;
    m_currentOffset = 0;
}

template<typename T>
void MemoryPool<T>::Reserve(size_t additionalCapacity)
{
    size_t numBlocks = (additionalCapacity + DefaultBlockSize - 1) / DefaultBlockSize;
    for (size_t i = 0; i < numBlocks; ++i) {
        m_blocks.push_back(std::make_unique<Block>(DefaultBlockSize));
        m_capacity += DefaultBlockSize;
    }
}

template<typename T>
void MemoryPool<T>::AllocateNewBlock()
{
    m_blocks.push_back(std::make_unique<Block>(DefaultBlockSize));
    m_capacity += DefaultBlockSize;
    m_currentBlockIndex = m_blocks.size() - 1;
    m_currentOffset = 0;
}

} // namespace Neural
} // namespace Engine
