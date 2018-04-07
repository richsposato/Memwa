
#pragma once

#include "Stopwatch.hpp"

#include "../functional/ChunkList.hpp"

#include <cassert>
#include <cstdlib>

// ----------------------------------------------------------------------------

void SetLoopCount( unsigned int count );

unsigned int GetLoopCount();

// ----------------------------------------------------------------------------

template< class Object >
bool TestSameOrder( StopwatchPair & timers )
{

	ChunkList chunks( GetLoopCount() );
	Object * place = nullptr;

	for ( unsigned int ii = 0; ii < GetLoopCount(); ++ii )
	{
		timers.allocateTimer.Start();
		place = new Object;
		timers.allocateTimer.Stop();
		chunks.AddChunk( place );
	}

	while ( chunks.GetCount() != 0 )
	{
		void * p = chunks.GetChunk( 0 );
		place = reinterpret_cast< Object * >( p );
		timers.releaseTimer.Start();
		delete place;
		timers.releaseTimer.Stop();
		chunks.RemoveChunk( 0 );
	}

	assert( chunks.GetCount() == 0 );
	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestReverseOrder( StopwatchPair & timers )
{

	ChunkList chunks( GetLoopCount() );
	Object * place = nullptr;

	for ( unsigned int ii = 0; ii < GetLoopCount(); ++ii )
	{
		timers.allocateTimer.Start();
		place = new Object;
		timers.allocateTimer.Stop();
		chunks.AddChunk( place );
	}

	while ( chunks.GetCount() != 0 )
	{
		void * p = chunks.GetTopChunk();
		place = reinterpret_cast< Object * >( p );
		timers.releaseTimer.Start();
		delete place;
		timers.releaseTimer.Stop();
		chunks.RemoveTopChunk();
	}

	assert( chunks.GetCount() == 0 );
	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestRandomOrder( StopwatchPair & timers )
{

	ChunkList chunks( GetLoopCount() );
	Object * place = nullptr;

	for ( unsigned int ii = 0; ii < GetLoopCount(); ++ii )
	{
		const Actions action = ChooseAction( chunks );
		switch ( action )
		{
			case Actions::AllocateOne :
			{
				timers.allocateTimer.Start();
				place = new Object;
				timers.allocateTimer.Stop();
				chunks.AddChunk( place );
				break;
			}
			case Actions::AllocateMany :
			{
				const unsigned int count = rand() % 256;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					timers.allocateTimer.Start();
					place = new Object;
					timers.allocateTimer.Stop();
					chunks.AddChunk( place );
				}
				break;
			}
			case Actions::AllocateOneHint :
			case Actions::AllocateManyHint :
			{
				break;
			}
			case Actions::ReleaseOneTop :
			{
				void * p = chunks.GetTopChunk();
				place = reinterpret_cast< Object * >( p );
				timers.releaseTimer.Start();
				delete place;
				timers.releaseTimer.Stop();
				chunks.RemoveTopChunk();
				break;
			}
			case Actions::ReleaseManyTop :
			{
				const unsigned int countBefore = chunks.GetCount();
				unsigned int count = rand() % 128;
				if ( count > countBefore )
				{
					count = rand() % countBefore;
				}
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					void * p = chunks.GetTopChunk();
					place = reinterpret_cast< Object * >( p );
					timers.releaseTimer.Start();
					delete place;
					timers.releaseTimer.Stop();
					chunks.RemoveTopChunk();
				}
				break;
			}
			case Actions::ReleaseOneBottom :
			{
				void * p = chunks.GetChunk( 0 );
				place = reinterpret_cast< Object * >( p );
				timers.releaseTimer.Start();
				delete place;
				timers.releaseTimer.Stop();
				chunks.RemoveChunk( 0 );
				break;
			}
			case Actions::ReleaseManyBottom :
			{
				const unsigned int countBefore = chunks.GetCount();
				unsigned int count = rand() % 128;
				if ( count > countBefore )
				{
					count = rand() % countBefore;
				}
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					void * p = chunks.GetChunk( 0 );
					place = reinterpret_cast< Object * >( p );
					timers.releaseTimer.Start();
					delete place;
					timers.releaseTimer.Stop();
					chunks.RemoveChunk( 0 );
				}
				break;
			}
			case Actions::ReleaseOneRandom :
			{
				const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
				place = reinterpret_cast< Object * >( spot.first );
				timers.releaseTimer.Start();
				delete place;
				timers.releaseTimer.Stop();
				unsigned int index = spot.second;
				chunks.RemoveChunk( index );
				break;
			}
			case Actions::ReleaseManyRandom :
			{
				const unsigned int countBefore = chunks.GetCount();
				unsigned int count = rand() % 128;
				if ( count > countBefore )
				{
					count = rand() % countBefore;
				}
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
					place = reinterpret_cast< Object * >( spot.first );
					timers.releaseTimer.Start();
					delete place;
					timers.releaseTimer.Stop();
					unsigned int index = spot.second;
					chunks.RemoveChunk( index );
				}
				break;
			}
			default:
			{
				assert( false );
			}
		}
	}

	while ( chunks.GetCount() != 0 )
	{
		void * p = chunks.GetTopChunk();
		place = reinterpret_cast< Object * >( p );
		timers.releaseTimer.Start();
		delete place;
		timers.releaseTimer.Stop();
		chunks.RemoveTopChunk();
	}

	return true;
}

// ----------------------------------------------------------------------------
