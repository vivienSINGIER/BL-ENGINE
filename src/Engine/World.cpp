#include "World.h"

void World::MoveEntity(EntityId _entity, EntityRecord& _rec, Archetype* _src, Archetype* _dst)
{
    uint32 srcRow = _rec.row;

    for (auto& [cid, col] : _src->storage.columns)
    {
        if (_dst->mask.test(cid))
        {
            uint64 stride = _src->storage.strides[cid];
            Byte* raw = _src->storage.GetRaw(cid, srcRow);

            Vector<Byte>& dstCol = _dst->storage.columns[cid];
            dstCol.insert(dstCol.end(), raw, raw + stride);
        }
    }

    uint32 dstRow =
}
