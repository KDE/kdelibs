/* This file is part of the KDE project
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KCMDLINEARGS_H_
#define _KCMDLINEARGS_H_

#include <kurl.h>

#include <qptrlist.h>
#include <qstring.h>
#include <qvaluelist.h>

typedef QValueList<QCString> QCStringList;

/**
 * @short Structure that holds command line options.
 */
struct KCmdLineOptions
{
   const char *name;
   const char *description;
   const char *def; // Default
};

#define KCmdLineLastOption { 0, 0, 0 }

class KCmdLineArgsList;
class KApplication;
class KUniqueApplication;
class KCmdLineParsedOptions;
class KCmdLineParsedArgs;
class KAboutData;
class KCmdLineArgsPrivate;

/**
 *  Simple access to the command-line arguments.
 *
 *  It takes into account Qt-specific options, KDE-specific options
 *  and application specific options.
 *
 *  This class is used in main() via the static method
 *  @ref init().
 *
 *  A typical KDE application should look like this:
 *
 *  \code
 *  int main(int argc, char *argv[])
 *  {
 *     // Initialize command line args
 *     KCmdLineArgs::init(argc, argv, appName, description, version);
 *
 *     // Tell which options are supported
 *     KCmdLineArgs::addCmdLineOptions( options );
 *
 *     // Add options from other components
 *     KUniqueApplication::addCmdLineOptions();
 *
 *     ....
 *
 *     // Create application object without passing 'argc' and 'argv' again.
 *     KUniqueApplication app;
 *
 *     ....
 *
 *     // Handle our own options/argments
 *     // A KApplication will usually do this in main but this is not
 *     // necassery.
 *     // A KUniqueApplication might want to handle it in newInstance().
 *
 *     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 *
 *     // A binary option (on / off)
 *     if (args->isSet("some-option"))
 *        ....
 *
 *     // An option which takes an additional argument
 *     QCString anotherOptionArg = args->getOption("another-option");
 *
 *     // Arguments (e.g. files to open)
 *     for(int i = 0; i < args->count(); i++) // Counting start at 0!
 *     {
 *        // don't forget to convert to Unicode!
 *        openFile( QFile::decodeName( args->arg(i)));
 *        // Or more convenient:
 *        // openURL( args->url(i));
 *
 *     }
 *
 *     args->clear(); // Free up some memory.
 *     ....
 *  }
 *  \endcode
 *
 *  options are defined as follow
 *
 *  <code>
 *  static KCmdLineOptions options[] =
 *  {
 *     { "a", I18N_NOOP("A short binary option."), 0 },
 *     { "b <file>", I18N_NOOP("A short option which takes an argument."), 0 },
 *     { "c <speed>", I18N_NOOP("As above but with a default value."), "9600" },
 *     { "option1", I18N_NOOP("A long binary option, off by default."), 0 },
 *     { "nooption2", I18N_NOOP("A long binary option, on by default."), 0 },
 *     { "option3 <file>", I18N_NOOP("A long option which takes an argument."), 0 },
 *     { "option3 <speed>", I18N_NOOP("As above with 9600 as default."), "9600" },
 *     { "d", 0, 0 },
 *     { "option4", I18N_NOOP("A long option which has a short option as alias."), 0 },
 *     { "e", 0, 0 },
 *     { "nooption5", I18N_NOOP("Another long option with an alias."), 0 },
 *     { "f", 0, 0 },
 *     { "option6 <speed>", I18N_NOOP("'--option6 speed' is same a '-f speed'"), 0 },
 *     { "!option7 <cmd>", I18N_NOOP("All options following this one will be treated as arguments", 0 },
 *     { "+file", I18N_NOOP("A required argument 'file'.), 0 },
 *     { "+[arg1]", I18N_NOOP("An optional argument 'arg1'."), 0 },
 *     { "!+command", I18N_NOOP("A required argument 'command', that can contain multiple words, even starting with '-'.), 0 },
 *     { 0, 0, 0 } // End of options.
 *  };
 *  </code>
 *
 *  The I18N_NOOP macro is used to indicate that these strings should be
 *  marked for translation. The actual translation is done by KCmdLineArgs.
 *  You can't use i18n() here because we are setting up a static data
 *  structure and can't do translations at compile time.
 *
 *  Note that a program should define the options before any arguments.
 *
 *  When a long option has a short option as alias. A program should
 *  only check for the long option.
 *
 *  With the above options a command line could look like:
 *  <pre>
 *     myapp -a -c 4800 --display localhost:0.0 --nooption5 -d /tmp/file
 *  </pre>
 *
 *  Long binary options can be in the form 'option' and 'nooption'.
 *  A command line may contain the same binary option multiple times,
 *  the last option determines the outcome:
 *  <pre>
 *     myapp --nooption4 --option4 --nooption4
 *  </pre>
 *  is the same as:
 *  <pre>
 *     myapp --nooption4
 *  </pre>
 *
 *  Normally if an option value is provided multiple times only the last
 *  value is used:
 *  <pre>
 *     myapp -c 1200 -c 2400 -c 4800
 *  </pre>
 *  is usually the same as:
 *  <pre>
 *     myapp -c 4800
 *  </pre>
 *
 *  However, an application can choose to use all values specified as well.
 *  E.g. to specify a number of directories to use:
 *  <pre>
 *     myapp -I /usr/include -I /opt/kde/include -I /usr/X11/include
 *  </pre>
 *  When an application does this it should mention this in the description
 *  of the option. @ref getOptionList()
 *
 *  Tips for end-users:
 *
 *  @li Single char options like "-a -b -c" may be combined into "-abc"
 *  @li The option "--foo bar" may also be written "--foo=bar"
 *  @li The option "-P lp1" may also be written "-P=lp1" or "-Plp1"
 *  @li The option "--foo bar" may also be written "-foo bar"
 *
 *  @short A class for command-line argument handling.
 *  @author Waldo Bastian
 *  @version 0.0.4
 */
