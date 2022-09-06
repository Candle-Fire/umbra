#pragma once
#include <string>
#include <vector>
namespace shadowutil {

    struct FileData {
        size_t size;
        std::vector<char> data;
    };

    // A testing stub; this should be deleted and wired into the asset system once that becomes ready.
    FileData* loadFile(std::string path);
}