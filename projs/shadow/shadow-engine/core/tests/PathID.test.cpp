#include "catch2/catch.hpp"

#include "shadow/core/PathID.h"

using Path = SH::Path;

TEST_CASE("Path normalization", "[Path]") {
    SECTION("Normalize path with backslashes and namespace") {
        std::string id = "example@namespace:C:\\path\\to\\file";
        REQUIRE(Path::normalise(id) == "example@namespace:C:/path/to/file");
    }

    SECTION("Normalize path with backslashes, no namespace") {
        std::string id = "example:C:\\path\\to\\file";
        REQUIRE(Path::normalise(id) == "example:C:/path/to/file");
    }

    SECTION("Normalize path already using forward slashes and namespace") {
        std::string id = "example@namespace:/already/normalized";
        REQUIRE(Path::normalise(id) == "example@namespace:/already/normalized");
    }

    SECTION("Normalize path already using forward slashes, no namespace") {
        std::string id = "example:/already/normalized";
        REQUIRE(Path::normalise(id) == "example:/already/normalized");
    }

    SECTION("Invalid path with no colon") {
        std::string id = "invalidpath";
        REQUIRE_THROWS_AS(Path::normalise(id), std::invalid_argument);
    }
}

TEST_CASE("Path getPrelude", "[Path]") {
    SECTION("Get prelude from path") {
        std::string id = "example@namespace:/path/to/file";
        std::string prelude = SH::Path::getPrelude(id);
        REQUIRE(prelude == "example");
    }
}

TEST_CASE("Path getDomain", "[Path]") {
    SECTION("Get domain from path with domain") {
        std::string id = "example@namespace:/path/to/file";
        REQUIRE(Path::getDomain(id) == "namespace");
    }

    SECTION("Get domain from path without domain") {
        std::string id = "example:/path/to/file";
        REQUIRE(Path::getDomain(id).empty());
    }

    SECTION("Invalid path with no colon") {
        std::string id = "invalidpath";
        REQUIRE_THROWS_AS(Path::getDomain(id), std::invalid_argument);
    }

    SECTION("Path with domain but no colon") {
        std::string id = "example@namespace";
        REQUIRE_THROWS_AS(Path::getDomain(id), std::invalid_argument);
    }
}

TEST_CASE("Path getDirectory", "[Path]") {
    SECTION("Get directory from path") {
        std::string id = "example@namespace:/path/to/file";
        std::string directory = SH::Path::getDirectory(id);
        REQUIRE(directory == "/path/to");
    }
}

TEST_CASE("Path getFilename", "[Path]") {
    SECTION("Get filename from path") {
        std::string id = "example@namespace:/path/to/file.txt";
        std::string filename = SH::Path::getFilename(id);
        REQUIRE(filename == "file.txt");
    }
}

TEST_CASE("Path getExtension", "[Path]") {
    SECTION("Get extension from path") {
        std::string id = "example@namespace:/path/to/file.txt";
        std::string extension = SH::Path::getExtension(id);
        REQUIRE(extension == "txt");
    }
}

TEST_CASE("Path hasExtension", "[Path]") {
    SECTION("Check if path has given extension") {
        std::string id = "example@namespace:/path/to/file.txt";
        std::string ext = "txt";
        bool result = SH::Path::hasExtension(id, ext);
        REQUIRE(result == true);
    }
}

TEST_CASE("Path replaceExtension", "[Path]") {
    SECTION("Replace extension of path") {
        std::string id = "example@namespace:/path/to/file.txt";
        std::string newExt = "jpg";
        std::string newID = SH::Path::replaceExtension(id, newExt);
        REQUIRE(newID == "example@namespace:/path/to/file.jpg");
    }
}