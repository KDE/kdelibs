/*
Copyright (c) 1999,2000 Preston Brown <pbrown@kde.org>
Copyright (c) 1999, 2000 Matthias Ettrich <ettrich@kde.org>

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

#ifndef _DCOPOBJECT_H
#define _DCOPOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <kdatastream.h> // needed for proper bool marshalling

class DCOPClient;
typedef QValueList<QCString> QCStringList;

// Makros for DCOP interfaces

#define K_DCOP \
public:        \
  virtual bool process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData); \
  QCStringList functions(); \
  QCStringList interfaces(); \
private:

#define k_dcop_signals public
#define k_dcop public
#define ASYNC void

/**
 * Provides an interface for receiving DCOP messages.
 *
 * This class provides an interface for receiving DCOP messages.  To use it,
 * simply multiply-inherit from DCOPObject and from some other class, and
 * then implement the @ref DCOPObject::process() method.  Because this method is
 * pure virtual, you @p must implement the method.
 *
 * @author Preston Brown <pbrown@kde.org>, Matthias Ettrich <ettrich@kde.org>
 */
class DCOPObject
{
public:
  /**
   * Creates a DCOPObject and calculates the object id
   * using its physical memory address.
   */
  DCOPObject();
  /**
   * Creates a DCOPObject and calculates the object id
   * using @ref QObject::name().
   */
  DCOPObject(QObject *obj);
  /**
   * Creates a DCOPObject with object Id @p objId.
   */
  DCOPObject(const QCString &objId);
  /**
   * Destroys the DCOPObject and removes it from the map
   * of known objects.
   */
  virtual ~DCOPObject();

  QCString objId() const;

  /**
   * Renames a dcop object, if no other with the same name exists
   * Use with care, all dcop signals are disconnected
   * 
   * The new name is @p objId
   **/
  bool setObjId(const QCString &objId);

  /**
   * Dispatches a message.
   *
   * Usually you want to use an IDL
   * compiler to automatically generate an implementation for
   * this function.
   *
   * If this function returns @p false, then @ref processDynamic()
   * is called.
   *
   * @param fun is the normalized function signature.
   *            Such a signature usually looks like
   *            @p foobar(QString,int). The return type,
   *            qualifiers like "const" etc. are not part of
   *            the signature.
   *
   * Note to implementators: remember to call the baseclasses
   * implementation.
   *
   * @see DCOPClient::normalizeFunctionSignature(), functions()
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData);


  /**
   * This function is of interest when you used an IDL compiler
   * to generate the implementation for @ref process() but
   * you still want to dispatch some functions dynamically.
   * Dynamically means that methods may appear and vanish
   * during runtime.
   *
   * @return @p false by default.
   *
   * @see process(), DCOPClient::normalizeFunctionSignature(), functions(),
   */
  virtual bool processDynamic(const QCString &fun, const QByteArray &data,
			      QCString& replyType, QByteArray &replyData);

   /**
   * This function is of interest when you used an IDL compiler
   * to generate the implementation for @ref functions() but
   * you still want to list some functions dynamically.
   * Dynamically means that the methods may appear and vanish
   * during runtime.
   *
   * @return @p A list of the additional functions, default is an empty list.
   *
   * @see functions(),
   */
  virtual QCStringList functionsDynamic();

    /**
   * This function is of interest when you used an IDL compiler
   * to generate the implementation for @ref interfaces() but
   * you still want to list some interfaces dynamically.
   * Dynamically means that they may appear and vanish
   * during runtime.
   *
   * @return @p A list of the aditional interfaces, default is an empty list.
   *
   * @see interfaces(),
   */
  virtual QCStringList interfacesDynamic();

  /**
   * Returns the names of the interfaces, specific ones last. The
   * functions gets reimplemented by the IDL compiler. If you don't
   * use the IDL compiler, consider implementing this function
   * manually if you want your object to be easily explorable.
   *
   * @see functions()
   */
  virtual QCStringList interfaces();

  /**
   * Returns the list of functions understood by the object. It gets
   * reimplemented by the IDL compiler. If you don't use the IDL
   * compiler, consider implementing this function manually if you
   * want your object to be easily scriptable.
   *
   * Rationale: functions() allows an interpreter to do client-side
   * type-casting properly.
   *
   * Note to implementators: remember to call the baseclasses
   * implementation.
   *
   * @see process(), processDynamic(), DCOPClient::normalizeFunctionSignature()
   */
  virtual QCStringList functions();

