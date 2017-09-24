
#pragma once

#include "UnitTest.hpp"

// ----------------------------------------------------------------------------

/** @class CommandLineArgs
 @brief Parses command line arguements, determines if they are valid, and then
  makes the information encoded within the arguements available.
 */
class CommandLineArgs
{
public:

	/** Parses through and validates the command line parameters.
	 @param[in] argc Count of parameters.
	 @param[in] argv Array of parameters.
	 */
	CommandLineArgs( unsigned int argc, const char * const argv[] );

	inline ~CommandLineArgs( void ) {}

	void ShowHelp( void ) const;

	inline bool IsValid( void ) const { return m_valid; }

	inline bool DoShowHelp( void ) const { return m_doShowHelp; }

	inline bool DoShowDataSizes() const { return m_showDataSizes; }

	inline bool DoOnlyPassingTest( void ) const
	{ return m_doOnlyPassingTests; }

	inline bool DoNoTests( void ) const { return m_doNoTests; }

	inline bool DoFatalTest( void ) const { return m_doFatalTest; }

	inline bool DoAssertOnFail( void ) const { return m_doAssertOnFail; }

	inline bool DoRepeatTests( void ) const { return m_doRepeatTests; }

	inline bool DeleteAtExitTime( void ) const { return m_deleteAtExitTime; }

	inline bool DoMakeTableAtExitTime( void ) const
	{ return m_tableAtExitTime; }

	ut::UnitTestSet::OutputOptions GetOutputOptions( void ) const
	{
		return static_cast< ut::UnitTestSet::OutputOptions >
			( m_outputOptions );
	}

	inline const char * GetHtmlFileName( void ) const
	{ return m_htmlFileName; }

	inline const char * GetTextFileName( void ) const
	{ return m_textFileName; }

	inline const char * GetXmlFileName( void ) const
	{ return m_xmlFileName; }

	inline const char * GetExeName( void ) const { return m_exeName; }

private:

	CommandLineArgs( void );
	CommandLineArgs( const CommandLineArgs & );
	CommandLineArgs & operator = ( const CommandLineArgs & );

	bool ParseOutputOptions( const char * ss );

	bool m_valid;		///< True if all command line parameters are valid.
	bool m_doShowHelp;
	bool m_showDataSizes;
	bool m_doFatalTest;
	bool m_doNoTests;
	bool m_doOnlyPassingTests;
	bool m_doAssertOnFail;
	bool m_doRepeatTests;
	bool m_tableAtExitTime;
	bool m_deleteAtExitTime;
	unsigned int m_outputOptions;
	const char * m_exeName;
	const char * m_xmlFileName;
	const char * m_htmlFileName;
	const char * m_textFileName;
};

// ----------------------------------------------------------------------------
