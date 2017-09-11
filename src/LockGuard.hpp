
#pragma once

#include <mutex>

class LockGuard
{
public:

	LockGuard( std::mutex & m, bool lock = true ) :
		locked_( lock ),
		m_( m )
	{
		if ( locked_ )
		{
			m_.lock();
		}
	}

	void Lock()
	{
		if ( !locked_ )
		{
			m_.lock();
			locked_ = true;
		}
	}

	void Unlock()
	{
		if ( locked_ )
		{
			m_.unlock();
			locked_ = false;
		}
	}

	~LockGuard()
	{
		if ( locked_ )
		{
			m_.unlock();
		}
	}

private:

	bool locked_;

	std::mutex & m_;

};


class ReentryGuard
{
public:

	explicit ReentryGuard( bool & entered ) : entered_( entered )
	{
		entered_ = true;
	}

	~ReentryGuard()
	{
		entered_ = false;
	}

	bool AlreadyEntered() const
	{
		return entered_;
	}

private:

	bool & entered_;

};


