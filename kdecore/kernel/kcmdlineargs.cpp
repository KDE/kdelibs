/*
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

#define QT_NO_CAST_FROM_ASCII

#include "kcmdlineargs.h"
#include <kdebug.h>

#include <config.h>

#include <sys/param.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QTextCodec>

#include "kaboutdata.h"
#include "klocale.h"
#include "kdeversion.h"
#include "kcomponentdata.h"
#include "kglobal.h"
#include "kstringhandler.h"
#include "kurl.h"

#include "kuitsemantics_p.h" // for escaping arguments in i18n

// -----------------------------------------------------------------------------
// Design notes:
//
// These classes deal with a lot of text, some of which needs to be
// marked for translation. Since at the time when these object and calls are
// made the translation catalogs are usually still not initialized, the
// translation has to be delayed. This is achieved by using KLocalizedString
// for translatable strings. KLocalizedStrings are produced by ki18n* calls,
// instead of the more usuall i18n* calls which produce QString by trying to
// translate immediately.
//
// All the non-translatable string arguments to methods are taken QByteArray,
// all the translatable are KLocalizedString. The getter methods always return
// proper QString: the non-translatable strings supplied by the code are
// treated with QString::fromUtf8(), those coming from the outside with
// QTextCodec::toUnicode(), and translatable strings are finalized to QStrings
// at the point of getter calls (i.e. delayed translation).
//
// The code below uses locally defined s->decodeInput(QByteArray) and
// s->encodeOutput(QString) calls to centralize the conversion of raw external
// bytes (instead of QString::to/fromLocal8Bit(), QFile::decodeName, etc.)
// -----------------------------------------------------------------------------

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#else
#define DISPLAY "NODISPLAY"
#endif

//
// Helper classes
//

class KCmdLineParsedOptions : public QHash<QByteArray,QByteArray>
{
public:
   KCmdLineParsedOptions() { }
};

class KCmdLineParsedArgs : public QList<QByteArray>
{
public:
   KCmdLineParsedArgs() { }
};


class KCmdLineArgsList: public QList<KCmdLineArgs*>
{
public:
   KCmdLineArgsList() { }
   ~KCmdLineArgsList() {
	   while (count())
		delete takeFirst();
   }
};

//
// KCmdLineOptions
//

class KCmdLineOptionsPrivate {
    public:
    QList<QByteArray> names;
    QList<KLocalizedString> descriptions;
    QStringList defaults;
};

KCmdLineOptions::KCmdLineOptions ()
: d(new KCmdLineOptionsPrivate)
{}

KCmdLineOptions::~KCmdLineOptions ()
{
    delete d;
}

KCmdLineOptions::KCmdLineOptions (const KCmdLineOptions &options)
: d(new KCmdLineOptionsPrivate(*(options.d)))
{
}

KCmdLineOptions& KCmdLineOptions::operator= (const KCmdLineOptions &options)
{
    if (this != &options) {
        *d = *(options.d);
    }
    return *this;
}

KCmdLineOptions &KCmdLineOptions::add (const QByteArray &name,
                                       const KLocalizedString &description,
                                       const QByteArray &defaultValue)
{
    d->names.append(name);
    d->descriptions.append(description);
    d->defaults.append(QString::fromUtf8(defaultValue));
    return *this;
}

KCmdLineOptions &KCmdLineOptions::add (const KCmdLineOptions &other)
{
    d->names += other.d->names;
    d->descriptions += other.d->descriptions;
    d->defaults += other.d->defaults;
    return *this;
}

//
// KCmdLineArgs static data and methods
//

class KCmdLineArgsStatic {
    public:

    KCmdLineArgsList *argsList; // All options.
    const KAboutData *about;

    int all_argc; // The original argc
    char **all_argv; // The original argv
    char *appName;
    bool parsed : 1; // Whether we have parsed the arguments since calling init
    bool ignoreUnknown : 1; // Ignore unknown options and arguments
    QByteArray mCwd; // Current working directory. Important for KUnqiueApp!
    KCmdLineArgs::StdCmdLineArgs mStdargs;

    KCmdLineOptions qt_options;
    KCmdLineOptions kde_options;

    KCmdLineArgsStatic ();

    ~KCmdLineArgsStatic ();

    QTextCodec *codec; // codec for converting raw input to QString

    /**
     * @internal
     * Convertes raw command line argument data to proper QString.
     *
     * @param rawstr raw text
     * @return properly decoded QString
     */
    static QString decodeInput(const QByteArray &rawstr);

    /**
     * @internal
     * Convertes QString to raw command line output.
     *
     * @param str string to be encoded
     * @return raw text
     */
    static QByteArray encodeOutput(const QString &str);

    /**
     * @internal
     * Shell output with proper decoding.
     */
    void printQ(const QString &msg);

    /**
     * @internal
     * Try to match given option in the list of options.
     * Returns match status.
     *
     * @return:
     *  0 - option not found.
     *  1 - option found      // -fork
     *  2 - inverse option found ('no') // -nofork
     *  3 - option + arg found    // -fork now
     *
     *  +4 - no more options follow         // !fork
     */
    static int findOption(const KCmdLineOptions &options, QByteArray &opt,
                          QByteArray &opt_name, QString &def, bool &enabled);

    /**
     * @internal
     *
     * Checks what to do with a single option
     */
    static void findOption(const QByteArray &optv, const QByteArray &_opt,
                           int &i, bool _enabled, bool &moreOptions);

    /**
     * @internal
     *
     * Parse all arguments, verify correct syntax and put all arguments
     * where they belong.
     */
    static void parseAllArgs();

    /**
     * @internal
     *
     * Remove named options.
     *
     * @param id The name of the options to be removed.
     */
    static void removeArgs(const QByteArray &id);
};

