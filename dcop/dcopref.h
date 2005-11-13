/*
Copyright (c) 2002 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _DCOPREF_H
#define _DCOPREF_H

#include <kdatastream.h> // needed for proper bool marshalling
#include <dcoptypes.h>
#include "kdelibs_export.h"

class QDataStream;
class DCOPObject;
class DCOPClient;

/**
 * Represents the return value of a DCOPRef:call() or
 * DCOPRef:send() invocation.
 *
 * @see DCOPRef
 * @see DCOPRef::call()
 * @see DCOPArg
 * @since 3.1
 */
class DCOP_EXPORT DCOPReply
{
public:
  /**
   * Casts the value to the type @p T. Requires that the
   * type @p T supports QDataStream deserialisation
   * and has a function dcopTypeName(T). This is true for most
   * basic types.
   */
    template<class T>
    operator T() {
	T t;
	dcopTypeInit(t);
	if ( typeCheck( dcopTypeName(t), true ) ) {
	    QDataStream reply( data );
	    reply.setVersion(QDataStream::Qt_3_1);
	    reply >> t;
	}
	return t;
    }
  /**
   * Retrieves the value from the type @p T. Requires that the
   * type @p T supports QDataStream deserialisation.
   * @param t the type will be written here, if successful
   * @param tname the signature type name
   * @return true if successful, false otherwise
   */
    template <class T> bool get(  T& t, const char* tname ) {
	if ( typeCheck( tname, false ) ) {
	    QDataStream reply( data );
	    reply.setVersion(QDataStream::Qt_3_1);
	    reply >> t;
	    return true;
	}
	return false;
    }
  /**
   * Retrieves the value from the type @p T. Requires that the
   * type @p T supports QDataStream deserialisation
   * and has a function dcopTypeName(T). This is true for most
   * basic types.
   * @param t the type will be written here, if successful
   * @return true if successful, false otherwise
   */
    template <class T> bool get(  T& t ) {
	if ( typeCheck( dcopTypeName(t), false ) ) {
	    QDataStream reply( data );
	    reply.setVersion(QDataStream::Qt_3_1);
	    reply >> t;
	    return true;
	}
	return false;
    }

    /**
     * Checks whether the type is valid.
     * @return true if valid, false otherwise
     */
    inline bool isValid() const { return !type.isNull(); }

    /// The serialized data.
    QByteArray data;
    /// The name of the type, or 0 if unknown.
    DCOPCString type;
private:
    bool typeCheck( const char* t );
    bool typeCheck( const char* t, bool warn );
};

/**
 * A generic DCOP argument.
 * This class allows you to use user-defined argument types for
 * DCOPRef::call() or DCOPRef::send().
 *
 * @see DCOPRef::call()
 * @see DCOPRef
 * @see DCOPReply
 * @since 3.1
 */
class DCOP_EXPORT DCOPArg  {
public:
    /**
     * Creates a DCOPArg for DCOPRef::call().
     * @param t the data that will be written to a QDataStream. It must
     *          overload writing to a QDataStream using the "<<"
     *          operator
     * @param tname_arg the name of the data that will appear in the
     *        function's signature
     */
    template <class T> DCOPArg( const T& t, const char* tname_arg )
	: tname(tname_arg)
	{
	    QDataStream ds( &data, QIODevice::WriteOnly );
	    ds.setVersion(QDataStream::Qt_3_1);
	    ds << t;
	}
    /**
     * Creates a DCOPArg for DCOPRef::call().
     * @param t the data that will be written to a QDataStream. It must
     *          overload writing to a QDataStream using the "<<"
     *          operator. The name of the type will be determined by
     *          calling the function dcopTypeName(T) that must be provided
     *          by you.
     */
    template <class T> DCOPArg( const T& t )
	: tname( dcopTypeName(t) )
	{
	    QDataStream ds( &data, QIODevice::WriteOnly );
	    ds.setVersion(QDataStream::Qt_3_1);
	    ds << t;
	}

