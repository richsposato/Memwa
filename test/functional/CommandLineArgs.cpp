
#include "CommandLineArgs.hpp"

#include <iostream>

#include <cstring>

using namespace std;


// ----------------------------------------------------------------------------

void CommandLineArgs::ShowHelp( void ) const
{
	cout << "Usage: " << m_exeName << endl;
	cout << " [-f] [-p] [-z] [-a] [-o:[ndhmptw]]" << endl;
	cout << " [-s] [-t:file] [-h:file] [-x:file] [-?] [--help]" << endl;
	cout << endl;
	cout << "Parameters: (order of parameters does not matter)" << endl;
	cout << "  -f  Do fatal tests.  Causes program to end abruptly." << endl;
	cout << "	  Incompatible with -p and -z." << endl;
	cout << "  -p  Do only passing tests." << endl;
	cout << "	  Incompatible with -f and -z." << endl;
	cout << "  -z  Do not do any tests - just make summary table." << endl;
	cout << "	  Incompatible with -f, -p, and -r." << endl;
	cout << "  -a  Assert when test fails." << endl;
	cout << "  -L  Let singleton live at exit time, do not delete it." << endl;	
	cout << "  -o  Set output options." << endl;
	cout << "	  n  No extra output options." << endl;
	cout << "		 This is incompatible with any other output option."
		 << endl;
	cout << "	  f  Use full weekday names, not abbreviations." << endl;
	cout << "	  h  Show headers for each unit test that fails." << endl;
	cout << "	  i  Show test index in each unit test output line." << endl;
	cout << "	  m  Show messages even if no test with message." << endl;
	cout << "	  p  Show contents of passing tests." << endl;
	cout << "	  t  Show beginning and ending timestamps." << endl;
	cout << "	  T  Show summary table once tests are done." << endl;
	cout << "	  w  Show failing warnings." << endl;
	cout << "	  d  Show divider lines in table and sections." << endl;
	cout << "	  D  Show default output options." << endl;
	cout << "		 You may combine this with other options." << endl;
	cout << "		 Default output option is same as -o:hmtTw." << endl;
	cout << "	  E  Send test results to standard error." << endl;
	cout << "	  S  Send test results to standard output." << endl;
	cout << "  -t  Send test results to text file." << endl;
	cout << "		\"file\" is a partial file name." << endl;
	cout << "  -h  Send test results to HTML file." << endl;
	cout << "		\"file\" is a partial file name for main HTML page."
		 << endl;
	cout << "  -x  Send test results to xml file." << endl;
	cout << "		\"file\" is a partial file name." << endl;
	cout << "  -e  Show summary table at program exit time." << endl;
	cout << "  -r  Show summary table and then repeat tests." << endl;
	cout << "	  Incompatible with -z." << endl;
	cout << "  -?  Show this help information." << endl;
	cout << "	  Help is mutually exclusive with any other arguement."
		 << endl;
	cout << "  --help  Show this help information." << endl;
}

// ----------------------------------------------------------------------------

CommandLineArgs::CommandLineArgs( unsigned int argc, const char * const argv[] ) :
	m_valid( false ),
	m_doShowHelp( false ),
	m_showDataSizes( false ),
	m_doFatalTest( false ),
	m_doNoTests( false ),
	m_doOnlyPassingTests( false ),
	m_doAssertOnFail( false ),
	m_doRepeatTests( false ),
	m_tableAtExitTime( false ),
	m_deleteAtExitTime( true ),
	m_outputOptions( ut::UnitTestSet::Nothing ),
	m_exeName( argv[0] ),
	m_xmlFileName( NULL ),
	m_htmlFileName( NULL ),
	m_textFileName( NULL )
{

	if ( 1 == argc )
	{
		return;
	}

	bool okay = true;
	bool parsedOutput = false;

	for ( unsigned int ii = 1; ( okay ) && ( ii < argc ); ++ii )
	{
		const char * ss = argv[ ii ];
		if ( ( NULL == ss ) || ( '-' != ss[0] ) )
		{
			okay = false;
			break;
		}
		const unsigned int length =
			static_cast< unsigned int >( strlen( ss ) );

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
			case 'a':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_doAssertOnFail;
				if ( okay )
					m_doAssertOnFail = true;
				break;
			case 'e':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_tableAtExitTime;
				if ( okay )
					m_tableAtExitTime = true;
				break;
			case 'f':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_doFatalTest;
				if ( okay )
					m_doFatalTest = true;
				break;
			case 'h':
				okay = ( 3 < length ) && ( ':' == ss[2] );
				if ( okay )
					okay = ( NULL == m_htmlFileName );
				if ( okay )
					m_htmlFileName = ss + 3;
				break;
			case 'L':
				okay = ( length == 2 );
				if ( okay )
					okay = m_deleteAtExitTime;
				if ( okay )
					m_deleteAtExitTime = false;
				break;
			case 'o':
				okay = ( 3 < length ) && ( ':' == ss[2] );
				if ( okay )
					okay = !parsedOutput;
				if ( okay )
					okay = ParseOutputOptions (ss+3);
				parsedOutput = true;
				break;
			case 'p':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_doOnlyPassingTests;
				if ( okay )
					m_doOnlyPassingTests = true;
				break;
			case 'r':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_doRepeatTests;
				if ( okay )
					m_doRepeatTests = true;
				break;
			case 't':
				okay = ( 3 < length ) && ( ':' == ss[2] );
				if ( okay )
					okay = ( NULL == m_textFileName );
				if ( okay )
					m_textFileName = ss + 3;
				break;
			case 'x':
				okay = ( 3 < length ) && ( ':' == ss[2] );
				if ( okay )
					okay = ( NULL == m_xmlFileName );
				if ( okay )
					m_xmlFileName = ss + 3;
				break;
			case 'z':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_doNoTests;
				if ( okay )
					m_doNoTests = true;
				break;
			case 'D':
				okay = ( length == 2 );
				if ( okay )
					okay = !m_showDataSizes;
				if ( okay )
					m_showDataSizes = true;
				break;
			default:
				okay = false;
				break;
		}
	}

	const bool standardError =
		( 0 != ( m_outputOptions & ut::UnitTestSet::SendToCerr ) );
	const bool standardOutput =
		( 0 != ( m_outputOptions |= ut::UnitTestSet::SendToCout ) );
	const bool noOutput = ( !m_doFatalTest ) && ( NULL == m_xmlFileName )
			&& ( !standardError ) && ( !standardOutput )
			&& ( NULL == m_textFileName ) && ( NULL == m_htmlFileName );
	if ( m_doShowHelp && okay )
	{
		okay = noOutput;
	}
	else
	{
		if ( m_doOnlyPassingTests && m_doFatalTest )
			okay = false;
		if ( m_doNoTests && m_doFatalTest )
			okay = false;
		if ( m_doNoTests && m_doOnlyPassingTests )
			okay = false;
		if ( m_doNoTests && m_doRepeatTests )
			okay = false;
		if ( noOutput )
			okay = false;
	}
	m_valid = okay;
}

