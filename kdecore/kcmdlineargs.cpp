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

#include "config.h"

#include "kcmdlineargs.h"
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <assert.h>
#include <stdio.h>
#include <qlist.h>
#include <qasciidict.h>

class KCmdLineParsedArgs : public QAsciiDict<char>
{
public:
   KCmdLineParsedArgs()
     : QAsciiDict<char>( 7, true, false) { }
};

class KCmdLineArgsList: public QList<KCmdLineArgs>
{
public:
   KCmdLineArgsList() { }
};

KCmdLineArgsList *KCmdLineArgs::argsList = 0;
int KCmdLineArgs::argc = 0;
char **KCmdLineArgs::argv = 0;
const char *KCmdLineArgs::appname = 0;
const char *KCmdLineArgs::description = 0;
const char *KCmdLineArgs::version = 0;
bool KCmdLineArgs::parsed = false;

//
// Static functions
//

void
KCmdLineArgs::init(int _argc, char **_argv, const char *_appname,
                   const char *_description, const char *_version)
{
   assert( argsList == 0 );	// Don't call init twice.
   assert( argc == 0 );		// Don't call init twice.
   assert( argv == 0 );		// Don't call init twice.
   assert( appname == 0 );	// Don't call init twice.
   assert( description == 0 );	// Don't call init twice.
   assert( version == 0 );	// Don't call init twice.
   argc = _argc;
   argv = _argv;
   appname = _appname;
   description = _description;
   version = _version;
   parsed = false;
}

void
KCmdLineArgs::addCmdLineOptions( const KCmdLineOptions *options, const char *name, const char *id)
{
   assert( parsed == false ); // You must add _ALL_ cmd line options
                              // before accessing the arguments!

   if (!argsList)
      argsList = new KCmdLineArgsList();

   KCmdLineArgs *args = new KCmdLineArgs(options, name, id);
   argsList->append(args);
}

KCmdLineArgs *KCmdLineArgs::parsedArgs(const char *id)
{
   assert(argsList != 0); // It's an error to call parsedArgs(...) without
                          // doing a corresponding addCmdLineOptions first!


   KCmdLineArgs *args = argsList->first();
   while(args)
   {
      if (strcmp(args->id, id) == 0)
      {
          if (!parsed)
             parseAllArgs();
          return args;
      }
      args = argsList->next();
   }

   assert( args ); // It's an error to call parsedArgs(...) without
                   // doing a corresponding addCmdLineOptions first!
   return args;
}

/*
 * @return:
 *  0 - option not found.
 *  1 - option found			// -fork
 *  2 - inverse option found ('no')	// -nofork
 *  3 - option + arg found		// -fork now
 *
 */
static int  
findOption(const KCmdLineOptions *options, const char *opt, 
           const char *&opt_name, const char *&def)
{
   bool inverse = false;
   int len = strlen(opt);
   while(options && options->name)
   {
      opt_name = options->name;
      if ((opt_name[0] == 'n') && (opt_name[1] == 'o'))
      {
         opt_name += 2;
         inverse = true;
      }
      if (strncmp(opt, opt_name, len) == 0)
      {
         opt_name += len;
         if (!opt_name[0])
         {
            if (inverse) 
               return 2;
            else 
               return 1;
         }
         if (opt_name[0] == ' ')
         {
            def = options->def;
            return 3;
         }
      }
         
      options++;
   }
   return 0;
}


void 
KCmdLineArgs::findOption(const char *opt, int &i, bool enabled)
{
   KCmdLineArgs *args = argsList->first();
   const char *opt_name;
   const char *def;

   int result = 0;
   while (args)
   {
      result = ::findOption(args->options, opt, opt_name, def);
      if (result) break;
      args = argsList->next();
   }

   if (!args || !result) 
   {
      enable_i18n();
      usage( i18n("Unknown option '--%1%2'.\n")
                    .arg( enabled? "" : "no").arg(opt));
   }

   if (result == 3) // This option takes an argument
   {
      if (!enabled) 
      {
         enable_i18n();
         usage( i18n("Unknown option '--%1%2'.\n")
                    .arg( "no").arg(opt));
      }
      i++;
      opt_name++;
      if (i >= argc) 
      {
         enable_i18n();
         usage( i18n("'%s' missing.\n").arg( opt_name));
      }
      args->setOption(opt, argv[i]);
   }   
   else
   {
      args->setOption(opt, enabled);
   }
}

