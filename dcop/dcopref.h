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

#include <qcstring.h>
#include <dcoptypes.h>
#include <kdatastream.h> // needed for proper bool marshalling

class QDataStream;
class DCOPObject;
class DCOPClient;


class DCOPReply
{
public:
    template<class T>
    operator T() {
	T t;
	if ( typeCheck( dcopTypeName(t) ) ) {
	    QDataStream reply( data, IO_ReadOnly );
	    reply >> t;
	} else {
	    dcopTypeInit(t);
	}
	return t;
    }
    template <class T> bool get(  T& t, const char* tname ) {
	if ( typeCheck( tname ) ) {
	    QDataStream reply( data, IO_ReadOnly );
	    reply >> t;
	    return true;
	}
	return false;
    }
    template <class T> bool get(  T& t ) {
	if ( typeCheck( dcopTypeName(t) ) ) {
	    QDataStream reply( data, IO_ReadOnly );
	    reply >> t;
	    return true;
	}
	return false;
    }

    inline bool isValid() const { return !type.isNull(); }
    QByteArray data;
    QCString type;
private:
    bool typeCheck( const char* t );
};

class DCOPArg  {
public:
    template <class T> DCOPArg( const T& t, const char* tname_arg )
	: tname(tname_arg)
	{
	    QDataStream ds( data, IO_WriteOnly );
	    ds << t;
	}
    template <class T> DCOPArg( const T& t )
	: tname( dcopTypeName(t) )
	{
	    QDataStream ds( data, IO_WriteOnly );
	    ds << t;
	}
    QByteArray data;
    const char* tname;
};
inline const char* dcopTypeName( const DCOPArg &arg )  { return arg.tname; }
inline QDataStream & operator << (QDataStream & str, const DCOPArg& arg )
   { str.writeRawBytes( arg.data.data(), arg.data.size() ); return str; }



/**
 * A DCOPRef(erence) encapsulates a remote DCOP object as a triple
 * <app,obj,type> where type is optional. It allows for calling and
 * passing DCOP objects.
 *
 * A DCOP reference makes it possible to return references to other DCOP
 * objects in a DCOP interface, for example in the method
 * giveMeAnotherObject() in an interface like this:
 *
 * <pre>
 *	class Example : public DCOPObject
 *	{
 *	   K_DCOP
 *	...
 *	k_dcop:
 *	   DCOPRef giveMeAnotherObject();
 *	   int doSomething( QString, float, bool );
 *	   ASYNC pingMe( QCString message );
 *	   UserType userFunction( UserType );
 *	};
 * </pre>
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
 * <pre>
 *	DCOPRef example( "foo", "example" );
 *	int result = example.call( "doSomething", "Hello World", (float)2.5, true );
 * </pre>
 *
 * If it is important for you to know whether the call succeeded or
 * not, you can use the slightly more elaborate pattern:
 *
 * <pre>
 *	DCOPRef example( "foo", "example" );
 *	DCOPReply reply = example.call( "doSomething", "Hello World", (float)2.5, true );
 *	if ( reply.isValid() ) {
 *	    int result = reply;
 *	    // ...
 *	}
 * </pre>
 *
 * For curiosity, here is how you would achieve the exactly same
 * functionality by using DCOPClient::call() directly:
 *
 * <pre>
 *    QByteArray data, replyData;
 *    QCString replyType;
 *    QDataStream arg( data, IO_WriteOnly );
 *    arg << QString("hello world" ), (float) 2.5 << true;
 *    if ( DCOPClient::mainClient()->call( app, obj,
 *			     "doSomething(QString,float,bool)",
 *			     data, replyType, replyData ) ) {
 *	if ( replyType == "int" ) {
 *	    int result;
 *	    QDataStream reply( replyData, IO_ReadOnly );
 *	    reply >> result;
 *	    // ...
 *	}
 *    }
 * </pre>
 *
 * As you might see from the code snippet, the DCOPRef has to "guess"
 * the names of the datatypes of the arguments to construct a dcop
 * call. This is done through global inline overloads of the
 * dcopTypeName function, for example
 *
 * <pre>
 *	inline const char* dcopTypeName( const QString& ) { return "QString"; }
 * </pre>
 *
 * If you use custom data types that do support QDataStream but have
 * no corrsponding dcopTypeName overload, you can either provide such
 * an overload or use a DCOPArg wrapper that allows you to specify the type.
 *
 * <pre>
 *	UserType userType;
 *	DCOPReply reply = example.call( "userFunction", DCOPArg( userType, "UserType" ) );
 * </pre>
 *
 * Similar, when you retrieve such a data type, you can use an
 * explicit call to DCOPReply::get():
 *
 * <pre>
 *	UserType userType;
 *	reply.get( userType, "UserType" );
 * </pre>
 *
 * The function send() works very similar to call(), only that it
 * returns a simple bool on whether the signal could be sent or not:
 *
 * <pre>
 *	if ( example.pingMe( "message" ) == false )
 *	   qWarning("could not ping example" );
 * </pre>
 *
 * A DCOP reference operates on DCOPClient::mainClient(), unless you
 * explicitely specify another client with setDCOPClient().
 *
 * @author Matthias Ettrich <ettrich@kde.org>, Torben Weis <weis@kde.org>
 */