  /**
   * Emit @p signal as DCOP signal from this object with @p data as
   * arguments
   */
  void emitDCOPSignal( const QCString &signal, const QByteArray &data);

  /**
   * Connects to a DCOP signal.
   * @param sender the name of the client that emits the signal. When empty
   * the signal will be passed from any client.
   * @param senderObj the name of the sending object that emits the signal.
   * @param signal the name of the signal. The arguments should match with slot.
   * @param slot The name of the slot to call. Its arguments should match with signal.
   * @param Volatile If true, the connection will not be reestablished when
   * @p sender unregisters and reregisters with DCOP. In this case the @p sender
   * must be registered when the connection is made.
   * If false, the connection will be reestablished when @p sender reregisters.
   * In this case the connection can be made even if @p sender is not registered
   * at that time.
   *
   * @return false if a connection could not be established.
   * This will be the case when
   * @li @p Volatile is true and @p sender  does not exist.
   * @li @p signal and @p slot do not have matching arguments.
   */
  bool connectDCOPSignal( const QCString &sender, const QCString &senderObj,
                          const QCString &signal,
                          const QCString &slot,
                          bool Volatile);

  /**
   * Disconnects a DCOP signal.
   * @param sender the name of the client that emits the signal.
   * @param senderObj the name of the object that emits the signal.
   * If empty all objects will be disconnected.
   * @param signal the name of the signal. The arguments should match with slot.
   * If empty all objects will be disconnected.
   * @param slot The name of the slot the signal is connected to.
   * If empty all slots will be disconnected.
   *
   * A special case is when both sender & signal are empty. In this
   * case all connections related to this object in the current client
   * are disconnected. (Both connections from as well as to this object!)
   *
   * @return false if no connection(s) where removed.
   */
  bool disconnectDCOPSignal( const QCString &sender, const QCString &senderObj,
                             const QCString &signal,
                             const QCString &slot);

  /**
   * @returns during a DCOP call the DCOPClient responsible for making
   * the call. This information is only guaranteed to be correct when 
   * entering your DCOP function.
   */
  DCOPClient *callingDcopClient();

  /**
   * @internal Sets DCOPClient returned by @ref callingDcopClient()
   */
  void setCallingDcopClient(DCOPClient *);

  /**
   * @return true if an object with the questionable @p objId is
   *         known in this process. This query does not ask proxies.
   *
   * @ref DCOPObjectProxy
   */
  static bool hasObject(const QCString &objId);

  /**
   * @return the  DCOPObject for the id @p objId.
   *
   * This function does not query the @ref DCOPObjectProxy.
   */
  static DCOPObject *find(const QCString &objId);


  /**
   *
   * @return a list of DCOPObjects beginning with the string
   * contained in @p partialId.
   *
   * This function is used for multicasting a DCOP message to
   * several objects inside a single process.
   */
  static QPtrList<DCOPObject> match(const QCString &partialId);

  /**
   * Creates an object id for the QObject @p obj. This is done
   * using the @ref QObject::name() function.
   */
  static QCString objectName( QObject* obj );

private:
  /**
   * The object id of this DCOPObject.
   */
  QCString ident;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class DCOPObjectPrivate;
  DCOPObjectPrivate *d;
};

class DCOPObjectProxyPrivate;
/**
 * You must use a proxy if you want to dispatch method calls for
 * object IDs which dont have (yet) a corresponding @ref DCOPObject.
 * This is somewhat like virtual object references in CORBA.
 *
 * @author Matthias Ettrich <ettrich@kde.org>
 */
class DCOPObjectProxy
{
public:
    /**
     * Creates a new proxy.
     */
    DCOPObjectProxy();

    /**
     * Obsolete, do not use. DCOP clients know about object proxies
     * automatically.
     *
     * @deprecated
     */
    DCOPObjectProxy( DCOPClient*);

    /**
     * Destroy the proxy.
     */
    virtual ~DCOPObjectProxy();

    /**
     * Reimplement this method to dispatch method calls.
     *
     * This method is called of all proxies if the @ref DCOPClient
     * knows no object with the id @p obj. If the first proxy returns
     * @p true, the @ref DCOPClient will no longer call other proxies.
     *
     * The object id @p obj may be empty for app-wide function calls no
     * associated with any object.
     */
    virtual bool process( const QCString& obj, const QCString& fun,
			  const QByteArray& data,
			  QCString& replyType, QByteArray &replyData );
private:
    void* unused;
    void* unused_too;
    friend class DCOPClient;
    static QPtrList<DCOPObjectProxy>* proxies;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    DCOPObjectProxyPrivate* d;
};



#endif
