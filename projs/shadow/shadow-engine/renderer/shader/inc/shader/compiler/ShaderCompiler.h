#pragma once
#include <cstdint>
#include <renderer/GraphicsDefine.h>

/**
 * Shader compiler, management and tracking.
 */
namespace rx::shader {

    enum class Flags : uint32_t {
        NONE,
        DISABLE_OPTIMIZATION = 1 // Tell the shader compiler to not aggressively optimize the shader when compiling. Useful for shaders with debug prints.
    };

    /**
     * Input data to a single shader compile operation.
     * Can be reused, just change the sourceName.
     */
    struct CompilerInput {
        Flags flags = Flags::NONE;                              // Optional flags for the compiler
        rx::ShaderFormat format = rx::ShaderFormat::NONE;       // The source format of the shader, to compile from
        rx::ShaderStage stage = rx::ShaderStage::SIZE;          // The expected stage of the shader.
        rx::ShaderModel model = rx::ShaderModel::SM50;          // The driver model of the shader, optional features that may be enabled depending on the shader.

        std::string sourceName;                                 // The full path & filename of the shader source
        std::string entryPoint = "main";                        // The entry point of the shader. Usually (and by default) "main".

        std::vector<std::string> includes;                      // Other files that the shader may depend on. Most of these should be ShaderBase and shader/modules/*.h.
        std::vector<std::string> defines;                       // Defines passed to the shader - recompiling the same shader with different defines may produce different binaries.
    };

    /**
     * Output data from a shader compile.
     * Pass to a Shader Module to store on GPU.
     */
    struct CompilerOutput {
        std::shared_ptr<void> internal;                         // Internal data related to the shader. NOT the shader binary itself.
        /**
         *
         * @return whether the compile completed, and we can expect shader data.
         */
        inline bool IsValid() const { return internal.get() != nullptr; }

        const uint8_t* data = nullptr;                          // Shader binary output, if successful.
        size_t dataLen = 0;                                     // The amount of bytes in the outputted shader binary data.
        std::vector<uint8_t> hash;                              // The hash of the shader, useful for indexing and caching.
        std::string error;                                      // Errors thrown by the compiler, if unsuccessful.
        std::vector<std::string> deps;                          // Other shader modules that may be required for this one to load.
    };

    /**
     * Compile the shader module from the given source input, storing into the given output.
     * @param input source input, data and metadata, and compiler options
     * @param out the binary data if successful, errors if unsuccessful
     */
    void Compile(const CompilerInput& input, CompilerOutput& out);

    /**
     * Save the metadata for a successful shader compile to disk, to be loaded later.
     * @param filename the name of the file to save the metadata to
     * @param out the compiler output (must be valid)
     * @return whether the data was saved successfully
     */
    bool SaveData(const std::string& filename, const CompilerOutput& out);

    /**
     * Check whether the metadata saved for a shader is up to date.
     * @param filename the shader binary file name to check 
     * @return whether the dependency data needs to be updated for the given shader.
     */
    bool IsMetadataOutdated(const std::string& filename);

    /**
     * Add the shader binary blob to an internally tracked list - so we can recompile them live
     * @param filename the shader binary file name to add
     */
    void RegisterShader(const std::string& filename);

    /**
     * @return the amount of shaders currently being tracked
     */
    size_t GetRegisteredShaders();

    /**
     * Iterate the registered shaders and check whether any of them need to be refreshed.
     * @return whether any of the registered shaders were updated
     */
    bool CheckRegisteredShadersOutdated();
}

template<>
struct enable_bitmask_operators<rx::shader::Flags> {
    static const bool enable = true;
};
