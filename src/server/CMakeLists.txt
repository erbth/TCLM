cmake_minimum_required (VERSION 3.1)

add_executable (tclmd
	"../errno_exception.cpp"

	"main.cpp"
	"../Communications_Manager.cpp"
	"../polled_fd.cpp"
	"../Connection.cpp"
	"../TCP_Connection.cpp"
	"../stream.c"
	"../ringbuffer.cpp"
	"TCP_Listener.cpp"
	"daemon.cpp"

	"backend_exceptions.cpp"
	"backend.cpp"
	"Process_map.cpp"
	"Process.cpp")

target_link_libraries (tclmd Threads::Threads)
