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

shared_ptr<vector<string>> Lock_Forest::split_path (string *path)
{
	auto v = make_shared<vector<string>>();

	int cbegin = 0;

	for (;;)
	{
		int cend = path->find ('.', cbegin);

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

int Lock_Forest::create (Process *p, string *path_str)
{
	unique_lock lk(m);
	auto path = split_path (path_str);

	/* Create a lock request */
	auto req = make_shared<Lock_Request>(
			LOCK_REQUEST_MODE_X,
			p,
			path,
			true);

	/* Does the root node exist already? */
	auto i_root = roots.find ((*path)[0]);
	if (i_root == roots.end())
	{
		auto l = new Lock ((*path)[0]);
		i_root = (roots.insert (pair ((*path)[0], l))).first;
		req->lock_created = true;
	}

	/* Is a subtree node requested? */
	auto ret = i_root->second->acquire(req);

	switch (ret)
	{
		case LOCK_ACQUIRE_ACQUIRED:
			if (req->lock_created)
				return LOCK_CREATE_CREATED;
			else
				return LOCK_CREATE_EXISTS;

		case LOCK_ACQUIRE_QUEUED:
			return LOCK_CREATE_QUEUED;

		default:
			/* Cannot happen. */
			return LOCK_CREATE_EXISTS;
	}
}

int Lock_Forest::acquire (Process *p, std::string *path_str, uint8_t mode)
{
	shared_lock lk(m);
	auto path = split_path (path_str);

	/* Look if the root exists */
	auto i_root = roots.find((*path)[0]);
	if (i_root == roots.end())
		return LOCK_ACQUIRE_NON_EXISTENT;

	/* Try to acquire it */
	return i_root->second->acquire (make_shared<Lock_Request>(mode, p, path));
}

int Lock_Forest::release (Process *p, std::string *path_str, uint8_t mode)
{
	shared_lock lk(m);
	auto path = split_path (path_str);

	/* Look if the root exists */
	auto i_root = roots.find ((*path)[0]);
	if (i_root == roots.end())
		return LOCK_RELEASE_NOT_HELD;

	/* Release it. */
	return i_root->second->release (p, mode, path);
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
