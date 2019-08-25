cmake_minimum_required(VERSION  3.1)

project(TCLM C CXX)

set (SERVER_VERSION_MAJOR 1)
set (SERVER_VERSION_MINOR 0)

# Find packages
set (THREAD_PREFER_PTHREADS_FLAG ON)
find_package (Threads REQUIRED)

# Set CXXFLAGS
set (CMAKE_CXX_FLAGS "-O2 -std=c++17")
set (CMAKE_CXX_FLAGS_DEBUG "-gdwarf-2 -DDEBUG")

include_directories (src)

# Add subdirectories
add_subdirectory (src)
