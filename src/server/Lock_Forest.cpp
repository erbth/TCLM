#include "Lock_Forest.h"
#include "Lock_Request.h"
#include <algorithm>

using namespace std;
using namespace server;

Lock_Forest::~Lock_Forest ()
{
	/* Delete the locks. */
	for_each (roots.begin(), roots.end(),
			[](pair<string, Lock*> p) {
				delete p.second;
			});
}

shared_ptr<vector<string>> Lock_Forest::split_path (const string *path)
{
	auto v = make_shared<vector<string>>();

	string::size_type cbegin = 0;

	for (;;)
	{
		string::size_type cend = path->find ('.', cbegin);

		if (cend == string::npos)
		{
			v->push_back (path->substr (cbegin));
			break;
		}
		else
		{
			if (cbegin < cend)
			{
				v->push_back (path->substr (cbegin, cend - cbegin));
			}

			cbegin = cend + 1;
		}
	}

	return v;
}

int Lock_Forest::create (Process *p, const string *path_str, const bool acquire_X)
{
	unique_lock lk(m);
	auto path = split_path (path_str);

	/* Create a lock request */
	auto req = Lock_Request::create(
			LOCK_REQUEST_MODE_X,
			p,
			path,
			true);

	/* Does the root node exist already? */
	auto i_root = roots.find ((*path)[0]);
	if (i_root == roots.end())
	{
		if (acquire_X)
		{
			/* We can create it only if we can acquire it. Otherwise we are not
			 * able to protect the new lock. */
			auto l = new Lock ((*path)[0]);
			i_root = (roots.insert (pair ((*path)[0], l))).first;
			req->lock_created = true;
		}
		else
			return LOCK_CREATE_PARENT_NOT_HELD;
	}

	/* Create subtree nodes */
	if (acquire_X)
	{
		auto ret = i_root->second->acquire(req);

		switch (ret)
		{
			case LOCK_ACQUIRE_ACQUIRED:
				p->add_held_lock (path_str, LOCK_REQUEST_MODE_X);
				if (req->lock_created)
					return LOCK_CREATE_CREATED;
				else
					return LOCK_CREATE_EXISTS;

			case LOCK_ACQUIRE_QUEUED:
				p->add_held_lock (path_str, LOCK_REQUEST_MODE_X);
				return LOCK_CREATE_QUEUED;

			default:
				/* Cannot happen. */
				return LOCK_CREATE_EXISTS;
		}
	}
	else
		return i_root->second->create_not_held(p, path);
}

int Lock_Forest::acquire (Process *p, const std::string *path_str, uint8_t mode)
{
	shared_lock lk(m);
	auto path = split_path (path_str);

	/* Look if the root exists */
	auto i_root = roots.find((*path)[0]);
	if (i_root == roots.end())
		return LOCK_ACQUIRE_NON_EXISTENT;

	/* Try to acquire it */
	auto ret = i_root->second->acquire (Lock_Request::create(mode, p, path));

	switch (ret)
	{
		case LOCK_ACQUIRE_ACQUIRED:
		case LOCK_ACQUIRE_QUEUED:
			p->add_held_lock(path_str, mode);
			break;
	}

	return ret;
}

std::pair<int,std::set<std::shared_ptr<Lock_Request>>> Lock_Forest::release (
		Process *p, const std::string *path_str, uint8_t mode)
{
	shared_lock lk(m);
	auto path = split_path (path_str);

	/* Look if the root exists */
	auto i_root = roots.find ((*path)[0]);
	if (i_root == roots.end())
		return pair(LOCK_RELEASE_NOT_HELD, set<shared_ptr<Lock_Request>>());

	/* Release it. */
	auto tr = i_root->second->release (p, mode, path);
	
	if (tr.first == LOCK_RELEASE_SUCCESS)
		p->remove_held_lock(path_str, mode);

	return tr;
}

void Lock_Forest::for_each_lock (function<void(const Lock *l, const uint32_t level)> f) const
{
	shared_lock lk(m);

	for (auto i = roots.cbegin(); i != roots.cend(); i++)
	{
		auto r = i->second;
		f (r, 0);
		r->for_each_child (f, 1);
	}
}
