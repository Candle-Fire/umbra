#include "shadow/assets/fs/file.h"

#include <vector>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <map>
#include "shadow/assets/fs/hash.h"
#include <shadow/assets/management/synchronization.h>

#include "shadow/assets/str/string.h"
#include "shadow/core/Time.h"
#include "shadow/profile/Profiler.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace SH {
    // Because fuck Linux? Need platform-specific source files!

    FileInput::FileInput() {
        handle = (void*)INVALID_HANDLE_VALUE;
    }

    FileInput::FileInput(const Path& path) {
        handle = (HANDLE)CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
    }

    FileInput::~FileInput() {
        if (INVALID_HANDLE_VALUE != (HANDLE)handle) {
            close();
        }
    }

    FileOutput::FileOutput() {
        error = false;
        handle = (void*)INVALID_HANDLE_VALUE;
    }

    FileOutput::~FileOutput() {
        if (handle != INVALID_HANDLE_VALUE) { flush(); close(); }
    }

    FileOutput::FileOutput(const Path& path) {
        handle = (HANDLE)CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        error = handle == INVALID_HANDLE_VALUE;
    }


    bool FileOutput::open(const Path& path) {
        handle = (HANDLE)CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        return INVALID_HANDLE_VALUE != handle;
    }

    bool FileOutput::touch(const Path& path) {
        handle = (HANDLE)CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE) return false;
        size_t written = 0;
        WriteFile((HANDLE)handle, 0, (DWORD)1, (LPDWORD)&written, nullptr);
        error = error | (written != 0);
        CloseHandle((HANDLE)handle);
        return !error;
    }

    bool FileInput::open(const Path& path) {
        handle = (HANDLE)CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        return INVALID_HANDLE_VALUE != handle;
    }

    void FileInput::close() {
        if (INVALID_HANDLE_VALUE != (HANDLE)handle) {
            CloseHandle((HANDLE)handle);
            handle = (void*)INVALID_HANDLE_VALUE;
        }
    }

    void FileOutput::close() {
        if (INVALID_HANDLE_VALUE != (HANDLE)handle) {
            CloseHandle((HANDLE)handle);
            handle = (void*)INVALID_HANDLE_VALUE;
        }
    }

    size_t FileInput::size() const {
        assert(INVALID_HANDLE_VALUE != handle);
        return GetFileSize((HANDLE)handle, 0);
    }

    size_t FileInput::pos() {
        assert(INVALID_HANDLE_VALUE != handle);
        return SetFilePointer((HANDLE)handle, 0, nullptr, FILE_CURRENT);
    }

    bool FileInput::seek(size_t pos) {
        assert(INVALID_HANDLE_VALUE != handle);
        LARGE_INTEGER distance;
        distance.QuadPart = pos;
        return SetFilePointer((HANDLE)handle, distance.u.LowPart, &distance.u.HighPart, FILE_BEGIN) !=
            INVALID_SET_FILE_POINTER;
    }

    bool FileInput::read(void* data, size_t size) {
        assert(INVALID_HANDLE_VALUE != handle);
        DWORD read = 0;
        BOOL success = ReadFile((HANDLE)handle, data, (DWORD)size, (LPDWORD)&read, nullptr);
        return success && size == read;
    }

    void FileOutput::flush() {
        assert(handle != nullptr);
        FlushFileBuffers((HANDLE)handle);
    }

    bool FileOutput::write(const void* data, size_t size) {
        assert(handle != INVALID_HANDLE_VALUE);
        size_t written = 0;
        WriteFile((HANDLE)handle, data, (DWORD)size, (LPDWORD)&written, nullptr);
        error = error | size != written;
        return !error;
    }

