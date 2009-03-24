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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCMDLINEARGS_H
#define KCMDLINEARGS_H

#include <kdecore_export.h>
#include <QtCore/QBool>

#include <klocale.h>

template <class T> class QList;
class QString;
class QStringList;
class QByteArray;
class QDataStream;
class KUrl;

class KCmdLineArgs;
class KCmdLineArgsPrivate;
class KCmdLineArgsStatic;
class KCmdLineOptionsPrivate;

/**
 * @short Class that holds command line options.
 *
 * This class is intended to be used with the KCmdLineArgs class, which
 * provides convenient and powerful command line argument parsing and
 * handling functionality.
 *
 * @see KCmdLineArgs for additional usage information
 */
class KDECORE_EXPORT KCmdLineOptions
{
    friend class KCmdLineArgs;
    friend class KCmdLineArgsStatic;

    public:
    /**
     * Constructor.
     */
    KCmdLineOptions ();

    /**
     * Copy constructor.
     */
    KCmdLineOptions (const KCmdLineOptions &options);

    /**
     * Assignment operator.
     */
    KCmdLineOptions& operator= (const KCmdLineOptions &options);

    /**
     * Destructor.
     */
    ~KCmdLineOptions ();

    /**
     * Add command line option, by providing its name, description, and
     * possibly a default value. These will print out when <i>myapp --help</i>
     * is called on the command line.
     *
     * Note that if the option name begins with "no" that you will need to test
     * for the name without the "no" and the result will be the inverse of what
     * is specified. i.e. if "nofoo" is the name of the option and
     * <i>myapp --nofoo</i> is called:
     *
     * @code
     * KCmdLineArgs::parsedArgs()->isSet("foo"); // false
     * @endcode
     *
     * @param name option name
     * @param description option description, made available for translation;
     *                    can be left off
     * @param defaultValue default option value, when the value is not specified
     *                     on the command line; can be left off
     */
    KCmdLineOptions &add (const QByteArray &name,
                          const KLocalizedString &description = KLocalizedString(),
                          const QByteArray &defaultValue = QByteArray());

    /**
     * Add all options from another KCmdLineOptions object.
     *
     * @param options options to add
     */
    KCmdLineOptions &add (const KCmdLineOptions &options);

    private:

    KCmdLineOptionsPrivate *d; //krazy:exclude=dpointer (for operator=)
};

class KCmdLineArgsList;
class KApplication;
class KAboutData;

