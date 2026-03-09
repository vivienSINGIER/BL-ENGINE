#ifndef COMPONENT_STORAGE_HPP_DEFINED
#define COMPONENT_STORAGE_HPP_DEFINED

#include "Engine.h"
#include "ComponentRegistry.hpp"

struct ComponentStorage
{
    UnorderedMap<ComponentId, Vector<Byte>> m_columns;
    UnorderedMap<ComponentId, uint64>       m_strides;
    uint64 m_count = 0;

    void RegisterColumn(ComponentId _column, uint64 _stride)
    {
        m_columns[_column] = {};
        m_strides[_column] = _stride;
    }

    template <typename T>
    T& Get(ComponentId _cId, uint64 _row)
    {
        return reinterpret_cast<T*>(m_columns[_cId].data())[_row];
    }

    Byte* GetRaw(ComponentId _cId, uint64 _row)
    {
        return m_columns[_cId].data() + _row * m_strides[_cId];
    }

    template <typename T>
    void Push(ComponentId _cId, T const& _val)
    {
        Vector<Byte>& col = m_columns[_cId];
        uint64 offset = sizeof(col);
        col.resize(offset + sizeof(T));

        std::memcpy(col.data() + offset, _val, sizeof(T));
    }

    void FinishPush()
    {
        m_count++;
    }

    void SwapRemove(uint64 _row)
    {
        for (auto& [cid, col] : m_columns)
        {
            uint64 stride = m_strides[cid];
            uint64 last = (m_count - 1) * stride;
            if (_row * stride != last)
            {
                std::memcpy(col.data() + _row * stride, col.data() + last, stride);   
            }
            col.resize(col.size() - 1);
        }
        m_count--;
    }
};

#endif