K_GLOBAL_STATIC(KCmdLineArgsStatic, s)

KCmdLineArgsStatic::KCmdLineArgsStatic () {
    // Global data
    argsList = 0;
    all_argc = 0;
    all_argv = 0;
    appName = 0;
    mCwd.clear();
    about = 0;
    parsed = false;
    ignoreUnknown = false;
    mStdargs = 0;

    // Text codec.
    codec = QTextCodec::codecForLocale();

    // Qt options
    //FIXME: Check if other options are specific to Qt/X11
#ifdef Q_WS_X11
    qt_options.add("display <displayname>", ki18n("Use the X-server display 'displayname'"));
#elif defined(Q_WS_QWS)
    qt_options.add("display <displayname>", ki18n("Use the QWS display 'displayname'"));
#else
#endif
    qt_options.add("session <sessionId>", ki18n("Restore the application for the given 'sessionId'"));
    qt_options.add("cmap", ki18n("Causes the application to install a private color\nmap on an 8-bit display"));
    qt_options.add("ncols <count>", ki18n("Limits the number of colors allocated in the color\ncube on an 8-bit display, if the application is\nusing the QApplication::ManyColor color\nspecification"));
    qt_options.add("nograb", ki18n("tells Qt to never grab the mouse or the keyboard"));
    qt_options.add("dograb", ki18n("running under a debugger can cause an implicit\n-nograb, use -dograb to override"));
    qt_options.add("sync", ki18n("switches to synchronous mode for debugging"));
    qt_options.add("fn");
    qt_options.add("font <fontname>", ki18n("defines the application font"));
    qt_options.add("bg");
    qt_options.add("background <color>", ki18n("sets the default background color and an\napplication palette (light and dark shades are\ncalculated)"));
    qt_options.add("fg");
    qt_options.add("foreground <color>", ki18n("sets the default foreground color"));
    qt_options.add("btn");
    qt_options.add("button <color>", ki18n("sets the default button color"));
    qt_options.add("name <name>", ki18n("sets the application name"));
    qt_options.add("title <title>", ki18n("sets the application title (caption)"));
#ifdef Q_WS_X11
    qt_options.add("visual TrueColor", ki18n("forces the application to use a TrueColor visual on\nan 8-bit display"));
    qt_options.add("inputstyle <inputstyle>", ki18n("sets XIM (X Input Method) input style. Possible\nvalues are onthespot, overthespot, offthespot and\nroot"));
    qt_options.add("im <XIM server>", ki18n("set XIM server"));
    qt_options.add("noxim", ki18n("disable XIM"));
#endif
#ifdef Q_WS_QWS
    qt_options.add("qws", ki18n("forces the application to run as QWS Server"));
#endif
    qt_options.add("reverse", ki18n("mirrors the whole layout of widgets"));
    qt_options.add("stylesheet <file.qss>", ki18n("applies the Qt stylesheet to the application widgets"));
    qt_options.add("graphicssystem <system>", ki18n("use a different graphics system instead of the default one, options are raster and opengl (experimental)"));
    // KDE options
    kde_options.add("caption <caption>",   ki18n("Use 'caption' as name in the titlebar"));
    kde_options.add("icon <icon>",         ki18n("Use 'icon' as the application icon"));
    kde_options.add("config <filename>",   ki18n("Use alternative configuration file"));
    kde_options.add("nocrashhandler",      ki18n("Disable crash handler, to get core dumps"));
#ifdef Q_WS_X11
    kde_options.add("waitforwm",           ki18n("Waits for a WM_NET compatible windowmanager"));
#endif
    kde_options.add("style <style>",       ki18n("sets the application GUI style"));
    kde_options.add("geometry <geometry>", ki18n("sets the client geometry of the main widget - see man X for the argument format (usually WidthxHeight+XPos+YPos)"));
#ifndef Q_WS_WIN
    kde_options.add("smkey <sessionKey>"); // this option is obsolete and exists only to allow smooth upgrades from sessions
#endif
}

KCmdLineArgsStatic::~KCmdLineArgsStatic ()
{
    delete argsList;
    // KAboutData object is deleted by ~KCleanUpGlobalStatic.
    //delete about;
}

//
// KCmdLineArgs private data and methods
//

class KCmdLineArgsPrivate
{
    friend class KCmdLineArgsStatic;
public:
    KCmdLineArgsPrivate(const KCmdLineOptions &_options, const KLocalizedString &_name, const QByteArray &_id)
        : options(_options)
        , name(_name)
        , id(_id)
        , parsedOptionList(0)
        , parsedArgList(0)
        , isQt(id == "qt")
    {
    }
    ~KCmdLineArgsPrivate()
    {
        delete parsedOptionList;
        delete parsedArgList;
    }
    const KCmdLineOptions options;
    const KLocalizedString name;
    const QByteArray id;
    KCmdLineParsedOptions *parsedOptionList;
    KCmdLineParsedArgs *parsedArgList;
    bool isQt;

    /**
     * @internal
     *
     *  Set a boolean option
     */
    void setOption(const QByteArray &option, bool enabled);

    /**
     * @internal
     *
     *  Set a string option
     */
    void setOption(const QByteArray &option, const QByteArray &value);

    /**
     * @internal
     *
     * Add an argument
     */
    void addArgument(const QByteArray &argument);

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
};

//
// Static functions
//

QString
KCmdLineArgsStatic::decodeInput(const QByteArray &rawstr)
{
    return s->codec->toUnicode(rawstr);
}

