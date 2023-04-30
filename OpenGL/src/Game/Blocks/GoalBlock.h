#pragma once

namespace Game::Blocks {
    using GoalBlock = Block<BlockType::GOAL>;
    
    template <> class Block<BlockType::GOAL> final : public BlockBase {
        friend BlockBase;
        
        protected:
            Block(const Maths::Vec3Int& position, stdu::ref<World> world) : BlockBase(position, world) {}
        public:
            Block() : BlockBase() {}
            Block(const BlockBase& copy) : BlockBase(copy) {}
            Block(BlockBase&& copy) : BlockBase(copy) {}

            ~Block() override = default;

            inline static constexpr BlockType TypeID = BlockType::GOAL;
            [[nodiscard]] BlockType ID() const override { return TypeID; }
    };
}
