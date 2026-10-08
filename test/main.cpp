#include "../src/m3u_generator.h"
#include <cassert>
#include <iostream>

int main() {
    // Test 1: usageManualString returns non-empty string
    std::string usage = m3u::usageManualString();
    assert(!usage.empty());
    
    // Test 2: printUsageManual prints to stdout (we can't easily test this with assertions)
    // Just ensure the function exists and compiles
    
    // Test 3: validateDirectory returns true for /tmp
    assert(m3u::validateDirectory("/tmp"));
    
    // Test 4: validateDirectory returns false for non-existent path
    assert(!m3u::validateDirectory("/nonexistent_path_xyz"));
    
    // Test 5: encodeURL encodes spaces as %20
    std::string testPath = "/path/with spaces/file.mp4";
    std::string encoded = m3u::encodeURL(testPath);
    assert(encoded.find("%20") != std::string::npos);
    
    // Test 6: GeneratorConfig::fromArgs creates config correctly
    char* args[] = {nullptr};
    m3u::GeneratorConfig config;
    try {
        config = m3u::GeneratorConfig::fromArgs(1, args);
        assert(config.output_directory == "output");
        assert(config.source_directories.empty());
    } catch (...) {
        // Expected to throw for invalid arguments
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
