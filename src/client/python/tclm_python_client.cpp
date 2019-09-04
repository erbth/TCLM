#include "tclm_client.hpp"
#include <boost/python.hpp>

using namespace std;
using namespace tclm_client;
using namespace boost::python;

shared_ptr<tclmc> create1 (const string servername)
{
	return tclmc::create (servername);
}

shared_ptr<tclmc> create2 (const string servername, uint16_t tcp_port)
{
	return tclmc::create (servername, tcp_port);
}

shared_ptr<tclmc> create3 (const string servername, uint16_t tcp_port, uint16_t udp_port)
{
	return tclmc::create (servername, tcp_port, udp_port);
}

BOOST_PYTHON_MODULE(tclm_python_client)
{
	class_<tclmc, boost::noncopyable>("tclmc", no_init)
		.def ("register_process", &tclmc::register_process)
		.def ("define_lock", &tclmc::define_lock)
	;

	def ("create_tclmc", create1);
	def ("create_tclmc", create2);
	def ("create_tclmc", create3);

	class_<Process, boost::noncopyable>("Process", no_init)
		.def ("get_id", &Process::get_id)
	;

	class_<Lock, boost::noncopyable>("Lock", no_init)
		.def ("get_path", &Lock::get_path)
		.def ("create", &Lock::create)
		.def ("destroy", &Lock::destroy)
	;

	register_ptr_to_python<std::shared_ptr<tclmc>> ();
	register_ptr_to_python<std::shared_ptr<Process>> ();
	register_ptr_to_python<std::shared_ptr<Lock>> ();
}
