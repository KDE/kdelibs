/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// Include our header without NDEBUG defined to avoid having the kDebugInfo
// functions inlined to noops (which would then conflict with their definition
// here).

#include "kdebug.h"

#ifdef NDEBUG
#undef kdDebug
#undef kdBacktrace
#endif

#include "kapplication.h"
#include "kglobal.h"
#include "kinstance.h"
#include "kstandarddirs.h"
#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qintdict.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>	// abort
#include <unistd.h>	// getpid
#include <stdarg.h>	// vararg stuff
#include <ctype.h>      // isprint
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>
#include "kstaticdeleter.h"
#include <config.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

class KDebugEntry;

class KDebugEntry
{
public:
    KDebugEntry (int n, QString d) {number=n; descr=d;}
    unsigned int number;
    QString descr;
};

static QIntDict<KDebugEntry> *KDebugCache;

static KStaticDeleter< QIntDict<KDebugEntry> > kdd;

static QString getDescrFromNum(unsigned int _num)
{
  if (!KDebugCache) {
    kdd.setObject(KDebugCache, new QIntDict<KDebugEntry>);
    // Do not call this deleter from ~KApplication
    KGlobal::unregisterStaticDeleter(&kdd);
    KDebugCache->setAutoDelete(true);
  }

  KDebugEntry *ent = KDebugCache->find( _num );
  if ( ent )
    return ent->descr;

  if ( !KDebugCache->isEmpty() ) // areas already loaded
    return QString::null;

  QString filename(locate("config","kdebug.areas"));
  QFile file(filename);
  if (!file.open(IO_ReadOnly)) {
    qWarning("Couldn't open %s", filename.local8Bit().data());
    file.close();
    return "";
  }

  unsigned long number = 0;
  bool longOK;

  QTextStream *ts = new QTextStream(&file);
  ts->setEncoding( QTextStream::Latin1 );
  while (!ts->eof()) {
    const QString data(ts->readLine());
    int i = 0;
    int len = data.length();

    QChar ch = data[0];
    if (ch == '#' || ch.isNull()) {
      continue;
    }
    while (ch.isSpace()) {
      if (!(i < len))
	continue;
      ++i;
      ch = data[i];
    }
    if (ch.isNumber()) {
	int numStart = i ;
	while (ch.isNumber())  {
	  if (!(i < len))
	    continue;
	  ++i;
	  ch = data[i];
	}
	number = data.mid(numStart,i).toULong(&longOK);
    }
    while (ch.isSpace()) {
      if (!(i < len))
	continue;
      ++i;
      ch = data[i];
    }
    const QString description(data.mid(i, len));
    //qDebug("number: [%i] description: [%s]", number, description.latin1());

    KDebugCache->insert(number, new KDebugEntry(number,description));
  }

  delete ts;
  file.close();

  ent = KDebugCache->find( _num );
  if ( ent )
      return ent->descr;

  return QString::null;
}

enum DebugLevels {
    KDEBUG_INFO=    0,
    KDEBUG_WARN=    1,
    KDEBUG_ERROR=   2,
    KDEBUG_FATAL=   3
};


struct kDebugPrivate {
  kDebugPrivate() : 
  	oldarea(0), config(0) { }
  	
  ~kDebugPrivate() { delete config; }

  QString aAreaName;
  unsigned int oldarea;
  KConfig *config;
};

static kDebugPrivate *kDebug_data = 0;
static KStaticDeleter<kDebugPrivate> pcd;

