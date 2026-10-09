#include <filesystem>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <curl/curl.h>

#include "m3u_generator.h"

namespace fs = std::filesystem;

// Implementation of validateDirectory from header
bool m3u::validateDirectory(const std::string& dir) {
    if (!fs::is_directory(dir)) {
        std::cerr << "Error: Provided directory \"" << dir << "\" is not a directory." << std::endl;
        std::cerr << m3u::usageManualString() << std::endl;
        return false;
    }
    return true;
}

// Implementation of encodeURL from header
std::string m3u::encodeURL(const std::string& path) {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string encoded = curl_easy_escape(curl, path.c_str(), 0);
        curl_free(curl);
        return encoded;
    }
    return path;  // fallback if curl init fails
}

// Implementation of usageManualString from header
std::string m3u::usageManualString() {
    return 
        "Usage: m3u_generator <directory_1> <directory_2> ... <output_directory>\n"
        "\n"
        "Parameters:\n"
        "  directories:   Directories to find media files within.\n"
        "  output_directory: Directory which to put the generated m3u playlists.\n"
        "\n";
}

// Implementation of printUsageManual from header
void m3u::printUsageManual() {
    std::cout << m3u::usageManualString();
}

int main(int argc, char* argv[]) {
    // Parse configuration using GeneratorConfig::fromArgs
    m3u::GeneratorConfig config;
    
    try {
        config = m3u::GeneratorConfig::fromArgs(argc, argv);
        
        // Validate all directories
        for (const auto& dir : config.source_directories) {
            if (!m3u::validateDirectory(dir)) {
                return 1;
            }
        }
        
        // Initialize libcurl
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Error: Failed to initialize curl" << std::endl;
            return 1;
        }
        
        // Prepare base paths
        std::string prependingPath = config.prefix_path.value_or("");
        std::string outputPath = config.output_directory;
        
        // Scan directories and collect files
        std::map<std::string, std::set<std::string>> directoryFiles;
        
        for (const auto& directory : config.source_directories) {
            directoryFiles[directory] = {};
            
            try {
                for (const auto& file : fs::directory_iterator(directory)) {
                    if (file.is_regular_file()) {
                        const std::string filename = file.path().filename().string();
                        directoryFiles[directory].insert(filename);
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not scan directory \"" << directory 
                          << "\": " << e.what() << std::endl;
            }
        }
        
        // Generate individual playlists for each directory
        for (const auto& [directoryName, files] : directoryFiles) {
            std::string outputPathDir = outputPath + "/" + directoryName + ".m3u";
            
            std::ofstream outputStream(outputPathDir);
            if (!outputStream) {
                std::cerr << "Error: Cannot write playlist \"" << directoryName 
                          << ".m3u\"" << std::endl;
                curl_free(curl);
                return 1;
            }
            
            outputStream << "#EXTM3U" << std::endl;
            
            for (const auto& filename : files) {
                std::string topDirWithFile = directoryName + "/" + filename;
                std::string urlEncoded = m3u::encodeURL(topDirWithFile);
                
                outputStream << "#EXTINF:0," << filename << std::endl;
                outputStream << prependingPath + directoryName + "/" + urlEncoded << std::endl;
            }
            
            outputStream.close();
        }
        
        // Generate total playlist
        std::string totalOutputPath = outputPath + "/Total.m3u";
        std::ofstream totalOutputStream(totalOutputPath);
        
        if (!totalOutputStream) {
            std::cerr << "Error: Cannot write Total.m3u" << std::endl;
            curl_free(curl);
            return 1;
        }
        
        totalOutputStream << "#EXTM3U" << std::endl;
        
        for (const auto& [directoryName, files] : directoryFiles) {
            for (const auto& filename : files) {
                std::string topDirWithFile = directoryName + "/" + filename;
                std::string urlEncoded = m3u::encodeURL(topDirWithFile);
                
                totalOutputStream << "#EXTINF:0," << filename << std::endl;
                totalOutputStream << prependingPath + directoryName + "/" + urlEncoded << std::endl;
            }
        }
        
        totalOutputStream.close();
        
        curl_free(curl);
        
        std::cout << "Playlists generated successfully in \"" << outputPath << "\"" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
