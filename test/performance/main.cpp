
#include "AllocatorManager.hpp"

#include "CommandLineArgs.hpp"
#include "Stopwatch.hpp"
#include "TestFunctions.hpp"

#include <iostream>

static unsigned int LoopCount = 100000;

// ----------------------------------------------------------------------------

extern bool TestMemwaPoolForward8( StopwatchPair & timers );
extern bool TestMemwaPoolForward64( StopwatchPair & timers );
extern bool TestMemwaPoolForward256( StopwatchPair & timers );

extern bool TestMemwaPoolReverse8( StopwatchPair & timers );
extern bool TestMemwaPoolReverse64( StopwatchPair & timers );
extern bool TestMemwaPoolReverse256( StopwatchPair & timers );

extern bool TestMemwaPoolRandom8( StopwatchPair & timers );
extern bool TestMemwaPoolRandom64( StopwatchPair & timers );
extern bool TestMemwaPoolRandom256( StopwatchPair & timers );


extern bool TestMemwaTinyForward8( StopwatchPair & timers );
extern bool TestMemwaTinyForward64( StopwatchPair & timers );

extern bool TestMemwaTinyReverse8( StopwatchPair & timers );
extern bool TestMemwaTinyReverse64( StopwatchPair & timers );

extern bool TestMemwaTinyRandom8( StopwatchPair & timers );
extern bool TestMemwaTinyRandom64( StopwatchPair & timers );


extern bool TestMemwaStackReverse8( StopwatchPair & timers );
extern bool TestMemwaStackReverse64( StopwatchPair & timers );
extern bool TestMemwaStackReverse256( StopwatchPair & timers );


extern bool TestDefaultForward8( StopwatchPair & timers );
extern bool TestDefaultForward64( StopwatchPair & timers );
extern bool TestDefaultForward256( StopwatchPair & timers );

extern bool TestDefaultReverse8( StopwatchPair & timers );
extern bool TestDefaultReverse64( StopwatchPair & timers );
extern bool TestDefaultReverse256( StopwatchPair & timers );

extern bool TestDefaultRandom8( StopwatchPair & timers );
extern bool TestDefaultRandom64( StopwatchPair & timers );
extern bool TestDefaultRandom256( StopwatchPair & timers );

// ----------------------------------------------------------------------------

void SetLoopCount( unsigned int count )
{
	LoopCount = count;
}

// ----------------------------------------------------------------------------

unsigned int GetLoopCount()
{
	return LoopCount;
}

// ----------------------------------------------------------------------------

