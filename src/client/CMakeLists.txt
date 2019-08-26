cmake_minimum_required(VERSION 3.1)

add_library (tclm_client SHARED
	"tclm_client.cpp"
	"tclmc_impl.cpp"
	"tclm_client_exceptions.cpp"
	"Access_Concentrator.cpp"
	"../polled_fd.cpp"
	"../Connection.cpp"
	"../TCP_Connection.cpp"
	"../Communications_Manager.cpp"
	"../stream.c"
	"../ringbuffer.cpp"
	"../errno_exception.cpp")

target_link_libraries (tclm_client Threads::Threads)

add_subdirectory (example)
