cmake_minimum_required (VERSION 3.1)

# Configure a config file
configure_file (tclm_config.h.in tclm_config.h)

include_directories (${CMAKE_BINARY_DIR}/src)

add_subdirectory (server)
add_subdirectory (manager)
