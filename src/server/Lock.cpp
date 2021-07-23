#include "Lock.h"
#include <algorithm>

using namespace std;
using namespace server;

Lock::Lock (Lock *parent, const string name) :
	name(name), parent(parent)
{
}

Lock::Lock (const string name) :
	name(name), parent(nullptr)
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

int Lock::create_not_held (Process *p, shared_ptr<vector<string>> path)
{
	if ((*path)[0] != name || !p)
		return LOCK_CREATE_PARENT_NOT_HELD;

	bool ensured = p == locker_X;

	/* If someone tried to create only the root node without meaning to own it
	 * immediately, path->size() will be one. If we further end up here, the
	 * root node must exist already. Otherwise the creation would have failed
	 * earlier. */
	if (path->size() <= 1)
		return ensured ? LOCK_CREATE_EXISTS : LOCK_CREATE_PARENT_NOT_HELD;

	return create_not_held (p, path, 1, path->size() - 1, ensured);
}

int Lock::create_not_held (Process *p, shared_ptr<vector<string>> path,
		uint32_t current_level, uint32_t level, bool ownership_ensured)
{
	unique_lock lk(m);

	/* Eventually ensure the ownership on the fly */
	ownership_ensured |= p == locker_X;

	/* See if the requested child exists */
	Lock *child = nullptr;

	for (auto i = children.begin(); i != children.end(); i++)
	{
		Lock *c = *i;

		if (c->get_name() == (*path)[current_level])
		{
			child = c;
			break;
		}
	}

	/* If not, create it. */
	bool created = false;

	if (!child)
	{
		if (!ownership_ensured)
			return LOCK_CREATE_PARENT_NOT_HELD;

		child = new Lock (this, (*path)[current_level]);
		children.push_back (child);

		created = true;
	}

	/* If the target lock is supposed to be further down the tree, go ahead. */
	if (current_level < level)
		return child->create_not_held (p, path, current_level + 1, level,
				ownership_ensured);
	else
		return ownership_ensured ?
			(created ? LOCK_CREATE_CREATED : LOCK_CREATE_EXISTS) :
			LOCK_CREATE_PARENT_NOT_HELD;
}

int Lock::acquire (shared_ptr<Lock_Request> r, bool insert_in_current_queue)
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
					r->acquire_status = LOCK_ACQUIRE_ACQUIRED;
					return LOCK_ACQUIRE_ACQUIRED;
				}
				break;

			case LOCK_REQUEST_MODE_Splus:
				/* S+ */
				if (locker_Splus == nullptr && lockers_ISplus.size() == 0 &&
						(locker_X == nullptr || locker_X == r->requester) &&
						(all_of (lockers_IX.cbegin(), lockers_IX.cend(),
								 [r](const Process* p){return r->requester == p;})))
				{
					locker_Splus = r->requester;
					r->acquire_status = LOCK_ACQUIRE_ACQUIRED;
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
					r->acquire_status = LOCK_ACQUIRE_ACQUIRED;
					return LOCK_ACQUIRE_ACQUIRED;
				}
				break;
		}

		if (insert_in_current_queue)
			lock_requests.push_back  (r);

		r->acquire_status = LOCK_ACQUIRE_QUEUED;
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
					if (insert_in_current_queue)
						lock_requests.push_back (r);

					r->acquire_status = LOCK_ACQUIRE_QUEUED;
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
					if (insert_in_current_queue)
						lock_requests.push_back (r);

					r->acquire_status = LOCK_ACQUIRE_QUEUED;
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
					if (insert_in_current_queue)
						lock_requests.push_back (r);

					r->acquire_status = LOCK_ACQUIRE_QUEUED;
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
				/* TODO if implementing multithreading:
				 * Don't block what we don't need. As we have a I? on this lock,
				 * no X can be acquired on any lock on the path to the root and
				 * hence the child cannot be deleted. Hence the mutex (lk) could
				 * be released.
				 * However than releasing the lock in case a child lock did not
				 * exist becomes more difficult because other lock requests may
				 * have been queued while searching for the child, which may now
				 * advance. */
				r->current_level++;
				auto ret = c->acquire (r, true);

				/* If a child lock did not exist, release the intent lock. */
				if (ret == LOCK_ACQUIRE_NON_EXISTENT)
				{
					switch (r->mode)
					{
						case LOCK_REQUEST_MODE_S:
							/* IS */
							lockers_IS.erase(lockers_IS.find(r->requester));
							break;

						case LOCK_REQUEST_MODE_Splus:
							/* IS+ */
							lockers_ISplus.erase(lockers_ISplus.find(r->requester));
							break;

						case LOCK_REQUEST_MODE_X:
							/* IX */
							lockers_IX.erase(lockers_IX.find(r->requester));
							break;
					}
				}

				return ret;
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
			return c->acquire(r, true);
		}
		else
		{
			/* Release intent lock */
			switch (r->mode)
			{
				case LOCK_REQUEST_MODE_S:
					/* IS */
					lockers_IS.erase(lockers_IS.find(r->requester));
					break;

				case LOCK_REQUEST_MODE_Splus:
					/* IS+ */
					lockers_ISplus.erase(lockers_ISplus.find(r->requester));
					break;

				case LOCK_REQUEST_MODE_X:
					/* IX */
					lockers_IX.erase(lockers_IX.find(r->requester));
					break;
			}

			r->acquire_status = LOCK_ACQUIRE_NON_EXISTENT;
			return LOCK_ACQUIRE_NON_EXISTENT;
		}
	}
}