QByteArray
KCmdLineArgsStatic::encodeOutput(const QString &str)
{
    return s->codec->fromUnicode(str);
}

void
KCmdLineArgsStatic::printQ(const QString &msg)
{
   fprintf(stdout, "%s", encodeOutput(msg).data());
}

void
KCmdLineArgs::init(int _argc, char **_argv,
                   const QByteArray &_appname,
                   const QByteArray &_catalog,
                   const KLocalizedString &_programName,
                   const QByteArray &_version,
                   const KLocalizedString &_description,
                   StdCmdLineArgs stdargs)
{
   init(_argc, _argv,
        new KAboutData(_appname, _catalog, _programName, _version, _description),
        stdargs);
}

void
KCmdLineArgs::initIgnore(int _argc, char **_argv, const QByteArray &_appname )
{
   init(_argc, _argv,
        new KAboutData(_appname, 0, ki18n(_appname), "unknown", ki18n("KDE Application")));
   s->ignoreUnknown = true;
}

void
KCmdLineArgs::init(const KAboutData* ab)
{
   char **_argv = (char **) malloc(sizeof(char *));
   _argv[0] = (char *) s->encodeOutput(ab->appName()).data();
   init(1,_argv,ab, CmdLineArgNone);
}


void
KCmdLineArgs::init(int _argc, char **_argv, const KAboutData *_about, StdCmdLineArgs stdargs)
{
   s->all_argc = _argc;
   s->all_argv = _argv;

   if (!s->all_argv)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Passing null-pointer to 'argv' is not allowed.\n\n");

      assert( 0 );
      exit(255);
   }

   // Strip path from argv[0]
   if (s->all_argc) {
     char *p = strrchr(s->all_argv[0], QDir::separator().toAscii());
     if (p)
       s->appName = p+1;
     else
       s->appName = s->all_argv[0];
   }

   s->about = _about;
   s->parsed = false;
   s->mCwd = QDir::currentPath().toLocal8Bit(); //currentPath() uses fromLocal8Bit internally apparently
   addStdCmdLineOptions(stdargs);
}

QString KCmdLineArgs::cwd()
{
   return QString::fromLocal8Bit(s->mCwd);
}

QString KCmdLineArgs::appName()
{
   if (!s->appName) return QString();
   return s->decodeInput(s->appName);
}

/**
  * Add Qt and KDE command line options to KCmdLineArgs.
  */
void KCmdLineArgs::addStdCmdLineOptions(StdCmdLineArgs stdargs) {
   if (stdargs & KCmdLineArgs::CmdLineArgQt) {
       KCmdLineArgs::addCmdLineOptions(s->qt_options, ki18n("Qt"), "qt");
   }
   if (stdargs & KCmdLineArgs::CmdLineArgKDE) {
       KCmdLineArgs::addCmdLineOptions(s->kde_options, ki18n("KDE"), "kde");
   }
   s->mStdargs = stdargs;
}

void
KCmdLineArgs::addCmdLineOptions( const KCmdLineOptions &options, const KLocalizedString &name,
         const QByteArray &id, const QByteArray &afterId)
{
   if (!s->argsList)
      s->argsList = new KCmdLineArgsList;

   int pos = s->argsList->count();
   // To make sure that the named options come before unnamed.
   if (pos > 0 && !id.isEmpty() && s->argsList->last()->d->name.isEmpty())
      pos--;

   KCmdLineArgsList::Iterator args;
   int i = 0;
   for(args = s->argsList->begin(); args != s->argsList->end(); ++args, i++)
   {
       if (id == (*args)->d->id) {
         return; // Options already present.
      }

      // Only check for afterId if it has been given non-empty, as the
      // unnamed option group should come after all named groups.
      if (!afterId.isEmpty() && afterId == (*args)->d->id)
         pos = i+1;
   }

   Q_ASSERT( s->parsed == false ); // You must add _ALL_ cmd line options
                                   // before accessing the arguments!
   s->argsList->insert(pos, new KCmdLineArgs(options, name, id));
}

void
KCmdLineArgs::saveAppArgs( QDataStream &ds)
{
   if (!s->parsed)
      s->parseAllArgs();

   // Remove Qt and KDE options.
   s->removeArgs("qt");
   s->removeArgs("kde");
   s->removeArgs("kuniqueapp");

   ds << s->mCwd;

   uint count = s->argsList ? s->argsList->count() : 0;
   ds << count;

   if (!count) return;

   KCmdLineArgsList::Iterator args;
   for(args = s->argsList->begin(); args != s->argsList->end(); ++args)
   {
      ds << (*args)->d->id;
      (*args)->d->save(ds);
   }
}

void
KCmdLineArgs::loadAppArgs( QDataStream &ds)
{
   s->parsed = true; // don't reparse argc/argv!

   // Remove Qt and KDE options.
   s->removeArgs("qt");
   s->removeArgs("kde");
   s->removeArgs("kuniqueapp");

   KCmdLineArgsList::Iterator args;
   if ( s->argsList ) {
      for(args = s->argsList->begin(); args != s->argsList->end(); ++args)
      {
         (*args)->clear();
      }
   }

   if (ds.atEnd())
      return;

   QByteArray qCwd;
   ds >> qCwd;

   s->mCwd = qCwd;

   uint count;
   ds >> count;

   while(count--)
   {
     QByteArray id;
     ds >> id;
     Q_ASSERT( s->argsList );
     bool found = false;
     for(args = s->argsList->begin(); args != s->argsList->end(); ++args)
     {
       if ((*args)->d->id  == id)
       {
          (*args)->d->load(ds);
          found = true;
          break;
       }
     }
     if (!found) {
         kWarning() << "Argument definitions for" << id << "not found!";
         // The next ds >> id will do nonsensical things...
     }
   }
   s->parsed = true;
}

