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
      opt_name = options->name+1;
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
      fprintf(stderr, "%s: Unkown option '-%s%s'.\n", appname, 
	enabled ? "" : "no", opt);
      usage();
   }

   if (result == 3) // This option takes an argument
   {
      if (!enabled) 
      {
         fprintf(stderr, "%s: Unkown option '-no%s'.\n", appname, opt);
         usage();
      }
      i++;
      opt_name++;
      if (i >= argc) 
      {
         fprintf(stderr, "%s: '%s' missing.\n", appname, opt_name);
         usage();
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
   KCmdLineArgs *appOptions = argsList->last();
   int i = 1;
   while (i < argc)
   {
      if (argv[i][0] == '-')    
      {
         bool enabled = true;
         const char *option = &argv[i][1];
         if (strcmp(option, "help") == 0)
            usage(true);
         if (strcmp(option, "version") == 0)
         {
            fprintf(stderr, "Qt: %s\n", qVersion());
            fprintf(stderr, "KDE: %s\n", VERSION);
            fprintf(stderr, "%s: %s\n", appname, version);
            exit(0);
         }
         if ((option[0] == 'n') && (option[1] == 'o'))
         {
            option += 2;
            enabled = false;
         }
         findOption(option, i, enabled);
      }
      else
      {
         // Check whether appOptions allows these arguments
      }
      i++;
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
   assert(argsList != 0); // It's an error to call KApplication(...) without
                          // doing a addCmdLineOptions first!

   KCmdLineArgs *args = parsedArgs("qt");
   assert(args); // No qt options have been added!

   delete args; // Clean up Qt specific arguments.

   return &argv;
}

void 
KCmdLineArgs::usage(bool complete)
{
   assert(argsList != 0); // It's an error to call usage(...) without
                          // having done addCmdLineOptions first!

   const char *optionFormatString	= " %-25s %s\n";
   const char *optionFormatStringDef	= " %-25s %s [%s]\n";
   const char *optionHeaderString	= "\n%s-options:\n";
   QString usage = "[Options]\n";
    
   KCmdLineArgs *args = argsList->first();
   while(args)
   {
      if (args->name) 
      {
         usage = QString("[%1-options] ").arg(args->name)+usage;
      }
      args = argsList->next();
   }
   printf("Usage: %s %s", appname, usage.ascii());
   printf("\n%s\n", description);

   printf(optionHeaderString, "Generic");
   printf(optionFormatString, "-help", "Show help about all options");
   printf(optionFormatString, "-version", "Show version information");

   KCmdLineArgs *last = argsList->last();

   if (complete)
      args = argsList->first();
   else
      args = argsList->last();

   while (args)
   {
      if (args == last)
         printf("\nOptions:\n");
      else if (args->name)
         printf(optionHeaderString, args->name);
      const KCmdLineOptions *option = args->options;
      while(option && option->name)
      {
         if (!option->def || !option->description)
            printf(optionFormatString , option->name, 
                 option->description ? option->description : "");
         else
            printf(optionFormatStringDef , option->name, 
                 option->description, option->def);
         
         option++;
      }
      args = argsList->next();
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
  parsedArgList = 0;
}

/**
 *  Destructor.
 */
KCmdLineArgs::~KCmdLineArgs()
{
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
   if (!parsedArgList) parsedArgList = new KCmdLineParsedArgs;

   if (enabled)
      parsedArgList->replace( opt, "t" );
   else
      parsedArgList->replace( opt, "f" );
}

void
KCmdLineArgs::setOption(const char *opt, const char *value)
{
   if (!parsedArgList) parsedArgList = new KCmdLineParsedArgs;

   parsedArgList->replace( opt, value );
}

const char *
KCmdLineArgs::getOption(const char *opt)
{
   const char *value = 0;
   if (parsedArgList)
   {
      value = parsedArgList->find(opt);
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
   if (parsedArgList)
   {
      value = parsedArgList->find(opt);
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
