cmake_minimum_required (VERSION 3.1)

if (BUILD_PYTHON_CLIENT)
	add_library (tclm_python_client SHARED "tclm_python_client.cpp")

	target_include_directories (tclm_python_client PRIVATE ${Python3_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
	target_link_libraries (tclm_python_client ${Boost_LIBRARIES} tclm_client)

	set_target_properties (tclm_python_client PROPERTIES PREFIX "")

	install (TARGETS tclm_python_client DESTINATION lib/python3/dist-packages)
endif()
