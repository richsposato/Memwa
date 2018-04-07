
#pragma once

#include <chrono>

// ----------------------------------------------------------------------------

/** @class Stopwatch This class measures how much time elapses
 between its Start and Stop calls. Start and Stop can be called many times.
 */
class Stopwatch
{
public:

	explicit Stopwatch( bool start = false );

	Stopwatch( const Stopwatch & that );

	~Stopwatch();

	Stopwatch & operator=( const Stopwatch & that );

	Stopwatch & Swap( Stopwatch & that );

	void Start();

	void Stop();

	/// Returns number of microseconds elapsed.
	unsigned long long GetDuration() const;

	void Clear();

	bool operator == ( const Stopwatch & that ) const;

	bool operator != ( const Stopwatch & that ) const;

	bool operator < ( const Stopwatch & that ) const;

	bool operator > ( const Stopwatch & that ) const;

	bool operator <= ( const Stopwatch & that ) const;

	bool operator >= ( const Stopwatch & that ) const;

private:

	std::chrono::microseconds duration_;

	std::chrono::system_clock::time_point moment_;

};

// ----------------------------------------------------------------------------

struct StopwatchPair
{
	Stopwatch allocateTimer;
	Stopwatch releaseTimer;
};

// ----------------------------------------------------------------------------
