#ifndef M3U_GENERATOR_H
#define M3U_GENERATOR_H

#include <filesystem>
#include <string>
#include <optional>
#include <set>
#include <vector>
#include <stdexcept>

namespace m3u {

struct GeneratorConfig {
    std::vector<std::string> source_directories;
    std::string output_directory;
    std::optional<std::string> prefix_path;  // Optional: prepending path
    std::optional<std::set<std::string>> file_extensions;  // Optional: only scan specific extensions

    static GeneratorConfig fromArgs(int argc, char* argv[]) {
        if (argc < 3) {
            throw std::runtime_error("Usage: m3u_generator <directory_1> <directory_2> ... <output_directory>");
        }

        GeneratorConfig config;
        for (int i = 1; i < argc - 1; ++i) {
            config.source_directories.push_back(argv[i]);
        }
        config.output_directory = argv[argc - 1];
        return config;
    }
};

// Validate directory exists and is a directory
bool validateDirectory(const std::string& dir);

// URL encode path using libcurl's curl_easy_escape
std::string encodeURL(const std::string& path);

// Get usage manual string
std::string usageManualString();

// Print usage to stdout
void printUsageManual();

}  // namespace m3u

#endif // M3U_GENERATOR_H
