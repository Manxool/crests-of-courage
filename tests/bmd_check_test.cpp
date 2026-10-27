// Standalone test for coop_bmd_file_ok: every .bmd under the given folder must pass, and a few
// damaged copies of the first one must fail. Build: see tests/README.md
#include "../src/util.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

std::filesystem::path path_ci(const std::filesystem::path& p) { return p; }

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s <models dir> <scratch dir>\n", argv[0]);
        return 2;
    }
    int ok = 0, bad = 0;
    std::filesystem::path first;
    for (const auto& e : std::filesystem::recursive_directory_iterator(argv[1])) {
        if (!e.is_regular_file() || e.path().extension() != ".bmd") continue;
        std::string why;
        if (coop_bmd_file_ok(e.path(), why)) {
            ++ok;
            if (first.empty()) first = e.path();
        } else {
            ++bad;
            std::printf("REJECTED %s: %s\n", e.path().string().c_str(), why.c_str());
        }
    }
    std::printf("%d valid, %d rejected\n", ok, bad);
    if (first.empty()) return 1;

    std::ifstream in(first, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(in)), {});
    const std::filesystem::path dir = argv[2];
    int caught = 0, cases = 0;
    auto expect_bad = [&](const char* name, std::vector<char> data) {
        ++cases;
        const auto p = dir / name;
        std::ofstream(p, std::ios::binary).write(data.data(), static_cast<std::streamsize>(data.size()));
        std::string why;
        const bool passed = coop_bmd_file_ok(p, why);
        std::printf("%-12s %s %s\n", name, passed ? "PASSED (wrong)" : "rejected:", why.c_str());
        if (!passed) ++caught;
    };
    expect_bad("truncated", std::vector<char>(bytes.begin(), bytes.begin() + bytes.size() / 2));
    auto magic = bytes; magic[0] = 'X'; expect_bad("magic", magic);
    auto secs = bytes; secs[12] = 0x7F; expect_bad("sections", secs);
    auto shortsec = bytes; shortsec[0x20 + 4] = 0x7F; expect_bad("section", shortsec);
    expect_bad("empty", std::vector<char>(16, 0));
    // trailing padding must still pass
    auto padded = bytes;
    padded.resize(bytes.size() + 64, 0);
    const auto pp = dir / "padded";
    std::ofstream(pp, std::ios::binary).write(padded.data(), static_cast<std::streamsize>(padded.size()));
    std::string why;
    const bool paddedOk = coop_bmd_file_ok(pp, why);
    std::printf("%-12s %s\n", "padded", paddedOk ? "accepted" : ("REJECTED (wrong): " + why).c_str());
    return (bad == 0 && caught == cases && paddedOk) ? 0 : 1;
}
