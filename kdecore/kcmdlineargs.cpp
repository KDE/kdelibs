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
#include <kaboutdata.h>
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kstringhandler.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <qlist.h>
#include <qasciidict.h>
#include <qstrlist.h>

template class QAsciiDict<char>;
template class QList<KCmdLineArgs>;

class KCmdLineParsedOptions : public QAsciiDict<QCString>
{
public:
   KCmdLineParsedOptions()
     : QAsciiDict<QCString>( 7 ) { }

   // WABA: Huh? 
   // The compiler doesn't find KCmdLineParsedOptions::write(s) by itself ???
   QDataStream& save( QDataStream &s) const
   { return QGDict::write(s); } 

   QDataStream& load( QDataStream &s) 
   { return QGDict::read(s); } 

protected:
   virtual QDataStream& write( QDataStream &s, QCollection::Item data) const
   {
      QCString *str = (QCString *) data;
      s << (*str);
      return s;
   }

   virtual QDataStream& read( QDataStream &s, QCollection::Item &item) 
   {
      QCString *str = new QCString;
      s >> (*str);
      item = (void *)str;
      return s;
   }

};

class KCmdLineParsedArgs : public QStrList
{
public:
   KCmdLineParsedArgs()
     : QStrList( true ) { }
   QDataStream& save( QDataStream &s) const
   { return QGList::write(s); } 

   QDataStream& load( QDataStream &s) 
   { return QGList::read(s); } 
};


class KCmdLineArgsList: public QList<KCmdLineArgs>
{
public:
   KCmdLineArgsList() { }
};

KCmdLineArgsList *KCmdLineArgs::argsList = 0;
int KCmdLineArgs::argc = 0;
char **KCmdLineArgs::argv = 0;
const KAboutData *KCmdLineArgs::about = 0;
bool KCmdLineArgs::parsed = false;

//
// Static functions
//

void
KCmdLineArgs::init(int _argc, char **_argv, const char *_appname,
                   const char *_description, const char *_version, bool noKApp)
{
   init(_argc, _argv, 
        new KAboutData(_appname, _appname, _version, _description, noKApp));
}

void
KCmdLineArgs::init(int _argc, char **_argv, const KAboutData *_about, bool noKApp)
{
   assert( argsList == 0 );	// Don't call init twice.
   assert( argc == 0 );		// Don't call init twice.
   assert( argv == 0 );		// Don't call init twice.
   assert( about == 0 );	// Don't call init twice.
   argc = _argc;
   argv = _argv;
   about = _about;
   parsed = false;
   if (!noKApp)
      KApplication::addCmdLineOptions();
}

void
KCmdLineArgs::addCmdLineOptions( const KCmdLineOptions *options, const char *name, 
				 const char *id, const char *afterId)
{
   if (!argsList)
      argsList = new KCmdLineArgsList();

   int pos = argsList->count();

   if (pos && id && argsList->last() && !argsList->last()->name)
      pos--; 

   KCmdLineArgs *args;
   int i = 0;
   for(args = argsList->first(); args; args = argsList->next(), i++)
   {
      if (!id && !args->id)
         return; // Options already present.
  
      if (id && args->id && (strcmp(id, args->id) == 0))
	 return; // Options already present.

      if (afterId && args->id && (strcmp(afterId, args->id) == 0))
         pos = i+1;
   }

   assert( parsed == false ); // You must add _ALL_ cmd line options
                              // before accessing the arguments!
   args = new KCmdLineArgs(options, name, id);
   argsList->insert(pos, args);
}

void
KCmdLineArgs::saveAppArgs( QDataStream &ds)
{
   if (!parsed)
      parseAllArgs();

   // Remove Qt and KDE options.
   removeArgs("qt");
   removeArgs("kde");

   uint count = argsList ? argsList->count() : 0;
   ds << count;

   if (!count) return;

   KCmdLineArgs *args;
   for(args = argsList->first(); args; args = argsList->next())
   {
      args->save(ds);
   }
}

void
KCmdLineArgs::loadAppArgs( QDataStream &ds)
{
   // Remove Qt and KDE options.
   removeArgs("qt");
   removeArgs("kde");

   KCmdLineArgs *args;
   if (argsList)
   {
      for(args = argsList->first(); args; args = argsList->next())
      {
         args->clear();
      }
   }

   uint count;
   ds >> count;

   if (count == 0)
      return;
   
   if (!argsList || (count != argsList->count()))
   {
      fprintf(stderr, "loadAppArgs:: Unexpected number of command line sets "
                      "(%d instead of %d)\n", count, argsList ? argsList->count() : 0);
      return; 
   }

   for(args = argsList->first(); args; args = argsList->next())
   {
      args->load(ds);
   }
}

