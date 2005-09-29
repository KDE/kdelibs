/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdebug.h"

#ifdef NDEBUG
#undef kdDebug
#undef kdBacktrace
#endif

#include "kdebugdcopiface.h"

#include "kapplication.h"
#include "kglobal.h"
#include "kinstance.h"
#include "kstandarddirs.h"

#include <qmessagebox.h>
#include <klocale.h>
#include <qfile.h>
#include <qintdict.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregion.h>
#include <qstringlist.h>
#include <qpen.h>
#include <qbrush.h>
#include <qsize.h>

#include <kurl.h>

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
    KDebugEntry (int n, const QCString& d) {number=n; descr=d;}
    unsigned int number;
    QCString descr;
};

static QIntDict<KDebugEntry> *KDebugCache;

static KStaticDeleter< QIntDict<KDebugEntry> > kdd;

static QCString getDescrFromNum(unsigned int _num)
{
  if (!KDebugCache) {
    kdd.setObject(KDebugCache, new QIntDict<KDebugEntry>( 601 ));
    // Do not call this deleter from ~KApplication
    KGlobal::unregisterStaticDeleter(&kdd);
    KDebugCache->setAutoDelete(true);
  }

  KDebugEntry *ent = KDebugCache->find( _num );
  if ( ent )
    return ent->descr;

  if ( !KDebugCache->isEmpty() ) // areas already loaded
    return QCString();

  QString filename(locate("config","kdebug.areas"));
  if (filename.isEmpty())
      return QCString();

  QFile file(filename);
  if (!file.open(IO_ReadOnly)) {
    qWarning("Couldn't open %s", filename.local8Bit().data());
    file.close();
    return QCString();
  }

  uint lineNumber=0;
  QCString line(1024);
  int len;

  while (( len = file.readLine(line.data(),line.size()-1) ) > 0) {
      int i=0;
      ++lineNumber;

      while (line[i] && line[i] <= ' ')
        i++;

      unsigned char ch=line[i];

      if ( !ch || ch =='#' || ch =='\n')
          continue; // We have an eof, a comment or an empty line

      if (ch < '0' && ch > '9') {
          qWarning("Syntax error: no number (line %u)",lineNumber);
          continue;
      }

      const int numStart=i;
      do {
          ch=line[++i];
      } while ( ch >= '0' && ch <= '9');

      const Q_ULONG number =line.mid(numStart,i).toULong();

      while (line[i] && line[i] <= ' ')
        i++;

      KDebugCache->insert(number, new KDebugEntry(number, line.mid(i, len-i-1)));
  }
  file.close();

  ent = KDebugCache->find( _num );
  if ( ent )
      return ent->descr;

  return QCString();
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

  QCString aAreaName;
  unsigned int oldarea;
  KConfig *config;
};

static kDebugPrivate *kDebug_data = 0;
static KStaticDeleter<kDebugPrivate> pcd;
static KStaticDeleter<KDebugDCOPIface> dcopsd;
static KDebugDCOPIface* kDebugDCOPIface = 0;