KCmdLineArgs *KCmdLineArgs::parsedArgs(const QByteArray &id)
{
   if (!s->argsList)
      return 0;
   KCmdLineArgsList::Iterator args = s->argsList->begin();
   while(args != s->argsList->end())
   {
      if ((*args)->d->id == id)
      {
          if (!s->parsed)
             s->parseAllArgs();
          return *args;
      }
      ++args;
   }

   return 0;
}

void KCmdLineArgsStatic::removeArgs(const QByteArray &id)
{
   if (!s->argsList)
      return;
   KCmdLineArgsList::Iterator args = s->argsList->begin();
   while(args != s->argsList->end())
   {
      if ((*args)->d->id == id)
      {
          if (!s->parsed)
             s->parseAllArgs();
          break;
      }
      ++args;
   }

   if (args != s->argsList->end()) {
      KCmdLineArgs *a = *args;
      s->argsList->erase(args);
      delete a;
   }
}

int
KCmdLineArgsStatic::findOption(const KCmdLineOptions &options, QByteArray &opt,
                               QByteArray &opt_name, QString &def, bool &enabled)
{
   int result;
   bool inverse;

   for (int i = 0; i < options.d->names.size(); i++)
   {
      result = 0;
      inverse = false;
      opt_name = options.d->names[i];
      if (opt_name.startsWith(':') || opt_name.isEmpty())
      {
         continue;
      }
      if (opt_name.startsWith('!'))
      {
         opt_name = opt_name.mid(1);
         result = 4;
      }
      if (opt_name.startsWith("no")) // krazy:exclude=strings
      {
         opt_name = opt_name.mid(2);
         inverse = true;
      }

      int len = opt.length();
      if (opt == opt_name.left(len))
      {
         opt_name = opt_name.mid(len);
         if (opt_name.isEmpty())
         {
            if (inverse)
               return result+2;

            if (options.d->descriptions[i].isEmpty())
            {
               i++;
               if (i >= options.d->names.size())
                  return result+0;
               QByteArray nextOption = options.d->names[i];
               int p = nextOption.indexOf(' ');
               if (p > 0)
                  nextOption = nextOption.left(p);
               if (nextOption.startsWith('!'))
                  nextOption = nextOption.mid(1);
               if (nextOption.startsWith("no")) // krazy:exclude=strings
               {
                  nextOption = nextOption.mid(2);
                  enabled = !enabled;
               }
               result = findOption(options, nextOption, opt_name, def, enabled);
               Q_ASSERT(result);
               opt = nextOption;
               return result;
            }

            return 1;
         }
         if (opt_name.startsWith(' '))
         {
            opt_name = opt_name.mid(1);
            def = options.d->defaults[i];
            return result+3;
         }
      }
   }
   return 0;
}

void
KCmdLineArgsStatic::findOption(const QByteArray &optv, const QByteArray &_opt,
                               int &i, bool _enabled, bool &moreOptions)
{
   KCmdLineArgsList::Iterator args = s->argsList->begin();
   QByteArray opt = _opt;
   QByteArray opt_name;
   QString def;
   QByteArray argument;
   int j = opt.indexOf('=');
   if (j != -1)
   {
      argument = opt.mid(j+1);
      opt = opt.left(j);
   }

   bool enabled = true;
   int result = 0;
   while (args != s->argsList->end())
   {
      enabled = _enabled;
      result = findOption((*args)->d->options, opt, opt_name, def, enabled);
      if (result) break;
      ++args;
   }
   if ((args == s->argsList->end()) &&
       (optv.startsWith('-') && !optv.startsWith("--")))
   {
      // Option not found check if it is a valid option
      // in the style of -Pprinter1 or ps -aux
      int p = 1;
      while (true)
      {
         QByteArray singleCharOption = " "; // krazy:exclude=doublequote_chars
         singleCharOption[0] = optv[p];
         args = s->argsList->begin();
         while (args != s->argsList->end())
         {
            enabled = _enabled;
            result = findOption((*args)->d->options, singleCharOption,
			          opt_name, def, enabled);
            if (result) break;
            ++args;
         }
         if (args == s->argsList->end())
            break; // Unknown argument

         p++;
         if (result == 1) // Single option
         {
            (*args)->d->setOption(singleCharOption, enabled);
            if (p < optv.length())
               continue; // Next option
            else
               return; // Finished
         }
         else if (result == 3) // This option takes an argument
         {
            if (argument.isEmpty())
            {
               argument = optv.mid(p);
            }
            (*args)->d->setOption(singleCharOption, argument);
            return;
         }
         break; // Unknown argument
      }
      args = s->argsList->end();
      result = 0;
   }

   if (args == s->argsList->end() || !result)
   {
      if (s->ignoreUnknown)
         return;
      KCmdLineArgs::enable_i18n();
      KCmdLineArgs::usageError( i18n("Unknown option '%1'.", QString::fromLocal8Bit(_opt)));
   }

   if ((result & 4) != 0)
   {
      result &= ~4;
      moreOptions = false;
   }

   if (result == 3) // This option takes an argument
   {
      if (!enabled)
      {
         if (s->ignoreUnknown)
            return;
         KCmdLineArgs::enable_i18n();
         KCmdLineArgs::usageError( i18n("Unknown option '%1'.", QString::fromLocal8Bit(_opt)));
      }
      if (argument.isEmpty())
      {
         i++;
         if (i >= s->all_argc)
         {
            KCmdLineArgs::enable_i18n();
            KCmdLineArgs::usageError( i18nc("@info:shell %1 is cmdoption name","'%1' missing.",  QString::fromLocal8Bit(opt_name)));
         }
         argument = s->all_argv[i];
      }
      (*args)->d->setOption(opt, argument);
   }
   else
   {
      (*args)->d->setOption(opt, enabled);
   }
}