std::pair<int,std::set<std::shared_ptr<Lock_Request>>> Lock::release (
		Process *p, uint8_t mode,
		std::shared_ptr<std::vector<std::string>> path)
{
	return release (p, mode, path, 0, path->size() - 1);
}

std::pair<int,std::set<std::shared_ptr<Lock_Request>>> Lock::release (
		Process *p, uint8_t mode,
		std::shared_ptr<std::vector<std::string>> path, uint32_t current_level,
		uint32_t level)
{
	set<shared_ptr<Lock_Request>> answered_requests;
	unique_lock lk(m);

	bool released = false;

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
						released = true;
					}
					break;
				}

			case LOCK_REQUEST_MODE_Splus:
				if (locker_Splus == p)
				{
					locker_Splus = nullptr;
					released = true;
				}
				break;

			case LOCK_REQUEST_MODE_X:
				if (locker_X == p)
				{
					locker_X = nullptr;
					released = true;
				}
				break;
		}
	}
	else
	{
		/* Try to unlock the child */
		for (auto i = children.begin(); i != children.end(); i++)
		{
			auto c = *i;

			if (c->name == (*path)[current_level + 1])
			{
				auto tr = c->release (p, mode, path, current_level + 1, level);
				answered_requests.merge(tr.second);

				if (tr.first == LOCK_RELEASE_SUCCESS)
					released = true;

				break;
			}
		}

		/* Release this lock if the child was held (or had a lock request). */
		if (released)
		{
			switch (mode)
			{
				case LOCK_REQUEST_MODE_S:
					/* IS */
					{
						auto i = lockers_IS.find(p);
						lockers_IS.erase(i);
					}
					break;

				case LOCK_REQUEST_MODE_Splus:
					/* IS+ */
					{
						auto i = lockers_ISplus.find(p);
						lockers_ISplus.erase(i);
					}
					break;

				case LOCK_REQUEST_MODE_X:
					/* IX */
					{
						auto i = lockers_IX.find(p);
						lockers_IX.erase(i);
					}
					break;
			}
		}
	}

	/* Potentially remove a lock request if it wasn't held */
	if (!released)
	{
		bool removed = false;

		for (auto i = lock_requests.begin(); i != lock_requests.end(); i++)
		{
			auto cr = *i;

			if (mode == cr->mode && p == cr->requester)
			{
				removed = true;
				lock_requests.erase(i);
				break;
			}
		}

		return pair(
				(removed ? LOCK_RELEASE_SUCCESS : LOCK_RELEASE_NOT_HELD),
				answered_requests);
	}
	else
	{
		/* Look if another lock request can be granted now */
		while (lock_requests.size() > 0)
		{
			/* The lock request may have advanced. Remove it from the queue in
			 * such a case. */
			auto r = lock_requests.front();
			auto old_level = r->current_level;

			if (acquire (r, false) != LOCK_ACQUIRE_QUEUED)
			{
				answered_requests.insert (lock_requests.front());
				lock_requests.pop_front();
			}
			else if (r->current_level > old_level)
			{
				lock_requests.pop_front();
			}
			else
			{
				break;
			}
		}

		return pair(LOCK_RELEASE_SUCCESS,answered_requests);
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

bool Lock::is_IS_locked () const
{
	scoped_lock lk(m);
	return lockers_IS.size() != 0;
}

bool Lock::is_ISplus_locked () const
{
	scoped_lock lk(m);
	return lockers_ISplus.size() != 0;
}

bool Lock::is_IX_locked () const
{
	scoped_lock lk(m);
	return lockers_IX.size() != 0;
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
