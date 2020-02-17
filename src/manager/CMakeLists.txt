cmake_minimum_required (VERSION 3.1)

if (BUILD_MANAGER)
	add_executable (tclm
		"../errno_exception.cpp"

		"main.cpp"
		"argument_parser.cpp"
		"../Connection.cpp"
		"../TCP_Connection.cpp"
		"../ringbuffer.cpp"
		"../stream.c"
		"../Communications_Manager.cpp"
		"../polled_fd.cpp"
		"../polled_eventfd.cpp")

	target_link_libraries (tclm Threads::Threads)

	install (TARGETS tclm DESTINATION bin)
endif()
