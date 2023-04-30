#pragma once

namespace Game::Blocks {
    using WallBlock = Block<BlockType::WALL>;
    
    template <> class Block<BlockType::WALL> final : public BlockBase {
        friend BlockBase;

        protected:
            Block(const Maths::Vec3Int& position, stdu::ref<World> world) : BlockBase(position, world) {}
        public:
            Block() : BlockBase() {}
            Block(const BlockBase& copy) : BlockBase(copy) {}
            Block(BlockBase&& copy) : BlockBase(copy) {}

            ~Block() override = default;

            inline static constexpr BlockType TypeID = BlockType::WALL;
            [[nodiscard]] BlockType ID() const override { return TypeID; }
    };
}
