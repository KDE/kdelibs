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

// $Id$

// Include our header without NDEBUG defined to avoid having the kDebugInfo
// functions inlined to noops (which would then conflict with their definition
// here).

#ifdef NDEBUG
#define NODEBUG
#undef NDEBUG
#endif
#include "kdebug.h"
#ifdef NODEBUG
#define NDEBUG
#undef NODEBUG
#endif

// Other includes needed by kDebug* functions.

#include "kglobal.h"
#include "kinstance.h"
#include "kstddirs.h"
#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qintdict.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>	// abort
#include <unistd.h>	// getpid
#include <stdarg.h>	// vararg stuff
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>
#include "kstaticdeleter.h"
#include <config.h>

class KDebugEntry;
template class QList<KDebugEntry>;

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

static KConfig *debug_Config = 0;
static KStaticDeleter<KConfig> pcd;

static void kDebugBackend( unsigned short nLevel, unsigned int nArea, const char *data)
{
  if ( !debug_Config && KGlobal::_instance )
  {
      pcd.setObject(debug_Config, new KConfig( "kdebugrc", false, false ));
      debug_Config->setGroup("0");
  }

  static QString aAreaName;
  static unsigned int oldarea = 0;
  if (debug_Config && oldarea != nArea) {
    debug_Config->setGroup( QString::number(static_cast<int>(nArea)) );
    oldarea = nArea;
    if ( nArea > 0 && KGlobal::_instance )
      aAreaName = getDescrFromNum(nArea);
    if ((nArea == 0) || aAreaName.isEmpty())
      if ( KGlobal::_instance )
        aAreaName = KGlobal::instance()->instanceName();
  }

  static short nOutput = 2;
  static ushort oldLevel = KDEBUG_FATAL + 1;
  static int nPriority = 0;
  static QString aCaption;

  if (oldLevel != nLevel) {
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

    nOutput = debug_Config ? debug_Config->readNumEntry(key, 2) : 2;
  }



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
                QString aOutputFileName = debug_Config->readEntry(aKey, "kdebug.dbg");

                char buf[4096] = "";
		int nSize;
                if ( !aAreaName.isEmpty() )
		    nSize = sprintf( buf, "%s: %s", aAreaName.ascii(), data);
		else
		    nSize = sprintf( buf, "%s", data);

                QFile aOutputFile( aOutputFileName );
                aOutputFile.open( IO_WriteOnly | IO_Append );
                aOutputFile.writeBlock( buf, nSize );
                aOutputFile.close();
                break;
          }
        case 1: // Message Box
          {
                // Since we are in kdecore here, we cannot use KMsgBox and use
                // QMessageBox instead
	      if ( !aAreaName.isEmpty() ) aCaption += QString("(")+aAreaName+")";
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
	      // if ( !aAreaName.isEmpty() ) fprintf( output, "%d %s: ", (int)getpid(), aAreaName.ascii() );
	      if ( !aAreaName.isEmpty() ) fprintf( output, "%s: ", aAreaName.ascii() );
	      fputs(  data, output);
	      break;
          }
        case 3: // syslog
          {
	      syslog( nPriority, "%s", data);
          }
        case 4: // nothing
          {
          }
        }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL ) && debug_Config &&
          ( debug_Config->readNumEntry( "AbortFatal", 1 ) ) )
        abort();
}

static void kDebugBackend2( unsigned short nLevel, unsigned int nArea, const char* fmt, va_list arguments )
{
    char buf[4096] = "";
    int nSize = vsnprintf( buf, 4096, fmt, arguments );
    if( nSize > 4094 ) nSize = 4094;
    buf[nSize] = '\n';
    buf[nSize+1] = '\0';
    kDebugBackend( nLevel, nArea, buf);
}

void kDebugInfo( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_INFO, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugInfo( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_INFO, area, fmt, arguments  );
    va_end( arguments );
}

void kDebugInfo( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugWarning( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_WARN, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugWarning( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_WARN, area, fmt, arguments );
    va_end( arguments );
}

void kDebugWarning( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugError( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_ERROR, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugError( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_ERROR, area, fmt, arguments );
    va_end( arguments );
}

void kDebugError( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugFatal(const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_FATAL, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugFatal(unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend2( KDEBUG_FATAL, area, fmt, arguments );
    va_end( arguments );
}

void kDebugFatal( bool cond, unsigned short area, const char* fmt, ... )
{
  if(cond)
    {
      va_list arguments;
      va_start( arguments, fmt );
      kDebugBackend2( KDEBUG_INFO, area, fmt, arguments );
      va_end( arguments );
    }
}

void kDebugPError( const char* fmt, ... )
{
    char buf[4096];
    va_list arguments;
    va_start( arguments, fmt );
    vsprintf( buf, fmt, arguments );
    kDebugError( "%s: %s", buf, strerror(errno) );
}

void kDebugPError( unsigned short area, const char* fmt, ... )
{
    char buf[4096];
    va_list arguments;
    va_start( arguments, fmt );
    vsprintf( buf, fmt, arguments );
    kDebugError( area, "%s: %s", buf, strerror(errno) );
}

kdbgstream &perror( kdbgstream &s) { return s << " " << QString::fromLocal8Bit(strerror(errno)); }
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
