#include "temp/model/Loader.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <sstream>

namespace vlkxtemp {

    std::vector<std::string> split (std::string_view s, char delim) {
        std::vector<std::string> result;
        std::stringstream ss ((std::string(s)));
        std::string item;

        while (std::getline (ss, item, delim)) {
            result.push_back (item);
        }

        return result;
    }

    Wavefront::Wavefront(std::string_view path, size_t index_base) {
        std::ifstream file(std::string(path), std::ios::binary);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> tex_coords;
        std::map<std::string, uint32_t> loaded_vertices;

        const auto parse_line = [&](std::string_view line) {
            const size_t non_space = line.find_first_not_of(' ');
            if (non_space == std::string::npos || line[0] == '#')
                return;

            switch (line[non_space]) {
                case 'v': {
                    switch (line[non_space + 1]) {
                        case ' ': {
                            const auto nums = split(line.substr(non_space + 2), ' ');
                            positions.emplace_back(stof(nums[0]), stof(nums[1]), stof(nums[2]));
                            break;
                        }
                        case 'n': {
                            // Normal.
                            const auto nums = split(line.substr(non_space + 3), ' ');
                            normals.emplace_back(glm::vec3{stof(nums[0]), stof(nums[1]), stof(nums[2])});
                            break;
                        }
                        case 't': {
                            // Texture coordinates.
                            const auto nums = split(line.substr(non_space + 3), ' ');
                            tex_coords.emplace_back(glm::vec2{stof(nums[0]), stof(nums[1])});
                            break;
                        }
                        default:
                            throw std::runtime_error("Unexpected symbol " + std::to_string(line[non_space + 1]));
                    }
                    break;
                }
                case 'f': {
                    for (const auto& seg : split(line.substr(non_space + 2), ' ')) {
                        const auto iter = loaded_vertices.find(seg);
                        if (iter != loaded_vertices.end()) {
                            indices.push_back(iter->second);
                        } else {
                            indices.push_back(vertices.size());
                            loaded_vertices[seg] = vertices.size();
                            const auto idxs = split(seg, '/');
                            vertices.push_back(Geo::VertexAll {
                                    positions.at(stoi(idxs[0]) - index_base),
                                    normals.at(stoi(idxs[2]) - index_base),
                                    tex_coords.at(stoi(idxs[1]) - index_base),
                            });
                        }
                    }
                    break;
                }
                default:
                    throw std::runtime_error("Unexpected symbol in OBJ file: " + std::to_string(line[non_space]));
            }
        };

        std::string line;
        int line_num = 1;
        try {
            for (; std::getline(file, line); ++line_num)
                parse_line(line);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to parse obj file, error on line " + std::to_string(line_num) + ": " + line + "; " + e.what());
        }
    }

    ModelLoader::ModelLoader(const std::string &model, const std::string &textures) {

    }
}