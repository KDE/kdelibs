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

class KCmdLineArgsList;
class KApplication;
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
public:
  // Static functions:
  
  /**
   * Initialization function.
   *
   * This function should be called as the very first thing in
   *  your application.
   * @param argc As passed to main(...).
   * @param argv As passed to main(...).
   * @param appname The untranslated name of your application. This should
   *                match with argv[0].
   * @param description A short description of what your application is about.
   * @param version A version.
   * @param noKApp Don't add commandline options for QApplication/KApplication
   */
  static void init(int _argc, char **_argv, 
                   const char *_appname, const char *_description,
                   const char *_version, bool noKApp = false);

  /**
   * Initialization function.
   *
   * This function should be called as the very first thing in
   *  your application.
   * @param argc As passed to main(...).
   * @param argv As passed to main(...).
   * @param about A KAboutData object describing your program.
   * @param noKApp Don't add commandline options for QApplication/KApplication
   */
  static void init(int _argc, char **_argv, 
                   const KAboutData *about, bool noKApp = false);

  /**
   * Add options to your application.
   *
   * You must make sure that all possible options have been added before
   * any class uses the command line arguments. 
   *
   * @param options A list of options which your code supplies
   * @param id A name with which these options can be identified.
   *          
   */
  static void addCmdLineOptions( const KCmdLineOptions *options, 
				 const char *name=0, const char *id = 0);

  /**
   * Access parsed arguments
   *
   * This function returns all command line arguments which your code
   * handles. If unknown command line arguments are encountered the program
   * is aborted and usage information is shown. 
   *
   * @param id The name of the options you are interested in.
   */
  static KCmdLineArgs *parsedArgs(const char *id=0);

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
   *  Destructor.
   */
  ~KCmdLineArgs();
  

  /**
   *  Read out a string option.
   *  
   *  @param option The name of the option without '-'.
   *
   *  @return The value of the option. If the option was not
   *          present on the command line the default is returned.
   */
  const char *getOption(const char *option);

  /**
   *  Read out a boolean option or check for the presence of string option.
   *  
   *  @param option The name of the option without '-' or '-no'.
   *
   *  @return The value of the option. If the option was not
   *          present on the cmdline the default is returned.
   *          If the option is listed as '-no<option>' the
   *          default is true.
   *          If the option is listed as '-<option>' the
   *          default is false.
   *
   *          If the option is listed as '-<option> <arg>'
   *          this function returns true if the option was present
   *          and false otherwise.
   *
   */
  bool isSet(const char *option);

  /**
   *  Read the number of arguments that aren't options (but
   *  e.g. filenames)
   *
   *  @return the number of arguments that aren't options
   */
  int count();
  
  /**
   *  Read out an argument
   *
   *  @param n The argument to read. 0 is the first argument.
   *           count()-1 is the last argument.
   *
   *  @return a const char * pointer to the n'th argument.
   */
  const char *arg(int n);

protected:   
  /**
   * @internal
   *  Constructor.
   */
  KCmdLineArgs( const KCmdLineOptions *_options, const char *_id,
		const char *_name);

  /**
   * @internal
   *
   * Checks what to do with a single option
   */
  static void findOption(const char *opt, int &i, bool enabled);

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
   *  Set a boolean option
   */
  void setOption(const char *option, bool enabled);

  /**
   * @internal
   *
   *  Set a string option
   */
  void setOption(const char *option, const char *value);

  /**
   * @internal 
   *
   * Add an argument
   */
  void addArgument(const char *argument);

  static void printQ(const QString &msg);
  
  const KCmdLineOptions *options;
  const char *name;
  const char *id;
  KCmdLineParsedArgs *parsedOptionList;
  QList<char> *parsedArgList;

  static KCmdLineArgsList *argsList; // All options.
  static const KAboutData *about;

  static int argc; // The original argc
  static char **argv; // The original argv
  static bool parsed; // Whether we have parsed the arguments since calling init
};

#endif