    /// The serialized data.
    QByteArray data;
    /// The signature type name of the data.
    const char* tname;
};

inline const char* dcopTypeName( const DCOPArg &arg )  { return arg.tname; }
inline QDataStream & operator << (QDataStream & str, const DCOPArg& arg )
   { str.writeRawData( arg.data.data(), arg.data.size() ); return str; }



/**
 * A DCOPRef(erence) encapsulates a remote DCOP object as a triple
 * <app,obj,type> where type is optional. It allows for calling and
 * passing DCOP objects.
 *
 * A DCOP reference makes it possible to return references to other DCOP
 * objects in a DCOP interface, for example in the method
 * giveMeAnotherObject() in an interface like this:
 *
 * \code
 *	class Example : public DCOPObject
 *	{
 *	   K_DCOP
 *	...
 *	k_dcop:
 *	   DCOPRef giveMeAnotherObject();
 *	   int doSomething( QString, float, bool );
 *	   ASYNC pingMe( DCOPCString message );
 *	   UserType userFunction( UserType );
 *	};
 * \endcode
 *
 * In addition, the reference can operate as a comfortable generic
 * stub to call remote DCOP objects in cases where no DCOPStub is
 * available. The advantage of using DCOPRef instead of the low-level
 * functions DCOPClient::call() and DCOPClient::send() are the nicer
 * syntax and the implicit runtime error checking.
 *
 * Say you want to call the method "doSomething" from the above
 * interface on an object called "example" that lives in application
 * "foo". Using DCOPRef, you would write
 *
 * \code
 *	DCOPRef example( "foo", "example" );
 *	int result = example.call( "doSomething", QString("Hello World"), (float)2.5, true );
 * \endcode
 *
 * If it is important for you to know whether the call succeeded or
 * not, you can use the slightly more elaborate pattern:
 *
 * \code
 *	DCOPRef example( "foo", "example" );
 *	DCOPReply reply = example.call( "doSomething", QString("Hello World"), (float)2.5, true );
 *	if ( reply.isValid() ) {
 *	    int result = reply;
 *	    // ...
 *	}
 * \endcode
 *
 * Note that you must pass a QString for the first argument. If you use a
 * regular char pointer, it will be converted to a DCOPCString.
 *
 * For curiosity, here is how you would achieve the exactly same
 * functionality by using DCOPClient::call() directly:
 *
 * \code
 *    QByteArray data, replyData;
 *    DCOPCString replyType;
 *    QDataStream arg( data, QIODevice::WriteOnly );
 *    arg << QString("hello world" ), (float) 2.5 << true;
 *    if ( DCOPClient::mainClient()->call( app, obj,
 *			     "doSomething(QString,float,bool)",
 *			     data, replyType, replyData ) ) {
 *	if ( replyType == "int" ) {
 *	    int result;
 *	    QDataStream reply( replyData );
 *	    reply >> result;
 *	    // ...
 *	}
 *    }
 * \endcode
 *
 * As you might see from the code snippet, the DCOPRef has to "guess"
 * the names of the datatypes of the arguments to construct a dcop
 * call. This is done through global inline overloads of the
 * dcopTypeName function, for example
 *
 * \code
 *	inline const char* dcopTypeName( const QString& ) { return "QString"; }
 * \endcode
 *
 * If you use custom data types that do support QDataStream but have
 * no corresponding dcopTypeName overload, you can either provide such
 * an overload or use a DCOPArg wrapper that allows you to specify the type.
 *
 * \code
 *	UserType userType;
 *	DCOPReply reply = example.call( "userFunction", DCOPArg( userType, "UserType" ) );
 * \endcode
 *
 * Similar, when you retrieve such a data type, you can use an
 * explicit call to DCOPReply::get():
 *
 * \code
 *	UserType userType;
 *	reply.get( userType, "UserType" );
 * \endcode
 *
 * The function send() works very similar to call(), only that it
 * returns a simple bool on whether the signal could be sent or not:
 *
 * \code
 *	if ( example.send( "pingMe", "message" ) == false )
 *	   qWarning("could not ping example" );
 * \endcode
 *
 * A DCOP reference operates on DCOPClient::mainClient(), unless you
 * explicitly specify another client with setDCOPClient().
 *
 * @see DCOPArg
 * @see DCOPReply
 * @see DCOPObject
 * @author Matthias Ettrich <ettrich@kde.org>, Torben Weis <weis@kde.org>
 */

