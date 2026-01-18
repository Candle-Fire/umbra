#include <algorithm>
#include <atomic>
#include <shadow/assets/compiler/AssetCompiler.h>

#include "shadow/assets/format/TextReader.h"
#include "shadow/assets/management/synchronization.h"
#include "shadow/assets/resource/ResourceManager.h"
#include "shadow/assets/str/string.h"
#include "shadow/core/ShadowApplication.h"
#include "shadow/core/Time.h"
#include "shadow/jobs/Job.h"
#include "shadow/profile/Profiler.h"

namespace SH {
    template<>
    struct HashFunc<Path> {
        static uint32_t GetHash(const Path& key) {
            const size_t hash = key.getHash().getHash();
            return (uint32_t) (hash ^ (hash >> 32));
        }
    };
}

struct CompileJob {
    size_t gen;
    SH::Path path;
    bool complete = false;
};

namespace SH::Asset {

    struct AssetCompilerImpl : AssetCompiler {

        struct LoadHook : ResourceManager::LoadHook {
            LoadHook(AssetCompilerImpl& c) : c(c) {}
            virtual Action load(Resource& r) override { return c.onLoad(r); }
            void loadRaw(const Path& requester, const Path& path) override { c.registerDependency( requester, path ); }

            AssetCompilerImpl& c;
        };

        static HeapHash dirHash(const Path& p) {
            std::string_view str = Path::getDirectory(p.get());
            if (!str.empty() && (str.back() == '\\' || str.back() == '/')) str.remove_suffix(1);
            return HeapHash(str.begin(), str.size());
        }

        AssetCompilerImpl() : hook(*this) {
            SH::ShadowApplication::Get().GetResourceManager().setLoadHook(&hook);
        }

        ~AssetCompilerImpl() {
            assert(modules.empty());
            writeResourceDB();
            ShadowApplication::Get().GetResourceManager().setLoadHook(nullptr);
        }

        DelegateList<void(const Path&)>& onResourceListChanged() override {
            return CB_ResourceListChanged;
        }

        DelegateList<void(Resource&, bool)>& onResourceCompiled() override {
            return CB_ResourceCompiled;
        }

        void addCompileJob(const Path& p) {
            auto iter = generations.find(p);
            if (iter == generations.end())
                iter = generations.emplace(p, 0).first;
            else
                iter->second++;

            CompileJob compile { .gen = iter->second, .path = p };
            toCompile.push_back(compile);
            compiling++;
            compileRemaining++;
        }

        bool compile(const Path& p) override {
            Module* mod = getModule(p);
            if (!mod) {
                spdlog::error("Unknown resource in compilation: %s", p.get());
                return false;
            }
            return mod->compile(p);
        }

        void singleShot() {
            if (toCompile.empty()) return;

            CompileJob j = toCompile.back();
            toCompile.pop_back();

            auto iter = generations.find(j.path);
            const bool mostRecent = j.gen == iter->second;
            if (!mostRecent) {
                --compileRemaining;
                return;
            }

            compilingRes = j.path.get();
            SH::Jobs::run([j, this]() mutable {
                ProfileBlock("Asset compile");
                SH::Profiler::PushString(j.path.c_str());
                j.complete = compile(j.path);
                if (!j.complete) spdlog::error("Failed to compile resource %s", j.path.c_str());
                MutexGuard lock(compileMut);
                compiled.push_back(j);
            });
        }

        void writeResourceDB() {
            if (scanCounter > 0) return;

            ProfileFunction();
            FileOutput file;
            FileSystem& fs = *SH::ShadowApplication::diskFS;
            if (fs.open(Path("res/res.db.tmp"), file)) {
                file << "resources = [\n";
                for (const ResourceItem& r : std::views::values(resources))
                    file << "\"" << r.path.get() << "\"\n";
                file << "]\n\n";
                file << "dependencies = {\n";
                for (auto iter : dependencies) {
                    file << "\t\"" << iter.first.get() << "\" = [\n";
                    for  (const Path& p : iter.second)
                        file << "\t\t\"" << p.get() << "\",\n";
                    file << "\t],\n";
                }
                file << "}\n";

                file.close();
                fs.deleteFile(Path("res/res.db"));
                fs.moveFile(Path("res/res.db.tmp"), Path("res/res.db"));
            } else {
                spdlog::error("Failed to save Resource DB.");
            }
        }

