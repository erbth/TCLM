#include "Lock.h"
#include <algorithm>

using namespace std;
using namespace server;

Lock::Lock (Lock *parent, const string name) :
	parent(parent), name(name)
{
}

Lock::Lock (const string name) :
	parent(nullptr), name(name)
{
}

Lock::~Lock ()
{
	/* Destroy all children */
	for (auto i = children.begin(); i != children.end(); i++)
		delete *i;
}

const string Lock::get_name () const
{
	scoped_lock lk(m);
	return name;
}

int Lock::acquire (shared_ptr<Lock_Request> r)
{
	unique_lock lk(m);

	if (r->current_level == r->level)
	{
		/* At the lock to be acquired */
		switch (r->mode)
		{
			case LOCK_REQUEST_MODE_S:
				/* S */
				if (locker_X == nullptr && lockers_IX.size() == 0)
				{
					lockers_S.insert (r->requester);
					return LOCK_ACQUIRE_ACQUIRED;
				}
				break;

			case LOCK_REQUEST_MODE_Splus:
				/* S+ */
				if (locker_Splus == nullptr && lockers_ISplus.size() == 0 &&
						(locker_X == nullptr || locker_X == r->requester))
				{
					locker_Splus = r->requester;
					return LOCK_ACQUIRE_ACQUIRED;
				}
				break;

			case LOCK_REQUEST_MODE_X:
				/* X */
				if (lockers_S.size() == 0 && locker_X == nullptr &&
						lockers_IS.size() == 0 && lockers_IX.size() == 0 &&
						(locker_Splus == nullptr || locker_Splus == r->requester) &&
						(all_of (lockers_ISplus.cbegin(), lockers_ISplus.cend(),
								 [r](const Process* p){return r->requester == p;})))
				{
					locker_X = r->requester;
					return LOCK_ACQUIRE_ACQUIRED;
				}
				break;
		}

		lock_requests.push_back  (r);
		return LOCK_ACQUIRE_QUEUED;
	}
	else
	{
		/* At an intermediate node in the path to the lock that is requested */
		switch (r->mode)
		{
			case LOCK_REQUEST_MODE_S:
				/* IS */
				if (locker_X == nullptr)
				{
					lockers_IS.insert (r->requester);
				}
				else
				{
					lock_requests.push_back (r);
					return LOCK_ACQUIRE_QUEUED;
				}
				break;

			case LOCK_REQUEST_MODE_Splus:
				/* IS+ */
				if (locker_Splus == nullptr &&
						(locker_X == nullptr || locker_X == r->requester))
				{
					lockers_ISplus.insert (r->requester);
				}
				else
				{
					lock_requests.push_back (r);
					return LOCK_ACQUIRE_QUEUED;
				}
				break;

			case LOCK_REQUEST_MODE_X:
				/* IX */
				if (lockers_S.size() == 0 && locker_X == nullptr &&
						(locker_Splus == nullptr || locker_Splus == r->requester))
				{
					lockers_IX.insert (r->requester);
				}
				else
				{
					lock_requests.push_back (r);
					return LOCK_ACQUIRE_QUEUED;
				}
				break;
		}

		/* Find the child and call it */
		for (auto i = children.begin(); i != children.end(); i++)
		{
			auto c = *i;
			if (c->name == (*(r->path))[r->current_level + 1])
			{
				/* Don't block what we don't need. As we have a I? on this lock,
				 * no X can be acquired on any lock on the path to the root and
				 * hence the child cannot be deleted. */
				r->current_level++;
				lk.unlock ();
				return c->acquire (r);
			}
		}

		/* The lock does not exist. If requested, create it. */
		if (r->create_missing)
		{
			auto c = new Lock (this, (*(r->path))[r->current_level + 1]);
			children.push_back(c);
			r->lock_created = true;
			r->current_level++;
			lk.unlock();
			return c->acquire(r);
		}
		else
		{
			lk.unlock();
			release (r->requester, r->mode, r->path, 0, r->level);
			return LOCK_ACQUIRE_NON_EXISTENT;
		}
	}
}