void
KCmdLineArgsStatic::parseAllArgs()
{
   bool allowArgs = false;
   bool inOptions = true;
   bool everythingAfterArgIsArgs = false;
   KCmdLineArgs *appOptions = s->argsList->last();
   if (appOptions->d->id.isEmpty())
   {
     foreach(const QByteArray& name, appOptions->d->options.d->names)
     {
       everythingAfterArgIsArgs = everythingAfterArgIsArgs || name.startsWith("!+");
       allowArgs = allowArgs || name.startsWith('+') || everythingAfterArgIsArgs;
     }
   }
   for(int i = 1; i < s->all_argc; i++)
   {
      if (!s->all_argv[i])
         continue;

      if ((s->all_argv[i][0] == '-') && s->all_argv[i][1] && inOptions)
      {
         bool enabled = true;
         QByteArray orig = s->all_argv[i];
         QByteArray option = orig.mid(1);
         if (option.startsWith('-'))
         {
            option = option.mid(1);
            s->all_argv[i]++;
            if (option.isEmpty())
            {
               inOptions = false;
               continue;
            }
         }
         if (option == "help")
         {
            KCmdLineArgs::usage();
         }
         else if (option.startsWith("help-")) // krazy:exclude=strings
         {
            KCmdLineArgs::usage(option.mid(5));
         }
#ifdef Q_WS_MAC
         // skip the finder -psn_* hint
         else if (option.startsWith("psn_")) // krazy:exclude=strings
         {
            continue;
         }
#endif
         else if ((option == "version") || (option == "v"))
         {
            KCmdLineArgs::enable_i18n();
            s->printQ(i18nc("@info:shell message on appcmd --version; do not translate 'Development Platform'"
                            "%3 application name, other %n version strings",
                            "Qt: %1\n"
                            "KDE Development Platform: %2\n"
                            "%3: %4\n",
                            QString::fromLatin1(qVersion()),
                            QString::fromLatin1(KDE_VERSION_STRING),
                            s->about->programName(), s->about->version()));
            exit(0);
         } else if (option == "license")
         {
            KCmdLineArgs::enable_i18n();
            s->printQ(s->about->license());
            s->printQ(QString::fromLatin1("\n"));
            exit(0);
         } else if (option == "author") {
             KCmdLineArgs::enable_i18n();
       if ( s->about ) {
         const QList<KAboutPerson> authors = s->about->authors();
         if ( !authors.isEmpty() ) {
           QString authorlist;
           for (QList<KAboutPerson>::ConstIterator it = authors.begin(); it != authors.end(); ++it ) {
             QString email;
             if ( !(*it).emailAddress().isEmpty() )
               email = QString::fromLatin1(" &lt;") + (*it).emailAddress() + QLatin1String("&gt;");
             authorlist += QString::fromLatin1("    ") + (*it).name() + email + QLatin1Char('\n');
           }
           s->printQ( i18nc("the 2nd argument is a list of name+address, one on each line","%1 was written by\n%2",   QString(s->about->programName()) ,  authorlist ) );
         }
       } else {
         s->printQ( i18n("This application was written by somebody who wants to remain anonymous.") );
       }
       if (s->about)
       {
         if (!s->about->customAuthorTextEnabled ())
         {
           if (s->about->bugAddress().isEmpty() || s->about->bugAddress() == QLatin1String("submit@bugs.kde.org") )
             s->printQ( i18n( "Please use http://bugs.kde.org to report bugs.\n" ) );
           else
             s->printQ( i18n( "Please report bugs to %1.\n" , s->about->bugAddress()) );
         }
         else
         {
           s->printQ(s->about->customAuthorPlainText()+QLatin1Char('\n'));
         }
       }
       exit(0);
         } else {
           if (option.startsWith("no")) // krazy:exclude=strings
           {
              option = option.mid(2);
              enabled = false;
           }
           s->findOption(orig, option, i, enabled, inOptions);
         }
      }
      else
      {
         // Check whether appOptions allows these arguments
         if (!allowArgs)
         {
            if (s->ignoreUnknown)
               continue;
            KCmdLineArgs::enable_i18n();
            KCmdLineArgs::usageError(i18n("Unexpected argument '%1'.", KuitSemantics::escape(s->decodeInput(s->all_argv[i]))));
         }
         else
         {
            appOptions->d->addArgument(s->all_argv[i]);
            if (everythingAfterArgIsArgs)
                inOptions = false;
         }
      }
   }
   s->parsed = true;
}

int & KCmdLineArgs::qtArgc()
{
   if (!s->argsList)
      addStdCmdLineOptions(CmdLineArgKDE|CmdLineArgQt); // Lazy bastards!

   static int qt_argc = -1;
   if( qt_argc != -1 )
      return qt_argc;

   if (!(s->mStdargs & KCmdLineArgs::CmdLineArgQt))
   {
     qt_argc = 2;
     return qt_argc;
   }

   KCmdLineArgs *args = parsedArgs("qt");
   Q_ASSERT(args); // No qt options have been added!
   if (!s->all_argv)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application has not called KCmdLineArgs::init(...).\n\n");

      assert( 0 );
      exit(255);
   }

   Q_ASSERT(s->all_argc >= (args->count()+1));
   qt_argc = args->count() +1;
   return qt_argc;
}

static char** s_qt_argv;

