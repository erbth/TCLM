cmake_minimum_required (VERSION 3.1)

add_executable (example
	"main.cpp")

target_link_libraries (example Threads::Threads tclm_client)