class KCmdLineArgs
{
  friend class KApplication;
  friend class KUniqueApplication;
  friend class QPtrList<KCmdLineArgs>;
public:
  // Static functions:

  /**
   * Initialize class.
   *
   * This function should be called as the very first thing in
   *  your application.
   * @param argc As passed to @p main(...).
   * @param argv As passed to @p main(...).
   * @param appname The untranslated name of your application. This should
   *                match with @p argv[0].
   * @param description A short description of what your application is about.
   * @param version A version.
   * @param noKApp Don't add commandline options for @ref QApplication/@ref KApplication
   */
  static void init(int _argc, char **_argv,
                   const char *_appname, const char *_description,
                   const char *_version, bool noKApp = false);

  /**
   * Initialize class.
   *
   * This function should be called as the very first thing in
   *  your application.
   * @param argc As passed to @p main(...).
   * @param argv As passed to @p main(...).
   * @param about A KAboutData object describing your program.
   * @param noKApp Don't add commandline options for @ref QApplication / @ref KApplication
   */
  static void init(int _argc, char **_argv,
                   const KAboutData *about, bool noKApp = false);

  /**
   * Initialize Class
   *
   * This function should be called as the very first thing in
   *  your application.
   * This method is exactly the same as calling
   * @ref init(0,0, const KAboutData *about, true)
   * This method will rarely be used
   *
   */
  static void init(const KAboutData *about);