KCmdLineArgs *KCmdLineArgs::parsedArgs(const char *id)
{
   KCmdLineArgs *args = argsList ? argsList->first() : 0;
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

   if (!args)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for parsedArgs(\"%s\") witout a prior call\n", id);
      fprintf(stderr, "to addCmdLineOptions( ..., \"%s\")\n\n", id);
 
      assert( 0 ); 
      exit(255);
   }
   return args;
}

void KCmdLineArgs::removeArgs(const char *id)
{
   KCmdLineArgs *args = argsList ? argsList->first() : 0;
   while(args)
   {
      if (strcmp(args->id, id) == 0)
      {
          if (!parsed)
             parseAllArgs();
          break;
      }
      args = argsList->next();
   }

   if (args)
      delete args;
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
   bool inverse;
   int len = strlen(opt);
   while(options && options->name)
   {
      inverse = false;
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
            
            if (!options->description)
            {
               options++;
               if (!options->name)
                  return 0;
               QCString nextOption = options->name;
               int p = nextOption.find(' ');
               if (p > 0)
                  nextOption = nextOption.left(p);
               return findOption(options, nextOption.data(), opt_name, def);
            }

            return 1;
         }
         if (opt_name[0] == ' ')
         {
            opt_name++;
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
      if (i >= argc) 
      {
         enable_i18n();
         usage( i18n("'%1' missing.\n").arg( opt_name));
      }
      args->setOption(opt, argv[i]);
   }   
   else
   {
      args->setOption(opt, enabled);
   }
}

void
KCmdLineArgs::printQ(const QString &msg)
{
   QCString localMsg = msg.local8Bit();
   fprintf(stdout, "%s", localMsg.data());  
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
            printQ( QString("Qt: %1\n").arg(qVersion()));
            printQ( QString("KDE: %1\n").arg(VERSION));
            printQ( QString("%1: %2\n").
			arg(about->appName()).arg(about->version()));
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
   if (!argsList)
      KApplication::addCmdLineOptions(); // Lazy bastards!

   KCmdLineArgs *args = parsedArgs("qt");

   assert(args); // No qt options have been added!

   assert(argc >= (args->count()+1));
   argc = args->count() +1;
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

   int i = 0;
   for(; i < args->count(); i++)
   {
      argv[i+1] = (char *) args->arg(i);
   }
   argv[i+1] = 0;

   return &argv;
}

void
KCmdLineArgs::enable_i18n()
{
   KInstance *instance = new KInstance(about);
   (void) instance->config();
   // Don't delete instance!
}


void
KCmdLineArgs::usage(const QString &error)
{
   QCString localError = error.local8Bit();
   fprintf(stderr, "%s: %s", about->appName(), localError.data());
   usage();
}

