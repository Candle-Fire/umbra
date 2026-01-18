#pragma once
#include <map>

#include "shadow/assets/resource/Resource.h"

namespace SH::Asset {
    /**
     * A compiler is a resource pre-processor.
     * For now, it only compresses assets into a uniform format (SERF).
     * In the future, it will also pack assets into a Virtual Filesystem, and configure the VFS to be "packed" before written to disk as SEPT.
     *
     * For now, the Compiler handles assets which have dependencies, and allows a UI to interface with the compilation process.
     *
     * The different ResourceTypes are handled by their own CompilerModule, so as to allow custom Resources added by Game Modules to have their own compilation behaviour.
     *
     * So as to not swell this class with massive amounts of utility functions, this is an interface; use the Create function to get an instance of the Implementation.
     */
    struct AssetCompiler {

        /**
         * A module handles the compilation for a specific ResourceType.
         * It allows the handling of different resource behaviours to be decentralised from the Compiler itself.
         * A Module tends to operate for a specific class (say, Texture) or for a specific extension (say, .stx)
         */
        struct Module {
            virtual ~Module() {}

            // Perform the necessary compilation steps for the resource at the given DFS path.
            virtual bool compile(const Path& p) = 0;

            // Models (Mesh + Texture + Animation) have subresources, and will scan for them. When signal is 0, all scans are done.
            virtual void addSubresource(AssetCompiler& c, const Path& p, std::atomic<uint32_t>& signal);

            // Called when the list of resources to be handled by this module is loaded and ready to operate over.
            virtual void loaded() {}
        };

        /**
         * A single resource being handled by the Compiler.
         * A file can contain multiple Resources, but support for this is yet to be implemented.
         */
        struct ResourceItem {
            Path path;
            ResourceType type;
            HeapHash hash;
        };

        // Multiple Asset Compilers can exist at once.
        static std::unique_ptr<AssetCompiler> create();

        virtual ~AssetCompiler();

        /**
         * The Project Directory is the root of all input and output resources.
         * The structure is generally like so:
         *  base/
         *  |- res.db               { a textual database of the resources, which types they correspond to, and whether or not they have been compiled }
         *  |- raw/                 { the input resources, in natural format: png, obj }
         *  |  L xyz/               { input resources can be arranged in folders corresponding to type, origin, purpose, or preference. }
         *  |- processed/           { the processed/compiled resources, in uniform .serf format, mirroring the raw/xyz/ directory structure }
         *  |- packed/              { the processed resources, in packed .sept format, corresponding to their VFS locations. }
         *  L  meta/                { dedicated metadata companions for all resources, mirroring the raw/xyz/ directory structure }
         */
        virtual void setProjectDir(std::string_view base) = 0;

        // Add the given compiler module, reading a given list of extensions. A null-terminated list of null-terminated strings.
        virtual void addModule(Module& mod, const char* exts) = 0;
        // Remove the given module. If it is the only module servicing its' extensions, those extensions will cease to be able to be compiled.
        virtual void removeModule(Module& mod) = 0;

        // Process the resource(s) at the given path according to the relevant type Module.
        virtual bool compile(const Path& p) = 0;
        // Reprocess all resources in the raw folder.
        virtual void recompileAll() = 0;

        // Retrieve the meta file contents for the given Raw resource path
        virtual bool getMeta(const Path& p, OutputMemoryStream& b) = 0;
        // Write the given dataspan into the Meta file contents for the given Raw resource path
        virtual void updateMeta(const Path* p, size_t s, uint8_t* ptr) const = 0;

        // Acquire a mutex lock for all of the tracked resources.
        virtual const std::map<PathHash, ResourceItem>& lockResources() = 0;
        // Release the mutex lock for all of the tracked resources.
        virtual void unlockResources() = 0;

        // Determine the ResourceType of the Raw resource at the given path, by extension.
        virtual ResourceType getType(std::string_view path) const = 0;

        // Register the given file extension to a given ResourceType.
        virtual void registerExt(const char* ext, ResourceType t) = 0;
        // Check whether the given ResourceType accepts the given file extension.
        virtual bool acceptsExt(std::string& ext, ResourceType t) const = 0;

        // Track the given raw resource.
        virtual void addResource(ResourceType t, const Path& p) = 0;
        // Register the dependent Resource as relying on the original resource.
        virtual void registerDependency(const Path& original, const Path& dependent) = 0;

        // Output the compiled resource dataspan to the Processed directory.
        virtual bool writeCompiled(const Path& p, size_t s, uint8_t* ptr) const = 0;

        // Fetch the callback list for the "Resource List Changed" event.
        virtual DelegateList<void(const Path&)>& onResourceListChanged() = 0;
        // Fetch the callback list for the "Resource Compiled" event.
        virtual DelegateList<void(Resource&, bool)>& onResourceCompiled() = 0;

        // Get a numerical representation of the compilation progress. 0 represents no ongoing compilation.
        virtual float getProgress() = 0;
    };
}