char **
KCmdLineArgs::qtArgv()
{
   if (!s->argsList)
      addStdCmdLineOptions(CmdLineArgKDE|CmdLineArgQt); // Lazy bastards!

   if( s_qt_argv != NULL )
      return s_qt_argv;

   if (!(s->mStdargs & KCmdLineArgs::CmdLineArgQt))
   {
     s_qt_argv = new char*[2];
     s_qt_argv[0] = qstrdup(s->all_argc?s->all_argv[0]:"");
     s_qt_argv[1] = 0;

     return s_qt_argv;
   }

   KCmdLineArgs *args = parsedArgs("qt");
   if (!args)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "The \"qt\" options have not be added to KCmdLineArgs!\n\n");

      assert( 0 );
      exit(255);
   }
   if (!s->all_argv)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application has not called KCmdLineArgs::init(...).\n\n");

      assert( 0 );
      exit(255);
   }

   int count=args->count();
   s_qt_argv = new char*[ count + 2 ];
   s_qt_argv[0] = qstrdup(s->all_argc?s->all_argv[0]:"");
   int i = 0;
   for(; i < count; i++)
   {
      s_qt_argv[i+1] = qstrdup(args->d->parsedArgList->at(i));
   }
   s_qt_argv[i+1] = 0;

   return s_qt_argv;
}

const KAboutData *
KCmdLineArgs::aboutData()
{
    return s->about;
}

void
KCmdLineArgs::enable_i18n()
{
    // called twice or too late
    if (KGlobal::hasLocale())
      return;

    if (!KGlobal::hasMainComponent()) {
        KComponentData mainComponentData(s->about);
        mainComponentData.config();
        // mainComponentData is now the main component and won't disappear until KGlobal deletes it
    }
}

void
KCmdLineArgs::usageError(const QString &error)
{
    Q_ASSERT(KGlobal::hasLocale());
    QByteArray localError = s->encodeOutput(error);
    if (localError.endsWith('\n'))
        localError.chop(1);
    fprintf(stderr, "%s: %s\n", s->appName, localError.data());

    QString tmp = i18n("Use --help to get a list of available command line options.");
    localError = s->encodeOutput(tmp);
    fprintf(stderr, "%s: %s\n", s->appName, localError.data());
    exit(254);
}

void
KCmdLineArgs::usage(const QByteArray &id)
{
   enable_i18n();
   Q_ASSERT(s->argsList != 0); // It's an error to call usage(...) without
                               // having done addCmdLineOptions first!

   QString optionFormatString = QString::fromLatin1("  %1 %2\n");
   QString optionFormatStringDef = QString::fromLatin1("  %1 %2 [%3]\n");
   QString tmp;
   QString usage;

   KCmdLineArgsList::Iterator args = --(s->argsList->end());

   if ((*args)->d->id.isEmpty() && ((*args)->d->options.d->names.size() > 0) &&
       !(*args)->d->options.d->names[0].startsWith('+'))
   {
      usage = i18n("[options] ")+usage;
   }

   while(true)
   {
      if (!(*args)->d->name.isEmpty())
      {
         usage = i18n("[%1-options]", (*args)->d->name.toString())+QLatin1Char(' ')+usage;
      }
      if (args == s->argsList->begin())
         break;
      --args;
   }

   KCmdLineArgs *appOptions = s->argsList->last();
   if (appOptions->d->id.isEmpty())
   {
     const KCmdLineOptions &option = appOptions->d->options;
     for (int i = 0; i < option.d->names.size(); i++)
     {
       QByteArray opt_name = option.d->names[i];
       if (opt_name.startsWith('+'))
           usage += QString::fromLatin1(opt_name.mid(1)) + QLatin1Char(' ');
       else if ( opt_name.startsWith("!+") )
          usage += QString::fromLatin1(opt_name.mid(2)) + QLatin1Char(' ');
     }
   }

   s->printQ(i18n("Usage: %1 %2\n", QString::fromLocal8Bit(s->appName), KuitSemantics::escape(usage)));
   s->printQ(QLatin1Char('\n')+s->about->shortDescription()+QLatin1Char('\n'));

   s->printQ(i18n("\nGeneric options:\n"));
   s->printQ(optionFormatString.arg(QString::fromLatin1("--help"), -25)
             .arg(i18n("Show help about options")));

   args = s->argsList->begin();
   while(args != s->argsList->end())
   {
      if (!(*args)->d->name.isEmpty() && !(*args)->d->id.isEmpty())
      {
          QString option = QString::fromLatin1("--help-%1").arg(QString::fromLatin1((*args)->d->id));
         QString desc = i18n("Show %1 specific options", (*args)->d->name.toString());

         s->printQ(optionFormatString.arg(option, -25).arg(desc));
      }
      ++args;
   }

   s->printQ(optionFormatString.arg(QString::fromLatin1("--help-all"),-25).arg(i18n("Show all options")));
   s->printQ(optionFormatString.arg(QString::fromLatin1("--author"),-25).arg(i18n("Show author information")));
   s->printQ(optionFormatString.arg(QString::fromLatin1("-v, --version"),-25).arg(i18n("Show version information")));
   s->printQ(optionFormatString.arg(QString::fromLatin1("--license"),-25).arg(i18n("Show license information")));
   s->printQ(optionFormatString.arg(QString::fromLatin1("--"), -25).arg(i18n("End of options")));

   args = s->argsList->begin(); // Sets current to 1st.

   bool showAll = (id == "all");

   if (!showAll)
   {
     while(args != s->argsList->end())
     {
       if (id == (*args)->d->id) break;
       ++args;
     }
   }

   while(args != s->argsList->end())
   {
     bool hasArgs = false;
     bool hasOptions = false;
     QString optionsHeader;
     if (!(*args)->d->name.isEmpty())
        optionsHeader = i18n("\n%1 options:\n", (*args)->d->name.toString());
     else
        optionsHeader = i18n("\nOptions:\n");

     while (args != s->argsList->end())
     {
       const KCmdLineOptions &option = (*args)->d->options;
       QByteArray opt;

       for (int i = 0; i < option.d->names.size(); i++)
       {
         QString description;
         QStringList dl;

         QString descriptionFull;
         if (!option.d->descriptions[i].isEmpty()) {
            descriptionFull = option.d->descriptions[i].toString();
         }

         // Option header
         if (option.d->names[i].startsWith(':'))
         {
            if (!descriptionFull.isEmpty())
            {
               optionsHeader = QLatin1Char('\n')+descriptionFull;
               if (!optionsHeader.endsWith(QLatin1Char('\n')))
                  optionsHeader.append(QLatin1Char('\n'));
               hasOptions = false;
            }
            continue;
         }

         // Free-form comment
         if (option.d->names[i].isEmpty())
         {
            if (!descriptionFull.isEmpty())
            {
               tmp = QLatin1Char('\n')+descriptionFull;
               if (!tmp.endsWith(QLatin1Char('\n')))
                  tmp.append(QLatin1Char('\n'));
               s->printQ(tmp);
            }
            continue;
         }

         // Options
         if (!descriptionFull.isEmpty())
         {
            dl = descriptionFull.split(QLatin1Char('\n'), QString::KeepEmptyParts);
            description = dl.first();
            dl.erase( dl.begin() );
         }
         QByteArray name = option.d->names[i];
         if (name.startsWith('!'))
             name = name.mid(1);

         if (name.startsWith('+'))
         {
            if (!hasArgs)
            {
               s->printQ(i18n("\nArguments:\n"));
               hasArgs = true;
            }

            name = name.mid(1);
            if (name.startsWith('[') && name.endsWith(']'))
                name = name.mid(1, name.length()-2);
            s->printQ(optionFormatString.arg(QString::fromLocal8Bit(name), -25).arg(description));
         }
         else
         {
            if (!hasOptions)
            {
               s->printQ(optionsHeader);
               hasOptions = true;
            }

            if ((name.length() == 1) || (name[1] == ' '))
               name = '-'+name;
            else
               name = "--"+name;
            if (descriptionFull.isEmpty())
            {
               opt = name + ", ";
            }
            else
            {
               opt = opt + name;
               if (option.d->defaults[i].isEmpty())
               {
                   s->printQ(optionFormatString.arg(QString::fromLatin1(opt), -25).arg(description));
               }
               else
               {
                   s->printQ(optionFormatStringDef.arg(QString::fromLatin1(opt), -25)
                            .arg(description, option.d->defaults[i]));
               }
               opt.clear();
            }
         }
         for(QStringList::Iterator it = dl.begin();
             it != dl.end();
             ++it)
         {
            s->printQ(optionFormatString.arg(QString(), -25).arg(*it));
         }
       }

       ++args;
       if (args == s->argsList->end() || !(*args)->d->name.isEmpty() || (*args)->d->id.isEmpty())
        break;
     }
     if (!showAll) break;
   }

   exit(0);
}

