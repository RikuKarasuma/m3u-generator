#include <gtest/gtest.h>
#include "../src/main.cpp"
#include <fstream>

// Test that playlist files are created with correct extension
TEST(PlaylistGenerationTest, OutputFileExtension) {
    const std::string tempDir = "/tmp/m3u_playlist_test";
    const std::string dir1 = tempDir + "/testdir";
    const std::string prefixPath = "file://";
    const std::string outputPath = tempDir + "/output";
    
    try {
        // Clean up and create test directory
        fs::remove_all(tempDir);
        fs::create_directories(dir1);
        fs::create_file(dir1 + "/test.mp4");
        
        // Generate playlist
        main(5, &dir1[0], &prefixPath[0], &outputPath[0], nullptr);
        
        // Check that output file exists with .m3u extension
        EXPECT_TRUE(fs::exists(outputPath + "testdir.m3u"));
        EXPECT_TRUE(fs::exists(outputPath + "Total.m3u"));
        
    } catch (const std::exception& e) {
        fs::remove_all(tempDir);
        FAIL() << "Failed to generate playlist: " << e.what();
    }
}

// Test that Total.m3u is always created
TEST(PlaylistGenerationTest, TotalPlaylistCreated) {
    const std::string tempDir = "/tmp/m3u_total_test";
    const std::string dir1 = tempDir + "/dir";
    const std::string prefixPath = "file://";
    const std::string outputPath = tempDir + "/output";
    
    try {
        fs::remove_all(tempDir);
        fs::create_directories(dir1);
        fs::create_file(dir1 + "/video.mp4");
        
        main(5, &dir1[0], &prefixPath[0], &outputPath[0], nullptr);
        
        EXPECT_TRUE(fs::exists(outputPath + "Total.m3u"));
    } catch (const std::exception& e) {
        fs::remove_all(tempDir);
        FAIL() << "Failed: " << e.what();
    }
}

// Test that directory-based playlists are created
TEST(PlaylistGenerationTest, DirectoryBasedPlaylistsCreated) {
    const std::string tempDir = "/tmp/m3u_dir_test";
    const std::string dir1 = tempDir + "/dir1";
    const std::string dir2 = tempDir + "/dir2";
    const std::string prefixPath = "file://";
    const std::string outputPath = tempDir + "/output";
    
    try {
        fs::remove_all(tempDir);
        fs::create_directories(dir1);
        fs::create_directories(dir2);
        fs::create_file(dir1 + "/video1.mp4");
        fs::create_file(dir2 + "/video2.mkv");
        
        main(5, &dir1[0], &prefixPath[0], &outputPath[0], nullptr);
        
        EXPECT_TRUE(fs::exists(outputPath + "dir1.m3u"));
        EXPECT_TRUE(fs::exists(outputPath + "dir2.m3u"));
    } catch (const std::exception& e) {
        fs::remove_all(tempDir);
        FAIL() << "Failed: " << e.what();
    }
}

// Test that playlist files start with #EXTM3U header
TEST(PlaylistGenerationTest, PlaylistHeader) {
    const std::string tempDir = "/tmp/m3u_header_test";
    const std::string dir1 = tempDir + "/dir";
    const std::string prefixPath = "file://";
    const std::string outputPath = tempDir + "/output";
    
    try {
        fs::remove_all(tempDir);
        fs::create_directories(dir1);
        fs::create_file(dir1 + "/test.mp4");
        
        main(5, &dir1[0], &prefixPath[0], &outputPath[0], nullptr);
        
        std::ifstream file(outputPath + "dir.m3u");
        std::string headerLine;
        if (std::getline(file, headerLine)) {
            EXPECT_TRUE(headerLine == "#EXTM3U");
        } else {
            FAIL() << "Failed to read header line";
        }
    } catch (const std::exception& e) {
        fs::remove_all(tempDir);
        FAIL() << "Failed: " << e.what();
    }
}