class DCOP_EXPORT DCOPRef
{
public:
    /**
     * Creates a null reference.
     * @see isNull()
     */
    DCOPRef();

    /**
     * Copy constructor.
     */
    DCOPRef( const DCOPRef& ref );
    /**
     *  Creates a reference for application @p app and object @p obj
     *
     * @param app The name of an application as registered
     *            by the dcopserver.
     * @param obj The name of the dcop object.
     */
    DCOPRef( const DCOPCString& app, const DCOPCString& obj = "" );

    /**
     * Creates a reference to an existing dcop object
     *
     * @param object The dcop object to create the ref to.
     */
    DCOPRef( DCOPObject *object );

    /**
     *  Creates a reference for application @p app and object @p obj
     * with a specified type @p type.
     *
     * @param app The name of an application as registered
     *            by the dcopserver.
     * @param obj The name of the dcop object
     * @param type The object's type
     */
    DCOPRef( const DCOPCString& app, const DCOPCString& obj, const DCOPCString& type );

    /**
     * Tests whether this is a null reference.
     * @return true if this object is a null reference
     * @see clear()
     */
    bool isNull() const;

    /**
     * Name of the application in which the object resides.
     * @return the application's id. Can be null or empty if not set.
     */
    DCOPCString app() const;

    /**
     * Object ID of the referenced object.
     * @return the id of the referenced object. Can be null or empty if not set.
     * @since 3.1
     */
    DCOPCString obj() const;

    /**
     * @obsolete
     */
    DCOPCString object() const;

    /**
     * Type of the referenced object. May be null (i.e. unknown).
     * @return the type of the referenced object, or null if unknown
     */
    DCOPCString type() const;


    /**
     * Assignment operator. Copies the references data.
     */
    DCOPRef& operator=( const DCOPRef& );

    /**
     * Changes the referenced object. Resets the type to unknown (null).
     * The type is not needed for call() and send().
     * @param app the application id.
     * @param obj the object id
     */
    void setRef( const DCOPCString& app, const DCOPCString& obj = "" );

    /**
     * Changes the referenced object.
     * @param app the application id.
     * @param obj the object id
     * @param type the object's type
     */
    void setRef( const DCOPCString& app, const DCOPCString& obj, const DCOPCString& type );


    /**
     * Makes this a null reference.
     * @see isNull()
     */
    void clear();


    /**
     * Returns the dcop client the reference operates on. If no client
     * has been set, this is the DCOPClient::mainClient().
     * @return the DCOPClient of this object
     * @since 3.1
     */
    DCOPClient* dcopClient() const;

    /**
     * Sets a specific dcop client for this reference. Otherwise
     * DCOPClient::mainClient() is used.
     * @param client the new DCOPClient of this object
     * @since 3.1
     */
    void setDCOPClient( DCOPClient *client );

