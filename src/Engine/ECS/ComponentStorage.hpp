#ifndef COMPONENT_STORAGE_HPP_DEFINED
#define COMPONENT_STORAGE_HPP_DEFINED

#include "../define.h"
#include "ComponentRegistry.hpp"

struct ComponentStorage
{
    UnorderedMap<ComponentId, Vector<Byte>> columns;
    UnorderedMap<ComponentId, Vector<bool>> activeStates;
    UnorderedMap<ComponentId, uint64>       strides;
    uint64 count = 0;

    void RegisterColumn(ComponentId _column, uint64 _stride)
    {
        columns[_column] = {};
        activeStates[_column] = {};
        strides[_column] = _stride;
    }

    bool GetActive(ComponentId _column, uint64 _row)
    {
        return activeStates[_column][_row];
    }

    void SetActive(ComponentId _column, uint64 _row, bool _value)
    {
        activeStates[_column][_row] = _value;
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
        auto it = columns.find(_cId);
        if (it == columns.end())
            RegisterColumn(_cId, sizeof(T));
        
        Vector<Byte>& col = columns[_cId];
        uint64 offset = col.size();
        col.resize(offset + sizeof(T));
        std::memcpy(col.data() + offset, &_val, sizeof(T));
        
        Vector<bool>& active = activeStates[_cId];
        active.resize(active.size() + sizeof(bool));
        active[active.size() - 1] = true;
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
        for (auto& [cid, col] : activeStates)
        {
            uint64 last = (count - 1);
            if (_row != last)
            {
                col[_row] = col[last];
            }
            col.resize(col.size() - 1);
        }
        
        count--;
    }
};

#endif