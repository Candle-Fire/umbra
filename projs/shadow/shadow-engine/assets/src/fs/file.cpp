#include <shadow/assets/fs/file.h>
#include <vector>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <shadow/assets/fs/path.h>
#include <map>
#include <shadow/util/Syncronization.h>

namespace ShadowEngine {

  // Because fuck Linux? Need platform-specific source files!
#ifdef _WIN32

  #define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shadow/util/Delegate.h>

    FileInput::FileInput() {
        handle = (void*) INVALID_HANDLE_VALUE;
    }

    FileOutput::FileOutput() {
        error = false;
        handle = (void*) INVALID_HANDLE_VALUE;
    }

    bool FileOutput::open(std::string& path) {
        handle = (HANDLE) CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        return INVALID_HANDLE_VALUE != handle;
    }

    bool FileInput::open(std::string& path) {
        handle = (HANDLE) CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        return INVALID_HANDLE_VALUE != handle;
    }

    void FileInput::close() {
        if (INVALID_HANDLE_VALUE != (HANDLE) handle) {
            CloseHandle((HANDLE) handle);
            handle = (void*) INVALID_HANDLE_VALUE;
        }
    }

    void FileOutput::close() {
        if (INVALID_HANDLE_VALUE != (HANDLE) handle) {
            CloseHandle((HANDLE) handle);
            handle = (void*) INVALID_HANDLE_VALUE;
        }
    }

    size_t FileInput::size() const {
        assert(INVALID_HANDLE_VALUE != handle);
        return GetFileSize((HANDLE) handle, 0);
    }

    size_t FileInput::pos() {
        assert(INVALID_HANDLE_VALUE != handle);
        return SetFilePointer((HANDLE) handle, 0, nullptr, FILE_CURRENT);
    }

