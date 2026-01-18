#pragma once
#include <memory>
#include "iostream.h"
#include "path.h"
#include "shadow/assets/management/delegate.h"

namespace SH {

    // An input stream that can read a file on disk.
    struct FileInput final : InputStream {
        FileInput();
        FileInput(const Path& p);
        ~FileInput();

        [[nodiscard]] bool open(const Path& path);
        void close();

        using InputStream::read;
        [[nodiscard]] bool read(void* data, size_t size) override;
        const void* getBuffer() const override { return nullptr; }

        size_t size() const override;
        size_t pos();

        [[nodiscard]] bool seek(size_t pos);

    private:
        void* handle;
    };

    // An output stream that can write to a file on disk.
    // Created three ways:
    //  1. FileOutput f(path);
    //  2. FileOutput f;
    //     f.open(path);
    //  3. FileOutput f;
    //     fs.open(path, &f);
    struct FileOutput final : OutputStream {
        FileOutput();
        FileOutput(const Path& path);

        ~FileOutput();

        [[nodiscard]] bool open(const Path& path);
        [[nodiscard]] bool touch(const Path& path);
        void close();
        void flush();
        bool errored() const { return error; }
        using OutputStream::write;
        [[nodiscard]] bool write(const void* data, size_t size) override;

    private:
        FileOutput(const FileOutput&) = delete;
        void* handle;
        bool error;
    };

    struct FileInfo {
        bool directory;
        std::string filename;
    };


    struct FileIterator {
        uint32_t offset = 0;
        bool isFirstElement = true;
        alignas(8) uint8_t buffer[4098];
        void* handle;

        bool getNext(FileInfo* f);
    };

    /**
     * A generic Filesystem API.
     * Allows interacting with files on disk the same as files in our Virtual Package Format.
     */
    struct FileSystem {
        // A function called when the data of a file is updated, such as when an asynchronous operation completes.
        using ContentCallback = Delegate<void(size_t, const uint8_t*, bool)>;
        // A handle for asynchronous data movement; such as reading or writing a file.
        struct AsyncHandle {
            static AsyncHandle invalid() { return AsyncHandle(0xffffffff); }
            explicit AsyncHandle(uint32_t val) : value(val) {}

            [[nodiscard]] bool valid() const { return value != 0xffffffff; }

            uint32_t value;
        };

        // Create a Filesystem that interacts with files on disk.
        static std::unique_ptr<FileSystem> createDiskFS(const Path& basePath);
        // Create a Virtual Filesystem based on the given path.
        static std::unique_ptr<FileSystem> createVFS(const Path& basePath);

        virtual ~FileSystem() = default;

        // Open a file for reading.
        virtual bool open(const Path& path, FileInput& input) = 0;
        // Open a file for writing.
        virtual bool open(const Path& path, FileOutput& output) = 0;
        // Create a File Iterator, to scan over all the files in a given directory.
        virtual FileIterator* iterateDirectory(const Path& dir) = 0;
        // Destroy a created File Iterator.
        virtual void destroyIterator(FileIterator* iter) = 0;
        // Check whether a file exists at the given path.
        virtual bool fileExists(const Path& path) = 0;
        // Check whether a directory exists at the given path.
        virtual bool dirExists(const Path& p) = 0;
        // Get the time a file at the given path was last modified.
        virtual size_t getLastModified(const Path& path) = 0;
        // Copy a file from one path to another.
        virtual bool copyFile(const Path& from, const Path& to) = 0;
        // Move a file from one path to another.
        virtual bool moveFile(const Path& from, const Path& to) = 0;
        // Disassociate any files at the given path (not an immediate delete)
        virtual bool deleteFile(const Path& path) = 0;

        // Get the path that this FileSystem originates at. The default is "/" for VFS, and whatever the Executable Path is for Disk FS.
        virtual Path const& getBasePath() const = 0;
        // Set a new base path for the FileSystem. Any operations involving file paths will be relative to this new path.
        virtual void setBasePath(const Path& path) = 0;

        // Process all the callbacks for async file operations.
        virtual void processCallbacks() = 0;
        // Check whether there are any outstanding async operations that need work.
        virtual bool hasWork() = 0;

        // Write new content to a file synchronously. The thread will be blocked when doing this.
        virtual bool saveSync(const Path& file, const uint8_t* content, size_t size) = 0;
        // Read content from a file synchronously. The thread will be blocked when doing this.
        virtual bool readSync(const Path& file, struct OutputMemoryStream& content) = 0;

        // Read a file asynchronously. The given callback will be called with the file content once it is available.
        virtual AsyncHandle readAsync(const Path& file, const ContentCallback& callback) = 0;
        // Cancel an asynchronous operation, if it is not already complete. The associated callback will be called with a special flag.
        virtual void cancelAsync(AsyncHandle& handle) = 0;
    };
}