void DoForwardTests8()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaTinyTimers;
	TestMemwaPoolForward8( memwaPoolTimers );
	TestMemwaTinyForward8( memwaTinyTimers );
	TestDefaultForward8( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Forward Order Performance Test." << std::endl
		<< "\t Object size is 8 bytes." << std::endl
		<< "\t Release in same order as Allocation." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaTinyTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaTinyTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Tiny:  " << memwaTinyTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaTinyTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoReverseTests8()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaTinyTimers;
	StopwatchPair memwaStackTimers;
	TestMemwaPoolReverse8( memwaPoolTimers );
	TestMemwaTinyReverse8( memwaTinyTimers );
	TestMemwaStackReverse8( memwaStackTimers );
	TestDefaultReverse8( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Reverse Order Performance Test." << std::endl
		<< "\t Object size is 8 bytes." << std::endl
		<< "\t Release in reverse order of Allocation." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaTinyTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaTinyTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Tiny:  " << memwaTinyTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaTinyTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaStackTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaStackTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Stack: " << memwaStackTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaStackTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoForwardTests64()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaTinyTimers;
	TestMemwaPoolForward64( memwaPoolTimers );
	TestMemwaTinyForward64( memwaTinyTimers );
	TestDefaultForward64( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Forward Order Performance Test." << std::endl
		<< "\t Object size is 64 bytes." << std::endl
		<< "\t Release in same order as Allocation." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaTinyTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaTinyTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Tiny:  " << memwaTinyTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaTinyTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoReverseTests64()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaTinyTimers;
	StopwatchPair memwaStackTimers;
	TestMemwaPoolReverse64( memwaPoolTimers );
	TestMemwaTinyReverse64( memwaTinyTimers );
	TestMemwaStackReverse64( memwaStackTimers );
	TestDefaultReverse64( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Reverse Order Performance Test." << std::endl
		<< "\t Object size is 64 bytes." << std::endl
		<< "\t Release in reverse order of Allocation." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaTinyTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaTinyTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Tiny:  " << memwaTinyTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaTinyTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaStackTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaStackTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Stack: " << memwaStackTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaStackTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoForwardTests256()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	TestMemwaPoolForward256( memwaPoolTimers );
	TestDefaultForward256( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Forward Order Performance Test." << std::endl
		<< "\t Object size is 256 bytes." << std::endl
		<< "\t Release in same order as Allocation." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoReverseTests256()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaStackTimers;
	TestMemwaPoolReverse256( memwaPoolTimers );
	TestMemwaStackReverse256( memwaStackTimers );
	TestDefaultReverse256( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Reverse Order Performance Test." << std::endl
		<< "\t Object size is 256 bytes." << std::endl
		<< "\t Release in reverse order of Allocation." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaStackTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaStackTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Stack: " << memwaStackTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaStackTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoRandomTests8()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaTinyTimers;
	TestMemwaPoolRandom8( memwaPoolTimers );
	TestMemwaTinyRandom8( memwaTinyTimers );
	TestDefaultRandom8( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Random Order Performance Test." << std::endl
		<< "\t Object size is 8 bytes." << std::endl
		<< "\t Allocations and Release occur in random order." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaTinyTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaTinyTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Tiny:  " << memwaTinyTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaTinyTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoRandomTests64()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	StopwatchPair memwaTinyTimers;
	TestMemwaPoolRandom64( memwaPoolTimers );
	TestMemwaTinyRandom64( memwaTinyTimers );
	TestDefaultRandom64( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Random Order Performance Test." << std::endl
		<< "\t Object size is 64 bytes." << std::endl
		<< "\t Allocations and Release occur in random order." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaTinyTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaTinyTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Tiny:  " << memwaTinyTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaTinyTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl << std::endl;
}

// ----------------------------------------------------------------------------

void DoRandomTests256()
{
	StopwatchPair defaultTimers;
	StopwatchPair memwaPoolTimers;
	TestMemwaPoolRandom256( memwaPoolTimers );
	TestDefaultRandom256( defaultTimers );

	float allocateRatio = 1.0F;
	float releaseRatio  = 1.0F;

	std::cout << "Random Order Performance Test." << std::endl
		<< "\t Object size is 256 bytes." << std::endl
		<< "\t Allocations and Release occur in random order." << std::endl
		<< "\t Times are in microseconds." << std::endl;
	std::cout << "Allocator    Allocate           Release" << std::endl;
	std::cout << "             Time      Ratio    Time     Ratio" << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << "Default:     " << defaultTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << defaultTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
	allocateRatio = static_cast< float > ( memwaPoolTimers.allocateTimer.GetDuration() ) / static_cast< float > ( defaultTimers.allocateTimer.GetDuration() );
	releaseRatio  = static_cast< float > ( memwaPoolTimers.releaseTimer.GetDuration()  ) / static_cast< float > ( defaultTimers.releaseTimer.GetDuration() );
	std::cout << "Memwa Pool:  " << memwaPoolTimers.allocateTimer.GetDuration() << '\t' << allocateRatio << '\t' << memwaPoolTimers.releaseTimer.GetDuration() << '\t' << releaseRatio << std::endl;
}

// ----------------------------------------------------------------------------

int main( int argc, const char * const argv[] )
{
	const CommandLineArgs args( argc, argv );
	if ( !args.IsValid() )
	{
		std::cout << "Your command line parameters are invalid!" << std::endl;
		args.ShowHelp();
		return 1;
	}
	if ( args.DoShowHelp() )
	{
		args.ShowHelp();
		return 0;
	}

	if ( args.GetLoopCount() != 0 )
	{
		SetLoopCount( args.GetLoopCount() );
	}
	std::cout << "Allocating " << GetLoopCount() << " objects per test." << std::endl << std::endl;
	std::cout.precision( 2 );
	std::cout.width( 6 );
	std::cout.setf( std::ios::right );
	std::cout.setf( std::ios::fixed );

	if ( args.DoForwardTest() )
	{
		DoForwardTests8();
		DoForwardTests64();
		DoForwardTests256();
	}
	if ( args.DoReverseTest() )
	{
		DoReverseTests8();
		DoReverseTests64();
		DoReverseTests256();
	}
	if ( args.DoRandomTest() )
	{
		DoRandomTests8();
		DoRandomTests64();
		DoRandomTests256();
	}

	return 0;
}

// ----------------------------------------------------------------------------

//		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