// ----------------------------------------------------------------------------

bool CommandLineArgs::ParseOutputOptions( const char * ss )
{

	bool okay = true;
	bool useDefault = false;
	bool useFullNames = false;
	bool showNothing = false;
	bool showHeader = false;
	bool showMessages = false;
	bool showPasses = false;
	bool showTimeStamp = false;
	bool showWarnings = false;
	bool showDividers = false;
	bool showIndexes = false;
	bool showSummaryTable = false;
	bool standardError = false;
	bool standardOutput = false;

	while ( okay && ( *ss != '\0' ) )
	{
		const char cc = *ss;
		switch ( cc )
		{
			case 'D':
				if ( useDefault )
					okay = false;
				else
					useDefault = true;
				break;
			case 'd':
				if ( showDividers )
					okay = false;
				else
					showDividers = true;
				break;
			case 'E':
				if ( standardError )
					okay = false;
				else
					standardError = true;
				break;
			case 'S':
				if ( standardOutput )
					okay = false;
				else
					standardOutput = true;
				break;
			case 'f':
				if ( useFullNames )
					okay = false;
				else
					useFullNames = true;
				break;
			case 'i':
				if ( showIndexes )
					okay = false;
				else
					showIndexes = true;
				break;
			case 'n':
				if ( showNothing )
					okay = false;
				else
					showNothing = true;
				break;
			case 'h':
				if ( showHeader )
					okay = false;
				else
					showHeader = true;
				break;
			case 'm':
				if ( showMessages )
					okay = false;
				else
					showMessages = true;
				break;
			case 'p':
				if ( showPasses )
					okay = false;
				else
					showPasses = true;
				break;
			case 't':
				if ( showTimeStamp )
					okay = false;
				else
					showTimeStamp = true;
				break;
			case 'T':
				if ( showSummaryTable )
					okay = false;
				else
					showSummaryTable = true;
				break;
			case 'w':
				if ( showWarnings )
					okay = false;
				else
					showWarnings = true;
				break;
			default:
				okay = false;
				break;
		}
		++ss;
	}

	const bool noOptions =
		( !showTimeStamp ) && ( !showDividers )
		&& ( !showPasses ) && ( !showMessages )
		&& ( !showHeader ) && ( !showWarnings );
	if ( showNothing )
		okay = noOptions;
	if ( noOptions && ( !showNothing ) )
		useDefault = true;
	if ( useDefault )
		m_outputOptions = ut::UnitTestSet::Default;
	if ( showDividers )
		m_outputOptions |= ut::UnitTestSet::Dividers;
	if ( useFullNames )
		m_outputOptions |= ut::UnitTestSet::FullDayName;
	if ( showHeader )
		m_outputOptions |= ut::UnitTestSet::Headers;
	if ( showMessages )
		m_outputOptions |= ut::UnitTestSet::Messages;
	if ( showPasses )
		m_outputOptions |= ut::UnitTestSet::Passes;
	if ( showTimeStamp )
		m_outputOptions |= ut::UnitTestSet::TimeStamp;
	if ( showWarnings )
		m_outputOptions |= ut::UnitTestSet::Warnings;
	if ( showIndexes )
		m_outputOptions |= ut::UnitTestSet::AddTestIndex;
	if ( showSummaryTable )
		m_outputOptions |= ut::UnitTestSet::SummaryTable;
	if ( standardError )
		m_outputOptions |= ut::UnitTestSet::SendToCerr;
	if ( standardOutput )
		m_outputOptions |= ut::UnitTestSet::SendToCout;

	return okay;
}

// ----------------------------------------------------------------------------