#endif

    /**
     * An async operation to be performed.
     * For reading files from disk into memory.
     */

    struct AsyncRead {
        enum Flags : uint32_t {
            FAILED = 0,   // The read failed due to some error.
            CANCELLED = 1 // The read was cancelled due to the resource not being needed any more.

        };

        AsyncRead() : data() {
        }

        bool isFailed() const { return flags == Flags::FAILED; }
        bool isCancelled() const { return flags == Flags::CANCELLED; }

        FileSystem::ContentCallback callback;
        OutputMemoryStream data;
        std::string path;
        uint32_t id = 0;
        Flags flags;
    };

    inline AsyncRead::Flags operator|=(AsyncRead::Flags a, AsyncRead::Flags b) {
        a = static_cast<AsyncRead::Flags>(static_cast<int>(a) | static_cast<int>(b));
        return a;
    }

    bool FileIterator::getNext(FileInfo* f) {
        if (offset == 0) {
            FILE_INFO_BY_HANDLE_CLASS classType = isFirstElement ? FileIdBothDirectoryRestartInfo : FileIdBothDirectoryInfo;
            isFirstElement = false;
            if (!GetFileInformationByHandleEx(handle, classType, buffer, sizeof(buffer))) {
                auto err = GetLastError();
                return false;
            }
        }

        FILE_ID_BOTH_DIR_INFO* dirInfo = (FILE_ID_BOTH_DIR_INFO*) (buffer + offset);
        if (dirInfo->FileNameLength == 0) return false;

        f->filename = SH::Str::toShort(dirInfo->FileName);
        f->directory = (dirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        offset += dirInfo->NextEntryOffset;
        if (dirInfo->NextEntryOffset == 0) offset = 0;
        return true;
    };

    // The FileSystem that operates on raw on-disk files.
    struct DiskFS : FileSystem {

        explicit DiskFS(const Path& path) : asyncReaderSemaphore(0, 0xffff) {
            setBasePath(path);

            // TODO: This is perhaps better as a Profiler-style Thread rather than as a Job?
            /*SH::Jobs::Run([this] {
                pthread_setname_np(pthread_self(), "SH::Job::Filesystem_Async_Reader");
                bool workFinished = false;
                while (!workFinished) {
                    asyncReaderSemaphore.wait();
                    if (workFinished) break;

                    Path p;
                    {
                        MutexGuard lock(asyncReaderMutex);
                        assert(!asyncWaitingQueue.empty());
                        p = asyncWaitingQueue.front().path;
                        if (asyncWaitingQueue.front().isCancelled()) {
                            asyncWaitingQueue.erase(asyncWaitingQueue.begin());
                            continue;
                        }
                    }

                    OutputMemoryStream data;
                    bool success = readSync(p, data);

                    {
                       MutexGuard lock(asyncReaderMutex);
                        if (!asyncWaitingQueue.front().isCancelled()) {
                            asyncFinished.emplace_back(asyncWaitingQueue.front());
                            asyncFinished.back().data = data;
                            if (!success)
                                asyncFinished.back().flags |= AsyncRead::Flags::FAILED;
                        }

                        asyncWaitingQueue.erase(asyncWaitingQueue.begin());
                    }

                }
            });*/
        }

        bool hasWork() override {
            return asyncWorkCounter != 0;
        }

        Path const& getBasePath() const override { return basePath; }

        void setBasePath(const Path& path) final {
            basePath = Path::normalise(path.get());
            if (!basePath.get().ends_with('/') && !basePath.get().ends_with('\\'))
                basePath.append("/");
        }

        bool saveSync(const Path& path, const uint8_t* data, const size_t size) override {
            FileOutput file;
            Path fullPath = basePath / path;

            if (!file.open(fullPath)) return false;
            bool res = file.write(data, size);
            file.close();

            return res;
        }

        bool readSync(const Path& path, struct OutputMemoryStream& content) override {
            ProfileFunction();
            FileInput file;
            Path fullPath = basePath / path;

            if (!file.open(fullPath)) return false;

            content.resize(file.size());
            if (!file.read(content.dataMut(), content.size())) {
                file.close();
                return false;
            }

            file.close();
            return true;
        }

        AsyncHandle readAsync(const Path& file, const ContentCallback& callback) override {
            if (!file.isEmpty()) return AsyncHandle::invalid();

            MutexGuard lock(asyncReaderMutex);
            asyncWorkCounter++;

            AsyncRead& read = asyncWaitingQueue.emplace_back();
            if (++lastID == 0) lastID++;

            read.id = lastID;
            read.path = file.c_str();
            read.callback = callback;
            asyncReaderSemaphore.raise();

            return AsyncHandle(read.id);
        }

        void cancelAsync(AsyncHandle& handle) override {
            MutexGuard lock(asyncReaderMutex);

            for (AsyncRead& read : asyncWaitingQueue) {
                if (read.id == handle.value) {
                    read.flags = AsyncRead::Flags::CANCELLED;
                    asyncWorkCounter--;
                    return;
                }
            }

            for (AsyncRead& read : asyncFinished) {
                if (read.id == handle.value) {
                    read.flags = AsyncRead::Flags::CANCELLED;
                    return;
                }
            }
        }

        bool open(const Path& path, FileInput& file) override {
            return file.open(basePath / path);
        }

        bool open(const Path& path, FileOutput& file) override {
            return file.open(basePath / path);
        }

        bool deleteFile(const Path& path) override {
            return std::remove((basePath / path).c_str());
        }

        bool moveFile(const Path& from, const Path& to) override {
            try {
                std::rename((basePath / from).c_str(), (basePath / to).c_str());
            }
            catch (std::filesystem::filesystem_error& e) {
                return false;
            }
            return true;
        }

        bool copyFile(const Path& from, const Path& to) override {
            try {
                std::filesystem::copy((basePath / from).c_str(), (basePath / to).c_str());
            }
            catch (std::filesystem::filesystem_error& e) {
                return false;
            }

            return true;
        }

        bool fileExists(const Path& path) override {
            return std::filesystem::is_regular_file(path.get());
        }

        bool dirExists(const Path& p) override {
            return std::filesystem::is_directory(p.get());
        }

        size_t getLastModified(const Path& path) override {
            return std::filesystem::last_write_time(path.get()).time_since_epoch().count();
        }

        SH::FileIterator* iterateDirectory(const Path& p) override {
            std::wstring wpath = SH::Str::toWide(p.get());
            HANDLE h = CreateFileW(wpath.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
            if (h == INVALID_HANDLE_VALUE) return nullptr;

            auto* iter = new FileIterator();
            iter->offset = 0;
            iter->isFirstElement = true;
            iter->handle = h;
            return iter;
        }

        void destroyIterator(SH::FileIterator* iter) override {
            CloseHandle(iter->handle);
            delete iter;
        }

        void processCallbacks() override {
            ProfileFunction();
            SH::Timer timer;

            for (;;) {
                asyncReaderMutex.enter();
                if (asyncFinished.empty()) {
                    asyncReaderMutex.exit();
                    break;
                }

                AsyncRead item = asyncFinished[0];
                asyncFinished.erase(asyncFinished.begin());
                --asyncWorkCounter;

                asyncReaderMutex.exit();

                if (!item.isCancelled())
                    item.callback.invoke(item.data.size(), (const uint8_t*)item.data.data(), !item.isFailed());

                // Run until we use up too much time.
                if (timer.elapsedSeconds() > 0.1f) break;
            }
        }

        Path basePath;
        std::vector<AsyncRead> asyncWaitingQueue;
        uint64_t asyncWorkCounter;
        std::vector<AsyncRead> asyncFinished;
        Mutex asyncReaderMutex;
        Semaphore asyncReaderSemaphore;

        uint32_t lastID;
    };

    struct VFS : DiskFS {
        VFS(const Path& root_pack_path) : DiskFS(Path("vfs:/")) {
            if (!pack.open(root_pack_path)) {
                spdlog::error("Unable to open " + root_pack_path.get() + ", please check paths");
                return;
            }

            const auto count = pack.read<size_t>();
            for (size_t i = 0; i < count; i++) {
                const auto hash = pack.read<PathHash>();
                PackFile& file = packFiles[hash];
                file.offset = pack.read<size_t>();
                file.size = pack.read<size_t>();
            }
        }

        ~VFS() { pack.close(); }

        bool readSync(const Path& path, OutputMemoryStream& content) override {
            std::string basename = Path::getFilename(path.get());
            PathHash hash = path.getHash();

            auto i = packFiles.find(hash);
            if (i == packFiles.end()) return false;

            content.resize(i->second.size);
            MutexGuard lock(asyncReaderMutex);

            const size_t headerSize = sizeof(uint32_t) + packFiles.size() * (2 * sizeof(size_t) + sizeof(PathHash));
            if (pack.seek(i->second.offset + headerSize) || !pack.read(content.dataMut(), content.size())) {
                spdlog::error("Could not read file " + path.get() + " from the pack file.");
                return false;
            }

            return true;
        }

        struct PackFile {
            size_t offset;
            size_t size;
        };

        std::map<PathHash, PackFile> packFiles;
        FileInput pack;
    };

    std::unique_ptr<FileSystem> FileSystem::createDiskFS(const Path& basePath) {
        return std::make_unique<DiskFS>(basePath);
    }

    std::unique_ptr<FileSystem> FileSystem::createVFS(const Path& basePath) {
        return std::make_unique<VFS>(basePath);
    }
}
