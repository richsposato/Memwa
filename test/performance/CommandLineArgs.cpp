
#include "CommandLineArgs.hpp"

#include <iostream>

#include <cstring>

using namespace std;


// ----------------------------------------------------------------------------

CommandLineArgs::CommandLineArgs( unsigned int argc, const char * const argv[] ) :
	m_valid( false ),
	m_doShowHelp( false ),
	m_forwardTest( false ),
	m_reverseTest( false ),
	m_randomTest( false ),
	m_loopCount( 0 ),
	m_exeName( argv[0] )
{

	if ( 1 == argc )
	{
		return;
	}

	bool okay = true;

	for ( unsigned int ii = 1; ( okay ) && ( ii < argc ); ++ii )
	{
		const char * ss = argv[ ii ];
		if ( ( NULL == ss ) || ( '-' != ss[0] ) )
		{
			okay = false;
			break;
		}
		const unsigned int length = static_cast< unsigned int >( strlen( ss ) );

		const char cc = ss[1];
		switch ( cc )
		{
			case '-':
				okay = ( strcmp( ss, "--help" ) == 0 );
				if ( okay )
					okay = !m_doShowHelp;
				if ( okay )
					m_doShowHelp = true;
				break;
			case '?':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_doShowHelp;
				if ( okay )
					m_doShowHelp = true;
				break;
			case 'f':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_forwardTest;
				if ( okay )
					m_forwardTest = true;
				break;
			case 'b':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_reverseTest;
				if ( okay )
					m_reverseTest = true;
				break;
			case 'r':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_randomTest;
				if ( okay )
					m_randomTest = true;
				break;
			case 'l':
				okay = ( length > 3 ) && ( '=' == ss[2] ) && ( std::isdigit( ss[3] ) );
				if ( okay )
					okay = ( 0 == m_loopCount );
				if ( okay )
					m_loopCount = std::atoi( ss+3 );
				break;
			default:
				okay = false;
				break;
		}
	}

	m_valid = okay;
}

// ----------------------------------------------------------------------------

void CommandLineArgs::ShowHelp( void ) const
{
	cout << "Usage: " << m_exeName << endl;
	cout << " [-f] [-b] [-l=#] [-?] [--help]" << endl;
	cout << endl;
	cout << "Parameters: (order of parameters does not matter)" << endl;
	cout << "  -r  Do random order tests." << endl;
	cout << "  -b  Do reverse order tests." << endl;
	cout << "  -f  Do forward order tests." << endl;
	cout << "  -l  Set loop count. Default is 100,000." << endl;
	cout << "  -?  Show this help information." << endl;
	cout << "	  Help is mutually exclusive with any other arguement." << endl;
	cout << "  --help  Show this help information." << endl;
}

// ----------------------------------------------------------------------------
