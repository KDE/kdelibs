/*****************************************************************

Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************/

#ifndef _DCOPOBJECT_H
#define _DCOPOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

class DCOPClient;

// Makros for DCOP interfaces

#define K_DCOP \
public:        \
  virtual bool process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData); \
private:

#define k_dcop public

/**
 * DCOPObject provides an interface for receiving DCOP messages.
 *
 * This class provides an interface for receiving DCOP messages.  To use it,
 * simply multiply-inherit from DCOPObject and from some other class, and
 * then implement the DCOPObject::process method.  Because this method is
 * pure virtual, you <em>must</em> implement the method.
 *
 * @author Preston Brown <pbrown@kde.org>, Matthias Ettrich <ettrich@kde.org>
 */
class DCOPObject
{
 public:
  /**
   * Create a non valid DCOPObject
   */
  DCOPObject();
  /**
   * Create a DCOPObject and calculate the object id
   * using @ref QObject::name.
   */
  DCOPObject(QObject *obj);
  /**
   * Creates a valid DCOPObject.
   */
  DCOPObject(const QCString &objId);
  /**
   * Destroys the DCOPObject and removes it from the map
   * of known objects.
   */
  virtual ~DCOPObject();

  QCString objId() const;

  /**
   * Dispatch a message. Usually you may want to use an IDL
   * compiler to automatically generate an implementation for
   * this function.
   *
   * If this function returns FALSE, then @ref #processDynamic
   * is called.
   *
   * @param fun is the normalized function signature.
   *            Such a signature usually looks like
   *            "foobar(QString,int)". The returntype,
   *            qualifiers like "const" etc. are not part of
   *            the signature.
   *
   * @see DCOPClient::normalizeFunctionSignature
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData);


  /**
   * This function is of interest when you used an IDL compiler
   * to generate the implementation for @ref #process but
   * you still want to dispatch some functions dynamically.
   * Dynamically means that methods may appear and vanish
   * during runtime.
   *
   * @return FALSE by default.
   *
   * @see #process
   */
  virtual bool processDynamic(const QCString &fun, const QByteArray &data,
			      QCString& replyType, QByteArray &replyData);

  /**
   * @retrun TRUE if an obejct with the questionable objId is
   *         known in this process. This query does not ask proxies.
   *
   * @ref DCOPObjectProxy
   */
  static bool hasObject(const QCString &objId);
    
  /**
   * @return the DCOPObject for the id @param objId.
   *
   * This function does not query the @ref DCOPObjectProxy.
   */
  static DCOPObject *find(const QCString &objId);
    
  /**
   * Creates an object id for the QObject @param obj. This is done
   * using the @ref QObject::name function.
   */
  static QCString objectName( QObject* obj );

 private:
  /**
   * The object id of this DCOPObject.
   */
  QCString ident;
};

/**
 * You must use a proxy if you want to dispatch method calls for
 * object IDs which dont have (yet) a corresponding @ref DCOPObject.
 * This is known as virtual object references in CORBA.
 *
 * @author Matthias Ettrich <ettrich@troll.no>
 */
class DCOPObjectProxy
{
public:
    /**
     * Create a new proxy and register it at the server.
     */
    DCOPObjectProxy( DCOPClient* client );
    /**
     * Destroys the proxy.
     */
    virtual ~DCOPObjectProxy();

    /**
     * Overload this method to disptatch method calls.
     *
     * This method is called of all proxies if the @ref DCOPClient
     * knows no object with the id @p obj. If the first proxy returns
     * TRUE, the DCOPClient will no longer call other proxies.
     */
    virtual bool process( const QCString& obj, const QCString& fun, const QByteArray& data,
			  QCString& replyType, QByteArray &replyData );
private:
    DCOPClient* parent;
};



#endif
