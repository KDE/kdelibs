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

#include "kdebug.h"
#include "kglobal.h"
#include "kinstance.h"
#include "kstddirs.h"
#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>	// abort
#include <stdarg.h>	// vararg stuff
#include <syslog.h>
#include <kconfig.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class KDebugEntry;
template class QList<KDebugEntry>;

class KDebugEntry
{
public:
  KDebugEntry (int n, QString d) {number=n; descr=d.copy();}
  KDebugEntry (QString d, int n) {number=n; descr=d.copy();}
  unsigned int number;
  QString descr;
};

static QList<KDebugEntry> *KDebugCache;
#define MAX_CACHE 20

static QString getDescrFromNum(unsigned short _num)
{
  if (!KDebugCache)
      KDebugCache = new QList<KDebugEntry>;

  for ( KDebugEntry *ent = KDebugCache->first();
		  ent != 0; ent = KDebugCache->next()) {
	  if (ent->number == _num) {
		  return ent->descr.copy();
	  }
  }

  QString data, filename(locate("config","kdebug.areas"));
  QFile file(filename);
  if (!file.open(IO_ReadOnly)) {
    warning("Couldn't open %s", filename.ascii());
    file.close();
    return "";
  }

  unsigned long number;
  bool longOK;

  QTextStream *ts = new QTextStream(&file);
  while (!ts->eof()) {
    data = ts->readLine().simplifyWhiteSpace();

    int pos = data.find("#");
    if ( pos != -1 )
      data.truncate( pos );

    if (data.isEmpty())
      continue;

    int space = data.find(" ");
    if (space == -1)
      continue;

    number = data.left(space).toULong(&longOK);
    if (!longOK)
      continue; // The first part wasn't a number

    if (number != _num)
      continue; // Not the number we're looking for

    data.remove(0, space+1);

    if (KDebugCache->count() >= MAX_CACHE)
      KDebugCache->removeFirst();
    KDebugCache->append(new KDebugEntry(number,data));
    delete ts;
    file.close();
    return data;
  }

  delete ts;
  file.close();
  return "";
}

// Private
/* still public for now
enum DebugLevels {
        KDEBUG_INFO=    0,
        KDEBUG_WARN=    1,
        KDEBUG_ERROR=   2,
        KDEBUG_FATAL=   3
}; */

void kDebugBackend( unsigned short nLevel, unsigned short nArea,
                    const char * pFormat, va_list arguments )
{

  KConfig * pConfig = 0L;
  if ( KGlobal::_instance )
  {
      pConfig = new KConfig( "kdebugrc", false );
      pConfig->setGroup( QString::number(nArea) );
  }

  /* Determine output */
  int nPriority = 0; // for syslog
  QString aCaption;
  QString aAreaName;
  if ( nArea > 0 && KGlobal::_instance )
    aAreaName = getDescrFromNum(nArea);
  if (aAreaName.isEmpty() && KGlobal::_instance)
    aAreaName = KGlobal::instance()->instanceName();

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
        };
  short nOutput = pConfig ? pConfig->readNumEntry(key, 2) : 2;

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
                QString aOutputFileName = pConfig->readEntry(aKey, "kdebug.dbg");

                char buf[4096] = "";
                int nPrefix = 0;
                if ( !aAreaName.isEmpty() )
                  nPrefix = sprintf( buf, "%s: ", aAreaName.ascii() );
                unsigned int nSize = vsnprintf( buf, sizeof(buf)-1, pFormat, arguments );
		nSize = QMIN(nSize, sizeof(buf)-2-nPrefix);
                buf[nSize] = '\n';
                buf[nSize+1] = '\0';
                QFile aOutputFile( aOutputFileName );
                aOutputFile.open( IO_WriteOnly );
                aOutputFile.writeBlock( buf, nSize+1 );
                aOutputFile.close();
                break;
          }
        case 1: // Message Box
          {
                // Since we are in kdecore here, we cannot use KMsgBox and use
                // QMessageBox instead
                char buf[4096]; // constants are evil, but this is evil code anyway
                int nSize = vsprintf( buf, pFormat, arguments );
                if( nSize > 4094 ) nSize = 4094;
                buf[nSize] = '\n';
                buf[nSize+1] = '\0';
                if ( !aAreaName.isEmpty() ) aCaption += QString("(")+aAreaName+")";
                QMessageBox::warning( 0L, aCaption, buf, i18n("&OK") );
                break;
          }
        case 2: // Shell
          {
                if ( !aAreaName.isEmpty() ) fprintf( stderr, "%s: ", aAreaName.ascii() );
                vfprintf( stderr, pFormat, arguments );
                fprintf( stderr, "\n" );
                break;
          }
        case 3: // syslog
          {
                char buf[4096] = "";
                int nPrefix = 0;
                if ( !aAreaName.isEmpty() ) nPrefix = sprintf( buf, "%s: ", aAreaName.ascii() );
                int nSize = vsprintf( &buf[nPrefix], pFormat, arguments );
                if( nSize > (4094-nPrefix) ) nSize = 4094-nPrefix;
                buf[nSize] = '\n';
                buf[nSize+1] = '\0';
                syslog( nPriority, buf );
          }
        case 4: // nothing
          {
          }
        }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL ) && pConfig &&
          ( pConfig->readNumEntry( "AbortFatal", 0 ) ) )
        abort();
  delete pConfig;
}


void kDebugInfo( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_INFO, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugInfo( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_INFO, area, fmt, arguments  );
    va_end( arguments );
}

void kDebugWarning( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_WARN, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugWarning( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_WARN, area, fmt, arguments );
    va_end( arguments );
}

void kDebugError( const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_ERROR, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugError( unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_ERROR, area, fmt, arguments );
    va_end( arguments );
}

void kDebugFatal(const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_FATAL, 0, fmt, arguments );
    va_end( arguments );
}

void kDebugFatal(unsigned short area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( KDEBUG_FATAL, area, fmt, arguments );
    va_end( arguments );
}

// For compatibility
void kdebug( ushort nLevel, ushort area, const char* fmt, ... )
{
    va_list arguments;
    va_start( arguments, fmt );
    kDebugBackend( nLevel, area, fmt, arguments );
    va_end( arguments );
}