        void readResourceDB() {
            if (modules.empty()) return;

            FileSystem& fs = *SH::ShadowApplication::diskFS;
            OutputMemoryStream fileContent;

            if (fs.readSync(Path("res/res.db"), fileContent)) {
                std::string content((const char*) fileContent.data());
                SH::TextReader reader(content, "res/res.db");
                for (;;) {                                                                                  // <-- Loop start
                    SH::TextReader::Token t = reader.tryNextToken(TextReader::Token::IDENTIFIER);
                    if (!t) break;                                                                          // <-- Loop break

                    if (t == "resources") {
                        // Read an array of strings. [ "abc", "xyz" ]
                        if (!reader.consume("=", "[")) goto loop_break;                                     // <-- Loop exit via goto
                        for (;;) {                                                                          //    <-- Loop start
                            t = reader.nextToken();
                            if (!t) goto loop_break;                                                        //    <-- Loop exit via goto
                            if (t == "]") break;                                                            //    <-- Loop break
                            if (t.type != TextReader::Token::STRING_LITERAL) {
                                spdlog::error("[%s:%d]: Expected string literal in resource file list, got %s", reader.filename, reader.getLine(), t.value);
                                reader.logErrorPosition(t.value.c_str());
                                goto loop_break;                                                            //    <-- Loop exit via goto
                            }

                            const Path p(t.value);
                            const ResourceType type = getType(p.get());
                            if (type.isValid()) {
                                if (fs.fileExists(p)) {
                                    MutexGuard lock(resourcesMut);
                                    resources.emplace(p.getHash(), ResourceItem { p, type, dirHash(p) });
                                } else {
                                    const Path processedPath(Path("res/processed") / p.get().substr(std::string("res/raw").length()));
                                    fs.deleteFile(processedPath);
                                }
                            }

                            t = reader.nextToken();
                            if (!t) goto loop_break;                                                        //    <-- Loop exit via goto
                            if (t == "]") break;                                                            //    <-- Loop break
                            if (t != ",") {
                                spdlog::error("[%s:%d]: Expected , separator or ] terminator in string list, got %s.", reader.filename, reader.getLine(), t.value);
                                reader.logErrorPosition(t.value.c_str());
                                goto loop_break;                                                            //    <-- Loop exit via goto
                            }
                        }                                                                                   //    <-- Loop end
                    } else if (t == "dependencies") {
                        // Read a list of STRING = ARRAY pairs.
                        if (!reader.consume("=", "{")) goto loop_break;                                     // <-- Loop exit via goto
                        for (;;) {                                                                          //    <-- Loop start
                            t = reader.nextToken();
                            if (!t) goto loop_break;                                                        //    <-- Loop exit via goto
                            if (t == "}") break;                                                            //    <-- Loop break
                            if (t.type != TextReader::Token::STRING_LITERAL) {
                                spdlog::error("[%s:%d]: Expected string literal in resource dependency definition list, got %s", reader.filename, reader.getLine(), t.value);
                                reader.logErrorPosition(t.value.c_str());
                                goto loop_break;                                                            //    <-- Loop exit via goto
                            }

                            if (!reader.consume("=", "[")) goto loop_break;                                 //    <-- Loop exit via goto

                            const Path key(t.value);
                            auto iter = dependencies.find(key);
                            if (iter != dependencies.end()) {
                                dependencies.emplace(key, std::vector<Path> ());
                                iter = dependencies.find(key);
                            }

                            // Parse the array of strings
                            for (;;) {                                                                      //    <-- Loop start
                                t = reader.nextToken();
                                if (!t) goto loop_break;                                                    //    <-- Loop exit via goto
                                if (t == "]") break;                                                        //    <-- Loop break
                                if (t.type != TextReader::Token::STRING_LITERAL) {
                                    spdlog::error("[%s:%d]: Expected string literal in dependency file list, got %s", reader.filename, reader.getLine(), t.value);
                                    reader.logErrorPosition(t.value.c_str());
                                    goto loop_break;                                                        //    <-- Loop exit via goto
                                }

                                // Add the string to the dependency vector
                                iter->second.emplace_back(t.value);

                                t = reader.nextToken();
                                if (!t) goto loop_break;                                                    //    <-- Loop exit via goto
                                if (t == "]") break;                                                        //    <-- Loop break
                                if (t != ",") {
                                    spdlog::error("[%s:%d]: Expected , separator or ] terminator in string list, got %s.", reader.filename, reader.getLine(), t.value);
                                    reader.logErrorPosition(t.value.c_str());
                                    goto loop_break;                                                       //    <-- Loop exit via goto
                                }
                            }

                            t = reader.nextToken();
                            if (!t) goto loop_break;                                                        //    <-- Loop exit via goto
                            if (t == "}") break;                                                            //    <-- Loop break
                            if (t != ",") {
                                spdlog::error("[%s:%d]: Expected , separator or }} terminator in definition list, got %s.", reader.filename, reader.getLine(), t.value);
                                reader.logErrorPosition(t.value.c_str());
                                goto loop_break;                                                            //    <-- Loop exit via goto
                            }
                        }                                                                                   //    <-- Loop end
                    } else {
                        spdlog::error("[%s:%d]: Unknown token in resource database: %s", reader.filename, reader.getLine(), t.value);
                        reader.logErrorPosition(t.value.c_str());
                        goto loop_break;                                                                    // <-- Loop exit via goto
                    }
                }                                                                                           // <-- Loop end
            }

            loop_break:
            {
                scanTimer.record();
                ProfileBlock("Asset Scan");
                const size_t lastModified = fs.getLastModified(Path("res/res.db"));
                recompileDir(Path("res/raw"), lastModified);
            }
            saveScannedResources = true;
        }