    bool FileInput::seek(size_t pos) {
        assert(INVALID_HANDLE_VALUE != handle);
        LARGE_INTEGER distance;
        distance.QuadPart = pos;
        return SetFilePointer((HANDLE) handle, distance.u.LowPart, &distance.u.HighPart, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
    }

    bool FileInput::read(void* data, size_t size) {
        assert(INVALID_HANDLE_VALUE != handle);
        DWORD read = 0;
        BOOL success = ReadFile((HANDLE) handle, data, (DWORD) size, (LPDWORD) &read, nullptr);
        return success && size == read;
    }

    void FileOutput::flush() {
        assert(handle != nullptr);
        FlushFileBuffers((HANDLE) handle);
    }

    bool FileOutput::write(const void* data, size_t size) {
        assert(handle != INVALID_HANDLE_VALUE);
        size_t written = 0;
        WriteFile((HANDLE) handle, data, (DWORD) size, (LPDWORD) &written, nullptr);
        error = error | size != written;
        return !error;
    }

#endif

  /**
   * An async operation to be performed.
   * For reading files from disk into memory.
   */

  struct AsyncRead {
    enum class Flags : uint32_t {
      FAILED = 0,     // The read failed due to some error.
      CANCELLED       // The read was cancelled due to the resource not being needed any more.
    };

    AsyncRead() : data() {}

    bool isFailed() const { return flags == Flags::FAILED; }
    bool isCancelled() const { return flags == Flags::CANCELLED; }

    FileSystem::ContentCallback callback;
    OutputMemoryStream data;
    std::string path;
    uint32_t id = 0;
    Flags flags;
  };

  // The FileSystem that operates on raw on-disk files.
  struct DiskFS;

  struct DiskFS : FileSystem {

    explicit DiskFS(std::string& path) : sem(0, 0xffff) {
        setBasePath(path);
    }

    bool hasWork() override {
        return workCounter != 0;
    }

    std::string const& getBasePath() const override { return basePath; }
    void setBasePath(std::string& path) final {
        basePath = Path::normalise(path);
        if (!basePath.ends_with('/') && !basePath.ends_with('\\'))
            basePath.append("/");
    }

    bool saveSync(const Path& path, const uint8_t* data, const size_t size) override {
        FileOutput file;
        std::string fullPath(basePath.append(path.c_str()));

        if (!file.open(fullPath)) return false;
        bool res = file.write(data, size);
        file.close();

        return res;
    }

    bool readSync(const Path& path, struct OutputMemoryStream& content) override {
        FileInput file;
        std::string fullPath(basePath.append(path.c_str()));

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

        MutexGuard lock(mutex);
        workCounter++;

        AsyncRead& read = queue.emplace_back();
        if (++lastID == 0) lastID++;

        read.id = lastID;
        read.path = file.c_str();
        read.callback = callback;
        sem.raise();

        return AsyncHandle(read.id);
    }

    void cancelAsync(AsyncHandle& handle) override {
        MutexGuard lock(mutex);

        for (AsyncRead& read : queue) {
            if (read.id == handle.value) {
                read.flags = AsyncRead::Flags::CANCELLED;
                workCounter--;
                return;
            }
        }

        for (AsyncRead& read : finished) {
            if (read.id == handle.value) {
                read.flags = AsyncRead::Flags::CANCELLED;
                return;
            }
        }
    }

    bool open(std::string& path, FileInput& file) override {
        return file.open(basePath.append(path));
    }

    bool open(std::string& path, FileOutput& file) override {
        return file.open(basePath.append(path));
    }

    bool deleteFile(std::string& path) override {
        return std::remove((basePath.append(path).c_str()));
    }

    bool moveFile(std::string& from, std::string& to) override {
        try {
            std::rename(basePath.append(from).c_str(), basePath.append(to).c_str());
        } catch (std::filesystem::filesystem_error& e) {
            return false;
        }
        return true;
    }

    bool copyFile(std::string& from, std::string& to) override {
        try {
            std::filesystem::copy(basePath.append(from).c_str(), basePath.append(to).c_str());
        } catch (std::filesystem::filesystem_error& e) {
            return false;
        }

        return true;
    }

    bool fileExists(std::string& path) override {
        return std::filesystem::exists(path);
    }

    size_t getLastModified(std::string& path) override {
        return std::filesystem::last_write_time(path).time_since_epoch().count();
    }

    // TODO: File iterators

    void processCallbacks() override {
        // TODO: Timeout this function!
        for (;;) {
            mutex.enter();
            if (finished.empty() || workCounter == 0) {
                mutex.exit();
                break;
            }

            AsyncRead item = finished[0];
            finished.erase(finished.begin());
            --workCounter;

            mutex.exit();

            if (!item.isCancelled())
                item.callback.invoke(item.data.size(), (const uint8_t*) item.data.data(), !item.isFailed());
        }
    }

    // TODO: Run Management
    std::string basePath;
    std::vector<AsyncRead> queue;
    uint64_t workCounter;
    std::vector<AsyncRead> finished;
    Mutex mutex;
    Semaphore sem;

    uint32_t lastID;

  };

  struct VFS : DiskFS {
    VFS(std::string& root_pack_path) : DiskFS((std::string &) "vfs:/") {
        if (!pack.open(root_pack_path)) {
            spdlog::error("Unable to open " + root_pack_path + ", please check paths");
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
        std::string basename = Path::getFilename(const_cast<std::string &>(path.get()));
        PathHash hash = path.getHash();

        auto i = packFiles.find(hash);
        if (i == packFiles.end()) return false;

        content.resize(i->second.size);
        MutexGuard lock(mutex);

        const size_t headerSize = sizeof(uint32_t) + packFiles.size() * (3 * sizeof(size_t));
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

  std::unique_ptr<FileSystem> FileSystem::createDiskFS(const std::string& basePath) {
      return std::make_unique<DiskFS>(basePath);
  }

  std::unique_ptr<FileSystem> FileSystem::createVFS(const std::string& basePath) {
      return std::make_unique<VFS>(basePath);
  }
}