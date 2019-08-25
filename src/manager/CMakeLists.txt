cmake_minimum_required (VERSION 3.1)

add_executable (tclm
	"../errno_exception.cpp"

	"main.cpp"
	"argument_parser.cpp"
	"../Connection.cpp"
	"../TCP_Connection.cpp"
	"../ringbuffer.cpp"
	"../stream.c"
	"../Communications_Manager.cpp"
	"../polled_fd.cpp")