        CompileJob getNextJob() {
            MutexGuard lock(compileMut);
            if (compiled.empty()) return {};
            CompileJob j = compiled.back();
            compiled.pop_back();
            --compileRemaining;
            if (compileRemaining == 0) compiling = 0;
            return j;
        }

        ResourceManager::LoadHook::Action onLoad(Resource& res) {
            if (!initFinished) {
                // Defer loading the resource until we have a project directory to put it into
                res.increaseReferences();
                deferredResources.push_back(&res);
                return LoadHook::Action::DEFERRED;
            }

            FileSystem& fs = *ShadowApplication::diskFS;

            if (!fs.fileExists(res.getPath())) return LoadHook::Action::IMMEDIATE;
            if (Path::getDirectory(res.getPath().get()).starts_with("res/raw/"))  { return LoadHook::Action::IMMEDIATE; }

            Path subPath = Path(res.getPath().get().substr(std::string("res/raw").length()));
            Path processedPath = Path("res/processed" / subPath);
            Path metaPath = Path("res/meta" / subPath);

            bool compile = true;

            if (fs.fileExists(processedPath)) {
                size_t compiledTime = fs.getLastModified(processedPath);
                compile = compiledTime < fs.getLastModified(res.getPath()) || compiledTime < fs.getLastModified(metaPath);
            }

            if (compile) {
                if (!getModule(res.getPath())) return LoadHook::Action::IMMEDIATE;
                // compiler queue
                addCompileJob(res.getPath());

                return LoadHook::Action::DEFERRED;
            }

            return LoadHook::Action::IMMEDIATE;
        }

        void onFileUpdate(const char* path) {
            if (std::string(path) == "res/processed" || std::string(path) == "res/meta" || std::string(path) == "latest.log") return;

            FileSystem& fs = *ShadowApplication::diskFS;
            if (fs.fileExists(Path(path))) {
                MutexGuard lock(changeMut);
                changedFiles.push_back(Path(path));
            } else {
                MutexGuard lock(changeMut);
                changedDirs.push_back(Path(path));
            }
        }

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
        void setProjectDir(std::string_view base) override {
            ProfileFunction();
            dependencies.clear();
            resources.clear();

            FileSystem& fs = *ShadowApplication::diskFS;
            Path root(base, "res");
            bool success = std::filesystem::create_directories(root.get());
            if (!success) spdlog::error("Could not create directory %d", root.getHash().getHash());

            initFinished = true;
            for (Resource* res : deferredResources) {
                LoadHook::Action act = onLoad(*res);
                if (act == LoadHook::Action::IMMEDIATE) {
                    hook.continueLoad(*res, true);
                    res->decreaseReferences();
                } else {
                    addCompileJob(res->getPath());
                }
            }

            deferredResources.clear();
            readResourceDB();
        }

        Resource* getResource(const Path& p) const {
            ResourceManager& man = SH::ShadowApplication::Get().GetResourceManager();
            for (ResourceTypeManager* rtm : std::views::values(man.getAll())) {
                auto iter = rtm->getResources().find(p.getHash());
                if (iter != rtm->getResources().end()) return iter->second;
            }
            return nullptr;
        }

