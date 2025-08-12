
#include <filesystem>
#include <set>
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <curl/curl.h>

using namespace std;
namespace fs = std::filesystem;

constexpr string usageManualString() {

    return 
        "Usage: m3u_generator <directory_1> <directory_2> <prefix_path>" 
        "<output_directory>" 
        "\n"
        "\n"
        "Parameters"
        "\n"
        "----------"
        "\n\n"
        "directories:      Directories to find media files within."
        "\n"
        "prefix_path:      The path which will be prefixed onto each file found"
        " within the specified directories in the final output playlists."
        "\n"
        "output_directory: Directory which to put the generated m3u playlists."
        "\n"
        "\n";
}

static void validateDirectory(const string dir) {

    if (!fs::is_directory(dir)) {

        throw runtime_error(
            "Provided directory " +
            dir +
            " is not a directory. \n\n" +
            usageManualString()
        );
    }
}

void printUsageManual() {
    cout << usageManualString();
}

int main(int numberArgs, char* args[]) {

    // todo
    // Validation for file extensions.
    // Optional prefix_path.
    // Optional total playlist, series playlists.

    bool error = false;

    if (numberArgs < 4) {
        cout << 
            "ERROR: Must specify at least one directory to generate from. " <<
            endl;
        printUsageManual();
        error = true;
        return error;
    }

    const string prependingPath = args[numberArgs - 2];
    const string outputPath = args[numberArgs - 1];

    validateDirectory(outputPath);
    
    for (int i = 1; i < numberArgs - 2 ; i ++) {

        validateDirectory(args[i]);
    }

    map<string, set<string>> files;

    for (int i = 1; i < numberArgs - 2; i ++) {

        string directory = string(args[i]);
        string topLevelDirectory = directory.substr(directory.find_last_of("/"));

        for ( const auto& file : fs::directory_iterator(directory)) {

            if (file.is_regular_file()) {

                const string filename = file.path().filename().string();
                cout << "Processing: " << filename << endl;

                files[topLevelDirectory].insert(filename);
            }
        }
    }

    CURL* curl = curl_easy_init();

    // Generate file by file
    for (const auto& [directoryName, files] : files) {

        ofstream outputStream(outputPath + directoryName + ".m3u");

        if (!outputStream) {
            throw runtime_error("Cannot write " + directoryName);
        }

        outputStream << "#EXTM3U" << endl;

        for ( const string filename : files) {

            string topDirWithFile(filename);
            string urlEncoded = curl_easy_escape(curl, topDirWithFile.c_str(), 0);

            outputStream << "#EXTINF:0," << filename << endl;
            outputStream << prependingPath + directoryName + "/" + urlEncoded << endl;
        }
    }

    ofstream totalOutputStream(outputPath + "Total.m3u");

    if (!totalOutputStream) {
        throw runtime_error("Cannot write Total.m3u");
    }

    totalOutputStream << "#EXTM3U" << endl;

    // Generate total playlist
    for (const auto& [directoryName, files] : files) {

        for ( const string filename : files) {

            string topDirWithFile(filename);
            string urlEncoded = curl_easy_escape(curl, topDirWithFile.c_str(), 0);

            totalOutputStream << "#EXTINF:0," << filename << endl;
            totalOutputStream << prependingPath + directoryName + "/" + urlEncoded << endl;
        }
    }

    curl_free(curl);

    return error;
}