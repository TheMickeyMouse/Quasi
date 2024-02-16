#pragma once
#include <string>
#include <fstream>

namespace stdu {
    inline std::string readfile(const std::string& fname) {
        if (std::ifstream in { fname, std::ios::in | std::ios::binary }) {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize((uint)in.tellg());
            // ill be honest idk what the compiler wants from me
            in.seekg(0, std::ios::beg);
            in.read(contents.data(), contents.size());
            in.close();
            return contents;
        }
        return "";
    }

    inline std::tuple<std::string_view, std::string_view> getfolder(const std::string& fname) {
        using namespace std::literals;
        const size_t pos = fname.find_last_of("\\/");
        return std::string::npos == pos ?
            std::tuple { ""sv, fname } :
            std::tuple { std::string_view { fname }.substr(0, pos),
                         std::string_view { fname }.substr(pos + 1) };
    }
}