        void addResource(ResourceType t, const Path& p) override {
            const PathHash hash = p.getHash();
            MutexGuard lock(resourcesMut);

            resources[hash] = { p, t, dirHash(p) };

            if (!resources.contains(hash))
                CB_ResourceListChanged.invoke(p);
        }

        void addResource(const Path& p) {
            std::string ext = Str::toLower(Path::getExtension(p.get()));
            auto iter = modules.find(HeapHash(ext));
            if (iter == modules.end()) return;

            iter->second->addSubresource(*this, p, scanCounter);
        }

        ResourceType getType(std::string_view path) const override {
            std::string ext = Path::getExtension(path.data());
            std::string low = Str::toLower(ext);
            if (low.length() > 4) return ResourceType::INVALID_RESOURCE;
            // Convert the 4-byte string in low to a uint32_t
            uint32_t key = *(uint32_t*)low.c_str();
            if (exts.contains(key)) return exts.at(key);
            return ResourceType::INVALID_RESOURCE;
        }

        void registerDependency(const Path& original, const Path& dependent) override {
            auto iter = dependencies.find(dependent);
            if (iter == dependencies.end())
                iter = dependencies.emplace(dependent, std::vector<Path>()).first;

            if (std::find(iter->second.begin(), iter->second.end(), original) != iter->second.end())
                iter->second.emplace_back(original);
        }

        bool acceptsExt(std::string& ext, ResourceType t) const override {
            std::string low = Str::toLower(ext);
            if (low.length() > 4) return false;
            // Convert the 4-byte string in low to a uint32_t
            uint32_t key = *(uint32_t*)low.c_str();
            return exts.contains(key);
        }

        void registerExt(const char* ext, ResourceType t) override {
            std::string ex = std::string(ext);
            std::string low = Str::toLower(ex);
            uint32_t key = *(uint32_t*)low.c_str();

            assert(!exts.contains(key));
            exts.emplace(key, t);
        }

        Module* getModule(const Path& p) {
            std::string ext = Str::toLower(Path::getExtension(p.get()));
            MutexGuard lock(moduleMut);
            auto iter = modules.find(HeapHash(ext));
            return (iter == modules.end()) ? iter->second : nullptr;
        }

        float getProgress() override {
            return ((float) compiling - compileRemaining) / compiling;
        }

        void recompileDir(Path dir, size_t lastModified) {
            FileSystem& fs = *SH::ShadowApplication::diskFS;
            auto* iter = fs.iterateDirectory(dir);
            FileInfo info;

            while (iter->getNext(&info)) {
                if (info.filename.starts_with("res/meta") || info.filename.starts_with("res/processed")) continue;

                Path constructed = dir / "/" / info.filename;
                if (info.directory) {
                    recompileDir(constructed, lastModified);
                } else {
                    if (fs.getLastModified(constructed) > lastModified || resources.find(constructed.getHash()) != resources.end())
                        addResource(constructed);
                }
            }

            fs.destroyIterator(iter);
        }

