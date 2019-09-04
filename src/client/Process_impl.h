#ifndef __PROCESS_IMPL_H
#define __PROCESS_IMPL_H

#include "tclm_client.hpp"
#include "tclmc_impl.h"
#include "Lock_impl.h"
#include <memory>

namespace tclm_client {

class Process_impl : public Process
{
protected:
	const uint32_t id;
	std::shared_ptr<tclmc_impl> tclmc;

public:
	Process_impl (std::shared_ptr<tclmc_impl> tclmc, const uint32_t id);
	~Process_impl () override;

	const uint32_t get_id () const override;
};

}

#endif /* __PROCESS_IMPL_H */
