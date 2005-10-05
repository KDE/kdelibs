/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)
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

#ifndef _KDEBUG_H_
#define _KDEBUG_H_

#include <qstring.h>
#include "kdelibs_export.h"

class QWidget;
class QDateTime;
class QDate;
class QTime;
class QPoint;
class QSize;
class QRect;
class QRegion;
class KURL;
class QStringList;
class QColor;
class QPen;
class QBrush;
class QVariant;
template <class T>
class QList;

class kdbgstream;
class kndbgstream;

/**
 * \addtogroup kdebug Debug message generators
 *  @{
 * KDE debug message streams let you and the user control just how many debug
 * messages you see. Debug message printing is controlled by (un)defining
 * NDEBUG when compiling your source. If NDEBUG is defined then no debug
 * messages are printed.
 */

typedef kdbgstream & (*KDBGFUNC)(kdbgstream &); // manipulator function
typedef kndbgstream & (*KNDBGFUNC)(kndbgstream &); // manipulator function

#ifdef __GNUC__
#define k_funcinfo "[" << __PRETTY_FUNCTION__ << "] "
#else
#define k_funcinfo "[" << __FILE__ << ":" << __LINE__ << "] "
#endif

#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "

/**
 * kdbgstream is a text stream that allows you to print debug messages.
 * Using the overloaded "<<" operator you can send messages. Usually
 * you do not create the kdbgstream yourself, but use kdDebug()
 * kdWarning(), kdError() or kdFatal() to obtain one.
 *
 * Example:
 * \code
 *    int i = 5;
 *    kdDebug() << "The value of i is " << i << endl;
 * \endcode
 */
class KDECORE_EXPORT kdbgstream {
 public:
    /**
     * @internal
     */
    kdbgstream(unsigned int _area, unsigned int _level, bool _print = true);
    /**
     * @internal
     */
    kdbgstream(const char * initialString, unsigned int _area, unsigned int _level, bool _print = true);
    /// Copy constructor
    kdbgstream(const kdbgstream &str);
    virtual ~kdbgstream();

    /**
     * Prints the given value.
     * @param i the boolean to print (as "true" or "false")
     * @return this stream
     */
    kdbgstream &operator<<(bool i)  {
        return *this << QString::fromLatin1(i ? "true" : "false" );
    }
    /**
     * Prints the given value.
     * @param i the short to print
     * @return this stream
     */
    kdbgstream &operator<<(short i)  {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned short to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned short i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param ch the char to print
     * @return this stream
     */
    kdbgstream& operator<<(char ch) {
        return *this << QLatin1Char(ch);
    }
    /**
     * Prints the given value.
     * @param ch the unsigned char to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned char ch) {
        return *this << QLatin1Char(ch);
    }
    /**
     * Prints the given value.
     * @param i the int to print
     * @return this stream
     */
    kdbgstream &operator<<(int i)  {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned int to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned int i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the long to print
     * @return this stream
     */
    kdbgstream &operator<<(long i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned long to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned long i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the long long to print
     * @return this stream
     */
    kdbgstream &operator<<(qlonglong i) {
        return *this << QString::number( i );
    }
    /**
     * Prints the given value.
     * @param i the unsigned long long to print
     * @return this stream
     */
    kdbgstream &operator<<(qulonglong i) {
        return *this << QString::number( i );
    }

    /**
     * Flushes the output.
     */
    virtual void flush();

    /**
     * Prints the given value.
     * @param ch the char to print
     * @return this stream
     * @since 3.3
     */
    kdbgstream &operator<<(QChar ch);
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const QString& string);
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const char *string) {
        return *this << QString::fromUtf8(string);
    }
    /**
     * Prints the given value.
     * @param p a pointer to print (in number form)
     * @return this stream
     */
    kdbgstream& operator<<(const void * p) {
        return form("%p", p);
    }
    /**
     * Invokes the given function.
     * @param f the function to invoke
     * @return the return value of @p f
     */
    kdbgstream& operator<<(KDBGFUNC f);
    /**
     * Prints the given value.
     * @param d the double to print
     * @return this stream
     */
    kdbgstream& operator<<(double d) {
        return *this << QString::number( d );
    }
    /**
     * Prints the string @p format which can contain
     * printf-style formatted values.
     * @param format the printf-style format
     * @return this stream
     */
    kdbgstream& form(const char *format, ...)
#ifdef __GNUC__
      __attribute__ ( ( format ( printf, 2, 3 ) ) )
#endif
     ;

    /** Operator to print out basic information about a QWidget.
     *  Output of class names only works if the class is moc'ified.
     * @param widget the widget to print
     * @return this stream
     */
    kdbgstream& operator << (const QWidget* widget);

    /**
     * Prints the given value.
     * @param dateTime the datetime to print
     * @return this stream
     */
    kdbgstream& operator << ( const QDateTime& dateTime );

    /**
     * Prints the given value.
     * @param date the date to print
     * @return this stream
     */
    kdbgstream& operator << ( const QDate& date );

    /**
     * Prints the given value.
     * @param time the time to print
     * @return this stream
     */
    kdbgstream& operator << ( const QTime& time );

    /**
     * Prints the given value.
     * @param point the point to print
     * @return this stream
     */
    kdbgstream& operator << ( const QPoint& point );

    /**
     * Prints the given value.
     * @param size the QSize to print
     * @return this stream
     */
    kdbgstream& operator << ( const QSize& size );

    /**
     * Prints the given value.
     * @param rect the QRect to print
     * @return this stream
     */
    kdbgstream& operator << ( const QRect& rect);

    /**
     * Prints the given value.
     * @param region the QRegion to print
     * @return this stream
     */
    kdbgstream& operator << ( const QRegion& region);

    /**
     * Prints the given value.
     * @param url the url to print
     * @return this stream
     */
    kdbgstream& operator << ( const KURL& url );

    /**
     * Prints the given value.
     * @param list the stringlist to print
     * @return this stream
     */
    kdbgstream& operator << ( const QStringList& list);

    /**
     * Prints the given value.
     * @param color the color to print
     * @return this stream
     */
    kdbgstream& operator << ( const QColor& color);

    /**
     * Prints the given value.
     * @param pen the pen to print
     * @return this stream
     * @since 3.2
     */
    kdbgstream& operator << ( const QPen& pen );

    /**
     * Prints the given value.
     * @param brush the brush to print
     * @return this stream
     */
    kdbgstream& operator << ( const QBrush& brush );

    /**
     * Prints the given value.
     * @param variant the variant to print
     * @return this stream
     * @since 3.3
     */
    kdbgstream& operator << ( const QVariant& variant );

    /**
     * Prints the given bytearray value, interpreting it as either
     * a raw byte array (if it contains non-printable characters)
     * or as a ascii string otherwise.
     * @param data the byte array to print
     * @return this stream
     * @since 3.3
     */
    kdbgstream& operator << ( const QByteArray& data );

    /**
     * Prints the given value
     * @param list the list to print
     * @return this stream
     * @since 3.3
     */
    template <class T>
    kdbgstream& operator << ( const QList<T> &list );

 private:
    class Private;
    mutable Private* d;
};

template <class T>
kdbgstream &kdbgstream::operator<<( const QList<T> &list )
{
    *this << "(";
    if ( !list.isEmpty() ) {
      typename QList<T>::ConstIterator it = list.begin();
      *this << *it;
      while (++it != list.end()) {
        *this << "," << *it;
      }
    }
    *this << ")";
    return *this;
}

/**
 * \relates KGlobal
 * Prints a newline to the stream.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline kdbgstream &endl( kdbgstream &s) { return s << "\n"; }

/**
 * \relates KGlobal
 * Flushes the stream.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline kdbgstream &flush( kdbgstream &s) { s.flush(); return s; }

/**
 * \relates KGlobal
 * Print a message describing the last system error.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 * @see perror(3)
 */
KDECORE_EXPORT kdbgstream &perror( kdbgstream &s);

/**
 * @internal
 * kndbgstream is a dummy variant of kdbgstream, it is only here to allow
 * compiling with/without debugging messages.
 * All functions do nothing.
 * @see kdbgstream
 */
class KDECORE_EXPORT kndbgstream {
 public:
    // Do not add dummy API docs to all methods, just ensure the whole class is skipped by doxygen