        void recompileAll() override {
            if (saveScannedResources && scanCounter == 0) {
                saveScannedResources = false;
                writeResourceDB();
                spdlog::info("Asset scan took %d seconds", scanTimer.elapsedSeconds());
            }

            // TODO: This looks weird as hell, but we need to run until the break condition is met, and we can't use the standard syntax because the check needs to happen at the start, not the end.
            for (;;) {                                                          // <-- Loop start
                singleShot();
                CompileJob job = getNextJob();
                if (job.path.isEmpty()) break;                                  // <--- Loop exit condition

                uint32_t gen = generations[job.path];
                if (job.gen != gen) continue;                                   // <--- Loop continue condition

                MutexGuard lock(resourcesMut);
                bool targetFound = false;

                // Check all resources for ones that match the path we were given
                for (const ResourceItem& res : std::views::values(resources)) {
                    // Keep iterating resources until one matches
                    if (!res.path.get().ends_with(job.path.get())) continue;

                    targetFound = true;
                    // Ensure we actually know how to get this Resource..
                    Resource* r = getResource(res.path);
                    if (!r) continue;

                    if (r->isReady() || r->isFailure()) r->getManager().reload(*r);
                    else if (r->isHooked()) hook.continueLoad(*r, job.complete);

                    CB_ResourceCompiled.invoke(*r, job.complete);
                }

                // Sanity check on resources that weren't located.
                // Engage clippy mode!
                if (!targetFound) {
                    spdlog::error("Resource %s not found..", job.path.get());
                    for (const ResourceItem& res : std::views::values(resources)) {
                        if (Str::toLower(res.path.get()).ends_with(Str::toLower(job.path.get())))
                            spdlog::error("Did you mean %s?", res.path.get());
                    }
                }

                // If there are dependencies registered, we process them even if the resource itself couldn't be found
                auto depIter = dependencies.find(job.path);
                if (depIter != dependencies.end()) {
                    for (const Path& p : depIter->second) {
                        addCompileJob(p);
                    }
                }
            }                                                               // <-- Loop exit

            // Update all folders with changed files.
            for (;;) {                                                      // <-- Loop start
                Path p;
                // Explicit block so that we don't mutex this whole loop.
                {
                    MutexGuard lock(changeMut);
                    if (changedDirs.empty()) break;                         // <-- Loop break condition

                    std::sort(changedDirs.begin(), changedDirs.end());
                    changedDirs.erase(std::unique(changedDirs.begin(), changedDirs.end()), changedDirs.end());
                    p = changedDirs.back();
                    changedDirs.pop_back();
                }

                if (!p.isEmpty()) {
                    FileSystem& fs = *SH::ShadowApplication::diskFS;
                    const size_t lastModified = fs.getLastModified(Path("res/res.db"));
                    if (fs.dirExists(p)) {
                        recompileDir(p, lastModified);
                        CB_ResourceListChanged.invoke(p);
                    } else {
                        MutexGuard lock(resourcesMut);
                        std::erase_if(resources, [&](const std::pair<PathHash, ResourceItem>& ri) {
                            if (!ri.second.path.get().starts_with(p.get())) return false;
                            return true;
                        });
                        CB_ResourceListChanged.invoke(p);
                    }
                }
            }                                                               // <-- Loop exit

            // Update all loose changed files
            for (;;) {                                                      // <-- Loop start
                Path p;
                {
                    MutexGuard lock(changeMut);
                    if (changedFiles.empty()) break;                        // <-- Loop break condition

                    p = changedFiles.back();
                    changedFiles.pop_back();
                    std::erase_if(changedFiles, [&](const Path& path) { return p == path;});
                }

                if (Path::hasExtension(p.get(), "meta")) {
                    // Find the raw for the meta.
                    // TODO: this is a reasonably common operation. Should we make a getRaw() function?
                    p.set("res/raw" + p.get().substr(std::string("res/meta").length()));
                }

                if (getType(p.get()) != ResourceType::INVALID_RESOURCE) {
                    FileSystem& fs = *SH::ShadowApplication::diskFS;
                    if (!fs.fileExists(p)) {
                        MutexGuard lock(resourcesMut);
                        std::erase_if(resources, [&](const std::pair<PathHash, ResourceItem>& ri) {
                            if (!Str::toLower(ri.second.path.get()).ends_with(Str::toLower(p.get()))) return false;
                            return true;
                        });
                        CB_ResourceListChanged.invoke(p);
                    } else {
                        addResource(p);
                        addCompileJob(p);
                    }
                } else {
                    auto depIter = dependencies.find(p);
                    if (depIter != dependencies.end()) {
                        for (const Path& p : depIter->second)
                            addCompileJob(p);
                    }
                }
            }                                                               // <-- Loop exit
        }

    private:

        std::map<HeapHash, Module*> modules;
        std::map<size_t, ResourceType> exts;
        std::map<PathHash, ResourceItem> resources;
        std::map<Path, std::vector<Path>> dependencies;
        std::map<Path, size_t> generations;

        Mutex compileMut;
        Mutex changeMut;
        Mutex moduleMut;
        Mutex resourcesMut;

        std::vector<Path> changedFiles;
        std::vector<Path> changedDirs;

        std::vector<CompileJob> toCompile;
        std::vector<CompileJob> compiled;

        std::vector<Resource*> deferredResources;

        LoadHook hook;

        std::atomic<uint32_t> scanCounter = 0;

        bool saveScannedResources = false;
        Timer scanTimer;

        DelegateList<void(const Path&)> CB_ResourceListChanged;
        DelegateList<void(Resource&, bool)> CB_ResourceCompiled;

        bool initFinished = false;

        size_t compiling = 0;
        size_t compileRemaining = 0;
        Path compilingRes;

    };

    void AssetCompiler::Module::addSubresource(AssetCompiler& c, const Path& p, std::atomic<uint32_t>& signal) {
        const ResourceType t = c.getType(p.get());
        if (!t.isValid()) return;
        c.addResource(t,p);
    }





}