    /**
     * Flag for allowing entering the event loop if the call blocks too long.
     * @p NoEventLoop disables entering the event loop.
     * @p UseEventLoop allows entering the event loop while waiting for long
     * blocking DCOP call, thus making the GUI repaint if needed, and possibly
     * allowing also other code in the application to be executed.
     * @see DCOPClient::call()
     */
    enum EventLoopFlag { NoEventLoop, UseEventLoop };
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    DCOPReply call( const DCOPCString& fun ) {
	QByteArray data;
	return callInternal( fun, "()", data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @since 3.2
     */
    DCOPReply callExt( const DCOPCString& fun, EventLoopFlag useEventLoop=NoEventLoop,
		    int timeout=-1 ) {
	QByteArray data;
	return callInternal( fun, "()", data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1>
    DCOPReply call( const DCOPCString& fun, const T1& t1 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s)",
		     dcopTypeName(t1) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @since 3.2
     */
    template <class T1>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1) {
	DCOPCString args;
	args = QString().sprintf( "(%s)",
		     dcopTypeName(t1) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1, class T2>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @since 3.2
     */
    template <class T1, class T2>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1, class T2, class T3>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @since 3.2
     */
    template <class T1, class T2, class T3>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @since 3.2
     */
    template <class T1,class T2,class T3,class T4>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @since 3.2
     */
    template <class T1,class T2,class T3,class T4,class T5>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @since 3.2
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t7 the seventh argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t7 the seventh argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @since 3.2
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t7 the seventh argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t8 the eigth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see send()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8>
    DCOPReply call( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7,
		    const T8& t8 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7),
		     dcopTypeName(t8) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
	return callInternal( fun, args, data );
    }

    /**
     * Like call(), with additional arguments allowing entering the event loop
     * and specifying timeout.
     * @param useEventLoop if UseEventLoop, the event loop will be started when
     *            the call blocks too long
     * @param timeout timeout for the call in miliseconds, or -1 for no timeout
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t7 the seventh argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t8 the eigth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @since 3.2
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8>
    DCOPReply callExt( const DCOPCString& fun,
		    EventLoopFlag useEventLoop, int timeout,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7,
		    const T8& t8) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7),
		     dcopTypeName(t8) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
	return callInternal( fun, args, data, useEventLoop, timeout );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @since 3.1
     */
    bool send( const DCOPCString& fun ) {
	QByteArray data;
	return sendInternal( fun, "()", data );
    }

    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1>
    bool send( const DCOPCString& fun, const T1& t1 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s)",
		     dcopTypeName(t1) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1, class T2>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1, class T2, class T3>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t7 the seventh argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7;
	return sendInternal( fun, args, data );
    }
    /**
     * Calls the function @p fun on the object referenced by this reference.
     * Unlike call() this method does not expect a return value.
     * @param fun the name of the DCOP function. This can be either the
     *            full function signature (e.g. "setName(QString)") or
     *            only the function's name (e.g. "setName"). In the
     *            latter case the exact signature will be guessed from
     *            the arguments
     * @param t1 the first argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t2 the second argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t3 the third argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t4 the fourth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t5 the fifth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t6 the sixth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t7 the seventh argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @param t8 the eigth argument of the function. This can be a
     *           supported base type or a DCOPArg object.
     * @return the DCOPReply object. Is invalid ( DCOPReply::isValid())
     *         when an error occurred.
     * @see call()
     * @see DCOPArg
     * @since 3.1
     */
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8>
    bool send( const DCOPCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7,
		    const T8& t8 ) {
	DCOPCString args;
	args = QString().sprintf( "(%s,%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7),
		     dcopTypeName(t8) ).toAscii();;
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds.setVersion(QDataStream::Qt_3_1);
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
	return sendInternal( fun, args, data );
    }



private:
    DCOPReply callInternal( const DCOPCString& fun, const DCOPCString& args, const QByteArray& data,
			    EventLoopFlag useEventLoop, int timeout );
    DCOPReply callInternal( const DCOPCString& fun, const DCOPCString& args, const QByteArray& data );
    bool sendInternal( const DCOPCString& fun, const DCOPCString& args, const QByteArray& data );

    DCOPCString m_app;
    DCOPCString m_obj;
    DCOPCString m_type;

    class DCOPRefPrivate;
    DCOPRefPrivate *d;
};

/**
 * Writes the reference (NOT the object itself) to the stream.
 */
DCOP_EXPORT QDataStream& operator<<( QDataStream&, const DCOPRef& ref );
/**
 * Reads a reference from the stream.
 */
DCOP_EXPORT QDataStream& operator>>( QDataStream&, DCOPRef& ref );

#endif