void 
KCmdLineArgs::usage(const char *id)
{
   if (!KGlobal::_locale) enable_i18n();
   assert(argsList != 0); // It's an error to call usage(...) without
                          // having done addCmdLineOptions first!

   QString optionFormatString		= "  %1 %2\n";
   QString optionFormatStringDef	= "  %1 %2 [%3]\n";
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

   printQ(i18n("Usage: %1 %2\n").arg(about->appName()).arg(usage));
   printQ("\n"+about->shortDescription()+"\n");

   printQ(optionHeaderString.arg(i18n("Generic")));
   printQ(optionFormatString.arg("--help", -25).arg(i18n("Show help about options")));
   
   args = argsList->first();
   while(args)
   {
      if (args->name && args->id)
      {
         QString option = QString("--help-%1").arg(args->id);
         QString desc = QString("Show %1 specific options").arg(args->name);
         
         printQ(optionFormatString.arg(option, -25).arg(desc));
      }
      args = argsList->next();
   }
   
   printQ(optionFormatString.arg("--help-all",-25).arg(i18n("Show all options")));
   printQ(optionFormatString.arg("-V, --version",-25).arg(i18n("Show version information")));
   printQ(optionFormatString.arg("--", -25).arg(i18n("End of options")));

   args = argsList->first(); // Sets current to 1st.

   bool showAll = (strcmp(id, "all") == 0);

   if (!showAll)
   {   
     while(args)
     {
       if (!id && !args->id) break;
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
       QCString opt = "";
//
       while(option && option->name)
       {
         QString description;
         QString descriptionRest;
         QStringList dl;
         if (option->description)
         {
            description = i18n(option->description);
            dl = KStringHandler::split(description, "\n");
            description = dl.first();
            dl.remove( dl.begin() );
         }
         QCString name = option->name;
         if (name[0] == '+')
         {
            name = name.mid(1);
            if (!hasArgs)
               printQ(i18n("\nArguments:\n"));
            hasArgs = true;
            printQ(optionFormatString.arg(name, -25)
		 .arg(description));
         }
         else 
         {
            if ((name.length() == 1) || (name[1] == ' '))
               name = "-"+name;
            else 
               name = "--"+name;
            if (!option->description)
            {
               opt = name + ", ";
            }
            else 
            {
               opt = opt + name;
               if (!option->def)
               {
                  printQ(optionFormatString.arg(opt, -25)
                         .arg(description));
               }
               else
               {
                  printQ(optionFormatStringDef.arg(opt, -25) 
                         .arg(description).arg(option->def));
               }
               opt = "";
            }
         }
         for(QStringList::Iterator it = dl.begin();
             it != dl.end();
             it++)
         {  
            printQ(optionFormatString.arg("", -25).arg(*it));
         }
         
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
  isQt = (strcmp(id, "qt") == 0);
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
KCmdLineArgs::clear()
{
   delete parsedArgList;
   parsedArgList = 0;
   delete parsedOptionList;
   parsedOptionList = 0;
}

void
KCmdLineArgs::save( QDataStream &ds) const
{
   uint count = 0;
   if (parsedOptionList)
      parsedOptionList->save( ds );
   else
      ds << count;

   if (parsedArgList)
      parsedArgList->save( ds );
   else
      ds << count;
}

void
KCmdLineArgs::load( QDataStream &ds)
{
   if (!parsedOptionList) parsedOptionList = new KCmdLineParsedOptions;
   if (!parsedArgList) parsedArgList = new KCmdLineParsedArgs;

   parsedOptionList->load( ds );
   parsedArgList->load( ds );

   if (parsedOptionList->count() == 0)
   {
      delete parsedOptionList;
      parsedOptionList = 0;
   }   
   if (parsedArgList->count() == 0)
   {
      delete parsedArgList;
      parsedArgList = 0;
   }   
}

void
KCmdLineArgs::setOption(const char *opt, bool enabled)
{
   if (isQt)
   {
      // Qt does it own parsing.
      QCString arg = "-";
      arg += opt;
      addArgument(arg);
      return;
   }
   if (!parsedOptionList) parsedOptionList = new KCmdLineParsedOptions;

   if (enabled)
      parsedOptionList->replace( opt, new QCString("t") );
   else
      parsedOptionList->replace( opt, new QCString("f") );
}

void
KCmdLineArgs::setOption(const char *opt, const char *value)
{
   if (isQt)
   {
      // Qt does it's own parsing.
      QCString arg = "-";
      arg += opt;
      addArgument(arg);
      addArgument(value);
      return;
   }
   if (!parsedOptionList) parsedOptionList = new KCmdLineParsedOptions;

   parsedOptionList->replace( opt, new QCString(value) );
}

QCString
KCmdLineArgs::getOption(const char *opt)
{
   QCString *value = 0;
   if (parsedOptionList)
   {
      value = parsedOptionList->find(opt);
   }

   if (value)
      return (*value);

   // Look up the default.
   const char *opt_name;
   const char *def;
   int result = ::findOption( options, opt, opt_name, def);

   assert(result == 3); // Make sure to add an option to 
                        // the list of options before querying it!
   if (result != 3)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for getOption(\"%s\") but the \"%s\" option\n", 
                      opt, opt);
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");
 
      assert( 0 ); 
      exit(255);
   }
   return QCString(def);         
}

bool
KCmdLineArgs::isSet(const char *opt)
{
   // Look up the default.
   const char *opt_name;
   const char *def;
   int result = ::findOption( options, opt, opt_name, def);

   if (result == 0)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for isSet(\"%s\") but the \"%s\" option\n", 
                      opt, opt);
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");
 
      assert( 0 ); 
      exit(255);
   }

   QCString *value = 0;
   if (parsedOptionList)
   {
      value = parsedOptionList->find(opt);
   }

   if (value)
   {
      if (result == 3)
         return true;
      else 
         return ((*value)[0] == 't');
   }

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
      parsedArgList = new KCmdLineParsedArgs;

   parsedArgList->append(argument);
}
