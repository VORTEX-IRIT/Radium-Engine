#pragma once
#include <Headless/CLI/CLI.hpp>
#include <Headless/RaHeadless.hpp>

namespace Ra {
namespace Headless {
/**
 * Base class for command line interface Radium application.
 * This class is just a command-line parser (simple wrapper to a CLI::App object).
 * Derived application should populate the parser using the addOptions() or addFlags methods.
 * Derived applications can also use the CLI11 API to populate the protected member m_cmdLineParser.
 *
 * Once the parser is populated, derived application should call the inherited init() method
 * which only parse the command line and return the result of this parsing.
 *
 * \see https://cliutils.github.io/CLI11/book/ for a description of the wrapped commandline parser.
 */
class HEADLESS_API CLIBaseApplication
{

  protected:
    /** Command-line parameters parser.
     * Derived classes should extend this parser to add their own parameters
     */
    CLI::App m_cmdLineParser;

  public:
    /// Base constructor.
    CLIBaseApplication() = default;
    /// Base destructor
    virtual ~CLIBaseApplication() = default;

    /// adapter allowing to add a command line option on an application the same way than using
    /// CLI11 directly.
    /// \see https://cliutils.github.io/CLI11/book/chapters/options.html
    template <typename... Args>
    CLI::Option* addOption( Args&&... args );

    /// adapter allowing to add a command line flag on an application the same way than using CLI11
    /// directly.
    /// \see https://cliutils.github.io/CLI11/book/chapters/flags.html
    template <typename... Args>
    CLI::Option* addFlag( Args&&... args );

    /**
     * Application initialization method.
     *
     * This method should be called by the derived application to parse the command line.
     * Overriden method should use the command line parsing result to initialize their internal
     * state and return 0 if initialization succeeded or an application dependant error code if not.
     *
     * \param argc number of parameter
     * \param argv array of string representing the parameters
     * \return 0 if the command line parsing success or the parsing error code if something went
     * wrong (see
     * https://cliutils.github.io/CLI11/class_c_l_i_1_1_app.html#aac000657ef11647125ba91af38fd7d9c).
     */
    virtual int init( int argc, const char* argv[] );
};

template <typename... Args>
CLI::Option* CLIBaseApplication::addOption( Args&&... args ) {
    return m_cmdLineParser.add_option( args... );
}

template <typename... Args>
CLI::Option* CLIBaseApplication::addFlag( Args&&... args ) {
    return m_cmdLineParser.add_flag( args... );
}

} // namespace Headless
} // namespace Ra
