#pragma once
#include <fstream>
#include <unordered_map>

#include "BlockTextureDispatch.h"
#include "Vector.h"

namespace Game::Serialization {
    struct TextureDispatcher {
        Maths::Vec2Int textureAtlasSize = {};
        std::unordered_map<BlockType, BlockTextureDispatcher> blockTextures;

        static TextureDispatcher Load(const std::string& filename);
    };
}

template <>
struct nlohmann::adl_serializer<Game::Serialization::TextureDispatcher> {
    using TextureDisp = Game::Serialization::TextureDispatcher;
    using BlockDisp = Game::Serialization::BlockTextureDispatcher;
    static void from_json(const json& js, TextureDisp& tp) {
        js.at("texSize").get_to(tp.textureAtlasSize);
        for (const auto& btex : js.at("textures")) {
            BlockDisp bp;
            btex.get_to<BlockDisp>(bp);
            tp.blockTextures.emplace(bp.type, bp);
        }
    }

    static void to_json(json& js, const TextureDisp& tp) {
        js = json {
                { "texSize",  tp.textureAtlasSize },
                { "textures", {} }
        };
        // funky code
        for (const auto& [ _, bp ] : tp.blockTextures) {
            js.at("textures") += { bp };
        }
    }
};

namespace Game::Serialization {
    inline TextureDispatcher TextureDispatcher::Load(const std::string& filename) {
        using nlohmann::json;
        std::ifstream in{ filename };
        json js = json::parse(in);
        return js.get<TextureDispatcher>();
    }
}
