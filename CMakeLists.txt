cmake_minimum_required(VERSION  3.1)

project(TCLM C CXX)

set (SERVER_VERSION_MAJOR 1)
set (SERVER_VERSION_MINOR 0)

# Find packages
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

find_package (PythonLibs 3 REQUIRED)
find_package (Boost REQUIRED COMPONENTS system python3)

# Set CXXFLAGS
set (CMAKE_CXX_FLAGS "-O2 -std=c++17 -Wall")
set (CMAKE_CXX_FLAGS_DEBUG "-gdwarf-2 -DDEBUG -Werror")

include_directories (src)
include_directories (include)

# Add subdirectories
add_subdirectory (src)

# Packaging
set (CPACK_SOURCE_VERSION_MAJOR ${SERVER_VERSION_MAJOR})
set (CPACK_SOURCE_VERSION_MINOR ${SERVER_VERSION_MINOR})
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_IGNORE_FILES "/build;/.git;${CPACK_SOURCE_IGNORE_FILES}")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_SOURCE_VERSION_MAJOR}.${CPACK_SOURCE_VERSION_MINOR}")
include(CPack)
