/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)

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
#define _KDEBUG_H_

#include <qstring.h>
#include <qobject.h> // for KDebug
class QWidget;

class kdbgstream;
class kndbgstream;
typedef kdbgstream & (*KDBGFUNC)(kdbgstream &); // manipulator function
typedef kndbgstream & (*KNDBGFUNC)(kndbgstream &); // manipulator function

#ifdef __GNUC__
#define k_funcinfo "[" << __PRETTY_FUNCTION__ << "] "
#else
#define k_funcinfo "[" << __FILE__ << ":" << __LINE__ << "] "
#endif

#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "

class kdbgstreamprivate;
/**
 * kdbgstream is a text stream that allows you to print debug messages.
 * Using the overloaded "<<" operator you can send messages. Usually 
 * you do not create the kdbgstream yourself, but use @ref kdDebug()
 * @ref kdWarning(), @ref kdError() or @ref kdFatal to obtain one.
 *
 * Example:
 * <pre>
 *    int i = 5;
 *    kdDebug() << "The value of i is " << i << endl;
 * </pre>
 * @see kndbgstream
 */
class kdbgstream {
 public:
  /**
   * @internal
   */
    kdbgstream(unsigned int _area, unsigned int _level, bool _print = true) :
      area(_area), level(_level),  print(_print) { }
    kdbgstream(const char * initialString, unsigned int _area, unsigned int _level, bool _print = true) :
      output(QString::fromLatin1(initialString)), area(_area), level(_level),  print(_print) { }
    /// Copy constructor
    kdbgstream(kdbgstream &str) :
      output(str.output), area(str.area), level(str.level), print(str.print) { str.output.truncate(0); }
    kdbgstream(const kdbgstream &str) :
      output(str.output), area(str.area), level(str.level), print(str.print) {}
    ~kdbgstream();
    /**
     * Prints the given value.
     * @param i the boolean to print (as "true" or "false")
     * @return this stream
     */
    kdbgstream &operator<<(bool i)  {
	if (!print) return *this;
	output += QString::fromLatin1(i ? "true" : "false");
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the short to print
     * @return this stream
     */
    kdbgstream &operator<<(short i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned short to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned short i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the char to print
     * @return this stream
     */
    kdbgstream &operator<<(char i);
    /**
     * Prints the given value.
     * @param i the unsigned char to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned char i) {
        return operator<<( static_cast<char>( i ) );
    }
    /**
     * Prints the given value.
     * @param i the int to print
     * @return this stream
     */
    kdbgstream &operator<<(int i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned int to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned int i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the long to print
     * @return this stream
     */
    kdbgstream &operator<<(long i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Prints the given value.
     * @param i the unsigned long to print
     * @return this stream
     */
    kdbgstream &operator<<(unsigned long i) {
        if (!print) return *this;
        QString tmp; tmp.setNum(i); output += tmp;
        return *this;
    }
    /**
     * Flushes the output.
     */
    void flush(); //AB: maybe this should be virtual! would save some trouble for some 3rd party projects

    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const QString& string) {
	if (!print) return *this;
	output += string;
	if (output.at(output.length() -1 ) == '\n')
	    flush();
	return *this;
    }
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const char *string) {
	if (!print) return *this;
	output += QString::fromUtf8(string);
	if (output.at(output.length() - 1) == '\n')
	    flush();
	return *this;
    }
    /**
     * Prints the given value.
     * @param string the string to print
     * @return this stream
     */
    kdbgstream &operator<<(const QCString& string) {
        *this << string.data();
        return *this;
    }
    /**
     * Prints the given value.
     * @param p a pointer to print (in number form)
     * @return this stream
     */
    kdbgstream& operator<<(const void * p) {
        form("%p", p);
        return *this;
    }
    /**
     * Invokes the given function.
     * @param f the function to invoke
     * @return the return value of @p f
     */
    kdbgstream& operator<<(KDBGFUNC f) {
	if (!print) return *this;
	return (*f)(*this);
    }
    /**
     * Prints the given value.
     * @param d the double to print
     * @return this stream
     */
    kdbgstream& operator<<(double d) {
      QString tmp; tmp.setNum(d); output += tmp;
      return *this;
    }
    /**
     * Prints the string @p format which can contain
     * printf-style formatted values.
     * @param format the printf-style format
     * @return this stream
     */
    kdbgstream &form(const char *format, ...);
    /** Operator to print out basic information about a QWidget.
     *  Output of class names only works if the class is moc'ified.
     * @param widget the widget to print
     * @return this stream
     */
    kdbgstream& operator << (QWidget* widget);
 private:
    QString output;
    unsigned int area, level;
    bool print;
    kdbgstreamprivate* d;
};

/**
 * Prints an "\n". 
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline kdbgstream &endl( kdbgstream &s) { s << "\n"; return s; }
/**
 * Flushes the stream.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 */
inline kdbgstream &flush( kdbgstream &s) { s.flush(); return s; }
kdbgstream &perror( kdbgstream &s);

/**
 * kndbgstream is a dummy variant of @ref kdbgstream. All functions do
 * nothing.
 * @see kndDebug()
 */
class kndbgstream {
 public:
  /// EMpty constructor.
    kndbgstream() {}
    ~kndbgstream() {}
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(short int )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(unsigned short int )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(char )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(unsigned char )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(int )  { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(unsigned int )  { return *this; }
    /**
     * Does nothing.
     */
    void flush() {}
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(const QString& ) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(const QCString& ) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &operator<<(const char *) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator<<(const void *) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator<<(void *) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator<<(double) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator<<(long) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator<<(unsigned long) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator<<(KNDBGFUNC) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream& operator << (QWidget*) { return *this; }
    /**
     * Does nothing.
     * @return this stream
     */
    kndbgstream &form(const char *, ...) { return *this; }
};

/**
 * Does nothing.
 * @param a stream
 * @return the given @p s
 */
inline kndbgstream &endl( kndbgstream & s) { return s; }
/**
 * Does nothing.
 * @param a stream
 * @return the given @p s
 */
inline kndbgstream &flush( kndbgstream & s) { return s; }
inline kndbgstream &perror( kndbgstream & s) { return s; }

/**
 * Returns a debug stream. You can use it to print debug
 * information.
 * @param area an id to identify the output, 0 for default
 * @see kndDebug()
 */
kdbgstream kdDebug(int area = 0);
kdbgstream kdDebug(bool cond, int area = 0);
/**
 * Returns a backtrace.
 * @return a backtrace
 */
QString kdBacktrace();
/**
 * Returns a backtrace.
 * @param levels the number of levels of the backtrace
 * @return a backtrace
 * @since 3.1
 */
QString kdBacktrace(int levels);
/**
 * Returns a dummy debug stream. The stream does not print anything.
 * @param area an id to identify the output, 0 for default
 * @see kdDebug()
 */
inline kndbgstream kndDebug(int = 0) { return kndbgstream(); }
inline kndbgstream kndDebug(bool , int  = 0) { return kndbgstream(); }
inline QString kndBacktrace() { return QString::null; }

/**
 * Returns a warning stream. You can use it to print warning
 * information.
 * @param area an id to identify the output, 0 for default
 */
kdbgstream kdWarning(int area = 0);
kdbgstream kdWarning(bool cond, int area = 0);
/**
 * Returns an error stream. You can use it to print error
 * information.
 * @param area an id to identify the output, 0 for default
 */
kdbgstream kdError(int area = 0);
kdbgstream kdError(bool cond, int area = 0);
/**
 * Returns a fatal error stream. You can use it to print fatal error
 * information.
 * @param area an id to identify the output, 0 for default
 */
kdbgstream kdFatal(int area = 0);
kdbgstream kdFatal(bool cond, int area = 0);

/**
 * Deletes the kdebugrc cache and therefore forces KDebug to reread the
 * config file
 */
void kdClearDebugConfig();


class KDebugDCOPIface;
/**
 * This class is to allow debug output to be emitted as signals. See @ref
 * notify.
 *
 * Connect @ref self to your slot if you want to use it:
 * <pre>
 * connect(KDebug::self(), SIGNAL(notify(const QString&, const char*, int)),
 *         this, SLOT(mySlot(const QString&, const char* , int)));
 * </pre>
 *
 * Note that the signal is emitted if you set the area to Output=5 in the
 * kdebugrc only. You might want to use kdebugdialog for this.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Class that emits debug output as signal
 * @since 3.2
 **/
class KDebug : public QObject {
    Q_OBJECT
 public:
    KDebug();
    ~KDebug();

    enum DebugLevels {
      KDEBUG_INFO = 0,
      KDEBUG_WARN = 1,
      KDEBUG_ERROR = 2,
      KDEBUG_FATAL = 3
    };

    /**
     * Create and return the KDebug object.
     * @return The KDebug instance of your program. The instance will be
     * created if not present.
     **/
    static KDebug* self();

    /**
     * Called internally. Emits @ref notify.
     **/
    void emitSignal(const QString& area, const char* data, int level);

 signals:
    /**
     * Emits a signal for the debug output generated by @ref kdDebug, @ref
     * kdWarning and @ref kdError. You could use this e.g. to display warnings
     * or errors in a separate widget of your application.
     * @param area The string that belongs to the specified debug area. The
     * string is specified in the kdebug.areas file. Usually this is simply
     * the instance name of the application, e.g. "boson" for the boson game 
     * if you use kdDebug() << endl; or kdDebug(0) << endl;
     * @param data The text of the debug output. Including the prefix, such
     * as "ERROR: " or "WARNING: ".
     * @param level What kind of debug output this is - see @ref
     * DebugLevels. The value depends on the function you called, e.g.
     * kdWarning() will always generate @ref KDEBUG_WARN.
     **/
    void notify(const QString& area, const char* data, int level);

 protected:
    virtual void connectNotify(const char* signal);

 private:
    static KDebug* mDebug;
    class KDebugPrivate;
    KDebugPrivate* d;
    KDebugDCOPIface* mIface;
    bool mEmitSignal;
};

#ifdef NDEBUG
#define kdDebug kndDebug
#define kdBacktrace kndBacktrace
#endif

#endif