  /**
   * Add options to your application.
   *
   * You must make sure that all possible options have been added before
   * any class uses the command line arguments.
   *
   * @param options A list of options thath your code supplies.
   * @param id A name with which these options can be identified.
   * @param afterId The options are inserted after this set of options.
   *
   * The list of options should look like this:
   *
   * static KCmdLineOptions options[] =
   * {
   *    { "option1 <argument>", I18N_NOOP("Description 1"), "default" },
   *    { "o", 0, 0 },
   *    { "option2", I18N_NOOP("Description 2"), 0 },
   *    { "nooption3", I18N_NOOP("Description 3"), 0 },
   *    { 0, 0, 0}
   * }
   *
   * @li "option1" is an option that requires an additional argument
   * @li "option2" is an option that can be turned on. The default is off.
   * @li "option3" is an option that can be turned off. The default is on.
   * @li "o" does not have a description. It is an alias for the option
   *     that follows. In this case "option2".
   * @li "+file" specifies an argument. The '+' is removed. If your program
   *     doesn't specify that it can use arguments your program will abort
   *     when an argument is passed to it.
   *
   * In BNF:
   * cmd = myapp [options] file
   * options = (option)*
   * option = --option1 <argument> |
   *          (-o | --option2 | --nooption2) |
   *          ( --option3 | --nooption3 )
   *
   * Instead of "--option3" one may also use "-option3"
   *
   * Usage examples:
   *
   * @li "myapp --option1 test"
   * @li "myapp" (same as "myapp --option1 default")
   * @li "myapp --option2"
   * @li "myapp --nooption2" (same as "myapp")
   * @li "myapp -o" (same as "myapp --option2")
   * @li "myapp --nooption3"
   * @li "myapp --option3 (same as "myapp")
   * @li "myapp --option2 --nooption2" (same as "myapp")
   * @li "myapp /tmp/file"
   */
  static void addCmdLineOptions( const KCmdLineOptions *options,
				 const char *name=0, const char *id = 0,
				 const char *afterId=0);

  /**
   * Access parsed arguments.
   *
   * This function returns all command line arguments that your code
   * handles. If unknown command-line arguments are encountered the program
   * is aborted and usage information is shown.
   *
   * @param id The name of the options you are interested in.
   */
  static KCmdLineArgs *parsedArgs(const char *id=0);

  /**
   * Get the CWD (Current Working Directory) associated with the
   * current command line arguments.
   *
   * Typically this is needed in KUniqueApplication::newInstance()
   * since the CWD of the process may be different from the CWD
   * where the user started a second instance.
   **/
  static QString cwd();

  /**
   * Get the appname according to argv[0]
   **/
  static const char *appName();

  /**
   * Print the usage help to stdout and exit.
   *
   * @param complete If true, print all available options.
   **/
  static void usage(const char *id = 0);

  /**
   * Print an error to stderr and the usage help to stdout and exit.
   *
   **/
  static void usage(const QString &error);

  /**
   * Enable i18n to be able to print a translated error message.
   *
   * N.B.: This function leaks memory, therefore you are expected to exit
   * afterwards (e.g., by calling @ref usage()).
   **/
  static void enable_i18n();

  // Member functions:


  /**
   *  Read out a string option.
   *
   *  @param option The name of the option without '-'.
   *
   *  @return The value of the option. If the option was not
   *          present on the command line the default is returned.
   *          If the option was present more than the value of the
   *          last occurence is used.
   */
  QCString getOption(const char *option) const;

  /**
   *  Read out all occurences of a string option.
   *
   *  @param option The name of the option without '-'.
   *
   *  @return A list of all option values. If no option was present
   *          on the command line, an empty list is returned.
   */
  QCStringList getOptionList(const char *option) const;

  /**
   *  Read out a boolean option or check for the presence of string option.
   *
   *  @param option The name of the option without '-' or '-no'.
   *
   *  @return The value of the option. If the option was not
   *          present on the command line the default is returned.
   *          If the option is listed as 'no<option>' the
   *          default is true.
   *          If the option is listed as '<option>' the
   *          default is false.
   *
   *          If the option is listed as '<option> <arg>'
   *          this function returns @p true if the option was present
   *          and @p false otherwise.
   *
   */
  bool isSet(const char *option) const;

  /**
   *  Read the number of arguments that aren't options (but,
   *  for example, filenames).
   *
   *  @return The number of arguments that aren't options
   */
  int count() const;

  /**
   *  Read out an argument.
   *
   *  @param n The argument to read. 0 is the first argument.
   *           @ref count()-1 is the last argument.
   *
   *  @return A @p const @p char @p * pointer to the n'th argument.
   */
  const char *arg(int n) const;

