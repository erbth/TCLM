/* The Access Concentrator is responsible for haveing a connection to a tclmd
 * and for handling any operation on that tclmd.
 * The Access Concentrator should operate like a NAT in some ways that is it
 * should not hold state on the actual locks and processes. It should not
 * remember if a certain process exists or acquired a certain lock. But it
 * must remember which operations where requested until the requests are
 * answered, like a router may do in a NAT table. Furthermore it is responsible
 * for asking if requests where successful if it didn't get an answer and to
 * try it multiple times if needed.
 *
 * Contracts on using requests in this class:
 * When a request is answered, it must not be accessed anymore because it could
 * by destroyed by the client alreaday. In fact it should be removed from the
 * corresponding vector/map atomically. */

#ifndef __ACCESS_CONCENTRATOR_H
#define __ACCESS_CONCENTRATOR_H

#include "Connection.h"
#include "register_process_request.h"
#include "unregister_process_request.h"
#include "create_lock_request.h"
#include "release_lock_request.h"
#include "Communications_Manager.h"
#include "stream.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace tclm_client {

/* Prototypes */
class request;
class register_process_request;
class unregister_process_request;
class create_lock_request;
class release_lock_request;

class Access_Concentrator
{
protected:
	/* A Communications Manager for our Connections. It is not protected by a
	 * mutex because it's not a pointer can care about itself. */
	Communications_Manager cm;

	/* A thread for the Communications Manager */
	std::thread cm_thread;

	void cm_thread_func_internal ();
	static void cm_thread_func (void *data);

	/* Information about the connection target */
	std::mutex m_servername;
	std::string servername;
	uint16_t tcp_port;
	uint16_t udp_port;

	/* A stream oriented connection */
	std::mutex m_tcp_connection;
	std::shared_ptr<Connection> tcp_connection;

	/* Try to create a TCP connection (over IPv4 or IPv6) */
	bool create_tcp_connection () noexcept;

	/* Returns true if the TCP connection exists and the message was sent
	 * through it. Swallows the stream only if the return value is true or an
	 * exception is thrown. Does NOT throw bad_alloc. */
	bool send_message_tcp (struct stream *s);

	/* Sends a message through TCP or UDP depending on if the connections exist
	 * and the messages size. Swallows s in any case. Does NOT throw bad_alloc.
	 * Returns true if the message was sent. */
	bool send_message_auto (struct stream *s);

	static void receive_message_tcp (Connection *c, struct stream *s, void *data);
	void receive_message_tcp_internal (Connection *c, struct stream *s);


	/* Helper constructs like nonces for specific request types with their
	 * mutexes */
	std::mutex m_register_process_next_nonce;
	uint32_t register_process_next_nonce = 0;

	/* Vectors/maps for various requests with corresponding mutexes */
	std::mutex m_register_process_requests;
	std::vector<register_process_request*> register_process_requests;

	std::mutex m_unregister_process_requests;
	std::vector<unregister_process_request*> unregister_process_requests;

	/* A map of (pid,lock_path) into create_lock_requests */
	std::mutex m_create_lock_requests;
	std::map<std::pair<uint32_t,std::string>, create_lock_request*> create_lock_requests;

	/* A map of (pid,lock_path,mode) into release_lock_requests */
	std::mutex m_release_lock_requests;
	std::map<std::tuple<uint32_t,std::string,uint8_t>, release_lock_request*> release_lock_requests;

	/* Functions for sending messages that can handle send failures
	 * appropriately. These sould be robust regarding any type of error. I.e.
	 * they should fail silently in case of OOM conditions, knowing that the
	 * main thread will try sending again. */
	void send_register_process_request (register_process_request *r);
	void send_unregister_process_request (unregister_process_request *r);
	void send_create_lock_request (create_lock_request *r);
	void send_release_lock_request (release_lock_request *r);

	/* Functions for receiving messages */
	void receive_register_process_response (Connection *c, struct stream *s, uint32_t length);
	void receive_unregister_process_response (Connection *c, struct stream *s, uint32_t length);
	void receive_create_lock_update (Connection *c, struct stream *s, uint32_t length);
	void receive_release_lock_response (Connection *c, struct stream *s, uint32_t length);

public:
	Access_Concentrator (const std::string &servername,
			const uint16_t tcp_port, const uint16_t udp_port);
	~Access_Concentrator ();

	/* To be used by the requests */
	void issue_register_process_request (register_process_request *r);
	void issue_unregister_process_request (unregister_process_request *r);
	void issue_create_lock_request (create_lock_request *r);
	void issue_release_lock_request (release_lock_request *r);
};

}

#endif /* __ACCESS_CONCENTRATOR_H */