/**
 *  @short A class for command-line argument handling.
 *
 *  KCmdLineArgs provides simple access to the command-line arguments
 *  for an application. It takes into account Qt-specific options,
 *  KDE-specific options and application specific options.
 *
 *  This class is used in %main() via the static method
 *  init().
 *
 *  A typical %KDE application using %KCmdLineArgs should look like this:
 *
 *  @code
 *  int main(int argc, char *argv[])
 *  {
 *     // Initialize command line args
 *     KCmdLineArgs::init(argc, argv, appName, programName, version, description);
 *
 *     // Define the command line options using KCmdLineOptions
 *     KCmdLineOptions options;
 *     ....
 *
 *     // Register the supported options
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
 *     // Handle our own options/arguments
 *     // A KApplication will usually do this in main but this is not
 *     // necessary.
 *     // A KUniqueApplication might want to handle it in newInstance().
 *
 *     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 *
 *     // A binary option (on / off)
 *     if (args->isSet("some-option"))
 *        ....
 *
 *     // An option which takes an additional argument
 *     QString anotherOptionArg = args->getOption("another-option");
 *
 *     // Arguments (e.g. files to open)
 *     for(int i = 0; i < args->count(); i++) // Counting start at 0!
 *     {
 *        openFile( args->arg(i));
 *        // Or more convenient:
 *        // openUrl( args->url(i));
 *
 *     }
 *
 *     args->clear(); // Free up some memory.
 *     ....
 *  }
 *  @endcode
 *
 *  The options that an application supports are configured using the
 *  KCmdLineOptions class. An example is shown below:
 *
 *  @code
 *  KCmdLineOptions options;
 *  options.add("a", ki18n("A short binary option"));
 *  options.add("b \<file>", ki18n("A short option which takes an argument"));
 *  options.add("c \<speed>", ki18n("As above but with a default value"), "9600");
 *  options.add("option1", ki18n("A long binary option, off by default"));
 *  options.add("nooption2", ki18n("A long binary option, on by default"));
 *  options.add(":", ki18n("Extra options:"));
 *  options.add("option3 \<file>", ki18n("A long option which takes an argument"));
 *  options.add("option4 \<speed>", ki18n("A long option which takes an argument, defaulting to 9600"), "9600");
 *  options.add("d").add("option5", ki18n("A long option which has a short option as alias"));
 *  options.add("e").add("nooption6", ki18n("Another long option with an alias"));
 *  options.add("f").add("option7 \<speed>", ki18n("'--option7 speed' is the same as '-f speed'"));
 *  options.add("!option8 \<cmd>", ki18n("All options following this one will be treated as arguments"));
 *  options.add("+file", ki18n("A required argument 'file'"));
 *  options.add("+[arg1]", ki18n("An optional argument 'arg1'"));
 *  options.add("!+command", ki18n("A required argument 'command', that can contain multiple words, even starting with '-'"));
 *  options.add("", ki18n("Additional help text not associated with any particular option"));
 *  @endcode
 *
 *  The ki18n calls are used for translation instead of the more usual i18n
 *  calls, because the translation needs to be delayed until after the
 *  message catalogs have been initialized.
 *
 *  Note that a program should define the options before any arguments.
 *
 *  When a long option has a short option as an alias, a program should
 *  only test for the long option.
 *
 *  With the above options a command line could look like:
 *  @code
 *     myapp -a -c 4800 --display localhost:0.0 --nooption5 -d /tmp/file
 *  @endcode
 *
 *  Long binary options can be in the form 'option' and 'nooption'.
 *  A command line may contain the same binary option multiple times,
 *  the last option determines the outcome:
 *  @code
 *     myapp --nooption4 --option4 --nooption4
 *  @endcode
 *  is the same as:
 *  @code
 *     myapp --nooption4
 *  @endcode
 *
 *  If an option value is provided multiple times, normally only the last
 *  value is used:
 *  @code
 *     myapp -c 1200 -c 2400 -c 4800
 *  @endcode
 *  is usually the same as:
 *  @code
 *     myapp -c 4800
 *  @endcode
 *
 *  However, an application can choose to use all values specified as well.
 *  As an example of this, consider that you may wish to specify a
 *  number of directories to use:
 *  @code
 *     myapp -I /usr/include -I /opt/kde/include -I /usr/X11/include
 *  @endcode
 *  When an application does this it should mention this in the description
 *  of the option. To access these options, use getOptionList()
 *
 *  Tips for end-users:
 *
 *  @li Single char options like "-a -b -c" may be combined into "-abc"
 *  @li The option "--foo bar" may also be written "--foo=bar"
 *  @li The option "-P lp1" may also be written "-P=lp1" or "-Plp1"
 *  @li The option "--foo bar" may also be written "-foo bar"
 *
 *  @author Waldo Bastian
 *  @version 0.0.4
 */
class KDECORE_EXPORT KCmdLineArgs
{
  friend class KApplication;
  friend class KCmdLineArgsList;
  friend class KCmdLineArgsStatic;
public:
  // Static functions:

  enum StdCmdLineArg {
    CmdLineArgQt = 0x01,
    CmdLineArgKDE = 0x02,
    CmdLineArgsMask=0x03,
    CmdLineArgNone = 0x00,
    Reserved = 0xff
  };
  Q_DECLARE_FLAGS(StdCmdLineArgs, StdCmdLineArg)
  /**
   * Initialize class.
   *
   * This function should be called as the very first thing in
   *  your application.
   * @param argc As passed to @p main(...).
   * @param argv As passed to @p main(...).
   * @param appname The untranslated name of your application. This should
   *                match with @p argv[0].
   * @param catalog Translation catalog name, if empty @p appname will be used.
   * @param programName A program name string to be used for display
   *        purposes. This string should be marked for translation.
   *        Example: ki18n("KEdit")
   * @param version A version.
   * @param description A short description of what your application is about.
   *                    Also marked for translation.
   * @param stdargs KDE/Qt or no default parameters
   */
   static void init(int argc, char **argv,
                    const QByteArray &appname,
                    const QByteArray &catalog,
                    const KLocalizedString &programName,
                    const QByteArray &version,
                    const KLocalizedString &description = KLocalizedString(),
                    StdCmdLineArgs stdargs=StdCmdLineArgs(CmdLineArgQt|CmdLineArgKDE));

  /**
   * Initialize class.
   *
   * This function should be called as the very first thing in
   *  your application. It uses KAboutData to replace some of the
   *  arguments that would otherwise be required.
   *
   * @param _argc As passed to @p main(...).
   * @param _argv As passed to @p main(...).
   * @param about A KAboutData object describing your program.
   * @param stdargs KDE/Qt or no default parameters
   */
  static void init(int _argc,
                   char **_argv,
                   const KAboutData *about,
                   StdCmdLineArgs stdargs=StdCmdLineArgs(CmdLineArgQt|CmdLineArgKDE));
  /**
   * Initialize Class
   *
   * This function should be called as the very first thing in your
   * application. This method will rarely be used, since it doesn't
   * provide any argument parsing. It does provide access to the
   * KAboutData information.
   * This method is exactly the same as calling
   * init(0,0, const KAboutData *about, CmdLineArgNone).
   *
   * @param about the about data.
   * @see KAboutData
   */
  static void init(const KAboutData *about);

