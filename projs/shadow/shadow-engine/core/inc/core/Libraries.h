#pragma once

namespace ShadowEngine {

    namespace Library {

        // Load the library from the given path.
        // Return the address of the data in RAM.
        static void* load(const char* path);
        // Unload the library at the given address.
        static void unload(void* handle);
    }
}