  /**
   *  Read out an argument representing a URL.
   *
   *  The argument can be
   *  @li an absolute filename
   *  @li a relative filename
   *  @li a URL
   *
   *  @param n The argument to read. 0 is the first argument.
   *           @ref count()-1 is the last argument.
   *
   *  @return a @p URL representing the n'th argument.
   */
  KURL url(int n) const;

  /**
   * Used by @ref #url
   * Made public for apps that don't use KCmdLineArgs
   * @param urlArgs the argument
   */
  static KURL makeURL( const char * urlArg );

  /**
   * Made public for apps that don't use KCmdLineArgs
   * To be done before @ref makeURL, to set the current working
   * directory in case @ref makeURL needs it.
   */
  static void setCwd( char * cwd ) { mCwd = cwd; }

  /**
   *  Clear all options and arguments.
   */
  void clear();


protected:
  /**
   * @internal
   *  Constructor.
   */
  KCmdLineArgs( const KCmdLineOptions *_options, const char *_id,
		const char *_name);

  /**
   *  @internal use only.
   *
   *  Use @ref clear() if you want to free up some memory.
   *
   *  Destructor.
   */
  ~KCmdLineArgs();

private:
  /**
   * @internal
   *
   * Checks what to do with a single option
   */
  static void findOption(const char *_opt, QCString opt, int &i, bool enabled, bool &moreOptions);

  /**
   * @internal
   *
   * Parse all arguments, verify correct syntax and put all arguments
   * where they belong.
   */
  static void parseAllArgs();

  /**
   * @internal for KApplication only:
   *
   * Return argc
   */
  static int *qt_argc();

  /**
   * @internal for KApplication only:
   *
   * Return argv
   */

  static char ***qt_argv();

  /**
   * @internal
   *
   * Remove named options.
   *
   * @param id The name of the options to be removed.
   */
  static void removeArgs(const char *id);


  /**
   * @internal for KUniqueApplication only:
   *
   * Save all but the Qt and KDE arguments to a stream.
   */
  static void saveAppArgs( QDataStream &);

  /**
   * @internal for KUniqueApplication only:
   *
   * Load arguments from a stream.
   */
  static void loadAppArgs( QDataStream &);

  /**
   * @internal
   *
   *  Set a boolean option
   */
  void setOption(const QCString &option, bool enabled);

  /**
   * @internal
   *
   *  Set a string option
   */
  void setOption(const QCString &option, const char *value);

  /**
   * @internal
   *
   * Add an argument
   */
  void addArgument(const char *argument);

  /**
   * @internal
   *
   * Save to a stream.
   */
  void save( QDataStream &) const;

  /**
   * @internal
   *
   * Restore from a stream.
   */
  void load( QDataStream &);

  /**
   * @internal for KApplication only
   *
   * Initialize class.
   *
   * This function should be called as the very first thing in
   *  your application.
   * @param argc As passed to @p main(...).
   * @param argv As passed to @p main(...).
   * @param appname The untranslated name of your application. This should
   *                match with @p argv[0].
   *
   * This function makes KCmdLineArgs ignore all unknown options as well as
   * all arguments.
   */
  static void initIgnore(int _argc, char **_argv, const char *_appname);

  static void printQ(const QString &msg);

  const KCmdLineOptions *options;
  const char *name;
  const char *id;
  KCmdLineParsedOptions *parsedOptionList;
  KCmdLineParsedArgs *parsedArgList;
  bool isQt;

  static KCmdLineArgsList *argsList; // All options.
  static const KAboutData *about;

  static int argc; // The original argc
  static char **argv; // The original argv
  static bool parsed; // Whether we have parsed the arguments since calling init
  static bool ignoreUnknown; // Ignore unknown options and arguments
  static char *mCwd; // Current working directory. Important for KUnqiueApp!

  KCmdLineArgsPrivate *d;
};

#endif