//
// Member functions
//

/**
 *  Constructor.
 *
 *  The given arguments are assumed to be constants.
 */
KCmdLineArgs::KCmdLineArgs( const KCmdLineOptions &_options,
                            const KLocalizedString &_name,
                            const QByteArray &_id)
  : d(new KCmdLineArgsPrivate(_options, _name, _id))
{
}

/**
 *  Destructor.
 */
KCmdLineArgs::~KCmdLineArgs()
{
  if (!s.isDestroyed() && s->argsList)
     s->argsList->removeAll(this);
  delete d;
}

void
KCmdLineArgs::setCwd( const QByteArray &cwd )
{
    s->mCwd = cwd;
}

void
KCmdLineArgs::clear()
{
   delete d->parsedArgList;     d->parsedArgList = 0;
   delete d->parsedOptionList;  d->parsedOptionList = 0;
}

void
KCmdLineArgs::reset()
{
   if ( s->argsList ) {
      delete s->argsList; s->argsList = 0;
   }
   s->parsed = false;
}

void
KCmdLineArgsPrivate::save( QDataStream &ds) const
{
   if (parsedOptionList)
      ds << (*(parsedOptionList));
   else
      ds << quint32(0);

   if (parsedArgList)
      ds << (*(parsedArgList));
   else
      ds << quint32(0);
}

void
KCmdLineArgsPrivate::load( QDataStream &ds)
{
   if (!parsedOptionList) parsedOptionList = new KCmdLineParsedOptions;
   if (!parsedArgList) parsedArgList = new KCmdLineParsedArgs;

   ds >> (*(parsedOptionList));
   ds >> (*(parsedArgList));

   if (parsedOptionList->count() == 0)
   {
      delete parsedOptionList;  parsedOptionList = 0;
   }
   if (parsedArgList->count() == 0)
   {
      delete parsedArgList;     parsedArgList = 0;
   }
}

void
KCmdLineArgsPrivate::setOption(const QByteArray &opt, bool enabled)
{
   if (isQt)
   {
      // Qt does it own parsing.
      QByteArray argString = "-"; // krazy:exclude=doublequote_chars
      if( !enabled )
          argString += "no";
      argString += opt;
      addArgument(argString);
   }
   if (!parsedOptionList) {
      parsedOptionList = new KCmdLineParsedOptions;
   }

   if (enabled)
      parsedOptionList->insert( opt, "t" ); // krazy:exclude=doublequote_chars
   else
      parsedOptionList->insert( opt, "f" ); // krazy:exclude=doublequote_chars
}

