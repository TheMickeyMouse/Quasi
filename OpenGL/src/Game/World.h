#pragma once

#include "GraphicsDevice.h"
#include "Serialization/WorldSerialization.h"
#include "stdu/sorted_vector.h"
#include "stdu/ref.h"

namespace Game {
    class BlockBase;
    class BlockPtr;
    
    class World {
        friend BlockBase;
        
        private:
            Maths::Vec3Int boundsMin, boundsMax;
        
            static int DefaultBlockComparison(const Maths::Vec3Int& vec);
            static int DefaultBlockComparison(const BlockPtr& x);
        
            stdu::sorted_vector<BlockPtr, int(*)(const BlockPtr&)> blocks = { DefaultBlockComparison };

            stdu::ref<BlockBase>  BlockAt(const Maths::Vec3Int& position, int startIndex = 0);
            stdu::cref<BlockBase> BlockAt(const Maths::Vec3Int& position, int startIndex = 0) const;
        public:
            World();
            World(const Serialization::WorldStructure& ws);
            ~World();

            void BlockRenderUpdate();
            void Render(Graphics::GraphicsDevice& gd);

            void Load(const std::string& levelname);
            void Build(const Serialization::WorldStructure& structure);
    };
}
    
