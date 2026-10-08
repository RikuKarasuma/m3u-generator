#include <gtest/gtest.h>
#include "../src/main.cpp"

// Test validateDirectory with valid directory
TEST(ValidateDirectoryTest, ValidDirectory) {
    // Use /tmp as a known valid directory
    EXPECT_NO_THROW(validateDirectory("/tmp"));
}

// Test validateDirectory with invalid/non-existent path
TEST(ValidateDirectoryTest, NonExistentPath) {
    const std::string invalidPath = "/nonexistent_directory_xyz123_nonexistent";
    // This should throw runtime_error
    EXPECT_THROW(validateDirectory(invalidPath), std::runtime_error);
}

// Test validateDirectory with empty string
TEST(ValidateDirectoryTest, EmptyString) {
    EXPECT_NO_THROW(validateDirectory(""));
}

// Test that exception message contains usage manual when directory is invalid
TEST(ValidateDirectoryTest, ExceptionMessageContainsUsage) {
    const std::string invalidPath = "/nonexistent_xyz";
    try {
        validateDirectory(invalidPath);
        FAIL() << "Expected runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        EXPECT_TRUE(e.what().find("Usage:") != std::string::npos);
    }
}
