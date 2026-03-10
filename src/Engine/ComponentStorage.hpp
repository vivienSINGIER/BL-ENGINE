#ifndef COMPONENT_STORAGE_HPP_DEFINED
#define COMPONENT_STORAGE_HPP_DEFINED

#include "Engine.h"
#include "ComponentRegistry.hpp"

struct ComponentStorage
{
    UnorderedMap<ComponentId, Vector<Byte>> columns;
    UnorderedMap<ComponentId, uint64>       strides;
    uint64 count = 0;

    void RegisterColumn(ComponentId _column, uint64 _stride)
    {
        columns[_column] = {};
        strides[_column] = _stride;
    }

    template <typename T>
    T& Get(ComponentId _cId, uint64 _row)
    {
        return reinterpret_cast<T*>(columns[_cId].data())[_row];
    }

    Byte* GetRaw(ComponentId _cId, uint64 _row)
    {
        return columns[_cId].data() + _row * strides[_cId];
    }

    template <typename T>
    void Push(ComponentId _cId, T const& _val)
    {
        Vector<Byte>& col = columns[_cId];
        uint64 offset = sizeof(col);
        col.resize(offset + sizeof(T));

        std::memcpy(col.data() + offset, _val, sizeof(T));
    }

    void FinishPush()
    {
        count++;
    }

    void SwapRemove(uint64 _row)
    {
        for (auto& [cid, col] : columns)
        {
            uint64 stride = strides[cid];
            uint64 last = (count - 1) * stride;
            if (_row * stride != last)
            {
                std::memcpy(col.data() + _row * stride, col.data() + last, stride);   
            }
            col.resize(col.size() - stride);
        }
        count--;
    }
};

#endif