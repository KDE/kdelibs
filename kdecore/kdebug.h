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

#ifndef _KDEBUG_H_
#define _KDEBUG_H_ "$Id$"

#include <qstring.h>
#include <qwidget.h>

/**
  * kDebug provides and advanced mechanism for displaying debugging
  * information to the user.  kDebug works like printf does but takes one
  * or two extra arguments.  If your program is compiled with NDEBUG defined,
  * the calls are rendered useless and your debugging statements
  * hidden from the end-user's view. This doesn't apply to kDebugWarning,
  * kDebugError and kDebugFatal, since those should always been shown to the
  * user.
  *
  *
  * There are two families of functions. The first one allows variable
  * arguments, much like printf or the previous kdebug, and has the notion of
  * level (see below). The second one does not allow variable arguments
  * and only applies to debug info, but adds the filename and line number
  * before the message printed.
  * You can't have both at the same time, for technical reasons
  * (first faimly is functions, second one is macros, which can't have variable
  * arguments since we support non-gcc compilers)
  *
  *
  * A kDebug level determines how important the message being displayed is.
  * The first family of functions define four functions, one for each level :
  *     kDebugInfo
  *       for debug output
  *     kDebugWarning
  *       for when something strange or unexpected happened.
  *     kDebugError
  *       for when an error has occured, but the program can continue.
  *     kDebugFatal
  *       for when a horrific error has happened and the program must stop.
  *
  * The first (and optional) argument is a debug "area".  This "area" tells
  * kDebug where the call to kDebug came from.  The "area" is an unsigned number
  * specified in kdebug.areas ($KDEDIR/share/config/kdebug.areas).  If
  * this number is zero or unspecified, the instance (e.g. application) name
  * will be used.
  *
  * A utility function for printing out "errno" is provided, similar to the
  * POSIX perror() function. It uses the "Error" level: kDebugPError.
  *
  * A separate program with a small configuration dialog box
  * will soon be written, to let one assign actions to each debug level on an
  * area by area basis.
  */

void kDebugInfo( const char* fmt, ... );
void kDebugInfo( unsigned short area, const char* fmt, ... );
void kDebugInfo( bool cond, unsigned short area, const char* fmt, ... );
void kDebugWarning( const char* fmt, ... );
void kDebugWarning( unsigned short area, const char* fmt, ... );
void kDebugWarning( bool cond, unsigned short area, const char* fmt, ... );
void kDebugError( const char* fmt, ... );
void kDebugError( unsigned short area, const char* fmt, ... );
void kDebugError( bool cond, unsigned short area, const char* fmt, ... );
void kDebugFatal( const char* fmt, ... );
void kDebugFatal( unsigned short area, const char* fmt, ... );
void kDebugFatal( bool cond, unsigned short area, const char* fmt, ... );
void kDebugPError( const char* fmt, ... );
void kDebugPError( unsigned short area, const char* fmt, ... );

#ifdef NDEBUG
inline void kDebugInfo( const char* , ... ) {}
inline void kDebugInfo( unsigned short , const char* , ... ) {}
inline void kDebugInfo( bool , unsigned short , const char* , ... ) {}
// All the others remained defined, even with NDEBUG
#endif

/**
 * The second family of functions have more feature for debug output.
 * Those print file and line information, which kDebugInfo can't do.
 * And they also natively support QString.
 *
 * Applications using areas, or libraries :
 * use kDebugArea( area, my_char_* ) and kDebugStringArea( area, my_QString )
 *
 * Applications not using areas :
 * use kDebug( my_char_* ) and kDebugString( my_QString )
 *
 */
#ifdef NDEBUG
#define kDebugArea(area, a) ;
#else
#define kDebugArea(area, a) kDebugInfo( area, "[%s:%d] %s", __FILE__, __LINE__, a )
#endif

inline const char* debugString(const QString& a) { if (a.isNull()) return "<null>"; else return a.ascii(); }

class kdbgstream;
class kndbgstream;
typedef kdbgstream & (*KDBGFUNC)(kdbgstream &); // manipulator function
typedef kndbgstream & (*KNDBGFUNC)(kndbgstream &); // manipulator function

#ifdef __GNUC__
#define k_funcinfo "[" << __PRETTY_FUNCTION__ << "]
#else
#define k_funcinfo "[" << __FILE__ << ":" << __LINE__ << "] "
#endif

#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "

class kdbgstream {
 public:
    kdbgstream(unsigned int _area, unsigned int _level, bool _print = true) :
      area(_area), level(_level),  print(_print) { }
    kdbgstream(const char * initialString, unsigned int _area, unsigned int _level, bool _print = true) :
      output(QString::fromLatin1(initialString)), area(_area), level(_level),  print(_print) { }
    ~kdbgstream();
    kdbgstream &operator<<(bool i)  {
	if (!print) return *this;
	output += QString::fromLatin1(i ? "true" : "false");
	return *this;
    }
    kdbgstream &operator<<(short i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    kdbgstream &operator<<(unsigned short i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    kdbgstream &operator<<(char i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(int(i)); output += tmp;
	return *this;
    }
    kdbgstream &operator<<(unsigned char i) {
        if (!print) return *this;
        QString tmp; tmp.setNum((unsigned int)(i)); output += tmp;
        return *this;
    }

    kdbgstream &operator<<(int i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    kdbgstream &operator<<(unsigned int i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    kdbgstream &operator<<(long i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    kdbgstream &operator<<(unsigned long i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    void flush();
    kdbgstream &operator<<(const QString& string) {
	if (!print) return *this;
	output += string;
	if (output.at(output.length() -1 ) == '\n')
	    flush();
	return *this;
    }
    kdbgstream &operator<<(const char *string) {
	if (!print) return *this;
	output += QString::fromUtf8(string);
	if (output.at(output.length() - 1) == '\n')
	    flush();
	return *this;
    }
    kdbgstream &operator<<(const QCString& string) {
      *this << string.data();
      return *this;
    }
    kdbgstream& operator<<(void * p) {
	form("%p", p);
	return *this;
    }
    kdbgstream& operator<<(KDBGFUNC f) {
	if (!print) return *this;
	return (*f)(*this);
    }
    kdbgstream& operator<<(double d) {
      form("%f", d);
      return *this;
    }
    kdbgstream &form(const char *format, ...);
    /** Operator to print out basic information about a QWidget.
     *  Output of class names only works if the class is moc'ified
     */
    kdbgstream& operator << (QWidget* widget);
 private:
    QString output;
    unsigned int area, level;
    bool print;
};

inline kdbgstream &endl( kdbgstream &s) { s << "\n"; return s; }
inline kdbgstream &flush( kdbgstream &s) { s.flush(); return s; }
kdbgstream &perror( kdbgstream &s);

class kndbgstream {
 public:
    kndbgstream() {}
    ~kndbgstream() {}
    kndbgstream &operator<<(short int )  { return *this; }
    kndbgstream &operator<<(unsigned short int )  { return *this; }
    kndbgstream &operator<<(char )  { return *this; }
    kndbgstream &operator<<(unsigned char )  { return *this; }
    kndbgstream &operator<<(int )  { return *this; }
    kndbgstream &operator<<(unsigned int )  { return *this; }
    void flush() {}
    kndbgstream &operator<<(const QString& ) { return *this; }
    kndbgstream &operator<<(const QCString& ) { return *this; }
    kndbgstream &operator<<(const char *) { return *this; }
    kndbgstream& operator<<(void *) { return *this; }
    kndbgstream& operator<<(double) { return *this; }
    kndbgstream& operator<<(long) { return *this; }
    kndbgstream& operator<<(unsigned long) { return *this; }
    kndbgstream& operator<<(KNDBGFUNC) { return *this; }
    kndbgstream& operator << (QWidget*) { return *this; }
    kndbgstream &form(const char *, ...) { return *this; }
};

inline kndbgstream &endl( kndbgstream & s) { return s; }
inline kndbgstream &flush( kndbgstream & s) { return s; }
inline kndbgstream &perror( kndbgstream & s) { return s; }

#ifndef NDEBUG
kdbgstream kdDebug(int area = 0);
kdbgstream kdDebug(bool cond, int area = 0);
#else
inline kndbgstream kdDebug(int = 0) { return kndbgstream(); }
inline kndbgstream kdDebug(bool , int  = 0) { return kndbgstream(); }
#endif
kdbgstream kdWarning(int area = 0);
kdbgstream kdWarning(bool cond, int area = 0);
kdbgstream kdError(int area = 0);
kdbgstream kdError(bool cond, int area = 0);
kdbgstream kdFatal(int area = 0);
kdbgstream kdFatal(bool cond, int area = 0);

// -----

/*  Script to help porting from kdebug to kDebug* :
#!/bin/sh
perl -pi -e 's/kdebug\([ ]*KDEBUG_INFO,/kDebugInfo\(/' $*
perl -pi -e 's/kdebug\([ ]*0,/kDebugInfo\(/' $*
perl -pi -e 's/kdebug\([ ]*KDEBUG_WARN,/kDebugWarning\(/' $*
perl -pi -e 's/kdebug\([ ]*KDEBUG_ERROR,/kDebugError\(/' $*
perl -pi -e 's/kdebug\([ ]*KDEBUG_FATAL,/kDebugFatal\(/' $*

(Then cvs update -A, check compile and cvs commit)
*/

#endif