    kndbgstream() {}
    ~kndbgstream() {}
    kndbgstream& operator<<(KNDBGFUNC) { return *this; }
    template <typename T>
    kndbgstream& operator<<(const T &) { return *this; }
    void flush() {}
    kndbgstream &form(const char *, ...) { return *this; }
};

inline kndbgstream &endl( kndbgstream & s) { return s; }
inline kndbgstream &flush( kndbgstream & s) { return s; }
inline kndbgstream &perror( kndbgstream & s) { return s; }

/**
 * \relates KGlobal
 * Returns a debug stream. You can use it to print debug
 * information.
 * @param area an id to identify the output, 0 for default
 * @see kndDebug()
 */
KDECORE_EXPORT kdbgstream kdDebug(int area = 0);
/**
 * \relates KGlobal
 * Returns a debug stream. You can use it to conditionally
 * print debug information.
 * @param cond the condition to test, if true print debugging info
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdDebug(bool cond, int area = 0);

/**
 * \relates KGlobal
 * Returns a backtrace.
 * @param levels the number of levels of the backtrace
 * @return a backtrace
 * @since 3.1
 */
KDECORE_EXPORT QString kdBacktrace(int levels=-1);

inline kndbgstream kndDebug(int = 0) { return kndbgstream(); }
inline kndbgstream kndDebug(bool , int  = 0) { return kndbgstream(); }
inline QString kndBacktrace(int = -1) { return QString(); }

/**
 * \relates KGlobal
 * Returns a warning stream. You can use it to print warning
 * information.
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdWarning(int area = 0);
/**
 * \relates KGlobal
 * Returns a warning stream. You can use it to conditionally
 * print warning information.
 * @param cond the condition to test, if true print warning
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdWarning(bool cond, int area = 0);
/**
 * \relates KGlobal
 * Returns an error stream. You can use it to print error
 * information.
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdError(int area = 0);
/**
 * \relates KGlobal
 * Returns an error stream. You can use it to conditionally
 * print error information
 * @param cond the condition to test, if true print error
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdError(bool cond, int area = 0);
/**
 * \relates KGlobal
 * Returns a fatal error stream. You can use it to print fatal error
 * information.
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdFatal(int area = 0);
/**
 * \relates KGlobal
 * Returns a fatal error stream. You can use it to conditionally
 * print error information
 * @param cond the condition to test, if true print error
 * @param area an id to identify the output, 0 for default
 */
KDECORE_EXPORT kdbgstream kdFatal(bool cond, int area = 0);

/**
 * \relates KGlobal
 * Deletes the kdebugrc cache and therefore forces KDebug to reread the
 * config file
 */
KDECORE_EXPORT void kdClearDebugConfig();

/** @} */

#ifdef NDEBUG
#define kdDebug kndDebug
#define kdBacktrace kndBacktrace
#endif

#endif