static void kDebugBackend( unsigned short nLevel, unsigned int nArea, const char *data)
{
  if ( !kDebug_data )
  {
      pcd.setObject(kDebug_data, new kDebugPrivate());
      // Do not call this deleter from ~KApplication
      KGlobal::unregisterStaticDeleter(&pcd);

      // create the dcop interface if it has not been created yet
      if (!kDebugDCOPIface)
      {
          kDebugDCOPIface = dcopsd.setObject(kDebugDCOPIface, new KDebugDCOPIface);
      }
  }

  if (!kDebug_data->config && KGlobal::_instance )
  {
      kDebug_data->config = new KConfig("kdebugrc", false, false);
      kDebug_data->config->setGroup("0");

      //AB: this is necessary here, otherwise all output with area 0 won't be
      //prefixed with anything, unless something with area != 0 is called before
      if ( KGlobal::_instance )
        kDebug_data->aAreaName = KGlobal::instance()->instanceName();
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
  else if ( nOutput == 4 && nLevel != KDEBUG_FATAL )
      return;

  const int BUFSIZE = 4096;
  char buf[BUFSIZE];
  if ( !kDebug_data->aAreaName.isEmpty() ) {
      strlcpy( buf, kDebug_data->aAreaName.data(), BUFSIZE );
      strlcat( buf, ": ", BUFSIZE );
      strlcat( buf, data, BUFSIZE );
  }
  else
      strlcpy( buf, data, BUFSIZE );


  // Output
  switch( nOutput )
  {
  case 0: // File
  {
      const char* aKey;
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
      QFile aOutputFile( kDebug_data->config->readPathEntry(aKey, "kdebug.dbg") );
      aOutputFile.open( IO_WriteOnly | IO_Append | IO_Raw );
      aOutputFile.writeBlock( buf, strlen( buf ) );
      aOutputFile.close();
      break;
  }
  case 1: // Message Box
  {
      // Since we are in kdecore here, we cannot use KMsgBox and use
      // QMessageBox instead
      if ( !kDebug_data->aAreaName.isEmpty() )
          aCaption += QString("(%1)").arg( kDebug_data->aAreaName );
      QMessageBox::warning( 0L, aCaption, data, i18n("&OK") );
      break;
  }
  case 2: // Shell
  {
      write( 2, buf, strlen( buf ) );  //fputs( buf, stderr );
      break;
  }
  case 3: // syslog
  {
      syslog( nPriority, "%s", buf);
      break;
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

kdbgstream::kdbgstream(kdbgstream &str)
 : output(str.output), area(str.area), level(str.level), print(str.print) 
{ 
    str.output.truncate(0); 
}

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
    vsnprintf( buf, sizeof(buf), format, arguments );
    va_end(arguments);
    *this << buf;
    return *this;
}

kdbgstream::~kdbgstream() {
    if (!output.isEmpty()) {
	fprintf(stderr, "ASSERT: debug output not ended with \\n\n");
        fprintf(stderr, "%s", kdBacktrace().latin1());
	*this << "\n";
    }
}

kdbgstream& kdbgstream::operator << (char ch)
{
  if (!print) return *this;
  if (!isprint(ch))
    output += "\\x" + QString::number( static_cast<uint>( ch ), 16 ).rightJustify(2, '0');
  else {
    output += ch;
    if (ch == '\n') flush();
  }
  return *this;
}

kdbgstream& kdbgstream::operator << (QChar ch)
{
  if (!print) return *this;
  if (!ch.isPrint())
    output += "\\x" + QString::number( ch.unicode(), 16 ).rightJustify(2, '0');
  else {
    output += ch;
    if (ch == '\n') flush();
  }
  return *this;
}

kdbgstream& kdbgstream::operator << (QWidget* widget)
{
    return *this << const_cast< const QWidget* >( widget );
}

kdbgstream& kdbgstream::operator << (const QWidget* widget)
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
/*
 * either use 'output' directly and do the flush if needed
 * or use the QString operator which calls the char* operator
 *
 */
kdbgstream& kdbgstream::operator<<( const QDateTime& time) {
    *this << time.toString();
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QDate& date) {
    *this << date.toString();

    return *this;
}
kdbgstream& kdbgstream::operator<<( const QTime& time ) {
    *this << time.toString();
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QPoint& p ) {
    *this << "(" << p.x() << ", " << p.y() << ")";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QSize& s ) {
    *this << "[" << s.width() << "x" << s.height() << "]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QRect& r ) {
    *this << "[" << r.x() << "," << r.y() << " - " << r.width() << "x" << r.height() << "]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QRegion& reg ) {
    *this<< "[ ";

    QMemArray<QRect>rs=reg.rects();
    for (uint i=0;i<rs.size();++i)
        *this << QString("[%1,%2 - %3x%4] ").arg(rs[i].x()).arg(rs[i].y()).arg(rs[i].width()).arg(rs[i].height() ) ;

    *this <<"]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const KURL& u ) {
    *this << u.prettyURL();
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QStringList& l ) {
    *this << "(";
    *this << l.join(",");
    *this << ")";

    return *this;
}
kdbgstream& kdbgstream::operator<<( const QColor& c ) {
    if ( c.isValid() )
        *this <<c.name();
    else
        *this << "(invalid/default)";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QPen& p ) {
    static const char* const s_penStyles[] = {
        "NoPen", "SolidLine", "DashLine", "DotLine", "DashDotLine",
        "DashDotDotLine" };
    static const char* const s_capStyles[] = {
        "FlatCap", "SquareCap", "RoundCap" };
    *this << "[ style:";
    *this << s_penStyles[ p.style() ];
    *this << " width:";
    *this << p.width();
    *this << " color:";
    if ( p.color().isValid() )
        *this << p.color().name();
    else
        *this <<"(invalid/default)";
    if ( p.width() > 0 ) // cap style doesn't matter, otherwise
    {
        *this << " capstyle:";
        *this << s_capStyles[ p.capStyle() >> 4 ];
        // join style omitted
    }
    *this <<" ]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const QBrush& b) {
    static const char* const s_brushStyles[] = {
        "NoBrush", "SolidPattern", "Dense1Pattern", "Dense2Pattern", "Dense3Pattern",
        "Dense4Pattern", "Dense5Pattern", "Dense6Pattern", "Dense7Pattern",
        "HorPattern", "VerPattern", "CrossPattern", "BDiagPattern", "FDiagPattern",
        "DiagCrossPattern" };

    *this <<"[ style: ";
    *this <<s_brushStyles[ b.style() ];
    *this <<" color: ";
    // can't use operator<<(str, b.color()) because that terminates a kdbgstream (flushes)
    if ( b.color().isValid() )
        *this <<b.color().name() ;
    else
        *this <<"(invalid/default)";
    if ( b.pixmap() )
        *this <<" has a pixmap";
    *this <<" ]";
    return *this;
}

kdbgstream& kdbgstream::operator<<( const QVariant& v) {
    *this << "[variant: ";
    *this << v.typeName();
    // For now we just attempt a conversion to string.
    // Feel free to switch(v.type()) and improve the output.
    *this << " toString=";
    *this << v.toString();
    *this << "]";
    return *this;
}

kdbgstream& kdbgstream::operator<<( const QByteArray& data) {
    if (!print) return *this;
    output += '[';
    unsigned int i = 0;
    unsigned int sz = QMIN( data.size(), 64 );
    for ( ; i < sz ; ++i ) {
        output += QString::number( (unsigned char) data[i], 16 ).rightJustify(2, '0');
        if ( i < sz )
            output += ' ';
    }
    if ( sz < data.size() )
        output += "...";
    output += ']';
    return *this;
}

QString kdBacktrace(int levels)
{
    QString s;
#ifdef HAVE_BACKTRACE
    void* trace[256];
    int n = backtrace(trace, 256);
    if (!n)
	return s;
    char** strings = backtrace_symbols (trace, n);

    if ( levels != -1 )
        n = QMIN( n, levels );
    s = "[\n";

    for (int i = 0; i < n; ++i)
        s += QString::number(i) +
             QString::fromLatin1(": ") +
             QString::fromLatin1(strings[i]) + QString::fromLatin1("\n");
    s += "]\n";
    if (strings)
        free (strings);
#endif
    return s;
}

QString kdBacktrace()
{
    return kdBacktrace(-1 /*all*/);
}

void kdClearDebugConfig()
{
    delete kDebug_data->config;
    kDebug_data->config = 0;
}


// Needed for --enable-final
#ifdef NDEBUG
#define kdDebug kndDebug
#endif
