#include "BlockBase.h"

#include "World.h"
#include "stdu/ref.h"

namespace Game {
    stdu::ref<BlockBase> BlockBase::BlockInDirection(Maths::Direction3D dir) {
        return ParentWorld->BlockAt(Position + dir);
    }
    
    stdu::cref<BlockBase> BlockBase::BlockInDirection(Maths::Direction3D dir) const {
        return ParentWorld->BlockAt(Position + dir);
    }
}
