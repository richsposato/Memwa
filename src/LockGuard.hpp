
#pragma once

#include <mutex>

typedef std::unique_lock< std::mutex > LockGuard;

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
