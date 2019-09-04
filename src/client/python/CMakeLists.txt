cmake_minimum_required (VERSION 3.1)

add_library (tclm_python_client SHARED "tclm_python_client.cpp")

target_include_directories (tclm_python_client PRIVATE ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries (tclm_python_client ${Boost_LIBRARIES} tclm_client)

set_target_properties (tclm_python_client PROPERTIES PREFIX "")