  /**
   * add standard Qt/KDE command-line args
   */
  static void addStdCmdLineOptions(StdCmdLineArgs stdargs=StdCmdLineArgs(CmdLineArgQt|CmdLineArgKDE));

  /**
   * Add options to your application.
   *
   * You must make sure that all possible options have been added before
   * any class uses the command line arguments.
   *
   * The list of options should look like this:
   *
   * @code
   * KCmdLineOptions options;
   * options.add("option1 \<argument>", ki18n("Description 1"), "my_extra_arg");
   * options.add("o");
   * options.add("option2", ki18n("Description 2"));
   * options.add("nooption3", ki18n("Description 3"));
   * options.add("+file", ki18n("A required argument 'file'"));
   * @endcode
   *
   * @li "option1" is an option that requires an additional argument,
   *     but if one is not provided, it uses "my_extra_arg".
   * @li "option2" is an option that can be turned on. The default is off.
   * @li "option3" is an option that can be turned off. The default is on.
   * @li "o" does not have a description. It is an alias for the option
   *     that follows. In this case "option2".
   * @li "+file" specifies an argument. The '+' is removed. If your program
   *     doesn't specify that it can use arguments your program will abort
   *     when an argument is passed to it. Note that the reverse is not
   *     true. If required, you must check yourself the number of arguments
   *     specified by the user:
   *     @code
   *       KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   *       if (args->count() == 0) KCmdLineArgs::usage(i18n("No file specified"));
   *     @endcode
   *
   * In BNF:
   * @code
   * cmd = myapp [options] file
   * options = (option)*
   * option = --option1 \<argument> |
   *          (-o | --option2 | --nooption2) |
   *          ( --option3 | --nooption3 )
   * @endcode
   *
   * Instead of "--option3" one may also use "-option3"
   *
   * Usage examples:
   *
   * @li "myapp --option1 test"
   * @li "myapp" (same as "myapp --option1 my_extra_arg")
   * @li "myapp --option2"
   * @li "myapp --nooption2" (same as "myapp", since it is off by default)
   * @li "myapp -o" (same as "myapp --option2")
   * @li "myapp --nooption3"
   * @li "myapp --option3 (same as "myapp", since it is on by default)
   * @li "myapp --option2 --nooption2" (same as "myapp", because it
   *     option2 is off by default, and the last usage applies)
   * @li "myapp /tmp/file"
   *
   * @param options A list of options that your code supplies.
   * @param name the name of the option list, as displayed by
   *             the help output. Can be empty.
   * @param id A name with which these options can be identified, can be empty.
   * @param afterId The options are inserted after this set of options, can be empty.
   */
  static void addCmdLineOptions(const KCmdLineOptions &options,
                                const KLocalizedString &name = KLocalizedString(),
                                const QByteArray &id = QByteArray(),
                                const QByteArray &afterId = QByteArray());

  /**
   * Access parsed arguments.
   *
   * This function returns all command line arguments that your code
   * handles. If unknown command-line arguments are encountered the program
   * is aborted and usage information is shown.
   *
   * @param id The name of the options you are interested in, can be empty.
   */
  static KCmdLineArgs *parsedArgs(const QByteArray &id = QByteArray());

  /**
   * Get the CWD (Current Working Directory) associated with the
   * current command line arguments.
   *
   * Typically this is needed in KUniqueApplication::newInstance()
   * since the CWD of the process may be different from the CWD
   * where the user started a second instance.
   * @return the current working directory
   **/
  static QString cwd();

  /**
   * Get the appname according to argv[0].
   * @return the name of the application
   **/
  static QString appName();

  /**
   * Print the usage help to stdout and exit.
   *
   * @param id if empty, print all options. If id is set, only print the
   *        option specified by id. The id is the value set by
   *        addCmdLineOptions().
   **/
  static void usage(const QByteArray &id = QByteArray());

  /**
   * Print an error to stderr and the usage help to stdout and exit.
   * @param error the error to print
   **/
  static void usageError(const QString &error);

  /**
   * Enable i18n to be able to print a translated error message.
   *
   * N.B.: This function leaks memory, therefore you are expected to exit
   * afterwards (e.g., by calling usage()).
   **/
  static void enable_i18n();

