cmake_minimum_required(VERSION 3.1)

add_library (tclm_client SHARED
	"tclm_client.cpp"
	"tclmc_impl.cpp"
	"Process_impl.cpp"
	"Lock_impl.cpp"
	"tclm_client_exceptions.cpp"
	"Access_Concentrator.cpp"
	"request.cpp"
	"register_process_request.cpp"
	"unregister_process_request.cpp"
	"create_lock_request.cpp"
	"acquire_lock_request.cpp"
	"release_lock_request.cpp"
	"../polled_fd.cpp"
	"../Connection.cpp"
	"../TCP_Connection.cpp"
	"../Communications_Manager.cpp"
	"../stream.c"
	"../ringbuffer.cpp"
	"../errno_exception.cpp"
	"../polled_eventfd.cpp")

target_link_libraries (tclm_client Threads::Threads)

set_target_properties (tclm_client PROPERTIES VERSION "${CLIENT_VERSION_MAJOR}.${CLIENT_VERSION_MINOR}.${CLIENT_VERSION_PATCH}" SOVERSION "${CLIENT_VERSION_MAJOR}")
set_target_properties (tclm_client PROPERTIES PUBLIC_HEADER "../../include/tclm_client_exceptions.hpp;../../include/tclm_client.hpp")

install (TARGETS tclm_client LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)

add_subdirectory (python)
add_subdirectory (example)
