#include <shadow/util/File.h>
#include <string>
#include <fstream>
namespace shadowutil {
    FileData* loadFile(std::string path) {
        // Verify the file
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        if (!file.is_open())
            throw std::runtime_error("Unable to open specified file: " + std::string(path));

        auto* data = new FileData {};

        // Read the file's size (opened with At The End, so just tellg the size)
        size_t size = file.tellg();
        data->data.resize(size);
        data->size = size;
        // Go to the front of the file
        file.seekg(0);
        // Read the file into the buffer
        file.read(data->data.data(), size);
        file.close();

        return data;
    }
}