static void kDebugBackend( unsigned short nLevel, unsigned int nArea, const char *data)
{
  if ( !kDebug_data )
  {
      pcd.setObject(kDebug_data, new kDebugPrivate());
      // Do not call this deleter from ~KApplication
      KGlobal::unregisterStaticDeleter(&pcd);
  }

  if (!kDebug_data->config && KGlobal::_instance )
  {
      kDebug_data->config = new KConfig("kdebugrc", false, false);
      kDebug_data->config->setGroup("0");
  }

  if (kDebug_data->config && kDebug_data->oldarea != nArea) {
    kDebug_data->config->setGroup( QString::number(static_cast<int>(nArea)) );
    kDebug_data->oldarea = nArea;
    if ( nArea > 0 && KGlobal::_instance )
      kDebug_data->aAreaName = getDescrFromNum(nArea);
    if ((nArea == 0) || kDebug_data->aAreaName.isEmpty())
      if ( KGlobal::_instance )
        kDebug_data->aAreaName = KGlobal::instance()->instanceName();
  }

  int nPriority = 0;
  QString aCaption;

    /* Determine output */

    QString key;
    switch( nLevel )
      {
      case KDEBUG_INFO:
	key = "InfoOutput";
	aCaption = "Info";
	nPriority = LOG_INFO;
	break;
      case KDEBUG_WARN:
	key = "WarnOutput";
	aCaption = "Warning";
	nPriority = LOG_WARNING;
	break;
      case KDEBUG_FATAL:
	key = "FatalOutput";
	aCaption = "Fatal Error";
	nPriority = LOG_CRIT;
	break;
      case KDEBUG_ERROR:
      default:
	/* Programmer error, use "Error" as default */
	key = "ErrorOutput";
	aCaption = "Error";
	nPriority = LOG_ERR;
	break;
      }

  short nOutput = kDebug_data->config ? kDebug_data->config->readNumEntry(key, 2) : 2;

  // If the application doesn't have a QApplication object it can't use
  // a messagebox.
  if (!kapp && (nOutput == 1))
    nOutput = 2;

  // Output
  switch( nOutput )
        {
        case 0: // File
          {
                QString aKey;
                switch( nLevel )
                {
                    case KDEBUG_INFO:
                        aKey = "InfoFilename";
                        break;
                    case KDEBUG_WARN:
                        aKey = "WarnFilename";
                        break;
                    case KDEBUG_FATAL:
                        aKey = "FatalFilename";
                        break;
                    case KDEBUG_ERROR:
                    default:
                        aKey = "ErrorFilename";
                        break;
                }
                QString aOutputFileName = kDebug_data->config->readEntry(aKey, "kdebug.dbg");

                const int BUFSIZE = 4096;
                char buf[BUFSIZE] = "";
		int nSize;
                if ( !kDebug_data->aAreaName.isEmpty() )
		    nSize = snprintf( buf, BUFSIZE, "%s: %s", kDebug_data->aAreaName.ascii(), data);
		else
		    nSize = snprintf( buf, BUFSIZE, "%s", data);

                QFile aOutputFile( aOutputFileName );
                aOutputFile.open( IO_WriteOnly | IO_Append );
                if ( ( nSize == -1 ) || ( nSize >= BUFSIZE ) )
                    aOutputFile.writeBlock( buf, BUFSIZE-1 );
                else
                    aOutputFile.writeBlock( buf, nSize );
                aOutputFile.close();
                break;
          }
        case 1: // Message Box
          {
                // Since we are in kdecore here, we cannot use KMsgBox and use
                // QMessageBox instead
	      if ( !kDebug_data->aAreaName.isEmpty() ) aCaption += QString("(")+kDebug_data->aAreaName+")";
	      QMessageBox::warning( 0L, aCaption, data, i18n("&OK") );
	      break;
          }
        case 2: // Shell
          {
              FILE *output;
              /* we used to use stdout for debug
              if (nPriority == LOG_INFO)
                  output = stderr;
              else */
                  output = stderr;
              // Uncomment this to get the pid of the app in the output (useful for e.g. kioslaves)
	      // if ( !kDebug_data->aAreaName.isEmpty() ) fprintf( output, "%d %s: ", (int)getpid(), kDebug_data->aAreaName.ascii() );
	      if ( !kDebug_data->aAreaName.isEmpty() ) fprintf( output, "%s: ", kDebug_data->aAreaName.ascii() );
	      fputs(  data, output);
	      break;
          }
        case 3: // syslog
          {
	      syslog( nPriority, data);
          }
        case 4: // nothing
          {
          }
        }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL )
      && ( !kDebug_data->config || kDebug_data->config->readNumEntry( "AbortFatal", 1 ) ) )
        abort();
}

