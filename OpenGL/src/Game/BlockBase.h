#pragma once
#include <memory>

#include "BlockRenderer.h"
#include "World.h"
#include "Serialization/BlockSerialization.h"
#include "Serialization/TextureDispatch.h"
#include "stdu/ref.h"

namespace Game {
    enum class BlockType {
        ERR = -1,
        NIL = 0,
        WALL,
        PLAYER,
        BOX,
        GOAL,
        BOUNDMIN = NIL + 1,
        BOUNDMAX = GOAL
    };
    IMPL_ENUM_OPERATORS(BlockType)
    
    namespace Blocks {
        template <BlockType ID> class Block {};
    }
    
    // great naming here
    class BlockBase {
        friend World;
        friend BlockRenderer;
        
        private:
            std::unique_ptr<BlockRenderer> Renderer { };
            
            Maths::Vec3Int Position;
            stdu::ref<World> ParentWorld = nullptr;
        protected:
            inline static const Serialization::TextureDispatcher DefaultTextureDispatch =
                Serialization::TextureDispatcher::Load("res/textures/block_texture_atl.json");
        
            BlockBase(const Maths::Vec3Int& position, const stdu::ref<World> world)
                : Renderer(new BlockRenderer(*this)), Position(position), ParentWorld(world) {}
            BlockBase(const Serialization::BlockStructure& bs, const stdu::ref<World> world)
                : Renderer(new BlockRenderer(*this)), ParentWorld(world) { BlockBase::Build(bs); }
        public:
            BlockBase() : Renderer(new BlockRenderer(*this)) {}
            BlockBase(const BlockBase& copy)
                : Renderer(new BlockRenderer(*copy.Renderer)),
                  Position(copy.Position),
                  ParentWorld(copy.ParentWorld) {}
            BlockBase(BlockBase&& copy) noexcept
                : Renderer(std::move(copy.Renderer)),
                  Position(copy.Position),
                  ParentWorld(copy.ParentWorld) {}
            BlockBase& operator=(const BlockBase& copy) {
                if (this == &copy)
                    return *this;
                Renderer = std::unique_ptr<BlockRenderer> { new BlockRenderer(*copy.Renderer) };
                Position = copy.Position;
                ParentWorld = copy.ParentWorld;
                return *this;
            }
            BlockBase& operator=(BlockBase&& copy) noexcept {
                if (this == &copy)
                    return *this;
                Renderer = std::move(copy.Renderer);
                Position = copy.Position;
                ParentWorld = copy.ParentWorld;
                return *this;
            }
            
            virtual ~BlockBase() = default;

            Maths::Vec3Int& GetPosition() { return Position; }
            [[nodiscard]] const Maths::Vec3Int& GetPosition() const { return Position; }
            BlockRenderer& GetRenderer() { return *Renderer; }
            [[nodiscard]] const BlockRenderer& GetRenderer() const { return *Renderer; }
            World& GetWorld() { return *ParentWorld; }
            [[nodiscard]] const World& GetWorld() const { return *ParentWorld; }

            stdu::ref<BlockBase>  BlockInDirection(Maths::Direction3D dir);
            stdu::cref<BlockBase> BlockInDirection(Maths::Direction3D dir) const;

            void Load(const std::string& levelname) { Build(Serialization::BlockStructure::Load(levelname)); }
            virtual void Build(const Serialization::BlockStructure& structure) {
                Position = structure.position;
                Renderer->UseTexture({ .all = structure.type });
            }

            virtual BlockType ID() const { return BlockType::NIL; }
            
            // * this is probably the most janky code ive ever written,
            template <BlockType CreateID>
            static std::unique_ptr<BlockBase> Create(stdu::ref<World> world, const Serialization::BlockStructure& structure)
            requires(BlockType::BOUNDMIN <= CreateID && CreateID <= BlockType::BOUNDMAX);
            // * runtime version of above
            template <BlockType CompareID = BlockType::BOUNDMIN>
            static std::unique_ptr<BlockBase> Create(stdu::ref<World> world, BlockType ID, const Serialization::BlockStructure& structure)
            requires(BlockType::BOUNDMIN <= CompareID && CompareID <= BlockType::BOUNDMAX);
    };
}

#include "BlockCreation.h"
