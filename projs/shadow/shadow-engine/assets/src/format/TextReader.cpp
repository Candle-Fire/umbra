#include <shadow/assets/format/TextReader.h>

#include "spdlog/spdlog.h"

namespace SH {
    static bool isSpace(char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
    }

    static bool isIdentifier(char c) {
        return c == '_' || isalnum(c);
    }

    TextReader::TextReader(const std::string& content, const char* filename) : content(content), ptr(content.c_str()), filename(filename) {
    }

    uint32_t TextReader::getLine() const {
        size_t line = 1;
        for (const char* c = content.c_str(); c < ptr; c++)
            if (*c == '\n') line++;
        return line;
    }

    TextReader::Token TextReader::nextToken() {
        Token t = tryNextToken();
        if (t.type == Token::END_OF_FILE)
            spdlog::error("[%s:%d] unexpected end of file", filename, getLine());
        return t;
    }

    TextReader::Token TextReader::tryNextToken(TextReader::Token::Type t) {
        Token tk = tryNextToken();
        if (!tk) return tk;
        if (tk.type != t) {
            spdlog::error("[%s:%d] Unexpected token %s", filename, getLine(), tk.value);
            logErrorPosition(tk.value.c_str());
            return Token::ERRORED;
        }
        return tk;
    }

    TextReader::Token TextReader::tryNextToken() {
        // Skip whitespace.
        while (ptr < &*content.end() && isSpace(*ptr)) ptr++;
        // Ensure the file wasn't whitespace all the way down.
        if (ptr >= &*content.end()) return Token::END_OF_FILE;

        // Check if we have a string.
        if (*ptr == '"') {
            Token t(Token::STRING_LITERAL);
            ptr++;
            // Set the start of the token string view to the first char of the string
            t.value = ptr;

            // Seek for the end of the string
            while (ptr < &*content.end() && *ptr != '"') ptr++;

            if (ptr >= &*content.end()) {
                spdlog::error("[%s:%d] unexpected end of file", filename, getLine());
                return Token::ERRORED;
            }

            t.value.resize(ptr - t.value.c_str());
            ptr++;
            return t;
        }

        bool neg = false;
        if (*ptr == '-') {
            ptr++;
            if (ptr >= &*content.end() || !isdigit(*ptr)) {
                Token t(Token::SYMBOL);
                t.value = ptr - 1;
                t.value.resize(1);
                return t;
            }
            neg = true;
        }

        if (isdigit(*ptr)) {
            Token t(Token::NUMBER);
            t.value = neg ? ptr - 1 : ptr;
            // Skip to the end of the number
            while (ptr < &*content.end() && isdigit(*ptr)) ptr++;
            if (ptr < &*content.end()) {
                // Handle the decimal
                if (*ptr == '.') {
                    ptr++;
                    while (ptr < &*content.end() && isdigit(*ptr)) ptr++;
                }
                if (ptr < &*content.end() && isIdentifier(*ptr)) {
                    spdlog::error("[%s:%d] Unexpected character in Number: %c", filename, getLine(), *ptr);
                    logErrorPosition(ptr);
                    return Token::ERRORED;
                }
            }
            t.value.resize(ptr - t.value.c_str());
            return t;
        }

        // Strings and numbers handled, all other non-identifiers are symbols.
        if (!isIdentifier(*ptr)) {
            Token t(Token::SYMBOL);
            t.value = ptr;
            ptr++;
            t.value.resize(1);
            return t;
        }

        // All that's left are identifiers.

        Token t(Token::IDENTIFIER);
        t.value = ptr;
        while (ptr < &*content.end() && isIdentifier(*ptr)) ptr++;
        t.value.resize(ptr - t.value.c_str());
        return t;
    }

    // TODO: This is a linear search each line. Surely this can be done better!
    std::string TextReader::readLine(size_t line) {
        size_t cnt = 0;
        const char* start = content.c_str();
        for (const char* c = content.c_str(); c < &*content.end(); c++) {
            if (*c == '\n') {
                cnt++;
                if (cnt == line)
                    return std::string(start, c - start);
                else
                    start = c;
            }
        }

        if (cnt == line - 1)
            return std::string(start, &*content.end() - start);
        else
            return std::string();
    }

