
#include "Stopwatch.hpp"

// ----------------------------------------------------------------------------

Stopwatch::Stopwatch( bool start ) :
	duration_( std::chrono::microseconds::zero() ),
	moment_( ( start ) ? std::chrono::system_clock::now() : std::chrono::system_clock::time_point::min() )
{
}

// ----------------------------------------------------------------------------

Stopwatch::Stopwatch( const Stopwatch & that ) :
	duration_( that.duration_ ),
	moment_( that.moment_ )
{
}

// ----------------------------------------------------------------------------

Stopwatch::~Stopwatch()
{
}

// ----------------------------------------------------------------------------

Stopwatch & Stopwatch::operator=( const Stopwatch & that )
{
	if ( this != &that )
	{
		moment_ = that.moment_;
		duration_ = that.duration_;
	}
	return *this;
}

// ----------------------------------------------------------------------------

Stopwatch & Stopwatch::Swap( Stopwatch & that )
{
	if ( this != &that )
	{
		const auto temp = that.moment_;
		that.moment_ = moment_;
		moment_ = temp;
		const auto d = that.duration_;
		that.duration_ = duration_;
		duration_ = d;
	}
	return *this;
}

// ----------------------------------------------------------------------------

bool Stopwatch::operator == ( const Stopwatch & that ) const
{
	return ( duration_ == that.duration_ );
}

// ----------------------------------------------------------------------------

bool Stopwatch::operator != ( const Stopwatch & that ) const
{
	return ( duration_ != that.duration_ );
}

// ----------------------------------------------------------------------------

bool Stopwatch::operator < ( const Stopwatch & that ) const
{
	return ( duration_ < that.duration_ );
}

// ----------------------------------------------------------------------------

bool Stopwatch::operator > ( const Stopwatch & that ) const
{
	return ( duration_ > that.duration_ );
}

// ----------------------------------------------------------------------------

bool Stopwatch::operator <= ( const Stopwatch & that ) const
{
	return ( duration_ <= that.duration_ );
}

// ----------------------------------------------------------------------------

bool Stopwatch::operator >= ( const Stopwatch & that ) const
{
	return ( duration_ >= that.duration_ );
}

// ----------------------------------------------------------------------------

void Stopwatch::Start()
{
	moment_ = std::chrono::high_resolution_clock::now();
}

// ----------------------------------------------------------------------------

void Stopwatch::Stop()
{
	const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	const std::chrono::microseconds now2 = std::chrono::duration_cast< std::chrono::microseconds >( now.time_since_epoch() );
	const std::chrono::microseconds then = std::chrono::duration_cast< std::chrono::microseconds >( moment_.time_since_epoch() );
	const std::chrono::microseconds diff = now2 - then;
	duration_ += std::chrono::duration_cast< std::chrono::microseconds >(diff);
}

// ----------------------------------------------------------------------------

unsigned long long Stopwatch::GetDuration() const
{
	const unsigned long long nanos = duration_.count();
	return nanos;
}

// ----------------------------------------------------------------------------

void Stopwatch::Clear()
{
	duration_ = std::chrono::microseconds::zero();
}

// ----------------------------------------------------------------------------

