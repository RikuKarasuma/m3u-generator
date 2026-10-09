cmake_minimum_required(VERSION 3.16)
project(m3u-generator VERSION 1.0.0 LANGUAGES CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(CURL REQUIRED)

# Add executable from main source file
add_executable(m3u-generator src/main.cpp)

# Link with libcurl
target_link_libraries(m3u-generator PRIVATE CURL::libcurl)

# Set compile definitions if needed
target_compile_definitions(m3u-generator PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
)

# Install rules
install(TARGETS m3u-generator DESTINATION bin)
install(FILES src/m3u_generator.h DESTINATION include)
