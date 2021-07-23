cmake_minimum_required(VERSION 3.1)

project(TCLM C CXX)

set (TCLM_VERSION_MAJOR 1)
set (TCLM_VERSION_MINOR 0)
set (TCLM_VERSION_PATCH 6)

set (SERVER_VERSION_MAJOR ${TCLM_VERSION_MAJOR})
set (SERVER_VERSION_MINOR ${TCLM_VERSION_MINOR})
set (SERVER_VERSION_PATCH ${TCLM_VERSION_PATCH})

set (CLIENT_VERSION_MAJOR ${TCLM_VERSION_MAJOR})
set (CLIENT_VERSION_MINOR ${TCLM_VERSION_MINOR})
set (CLIENT_VERSION_PATCH ${TCLM_VERSION_PATCH})

# Options
option(BUILD_SERVER "Build tclmd" OFF)
option(BUILD_MANAGER "Build tclm" OFF)
option(BUILD_CPP_CLIENT "Build libtclm_client" ON)
option(BUILD_PYTHON_CLIENT "Build the tclm_python_client python module" OFF)

option(BUILD_ALL "Build all components" OFF)

if (BUILD_PYTHON_CLIENT)
	set(BUILD_CPP_CLIENT ON)
endif()

if (BUILD_ALL)
	set(BUILD_SERVER ON)
	set(BUILD_MANAGER ON)
	set(BUILD_CPP_CLIENT ON)
	set(BUILD_PYTHON_CLIENT ON)
endif()

# Find packages
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

if (BUILD_PYTHON_CLIENT)
	find_package (PythonLibs 3 REQUIRED)
	find_package (Boost REQUIRED COMPONENTS python3)
endif()

# Set CXXFLAGS
set (CMAKE_CXX_FLAGS "-O2 -std=c++17 -Wall -gdwarf-2")
set (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -Werror")

include_directories (src)
include_directories (include)

# Add subdirectories
add_subdirectory (src)

# Packaging
set (CPACK_SOURCE_VERSION_MAJOR ${TCLM_VERSION_MAJOR})
set (CPACK_SOURCE_VERSION_MINOR ${TCLM_VERSION_MINOR})
set (CPACK_SOURCE_VERSION_PATCH ${TCLM_VERSION_PATCH})
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_IGNORE_FILES "/build;/\.git;.*\.swp;mysession\.vim;${CPACK_SOURCE_IGNORE_FILES}")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_SOURCE_VERSION_MAJOR}.${CPACK_SOURCE_VERSION_MINOR}.${CPACK_SOURCE_VERSION_PATCH}")
include(CPack)