void
KCmdLineArgsPrivate::setOption(const QByteArray &opt, const QByteArray &value)
{
   if (isQt)
   {
      // Qt does it's own parsing.
      QByteArray argString = "-"; // krazy:exclude=doublequote_chars
      argString += opt;
      addArgument(argString);
      addArgument(value);

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
      // Hack coming up!
      if (argString == "-display")
      {
         setenv(DISPLAY, value.data(), true);
      }
#endif
   }
   if (!parsedOptionList) {
      parsedOptionList = new KCmdLineParsedOptions;
   }

   parsedOptionList->insertMulti( opt, value );
}

QString
KCmdLineArgs::getOption(const QByteArray &_opt) const
{
   QByteArray opt = _opt;
   QByteArray value;
   if (d->parsedOptionList)
   {
      value = d->parsedOptionList->value(opt);
   }
   if (!value.isEmpty())
       return QString::fromLocal8Bit(value);

   // Look up the default.
   QByteArray opt_name;
   QString def;
   bool dummy = true;
   int result = s->findOption( d->options, opt, opt_name, def, dummy) & ~4;

   if (result != 3)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for getOption(\"%s\") but the \"%s\" option\n",
                      opt.data(), opt.data());
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");

      Q_ASSERT( 0 );
      exit(255);
   }
   return def;
}

QStringList
KCmdLineArgs::getOptionList(const QByteArray &opt) const
{
   QStringList result;
   if (!d->parsedOptionList)
      return result;

   while(true)
   {
      QByteArray value = d->parsedOptionList->take(opt);
      if (value.isEmpty())
         break;
      result.prepend(QString::fromLocal8Bit(value));
   }

   // Reinsert items in dictionary
   // WABA: This is rather silly, but I don't want to add restrictions
   // to the API like "you can only call this function once".
   // I can't access all items without taking them out of the list.
   // So taking them out and then putting them back is the only way.
   Q_FOREACH(const QString &str, result)
   {
      d->parsedOptionList->insertMulti(opt, str.toLocal8Bit());
   }
   return result;
}

bool
KCmdLineArgs::isSet(const QByteArray &_opt) const
{
   // Look up the default.
   QByteArray opt = _opt;
   QByteArray opt_name;
   QString def;
   int result = 0;
   KCmdLineArgsList::Iterator args = s->argsList->begin();
   while (args != s->argsList->end())
   {
      bool dummy = true;
      result = s->findOption((*args)->d->options, opt, opt_name, def, dummy) & ~4;
      if (result) break;
      ++args;
   }

   if (result == 0)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for isSet(\"%s\") but the \"%s\" option\n",
                      opt.data(), opt.data());
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");

      Q_ASSERT( 0 );
      exit(255);
   }

   QByteArray value;
   if (d->parsedOptionList)
   {
      value = d->parsedOptionList->value(opt);
   }

   if (!value.isEmpty())
   {
      if (result == 3)
         return true;
      else
         return (value.at(0) == 't');
   }

   if (result == 3)
      return false; // String option has 'false' as default.

   // We return 'true' as default if the option was listed as '-nofork'
   // We return 'false' as default if the option was listed as '-fork'
   return (result == 2);
}

int
KCmdLineArgs::count() const
{
   return d->parsedArgList?d->parsedArgList->count():0;
}

QString
KCmdLineArgs::arg(int n) const
{
   if (!d->parsedArgList || (n >= (int) d->parsedArgList->count()))
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs): Argument out of bounds\n");
      fprintf(stderr, "Application requests for arg(%d) without checking count() first.\n",
                      n);

      Q_ASSERT( 0 );
      exit(255);
   }

   return QString::fromLocal8Bit(d->parsedArgList->at(n));
}

KUrl
KCmdLineArgs::url(int n) const
{
   return makeURL( arg(n).toUtf8() );
}

KUrl KCmdLineArgs::makeURL(const QByteArray &_urlArg)
{
    const QString urlArg = QString::fromUtf8(_urlArg);
    QFileInfo fileInfo(urlArg);
    if (!fileInfo.isRelative()) { // i.e. starts with '/', on unix
        KUrl result;
        result.setPath(QDir::fromNativeSeparators(urlArg));
        return result; // Absolute path.
    }

    if ( KUrl::isRelativeUrl(urlArg) || fileInfo.exists() ) {
        KUrl result;
        result.setPath(cwd()+QLatin1Char('/')+urlArg);
        result.cleanPath();
        return result;  // Relative path
    }

    return KUrl(urlArg); // Argument is a URL
}

void
KCmdLineArgsPrivate::addArgument(const QByteArray &argument)
{
   if (!parsedArgList)
      parsedArgList = new KCmdLineParsedArgs;

   parsedArgList->append(argument);
}

void
KCmdLineArgs::addTempFileOption()
{
    KCmdLineOptions tmpopt;
    tmpopt.add( "tempfile", ki18n("The files/URLs opened by the application will be deleted after use") );
    KCmdLineArgs::addCmdLineOptions( tmpopt, ki18n("KDE-tempfile"), "kde-tempfile" );
}

bool KCmdLineArgs::isTempFileSet()
{
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs( "kde-tempfile" );
    return args && args->isSet( "tempfile" );
}

QStringList KCmdLineArgs::allArguments()
{
    QStringList lst;

    for(int i = 0; i < s->all_argc; i++) {
        char* arg = s->all_argv[i];
        if (!arg)
            continue;
        lst.append(QString::fromLocal8Bit(arg));
    }
    return lst;
}
