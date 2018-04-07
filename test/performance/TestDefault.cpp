
#include "TestFunctions.hpp"

// ----------------------------------------------------------------------------

template< std::size_t ArraySize >
class Blob
{
public:
	Blob() {}
private:
	char array[ ArraySize ];
};

typedef Blob<   8 >   Blob8;
typedef Blob<  64 >  Blob64;
typedef Blob< 256 > Blob256;

// ----------------------------------------------------------------------------

bool TestDefaultForward8( StopwatchPair & timers )
{
	TestSameOrder< Blob8 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultReverse8( StopwatchPair & timers )
{
	TestReverseOrder< Blob8 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultRandom8( StopwatchPair & timers )
{
	TestRandomOrder< Blob8 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultForward64( StopwatchPair & timers )
{
	TestSameOrder< Blob64 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultReverse64( StopwatchPair & timers )
{
	TestReverseOrder< Blob64 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultRandom64( StopwatchPair & timers )
{
	TestRandomOrder< Blob64 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultForward256( StopwatchPair & timers )
{
	TestSameOrder< Blob256 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultReverse256( StopwatchPair & timers )
{
	TestReverseOrder< Blob256 >( timers );
	return true;
}

// ----------------------------------------------------------------------------

bool TestDefaultRandom256( StopwatchPair & timers )
{
	TestRandomOrder< Blob256 >( timers );
	return true;
}

// ----------------------------------------------------------------------------