class DCOPRef
{
public:
    /**
     * Creates a null reference.
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
     * @param obj The name of the dcop object
     */
    DCOPRef( const QCString& app, const QCString& obj = "" );

    /**
     * Creates a reference to an exising dcop object
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
    DCOPRef( const QCString& app, const QCString& obj, const QCString& type );

    /**
     * Tests wether it is a null reference.
     */
    bool isNull() const;

    /**
     * Name of the application in which the object resides.
     */
    QCString app() const;

    /**
     * Object ID of the referenced object.
     */
    QCString obj() const;

    /**
     * @obsolete
     */
    QCString object() const;

    /**
     * type of the referenced object. May be null (i.e. unknown).
     */
    QCString type() const;


    DCOPRef& operator=( const DCOPRef& );

    /**
     * Changes the referenced object. Resets the type to unknown (null).
     */
    void setRef( const QCString& app, const QCString& obj = "" );

    /**
     * Changes the referenced object
     */
    void setRef( const QCString& app, const QCString& obj, const QCString& type );


    /**
     * Makes this a null reference.
     */
    void clear();


    /**
     * Returns the dcop client the reference operates on. If no client
     * has been set, this is the DCOPClient::mainClient().
     */
    DCOPClient* dcopClient() const;

    /**
     * Sets a specific dcop client for this reference. Otherwise
     * DCOPClient::mainClient() is used.
     */
    void setDCOPClient( DCOPClient* );

    DCOPReply call( const QCString& fun ) {
	QByteArray data;
	return callInternal( fun, "()", data );
    }
    template <class T1>
    DCOPReply call( const QCString& fun, const T1& t1 ) {
	QCString args;
	args.sprintf( "(%s)",
		     dcopTypeName(t1) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1;
	return callInternal( fun, args, data );
    }
    template <class T1, class T2>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2 ) {
	QCString args;
	args.sprintf( "(%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2;
	return callInternal( fun, args, data );
    }
    template <class T1, class T2, class T3>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3;
	return callInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4;
	return callInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5;
	return callInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5,class T6>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5 << t6;
	return callInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7;
	return callInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8>
    DCOPReply call( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7,
		    const T8& t8 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7),
		     dcopTypeName(t8) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
	return callInternal( fun, args, data );
    }


    bool send( const QCString& fun ) {
	QByteArray data;
	return sendInternal( fun, "()", data );
    }
    template <class T1>
    bool send( const QCString& fun, const T1& t1 ) {
	QCString args;
	args.sprintf( "(%s)",
		     dcopTypeName(t1) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1;
	return sendInternal( fun, args, data );
    }
    template <class T1, class T2>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2 ) {
	QCString args;
	args.sprintf( "(%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2;
	return sendInternal( fun, args, data );
    }
    template <class T1, class T2, class T3>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3;
	return sendInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4;
	return sendInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5;
	return sendInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5,class T6>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5 << t6;
	return sendInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7;
	return sendInternal( fun, args, data );
    }
    template <class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8>
    bool send( const QCString& fun,
		    const T1& t1,
		    const T2& t2,
		    const T3& t3,
		    const T4& t4,
		    const T5& t5,
		    const T6& t6,
		    const T7& t7,
		    const T8& t8 ) {
	QCString args;
	args.sprintf( "(%s,%s,%s,%s,%s,%s,%s,%s)",
		     dcopTypeName(t1),
		     dcopTypeName(t2),
		     dcopTypeName(t3),
		     dcopTypeName(t4),
		     dcopTypeName(t5),
		     dcopTypeName(t6),
		     dcopTypeName(t7),
		     dcopTypeName(t8) );
	QByteArray data;
	QDataStream ds( data, IO_WriteOnly );
	ds << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
	return sendInternal( fun, args, data );
    }



private:
    DCOPReply callInternal( const QCString& fun, const QCString& args, const QByteArray& data );
    bool sendInternal( const QCString& fun, const QCString& args, const QByteArray& data );

    QCString m_app;
    QCString m_obj;
    QCString m_type;

    class DCOPRefPrivate;
    DCOPRefPrivate *d;
};

/**
 * Writes the reference (NOT the object itself) to the stream.
 */
QDataStream& operator<<( QDataStream&, const DCOPRef& ref );
/**
 * Reads a reference from the stream.
 */
QDataStream& operator>>( QDataStream&, DCOPRef& ref );

#endif
