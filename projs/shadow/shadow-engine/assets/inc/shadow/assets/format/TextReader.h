#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace SH {
    /**
     * A utility class for parsing structured text files.
     * Intended for the Resource Database in the Asset System, but can be used for any file containing key-value pairs.
     *
     * The recognized types are:
     *  - Number
     *    (?:[0-9]+|[0-9]*\.[0-9]+)
     *    25 15.6 0.15151515 .5
     *  - String
     *    "any length of text surrounded by double quotes"
     *  - Identifier
     *    [a-z_][a-z0-9_]*
     *    hello _temp snake_case_9000 a
     *  - Symbol
     *    [,./<>?;:'#@~-=_+()*!$%^&\[\]\\\|]
     *  - Vector2 / Vector3 / Vector4
     *    { (Number,){2,3,4} }
     *    {1, 2, 3, 4} {5.6, 7.5, 8.2} {.5, .5}
     *
     * If you have a known set of key-value pairs you're trying to parse out, the static ::parse method might be what you want.
     * If you know there are no symbols or identifiers in the output, you can use Variant to pull out the values automatically without needing to manually check the type and switch.
     */
    struct TextReader {

        struct Vec3 {
            float x,y,z;
        };

        /**
         * A single stream of glyphs as part of a token.
         * Tokens are delineated either by the start of another token, or by a breaking space.
         */
        struct Token {
            enum Type {
                UNDEFINED,
                END_OF_FILE,
                ERRORED,
                NUMBER,
                STRING_LITERAL,
                IDENTIFIER,
                SYMBOL
            };

            Token() : type(UNDEFINED) {}
            Token(Type type) : type(type) {}
            Token(Type type, std::string value) : type(type), value(std::move(value)) {}

            operator bool() const { return type != ERRORED && type != END_OF_FILE; }
            bool operator== (const char* rs) const { return value == std::string(rs); }
            bool operator== (char c) const = delete;

            Type type;
            std::string value;
        };

        /**
         * Variants are a union of three of the possible types in a Token: float, string, vector.
         */
        struct Variant {
            enum Type {
                NONE,
                NUMBER,
                STRING,
                VEC2,
                VEC3,
                VEC4
            };

            Variant() : type(NONE), vector() {
            }

            Type type;
            float number;
            std::string string;
            float vector[4];
        };

        /**
         * A single TextEntry is a single item that can be parsed out of a file.
         * In other words, a fully parsed file is a list of TextEntries.
         */
        struct TextEntry {
            TextEntry(const char* name, bool* val) : name(name), boolVal(val), type(BOOL) {}
            TextEntry(const char* name, float* val) : name(name), floatVal(val), type(FLOAT) {}
            TextEntry(const char* name, std::string* val) : name(name), stringVal(val), type(STRING) {}
            TextEntry(const char* name, int32_t* val) : name(name), i32Val(val), type(INT32) {}
            TextEntry(const char* name, uint32_t* val) : name(name), u32Val(val), type(UINT32) {}

            const char* name;
            union {
                bool* boolVal;
                float* floatVal;
                int32_t* i32Val;
                uint32_t* u32Val;
                std::string* stringVal;
            };

            enum Type {
                BOOL,
                FLOAT,
                INT32,
                UINT32,
                STRING,
                ARRAY
            } type;
        };

        /**
         * Tokenise content, parse it according to entries.
         * content must be a sequence of key = value.
         * If there is an unexpected token, an error is printed to the console.
         * descs is searched linearly, so prefer content to be smaller.
         * @param content the full textual content of a file to be parsed.
         * @param path the location of this file (used only in error output)
         * @param entries a map of parsed entries.
         * @return whether the content was parsed successfully
         */
        static bool parse(std::string content, const char* path, std::vector<TextEntry> entries);

        // Coerce a single token to float type.
        static float toFloat(Token t);

        TextReader(const std::string& content, const char* filename);

        // Get the line number that is currently being read. Starts at 1.
        uint32_t getLine() const;

        // Fetch next token. Prints error to the console if Token.Type == ERRORED.
        Token tryNextToken();
        std::string readLine(size_t line);
        // Fetch next token. Prints error to the console if Token.Type != type.
        Token tryNextToken(Token::Type type);
        // Fetch next token. Prints error to the console if Token.Type == ERRORED or END_OF_FILE.
        Token nextToken();

        // If the next token is a vector, put it in out, write size. If not a vector, print error. Returns whether the token was in fact a vector.
        bool consumeVec(float* out, uint32_t& size);

        // If the next token is not Variant-compatible, the Type is NONE, and an error is printed.
        Variant consumeVariant();

        // If the token is compatible with the output type, the output reference is filled and true returned. Otherwise, error printed to console and false returned.
        bool consume(std::string& out);
        bool consume(uint32_t& out);
        bool consume(int32_t& out);
        bool consume(bool& out);
        bool consume(float& out);
        bool consume(Vec3& out);
        bool consume(const char* val);

        // Read a string of the given fixed size.
        template <int size>
        bool consume(char (&out)[size]) {
            std::string tmp;
            if (!consume(tmp)) return false;
            std::memcpy(out, tmp.c_str(), tmp.length());
            return true;
        }

        // Iteratively consume varargs of the same type.
        template <typename... Args>
        bool consume(Args&... args) {
            return (consume(args) && ...);
        }

        // Pretty-print current text output to the console.
        void logErrorPosition(const char* ptr);

        std::string content;
        const char* ptr;
        const char* filename;
    };

}
