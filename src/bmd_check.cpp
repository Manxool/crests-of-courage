// Header check for J3D model files, kept free of game headers so it can be tested on its own.

#include "util.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

static uint32_t be32(const unsigned char* b) {
    return (uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) | (uint32_t(b[2]) << 8) | uint32_t(b[3]);
}

// Checks a .bmd/.bdl without loading it: the J3D header, that the file is as long as it says, that
// every section fits inside it, and that the sections a model cannot be drawn without are there.
// A file that fails is treated as missing, so Link's own part is used and it never reaches the
// renderer.
bool coop_bmd_file_ok(const std::filesystem::path& path, std::string& why) {
    std::error_code ec;
    const auto& real = path;
    const uintmax_t size = std::filesystem::file_size(real, ec);
    if (ec) {
        why = "cannot be read";
        return false;
    }
    if (size < 0x20 || size > 64u * 1024u * 1024u) {
        why = "has an impossible size";
        return false;
    }
    std::ifstream file(real, std::ios::binary);
    unsigned char head[0x20];
    if (!file || !file.read(reinterpret_cast<char*>(head), sizeof(head))) {
        why = "cannot be read";
        return false;
    }
    if (std::memcmp(head, "J3D2", 4) != 0 ||
        (std::memcmp(head + 4, "bmd", 3) != 0 && std::memcmp(head + 4, "bdl", 3) != 0)) {
        why = "is not a J3D model";
        return false;
    }
    // padding after the model is harmless (some exporters align the file), a short file is not
    const uint64_t declared = be32(head + 8);
    if (declared > size || declared < 0x20) {
        why = "is cut short (header says " + std::to_string(declared) + " bytes, file has " +
              std::to_string(static_cast<unsigned long long>(size)) + ")";
        return false;
    }
    const uint32_t sections = be32(head + 12);
    if (sections == 0 || sections > 16) {
        why = "has a broken section table";
        return false;
    }
    bool inf = false, vtx = false, jnt = false, shp = false, mat = false;
    uint64_t at = 0x20;
    for (uint32_t i = 0; i < sections; ++i) {
        unsigned char sec[8];
        if (at + 8 > declared || !file.seekg(static_cast<std::streamoff>(at)) ||
            !file.read(reinterpret_cast<char*>(sec), 8)) {
            why = "has a section past its end";
            return false;
        }
        const uint32_t len = be32(sec + 4);
        if (len < 8 || at + len > declared) {
            why = "has a section past its end";
            return false;
        }
        if (std::memcmp(sec, "INF1", 4) == 0) inf = true;
        else if (std::memcmp(sec, "VTX1", 4) == 0) vtx = true;
        else if (std::memcmp(sec, "JNT1", 4) == 0) jnt = true;
        else if (std::memcmp(sec, "SHP1", 4) == 0) shp = true;
        else if (std::memcmp(sec, "MAT3", 4) == 0 || std::memcmp(sec, "MAT2", 4) == 0) mat = true;
        at += len;
    }
    if (!(inf && vtx && jnt && shp && mat)) {
        why = "is missing model sections";
        return false;
    }
    return true;
}