int Lock::release (Process *p, uint8_t mode,
		std::shared_ptr<std::vector<std::string>> path, uint32_t current_level,
		uint32_t level)
{
	unique_lock lk(m);

	/* Release bottom up */
	if (current_level == level)
	{
		/* Release this lock */
		switch (mode)
		{
			case LOCK_REQUEST_MODE_S:
				{
					auto i = lockers_S.find(p);
					if (i != lockers_S.end())
					{
						lockers_S.erase (i);
						return LOCK_RELEASE_SUCCESS;
					}
				}

			case LOCK_REQUEST_MODE_Splus:
				if (locker_Splus == p)
				{
					lockers_S.erase (p);
					return LOCK_RELEASE_SUCCESS;
				}

			case LOCK_REQUEST_MODE_X:
				if (locker_X == p)
				{
					lockers_S.erase (p);
					return LOCK_RELEASE_SUCCESS;
				}
		}

		/* For the compiler */
		return LOCK_RELEASE_SUCCESS;
	}
	else
	{
		/* Try to unlock the child */
		for (auto i = children.begin(); i != children.end(); i++)
		{
			auto c = *i;

			if (c->name == (*path)[current_level + 1])
			{
				lk.unlock();
				c->release (p, mode, path, current_level + 1, level);
				lk.lock();
				break;
			}
		}

		/* Release this lock if held */
		bool released = false;

		switch (mode)
		{
			case LOCK_REQUEST_MODE_S:
				/* IS */
				{
					auto i = lockers_IS.find(p);
					if (i != lockers_IS.end())
					{
						lockers_IS.erase(i);
						released = true;
					}
				}
				break;

			case LOCK_REQUEST_MODE_Splus:
				/* IS+ */
				{
					auto i = lockers_ISplus.find(p);
					if (i != lockers_ISplus.end())
					{
						lockers_ISplus.erase(i);
						released = true;
					}
				}
				break;

			case LOCK_REQUEST_MODE_X:
				/* X */
				{
					auto i = lockers_IX.find(p);
					if (i != lockers_IX.end())
					{
						lockers_IX.erase(i);
						released = true;
					}
				}
				break;
		}

		/* Potentially remove a lock request if it wasn't held */
		if (!released)
		{
			bool removed = false;

			remove_if (lock_requests.begin(), lock_requests.end(),
					[p, mode, &removed](shared_ptr<Lock_Request> cr){
						auto pred = mode == cr->mode && p == cr->requester;
						removed |= pred;
						return pred;
					});

			return removed ? LOCK_RELEASE_SUCCESS : LOCK_RELEASE_NOT_HELD;
		}
		else
		{
			/* Look if another lock request can be granted now */
			bool granted = true;
			while (granted)
			{
				granted = false;
			}

			return LOCK_RELEASE_SUCCESS;
		}
	}
}

pair<int, Lock*> Lock::destroy (Process *p, std::shared_ptr<std::vector<std::string>> path,
		uint32_t current_level, uint32_t level)
{
	unique_lock lk(m);

	if (current_level == level)
	{
		if (level > 0)
		{
			if (locker_X != p || locker_Splus != nullptr || lockers_ISplus.size() != 0)
				return pair(LOCK_DESTROY_NON_EXCLUSIVE, nullptr);

			/* Hold parent and remove child */
			{
				scoped_lock plk(parent->m);
				remove_if (parent->children.begin(), parent->children.end(),
						[this](Lock *l){ return l == this; });

				parent->release (p, LOCK_REQUEST_MODE_X, path, current_level - 1, level - 1);
			}
		}

		return pair(LOCK_DESTROY_SUCCESS, this);
	}
	else
	{
		/* See if it's a child of this */
		for (auto i = children.begin(); i != children.end(); i++)
		{
			auto c = *i;

			if (c->name == (*path)[current_level+1])
			{
				lk.unlock();
				return c->destroy (p, path, current_level+1, level);
			}
		}

		return pair(LOCK_DESTROY_NON_EXISTENT, nullptr);
	}
}

int Lock::query (Process *p, std::shared_ptr<std::vector<std::string>> path,
			uint32_t current_level, uint32_t level)
{
	return 0;
}

void Lock::for_each_child (function<void(const Lock *l, const uint32_t level)> f, uint32_t level) const
{
	scoped_lock lk(m);

	for (auto i = children.cbegin(); i != children.cend(); i++)
	{
		auto c = *i;
		f(c, level);
		c->for_each_child (f, level + 1);
	}
}

bool Lock::is_S_locked () const
{
	scoped_lock lk(m);
	return lockers_S.size() != 0;
}

bool Lock::is_Splus_locked () const
{
	scoped_lock lk(m);
	return locker_Splus != nullptr;
}

bool Lock::is_X_locked () const
{
	scoped_lock lk(m);
	return locker_X != nullptr;
}