    void TextReader::logErrorPosition(const char* loc) {
        assert(loc >= &*content.begin() && loc < &*content.end());
        const char* start = loc;
        // Find the start of the line that loc points into
        while (start > &*content.begin() && start[-1] != '\n') --start;

        size_t line = getLine();
        // Print context
        spdlog::error("%-4d | %s", line-2, readLine(line - 2));
        spdlog::error("%-4d | %s", line-1, readLine(line - 1));
        spdlog::error("%-4d | %s", line, readLine(line));
        // Print the pointer
        size_t offset = loc - start;
        std::string tmp;
        for (size_t i = 0; i < offset; i++) tmp[i] = ' ';
        tmp[offset] = '^';
        spdlog::error("     | %s", tmp);
        spdlog::error("%-4d | %s", line+1, readLine(line + 1));
        spdlog::error("%-4d | %s", line+2, readLine(line + 2));
    }

    float TextReader::toFloat(Token t) {
        assert(t.type == Token::NUMBER);
        return atof(t.value.c_str());
    }


    bool TextReader::consume(bool& out) {
        Token t = nextToken();
        if (!t) return false;

        if (t.value == "true") { out = true; return true; }
        if (t.value == "false") { out = false; return true; }

        spdlog::error("[%s:%d] Expected boolean, got %s", filename, getLine(), t.value.c_str());
        logErrorPosition(t.value.c_str());
        return false;
    }

    bool TextReader::consume(float& out) {
        Token t = nextToken();
        if (!t) return false;

        if (t.type == Token::NUMBER) {
            out = atof(t.value.c_str());
            return true;
        }

        spdlog::error("[%s:%d] Expected Float Number, got %s", filename, getLine(), t.value.c_str());
        logErrorPosition(t.value.c_str());
        return false;
    }

    bool TextReader::consume(uint32_t& out) {
        Token t = nextToken();
        if (!t) return false;

        if (t.type == Token::NUMBER) {
            char* end = &*t.value.end();
            out = strtoul(t.value.c_str(), &end, 10);
            return true;
        }

        spdlog::error("[%s:%d] Expected Unsigned Number, got %s", filename, getLine(), t.value.c_str());
        logErrorPosition(t.value.c_str());
        return false;
    }

    bool TextReader::consume(int32_t& out) {
        Token t = nextToken();
        if (!t) return false;

        if (t.type == Token::NUMBER) {
            char* end = &*t.value.end();
            out = atoi(t.value.c_str());
            return true;
        }

        spdlog::error("[%s:%d] Expected Signed Number, got %s", filename, getLine(), t.value.c_str());
        logErrorPosition(t.value.c_str());
        return false;
    }

    bool TextReader::consume(std::string& out) {
        Token t = nextToken();
        if (!t) return false;

        if (t.type != Token::STRING_LITERAL && t.type != Token::IDENTIFIER) {
            spdlog::error("[%s:%d] Expected String or Identifier, got %s", filename, getLine(), t.value.c_str());
            logErrorPosition(t.value.c_str());
            return false;
        }

        out = t.value;
        return true;
    }

    bool TextReader::consume(const char* val) {
        Token t = nextToken();
        if (!t) return false;

        if (t.value == val) return true;

        spdlog::error("[%s:%d] Expected string literal %s, got %s.", filename, getLine(), val, t.value.c_str());
        logErrorPosition(t.value.c_str());
        return false;
    }

    bool TextReader::consume(Vec3& out) {
        float tmp[4];
        uint32_t size;
        if (!consumeVec(tmp, size)) return false;

        if (size != 3) {
            spdlog::error("[%s:%d]: Expected 3 vector elements, got %d.", filename, getLine(), size);
            logErrorPosition(ptr);
            return false;
        }

        memcpy(&out, tmp, sizeof(out));
        return true;
    }


    bool TextReader::consumeVec(float* out, uint32_t& size) {
        size_t i;
        for (i = 0; i < 5; i++ ) {
            Token v = nextToken();
            if (!v) return false;

            // After the first entry, we expect either a break '}', a terminating ',' or the end of the list
            if (i > 0) {
                if (v.value[0] == '}') break;

                if (i == 4) {
                    spdlog::error("[%s:%d]: Expected }} to terminate vector, got %s.", filename, getLine(), v.value);
                    logErrorPosition(v.value.c_str());
                    return false;
                }

                if (v.value[0] != ',') {
                    spdlog::error("[%s:%d]: Expected ',' to terminate entry in a vector, got %s", filename, getLine(), v.value);
                    logErrorPosition(v.value.c_str());
                    return false;
                }

                v = nextToken();
                if (!v) return false;
            } else if (v.value[0] == '}') {
                spdlog::error("[%s:%d]: Expected number in vector, got a closing }} before any numbers were parsed: %s", filename, getLine(), v.value);
                logErrorPosition(v.value.c_str());
                return false;
            }

            // We should now have the number between { , } tokens
            if (v.type != Token::NUMBER) {
                spdlog::error("[%s:%d] Expected number in vector list, got %s", filename, getLine(), v.value.c_str());
                logErrorPosition(v.value.c_str());
                return false;
            }

            out[i] = atof(v.value.c_str());
        }

        size = i;
        return true;
    }

