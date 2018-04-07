
#pragma once

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

	inline bool DoSingleThreadedTest() const;

	inline bool DoMultiThreadedTest() const;

	inline bool DoForwardTest() const { return m_forwardTest; }

	inline bool DoReverseTest() const { return m_reverseTest; }

	inline bool DoRandomTest() const { return m_randomTest; }

	inline unsigned int GetLoopCount() const { return m_loopCount; }

	inline const char * GetExeName( void ) const { return m_exeName; }

private:

	CommandLineArgs( void );
	CommandLineArgs( const CommandLineArgs & );
	CommandLineArgs & operator = ( const CommandLineArgs & );

	bool ParseOutputOptions( const char * ss );
	bool ParseTestTypeOptions( const char * ss );

	bool m_valid;		///< True if all command line parameters are valid.
	bool m_doShowHelp;
	bool m_forwardTest;
	bool m_reverseTest;
	bool m_randomTest;
	unsigned int m_loopCount;
	const char * m_exeName;
};

// ----------------------------------------------------------------------------