kdbgstream &perror( kdbgstream &s) { return s << QString::fromLocal8Bit(strerror(errno)); }
kdbgstream kdDebug(int area) { return kdbgstream(area, KDEBUG_INFO); }
kdbgstream kdDebug(bool cond, int area) { if (cond) return kdbgstream(area, KDEBUG_INFO); else return kdbgstream(0, 0, false); }

kdbgstream kdError(int area) { return kdbgstream("ERROR: ", area, KDEBUG_ERROR); }
kdbgstream kdError(bool cond, int area) { if (cond) return kdbgstream("ERROR: ", area, KDEBUG_ERROR); else return kdbgstream(0,0,false); }
kdbgstream kdWarning(int area) { return kdbgstream("WARNING: ", area, KDEBUG_WARN); }
kdbgstream kdWarning(bool cond, int area) { if (cond) return kdbgstream("WARNING: ", area, KDEBUG_WARN); else return kdbgstream(0,0,false); }
kdbgstream kdFatal(int area) { return kdbgstream("FATAL: ", area, KDEBUG_FATAL); }
kdbgstream kdFatal(bool cond, int area) { if (cond) return kdbgstream("FATAL: ", area, KDEBUG_FATAL); else return kdbgstream(0,0,false); }

void kdbgstream::flush() {
    if (output.isEmpty() || !print)
	return;
    kDebugBackend( level, area, output.local8Bit().data() );
    output = QString::null;
}

kdbgstream &kdbgstream::form(const char *format, ...)
{
    char buf[4096];
    va_list arguments;
    va_start( arguments, format );
    vsprintf( buf, format, arguments );
    va_end(arguments);
    *this << buf;
    return *this;
}

kdbgstream::~kdbgstream() {
    if (!output.isEmpty()) {
	fprintf(stderr, "ASSERT: debug output not ended with \\n\n");
	*this << "\n";
    }
}

kdbgstream& kdbgstream::operator << (char ch)
{
  if (!print) return *this;
  if (!isprint(ch))
    output += "\\x" + QString::number( static_cast<uint>( ch ) + 0x100, 16 ).right(2);
  else {
    output += ch;
    if (ch == '\n') flush();
  }
  return *this;
}

kdbgstream& kdbgstream::operator << (QWidget* widget)
{
  QString string, temp;
  // -----
  if(widget==0)
    {
      string=(QString)"[Null pointer]";
    } else {
      temp.setNum((ulong)widget, 16);
      string=(QString)"["+widget->className()+" pointer "
	+ "(0x" + temp + ")";
      if(widget->name(0)==0)
	{
	  string += " to unnamed widget, ";
	} else {
	  string += (QString)" to widget " + widget->name() + ", ";
	}
      string += "geometry="
	+ QString().setNum(widget->width())
	+ "x"+QString().setNum(widget->height())
	+ "+"+QString().setNum(widget->x())
	+ "+"+QString().setNum(widget->y())
	+ "]";
    }
  if (!print)
    {
      return *this;
    }
  output += string;
  if (output.at(output.length() -1 ) == '\n')
    {
      flush();
    }
  return *this;
}

QString kdBacktrace(int levels)
{
    QString s;
#ifdef HAVE_BACKTRACE
    void* trace[256];
    int n = backtrace(trace, 256);
    char** strings = backtrace_symbols (trace, n);

    if ( levels != -1 )
        n = QMIN( n, levels );
    s = "[\n";

    for (int i = 0; i < n; ++i)
        s += QString::number(i) +
             QString::fromLatin1(": ") +
             QString::fromLatin1(strings[i]) + QString::fromLatin1("\n");
    s += "]\n";
    free (strings);
#endif
    return s;
}

QString kdBacktrace()
{
    return kdBacktrace(-1 /*all*/);
}

// Needed for --enable-final
#ifdef NDEBUG
#define kdDebug kndDebug
#endif