void
KCmdLineArgs::parseAllArgs()
{
   bool allowArgs = false;
   bool inOptions = true;
   KCmdLineArgs *appOptions = argsList->last();
   if (!appOptions->id)
   {
     const KCmdLineOptions *option = appOptions->options;
     while(option && option->name && !allowArgs)
     {
       if (option->name[0] == '+')
          allowArgs = true;
       option++;
     }
   }
   for(int i = 1; i < argc; i++)
   {
      if ((argv[i][0] == '-') && inOptions)
      {
         bool enabled = true;
         const char *option = &argv[i][1]; 
         if (option[0] == '-')
         {
            option++;
            argv[i]++;
            if (!option[0])
            { 
               inOptions = false;
               continue;
            }
         }
         if (strcmp(option, "help") == 0)
         {
            usage(0);
         }
         else if (strncmp(option, "help-",5) == 0)
         {
            usage(option+5);
         }
         else if ( (strcmp(option, "version") == 0) ||
                   (strcmp(option, "V") == 0))
         {
            fprintf(stdout, "Qt: %s\n", qVersion());
            fprintf(stdout, "KDE: %s\n", VERSION);
            fprintf(stdout, "%s: %s\n", appname, version);
            exit(0);
         }
         else {
           if ((option[0] == 'n') && (option[1] == 'o'))
           {
              option += 2;
              enabled = false;
           }
           findOption(option, i, enabled);
         }
      }
      else
      {
         // Check whether appOptions allows these arguments
         if (!allowArgs)
         {
            enable_i18n();
            usage( i18n("Unexpected argument '%1'.\n").arg( argv[i]));
         }
         else
         {
            appOptions->addArgument(argv[i]);
         }
      }
   }
   parsed = true;
}

/**
 * For KApplication only:
 *
 * Return argc 
 */
int *
KCmdLineArgs::qt_argc()
{
   assert(argsList != 0); // It's an error to call KApplication(...) without
                          // doing a addCmdLineOptions first!

   return &argc;
}

/**
 * For KApplication only:
 *
 * Return argv
 */
char ***
KCmdLineArgs::qt_argv()
{
   if (!argsList)
      KApplication::addCmdLineOptions(); // Lazy bastards!

   KCmdLineArgs *args = parsedArgs("qt");
   assert(args); // No qt options have been added!

   delete args; // Clean up Qt specific arguments.

   return &argv;
}

void
KCmdLineArgs::enable_i18n()
{
   KInstance *instance = new KInstance(appname);
   (void) instance->config();
   // Don't delete instance!
}

void
KCmdLineArgs::printQ(const QString &msg)
{
   QCString localMsg = msg.local8Bit();
   fprintf(stdout, "%s", localMsg.data());  
}

void
KCmdLineArgs::usage(const QString &error)
{
   QCString localError = error.local8Bit();
   fprintf(stderr, "%s: %s", appname, localError.data());
   usage();
}