    TextReader::Variant TextReader::consumeVariant() {
        Variant v;
        Token t = nextToken();
        if (!t) return v;

        if (t.type == Token::NUMBER) {
            v.type = Variant::NUMBER;
            v.number = atof(t.value.c_str());
            return v;
        }

        if (t.type == Token::STRING_LITERAL) {
            v.type = Variant::STRING;
            v.string = t.value;
            return v;
        }

        if (t.value[0] == '{') {
            if (!consume(v.vector[0], ",", v.vector[1])) return {};
            Token iter = tryNextToken();
            if (iter == "}") {
                v.type = Variant::VEC2;
                return v;
            } else if (iter != ",") {
                spdlog::error("[%s:%d]: Expected ',' or '}}' in vector2, got %s.", filename, getLine(), iter.value);
                logErrorPosition(iter.value.c_str());
                return {};
            }

            if (!consume(v.vector[2])) return {};

            iter = tryNextToken();
            if (iter == "}") {
                v.type = Variant::VEC3;
                return v;
            } else if (iter != ",") {
                spdlog::error("[%s:%d]: Expected ',' or '}}' in vector3, got %s.", filename, getLine(), iter.value);
                logErrorPosition(iter.value.c_str());
                return {};
            }

            if (!consume(v.vector[3], "}")) return {};
            v.type = Variant::VEC4;
            return v;
        }

        spdlog::error("[%s:%d]: Unexpected token after assumed end of vector: %s", filename, getLine(), t.value);
        logErrorPosition(t.value.c_str());
        return v;
    }

    bool TextReader::parse(std::string content, const char* path, std::vector<TextEntry> entries) {
        TextReader t(content, path);
        for (;;) {                                                         // <-- Loop start
            Token tk = t.tryNextToken(Token::IDENTIFIER);
            if (!tk) return false;                                         // <-- Return from loop

            bool found = false;
            for (const TextEntry& e : entries) {
                if (tk == e.name) {
                    found = true;
                    if (!t.consume("=")) return false;                 // <-- Return from loop
                    switch (e.type) {
                    case TextEntry::BOOL:
                        if (!t.consume(*e.boolVal)) return false;       // <-- Return from loop
                        break;
                    case TextEntry::INT32:
                        if (!t.consume(*e.i32Val)) return false;        // <-- Return from loop
                        break;
                    case TextEntry::UINT32:
                        if (!t.consume(*e.u32Val)) return false;        // <-- Return from loop
                        break;
                    case TextEntry::FLOAT:
                        if (!t.consume(*e.floatVal)) return false;      // <-- Return from loop
                        break;
                    case TextEntry::STRING:
                        if (!t.consume(*e.stringVal)) return false;     // <-- Return from loop
                        break;
                    case TextEntry::ARRAY: {
                        Token n = t.nextToken();
                        if (!n) return false;                              // <-- Return from loop
                        if (n.value[0] != '[') {
                            spdlog::error("[%s:%d]: Expected [ to begin array, got %s.", path, t.getLine(), n.value);
                            t.logErrorPosition(n.value.c_str());
                            return false;                                  // <-- Return from loop
                        }
                        *e.stringVal = n.value;

                        uint32_t depth = 1; // We've already parsed the first [, so we're already at depth 1 inside the array.

                        // Continue recursing into the array, adding up instances of [, until we reach the ] corresponding to this entry.
                        // TODO: this does no verification to what we find inside the array. It's treated as a single string.
                        for (;;) {
                            n = t.nextToken();
                            if (!n) return false;                        // <-- Return from loop
                            if (n.value[0] == '[') depth++;
                            else if (n.value[0] == ']') {
                                depth--;
                                if (depth == 0) {
                                    e.stringVal->resize(&*n.value.end() - e.stringVal->c_str());
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    }
                }
            }

            if (!found) {
                spdlog::error("[%s:%d]: Found token that wasn't in the list of entries we expected to find in this file: %s", t.filename, t.getLine(), tk.value);
                t.logErrorPosition(tk.value.c_str());
            }
        }                                                                   // <-- Loop end
    }
}