  // Member functions:


  /**
   *  Read out a string option.
   *
   *  The option must have a corresponding KCmdLineOptions entry
   *  of the form:
   *  @code
   *    options.add("option \<argument>", ki18n("Description"), "default");
   *  @endcode
   *  You cannot test for the presence of an alias - you must always
   *  test for the full option.
   *
   *  @param option The name of the option without '-'.
   *
   *  @return The value of the option. If the option was not
   *          present on the command line the default is returned.
   *          If the option was present more than once, the value of the
   *          last occurrence is used.
   */
  QString getOption(const QByteArray &option) const;

  /**
   *  Read out all occurrences of a string option.
   *
   *  The option must have a corresponding KCmdLineOptions entry
   *  of the form:
   *  @code
   *    options.add("option \<argument>", ki18n("Description"), "default");
   *  @endcode
   *  You cannot test for the presence of an alias - you must always
   *  test for the full option.
   *
   *  @param option The name of the option, without '-' or '-no'.
   *
   *  @return A list of all option values. If no option was present
   *          on the command line, an empty list is returned.
   */
  QStringList getOptionList(const QByteArray &option) const;

  /**
   *  Read out a boolean option or check for the presence of string option.
   *
   *  @param option The name of the option without '-' or '-no'.
   *
   *  @return The value of the option. It will be true if the option
   *  was specifically turned on in the command line, or if the option
   *  is turned on by default (in the KCmdLineOptions list) and was
   *  not specifically turned off in the command line. Equivalently,
   *  it will be false if the option was specifically turned off in
   *  the command line, or if the option is turned off by default (in
   *  the KCmdLineOptions list) and was not specifically turned on in
   *  the command line.
   */
  bool isSet(const QByteArray &option) const;

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
   *           count()-1 is the last argument.
   *
   *  @return n-th argument
   */
  QString arg(int n) const;

  /**
   *  Read out an argument representing a URL.
   *
   *  The argument can be
   *  @li an absolute filename
   *  @li a relative filename
   *  @li a URL
   *
   *  @param n The argument to read. 0 is the first argument.
   * count()-1 is the last argument.
   *
   *  @return a URL representing the n'th argument.
   */
  KUrl url(int n) const;

  /**
   * Used by url().
   * Made public for apps that don't use KCmdLineArgs
   * @param urlArg the argument
   * @return the url.
   */
  static KUrl makeURL( const QByteArray &urlArg );

  /**
   * Made public for apps that don't use KCmdLineArgs
   * To be done before makeURL, to set the current working
   * directory in case makeURL needs it.
   * @param cwd the new working directory
   */
  static void setCwd( const QByteArray &cwd );

  /**
   *  Clear all options and arguments.
   */
  void clear();

  /**
   *  Reset all option definitions, i.e. cancel all addCmdLineOptions calls.
   *  Note that KApplication's options are removed too, you might want to
   *  call KApplication::addCmdLineOptions if you want them back.
   *
   *  You usually don't want to call this method.
   */
  static void reset();

  /**
   * Load arguments from a stream.
   */
  static void loadAppArgs( QDataStream &);

  /**
   * @internal for KUniqueApplication only:
   *
   * Save all but the Qt and KDE arguments to a stream.
   */
  static void saveAppArgs( QDataStream &);

  /**
   * Add standard option --tempfile
   */
  static void addTempFileOption();

  // this avoids having to know the "id" used by addTempFileOption
  // but this approach doesn't scale well, we can't have 50 standard options here...
  /**
   * @return true if --tempfile was set
   */
  static bool isTempFileSet();

  /**
   * Returns the number of arguments returned by qtArgv()
   *
   * @see qtArgv
   */
  static int &qtArgc();

  /**
   * Returns command line options for consumption by Qt after parsing them in a way that
   * is consistent with KDE's general command line handling. In particular this ensures
   * that Qt command line options can be specified as either -option or --option and that
   * any options specified after '--' will be ignored.
   *
   * @see qt_argc
   */
  static char **qtArgv();

  /**
   * Returns the KAboutData for consumption by KComponentData
   */
  static const KAboutData *aboutData();

protected:
  /**
   * @internal
   *  Constructor.
   */
  KCmdLineArgs( const KCmdLineOptions &_options, const KLocalizedString &_name,
                const QByteArray &_id);

  /**
   *  @internal use only.
   *
   *  Use clear() if you want to free up some memory.
   *
   *  Destructor.
   */
  ~KCmdLineArgs();

private:

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
  static void initIgnore(int _argc, char **_argv, const QByteArray &_appname);

  KCmdLineArgsPrivate *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCmdLineArgs::StdCmdLineArgs)

#endif

