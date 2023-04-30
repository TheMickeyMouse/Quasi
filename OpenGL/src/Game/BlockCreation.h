#pragma once

#include "BlockBase.h"
#include "Blocks/WallBlock.h"
#include "Blocks/PlayerBlock.h"
#include "Blocks/BoxBlock.h"
#include "Blocks/GoalBlock.h"

namespace Game {
    template <BlockType CreateID> // ! not using the name ID because it clashes with BlockBase::ID(). why. thids took me 3 days to fix
    std::unique_ptr<BlockBase> BlockBase::Create(stdu::ref<World> world, const Serialization::BlockStructure& structure)
    requires(BlockType::BOUNDMIN <= CreateID && CreateID <= BlockType::BOUNDMAX) {
        auto block = std::unique_ptr<BlockBase> { (BlockBase*) new Blocks::Block<CreateID> { Maths::Vec3Int {}, world } };
        block->Build(structure);
        return block;
    }
    
    template <BlockType CompareID>
    std::unique_ptr<BlockBase> BlockBase::Create(stdu::ref<World> world, BlockType ID, const Serialization::BlockStructure& structure)
    requires(BlockType::BOUNDMIN <= CompareID && CompareID <= BlockType::BOUNDMAX) {
        if (CompareID == ID) return Create<CompareID>(world, structure);
        if constexpr (CompareID + 1 <= BlockType::BOUNDMAX) return Create<CompareID + 1>(world, ID, structure);
        else return nullptr;
    }
}
