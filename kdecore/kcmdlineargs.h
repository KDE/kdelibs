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

#include <qlist.h>
#include <qstring.h>

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

/**
 *  Simple access to the command-line arguments.
 *
 *  It takes into account Qt-specific options, KDE-specific options
 *  and application specific options.
 *
 *  This class is used in @ds main() via the static method
 *   @ref init().
 *
 *  @short A class for command-line argument handling.
 *  @author Waldo Bastian
 *  @version 0.0.1
 */
class KCmdLineArgs
{
  friend KApplication;
  friend KUniqueApplication;
  friend QList<KCmdLineArgs>;
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
   * Add options to your application.
   *
   * You must make sure that all possible options have been added before
   * any class uses the command line arguments. 
   *
   * @param options A list of options which your code supplies.
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
   * @li "option1" is an option which requires an additional argument
   * @li "option2" is an option which can be turned on. The default is off.
   * @li "option3" is an option which can be turned off. The default is on.
   * @li "o" does not have a description. It is an alias for the option
   *     which follows. In this case "option2".
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
   * This function returns all command line arguments which your code
   * handles. If unknown command line arguments are encountered the program
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
   * Print the usage help to @ds stdout and exit.
   *
   * @param complete If true, print all available options.
   **/
  static void usage(const char *id = 0);

  /**
   * Print an error to @ds stderr and the usage help to @ds stdout and exit.
   *
   **/
  static void usage(const QString &error);

  /**
   * Enable i18n to be able to print a translated error message.
   * 
   * N.B.: This function leaks memory so you are expected to exit 
   * afterwards, e.g. by calling usage().
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
   */
  QCString getOption(const char *option);

  /**
   *  Read out a boolean option or check for the presence of string option.
   *  
   *  @param option The name of the option without '-' or '-no'.
   *
   *  @return The value of the option. If the option was not
   *          present on the cmdline the default is returned.
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
  bool isSet(const char *option);

  /**
   *  Read the number of arguments that aren't options (but
   *  e.g. filenames).
   *
   *  @return the number of arguments that aren't options
   */
  int count();
  
  /**
   *  Read out an argument.
   *
   *  @param n The argument to read. 0 is the first argument.
   *           @ref count()-1 is the last argument.
   *
   *  @return a @p const @p char @p * pointer to the n'th argument.
   */
  const char *arg(int n);

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

  /**
   * @internal
   *
   * Checks what to do with a single option
   */
  static void findOption(const char *_opt, QCString opt, int &i, bool enabled);

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
  static char *mCwd; // Current working directory. Important for KUnqiueApp!  
};

#endif