void 
KCmdLineArgs::usage(const char *id)
{
   if (!KGlobal::_locale) enable_i18n();
   assert(argsList != 0); // It's an error to call usage(...) without
                          // having done addCmdLineOptions first!

   QString optionFormatString		= "  --%1 %2\n";
   QString optionFormatStringDef	= "  --%1 %2 [%3]\n";
   QString optionFormatStringArg	= "  %1 %2\n";
   QString optionHeaderString = i18n("\n%1-options:\n");
   QString tmp;
   QString usage;
    
   KCmdLineArgs *args = argsList->last();
   
   if (!args->id) 
   {
      usage = i18n("[options]")+usage;
   }  

   while(args)
   {
      if (args->name) 
      {
         usage = QString(i18n("[%1-options]")).arg(args->name)+" "+usage;
      }
      args = argsList->prev();
   }

   KCmdLineArgs *appOptions = argsList->last();
   if (!appOptions->id)
   {
     const KCmdLineOptions *option = appOptions->options;
     while(option && option->name)
     {
       if (option->name[0] == '+')
       {
          usage = usage + " " + (option->name+1);
       }

       option++;
     }
   }

   printQ(i18n("Usage: %1 %2\n").arg(appname).arg(usage));
   printQ("\n"+i18n(description)+"\n");

   printQ(optionHeaderString.arg(i18n("Generic")));
   printQ(optionFormatString.arg("help", -23).arg(i18n("Show help about options")));
   
   args = argsList->first();
   while(args)
   {
      if (args->name && args->id)
      {
         QString option = QString("help-%1").arg(args->id);
         QString desc = QString("Show %1 specific options").arg(args->name);
         
         printQ(optionFormatString.arg(option, -23).arg(desc));
      }
      args = argsList->next();
   }
   
   printQ(optionFormatString.arg("help-all",-23).arg(i18n("Show all options")));
   printQ(optionFormatStringArg.arg("-V, --version",-25).arg(i18n("Show version information")));
   printQ(optionFormatString.arg( "", -23).arg(i18n("End of options")));

   args = argsList->first(); // Sets current to 1st.

   bool showAll = (strcmp(id, "all") == 0);

   if (!showAll)
   {   
     while(args)
     {
       if (!id && !args->name) break;
       if (id && (strcmp(args->id, id) == 0)) break;
       args = argsList->next();
     }
   }

   bool hasArgs = false;

   while(args)
   {
     if (args)
     {
       if (!args->name)
         printQ(i18n("\nOptions:\n"));
       else if (args->name)
         printQ(optionHeaderString.arg(args->name));
     }

     while (args)
     {
       const KCmdLineOptions *option = args->options;
       while(option && option->name)
       {
         if (option->name[0] == '+')
         {
            if (!hasArgs)
               printQ(i18n("\nArguments:\n"));
            hasArgs = true;
            printQ(optionFormatStringArg.arg(option->name+1, -25)
		 .arg(i18n(option->description)));
         }
         else if (!option->def || !option->description)
            printQ(optionFormatString.arg(option->name, -23)
		.arg(i18n(option->description ? option->description : "")));
         else
            printQ(optionFormatStringDef.arg(option->name, -23) 
                 .arg(i18n(option->description)).arg(option->def));
         
         option++;
       }
       args = argsList->next();
       if (!args || args->name || !args->id) break;
     }
     if (!showAll) break;
   }

   exit(254);
}

//
// Member functions
//

/**
 *  Constructor.
 *
 *  The given arguments are assumed to be constants.
 */
KCmdLineArgs::KCmdLineArgs( const KCmdLineOptions *_options, 
                            const char *_name, const char *_id)
  : options(_options), name(_name), id(_id)
{
  parsedOptionList = 0;
  parsedArgList = 0;
}

/**
 *  Destructor.
 */
KCmdLineArgs::~KCmdLineArgs()
{
  delete parsedOptionList;
  delete parsedArgList;
  if (argsList)
  {
     argsList->removeRef(this);
     if (argsList->count() == 0)
     {
        delete argsList;
        argsList = 0;
     }
  }
}

void
KCmdLineArgs::setOption(const char *opt, bool enabled)
{
   if (!parsedOptionList) parsedOptionList = new KCmdLineParsedArgs;

   if (enabled)
      parsedOptionList->replace( opt, "t" );
   else
      parsedOptionList->replace( opt, "f" );
}

void
KCmdLineArgs::setOption(const char *opt, const char *value)
{
   if (!parsedOptionList) parsedOptionList = new KCmdLineParsedArgs;

   parsedOptionList->replace( opt, value );
}

const char *
KCmdLineArgs::getOption(const char *opt)
{
   const char *value = 0;
   if (parsedOptionList)
   {
      value = parsedOptionList->find(opt);
   }

   if (!value)
   {
      // Look up the default.
      const char *opt_name;
      const char *def;
      int result = ::findOption( options, opt, opt_name, def);

      assert(result == 3); // Make sure to add an option to 
                          // the list of options before querying it!
      return def;         
   }
   return value;
}

bool
KCmdLineArgs::isSet(const char *opt)
{
   const char *value = 0;
   if (parsedOptionList)
   {
      value = parsedOptionList->find(opt);
   }
   if (value)
      return (value[0] == 't');

   // Look up the default.
   const char *opt_name;
   const char *def;
   int result = ::findOption( options, opt, opt_name, def);

   assert(result != 0); // Make sure to add an option to 
                        // the list of options before querying it!
   if (result == 3)
      return false; // String option has 'false' as default.

   // We return 'true' as default if the option was listed as '-nofork'
   // We return 'false' as default if the option was listed as '-fork'
   return (result == 2); 
}

int
KCmdLineArgs::count()
{
   if (!parsedArgList)
      return 0;
   return parsedArgList->count();
}

const char *
KCmdLineArgs::arg(int n)
{
   assert(parsedArgList);

   assert(n < (int) parsedArgList->count());

   return parsedArgList->at(n);
}

void 
KCmdLineArgs::addArgument(const char *argument)
{
   if (!parsedArgList)
      parsedArgList = new QList<char>;

   parsedArgList->append(argument);
}
