#include <iostream>
#include "../src/m3u_generator.h"

int main() {
    std::cout << "Testing m3u-generator header..." << std::endl;
    
    // Test 1: Check if usageManualString compiles and returns string
    std::string usage = usageManualString();
    std::cout << "Test 1: usageManualString() returns string - PASSED" << std::endl;
    std::cout << "Usage: " << usage << std::endl;
    
    // Test 2: Check printUsageManual compiles (void function)
    printUsageManual();
    std::cout << "Test 2: printUsageManual() executes - PASSED" << std::endl;
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}
