#pragma once
#include <algorithm>
#include <string>
#include <fstream>
#include "NumTypes.h"

namespace stdu {
    inline std::string readfile(const std::string& fname) {
        if (std::ifstream in { fname, std::ios::in | std::ios::binary }) {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize((uint)in.tellg());
            // ill be honest idk what the compiler wants from me
            in.seekg(0, std::ios::beg);
            in.read(contents.data(), (long long)contents.size());
            in.close();
            return contents;
        }
        return "";
    }

    inline std::tuple<std::string_view, std::string_view> getfolder(std::string_view fname) {
        using namespace std::literals;
        const size_t pos = fname.find_last_of("\\/");
        return std::string::npos == pos ?
            std::tuple { ""sv, fname } :
            std::tuple { fname.substr(0, pos),
                         fname.substr(pos + 1) };
    }

    inline std::string tolower(std::string_view string) {
        std::string lower;
        lower.resize(string.size());
        std::ranges::transform(string, lower.begin(), [](const char c) { return std::tolower(c); });
        return lower;
    }
}