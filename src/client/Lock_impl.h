#ifndef __LOCK_IMPL_H
#define __LOCK_IMPL_H

#include "tclm_client.hpp"
#include "tclmc_impl.h"
#include <memory>

namespace tclm_client {

class Lock_impl : public Lock
{
protected:
	const std::string path;
	std::shared_ptr<tclmc_impl> tclmc;

public:
	Lock_impl (std::shared_ptr<tclmc_impl> tclmc, const std::string path);

	const std::string get_path () const override;

	/* Returns true if the lock was created. Otherwise false is returned and
	 * a X lock acquired. */
	bool create (std::shared_ptr<Process> p, const bool acquire_X) override;
	void destroy(std::shared_ptr<Process> p) override;

	void acquire_S (std::shared_ptr<Process> p) override;
	void acquire_Splus (std::shared_ptr<Process> p) override;
	void acquire_X (std::shared_ptr<Process> p) override;

	void release_S (std::shared_ptr<Process> p) override;
	void release_Splus (std::shared_ptr<Process> p) override;
	void release_X (std::shared_ptr<Process> p) override;

	/* Mode is one out of MSG_LOCK_MODE_{S,Splus,X} */
	void acquire (std::shared_ptr<Process> p, uint8_t mode);
	void release (std::shared_ptr<Process> p, uint8_t mode);
};

}

#endif /* __LOCK_IMPL_H */
