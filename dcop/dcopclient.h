/*
Copyright (c) 1999 Preston Brown <pbrown@kde.org>
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

#ifndef _DCOPCLIENT_H
#define _DCOPCLIENT_H

#include <qobject.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <kdatastream.h> // needed for proper bool marshalling

class DCOPObjectProxy;
class DCOPClientPrivate;
class DCOPClientTransaction;

typedef QValueList<QCString> QCStringList;

/**
 * Inter-process communication and remote procedure calls
 * for KDE applications.
 *
 * This class provides IPC and RPC for KDE applications.  Usually you
 * will not have to instantiate one yourself because @ref KApplication
 * contains a method to return a pointer to a DCOPClient object which
 * can be used for your whole application.
 *
 * Before being able to send or receive any DCOP messages, you will have
 * to attach your client object to the DCOP server, and then register
 * your application with a specific name. See @ref attach()
 * and @ref registerAs() for
 * more information.
 *
 * Data to be sent should be serialized into a @ref QDataStream which was
 * initialized with the @ref QByteArray that you actually intend to send
 * the data in.  An example of how you might do this:
 *
 * <pre>
 *   QByteArray data;
 *   QDataStream arg(data, IO_WriteOnly);
 *   arg << QString("This is text I am serializing");
 *   client->send("someApp", "someObject", "someFunction", arg);
 * </pre>
 *
 * @see KApplication::dcopClient()
 * @author Preston Brown <pbrown@kde.org>, Matthias Ettrich <ettrich@kde.org>
 */
class DCOPClient : public QObject
{
  Q_OBJECT

 public:
  /**
   * Constructs a new DCOP client, but does not attach to any server.  */
  DCOPClient();

  /**
   * Cleans up any open connections and dynamic data.
   */
  virtual ~DCOPClient();

  /**
   * Sets the address of a server to use upon attaching.
   *
   * If no server address is ever specified, attach will try its best to
   * find the server anyway.
   * @param addr the new address of the server
   */
  static void setServerAddress(const QCString &addr);

  /**
   * Attaches to the DCOP server.
   *
   * If the connection was already attached,
   * the connection will be re-established with the current server address.
   *
   * Naturally, only attached application can use DCOP services.
   *
   * If a @ref QApplication object exists then client registers itself as
   * @ref QApplication::name() + "-" + <pid>.
   * If no @ref QApplication object exists the client registers itself as
   * "anonymous".
   *
   * If you want to register differently, you should use @ref registerAs()
   * instead.
   *
   * @return true if attaching was successful.
   */
  bool attach();

  /**
   * @internal
   * Internal function for @ref KUniqueApplication to register the @ref DCOPClient
   * with the application in case the application didn't exist at the
   * time the @ref DCOPClient was created.
   */
  void bindToApp();

  /**
   * Detaches from the DCOP server.
   * @return true if successful, false otherwise
   */
  bool detach();

  /**
   * Returns whether or not the client is attached to the server.
   * @return true if attached, false if not
   */
  bool isAttached() const;

  /**
   * Returns whether the client is attached to a server owned by
   * another user.
   * @return true if attached to a foreign server, false if not
   */
  bool isAttachedToForeignServer() const;

  /**
   * Returns whether the client handles incoming calls.
   * @return true if the client accepts calls
   */
  bool acceptCalls() const;

  /**
   * Specify whether the client should accept incoming calls.
   * By default clients accept incoming calls, but not when connected
   * to a foreign server.
   * @param b true to accept calls, false to reject them
   */
  void setAcceptCalls(bool b);

  /**
   * Returns whether the DCOP - Qt bridge is enabled.
   * By default the DCOP - Qt bridge is enabled.
   * @return true if Qt objects are accessible over DCOP
   * @since 3.1
   */
  bool qtBridgeEnabled(); // ### KDE 4.0: make const

  /**
   * Specify whether Qt objects of the application should be accessible
   * via DCOP.
   * By default the DCOP - Qt bridge is enabled.
   * @param b true to make Qt objects accessible over DCOP
   * @since 3.1
   */
  void setQtBridgeEnabled(bool b);

  /**
   * Registers at the DCOP server.
   *
   * If the application was already registered,
   * the registration will be re-done with the new @ref appId.
   *
   * @p appId is a @p unique application/program id that the server
   * will use to associate requests with. If there is already an application
   * registered with the same name, the server will add a number to the
   * id to unify it. If @p addPID is true, the PID of the current process
   * will be added to id.
   *
   * Registration is necessary if you want to allow other clients to talk
   * to you.  They can do so using your @p appId as first parameter
   * for @ref send() or @ref call(). If you just want to talk to
   *  other clients, you
   * do not need to register at the server. In that case @ref attach() is
   * enough.
   * It will implicitly register you as "anonymous".
   *
   * @param appId the id of the application
   * @param addPID true to add the process id
   * @return The actual @p appId used for the registration or a null string
   * if the registration wasn't successful.
   */
  QCString registerAs( const QCString &appId, bool addPID = true );

  /**
   * Returns whether or not the client is registered at the server.
   * @return true if registered at the server
   */
  bool isRegistered() const;

  /**
   * Returns the current app id or a null string if the application
   * hasn't yet been registered.
   * @param the application id, or QString::null if not registered
   */
  QCString appId() const;

  /**
   * Returns the socket fd that is used for communication with the server.
   * @return The socket over which DCOP is communicating with the server.
   */
  int socket() const;

  /**
   * Temporarily suspends processing of DCOP events.
   * This can be usefull if you need to show e.g. a dialog before
   * your application is ready to accept DCOP requests. Normally the
   * dialog would start an event loop and in this event loop DCOP
   * requests will be handled.
   *
   * Be aware that not responding to DCOP requests may cause other
   * programs that want to communicate with your application, to hang.
   * @see resume()
   * @see isSuspended()
   */
  void suspend();

  /**
   * Resumes the processing of DCOP events.
   * @see suspend().
   * @see isSuspended()
   */
  void resume();

  /**
   * Returns whether DCOP events are being processed.
   * @see suspend()
   * @see resume().
   * @since 3.1
   */
  bool isSuspended() const;

  /**
   * Sends a data block to the server.
   *
   * @param remApp The remote application id.
   * @param remObj The name of the remote object.
   * @param remFun The remote function in the specified object to call.
   * @param data The data to provide to the remote function.
   *
   * @return Whether or not the server was able to accept the send.
   */
  bool send(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QByteArray &data);

  /**
   * This function acts exactly the same as the above, but the data
   * parameter can be specified as a @ref QString for convenience.
   * 
   * @param remApp The remote application id.
   * @param remObj The name of the remote object.
   * @param remFun The remote function in the specified object to call.
   * @param data The data to provide to the remote function.
   *
   * @return Whether or not the server was able to accept the send.
   */
  bool send(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QString &data);

  /**
   * Performs a synchronous send and receive.
   *
   *  The parameters are the same as for send, with the exception of
   *  another @ref QByteArray being provided for results to be
   *  (optionally) returned in.
   *
   * A call blocks the application until the process receives the
   * answer.
   *
   * If @p useEventLoop is true, a local event loop will be started after
   * 1/10th of a second in order to keep the user interface updated
   * (by processing paint events and such) until an answer is received.
   *
   * @param remApp the remote application's id
   * @param remObj the remote object id
   * @param remFun the remote function id
   * @param data the data to send
   * @param replyType the type of the reply will be written here
   * @param replyData the data of the reply will be written here
   * @param useEventLoop if true the event loop will be started when
   *         the call blocks too long
   * @return true if successful, false otherwise
   * @see send()
   */
  bool call(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QByteArray &data,
	    QCString& replyType, QByteArray &replyData,
	    bool useEventLoop=false);

  /**
   * Searches for an object which matches a criteria.
   *
   * findObject calls @p remFun in the applications and objects identified
   * by @p remApp and @p remObj until @p remFun returns true. The name of
   * the application and object that returned true are returned in
   * @p foundApp and @p foundObj respectively.
   *
   * If @p remFun is empty a default function is called in the object
   * which always returns @p true.
   *
   * A findObject blocks the application until the process receives the
   * answer.
   *
   * If @p useEventLoop is true, a local event loop will be started after
   * 1/10th of a second in order to keep the user interface updated
   * (by processing paint events and such) until an answer is received.
   *
   * @param remApp The remote application id.
   * @param remObj The name of the remote object.
   * @param remFun The remote function in the specified object to call.
   *               This function should return a bool and is used as
   *               criteria.
   * @param data The data to provide to the remote function.
   * @param foundApp The remote application id that matched the criteria.
   * @param foundObj The remote object that matched the criteria.
   * @param useEventLoop if true the event loop will be started when
   *         the call blocks too long
   * @return true is returned when an object was found for which @p remFun
   *         returned true. If no such object is the function returns false.
   * @see send()
   */
  bool findObject(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QByteArray &data,
	    QCString &foundApp, QCString &foundObj,
	    bool useEventLoop=false);


  /**
   * Emits @p signal as DCOP signal from object @p object with @p data as
   * arguments.
   */
  void emitDCOPSignal( const QCString &object, const QCString &signal,
                       const QByteArray &data);

  /* For backwards compatibility */
  void emitDCOPSignal( const QCString &signal, const QByteArray &data);

  /**
   * Connects to a DCOP signal.
   * @param sender the name of the client that emits the signal. When empty
   * the signal will be passed from any client.
   * @param senderObj the name of the sending object that emits the signal.
   * @param signal the name of the signal. The arguments should match with slot.
   * @param receiverObj The name of the object to call
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
                          const QCString &receiverObj, const QCString &slot,
                          bool Volatile);

  /** 
   * @deprecated
   * For backwards compatibility 
   */
  bool connectDCOPSignal( const QCString &sender, const QCString &signal,
                          const QCString &receiverObj, const QCString &slot,
                          bool Volatile);

  /**
   * Disconnects a DCOP signal.
   *
   * A special case is when both @p sender & @p signal are empty. In this
   * case all connections related to @p receiverObj in the current client
   * are disconnected. (Both connections from as well as to this object!)
   *
   * @param sender the name of the client that emits the signal.
   * @param senderObj the name of the object that emits the signal.
   * If empty all objects will be disconnected.
   * @param signal the name of the signal. The arguments should match with slot.
   * @param receiverObj The name of the object the signal is connected to.
   * If empty all objects will be disconnected.
   * @param slot The name of the slot the signal is connected to.
   * If empty all slots will be disconnected.
   * @return false if no connection(s) where removed.
   */
  bool disconnectDCOPSignal( const QCString &sender, const QCString &senderObj,
                          const QCString &signal,
                          const QCString &receiverObj, const QCString &slot);

  /**
   * @deprecated
   * For backwards compatibility 
   */
  bool disconnectDCOPSignal( const QCString &sender, const QCString &signal,
                          const QCString &receiverObj, const QCString &slot);

  /**
   * Reimplement this function to handle app-wide function calls unassociated w/an object.
   *
   * Note that @p fun is normalized. See @ref normalizeFunctionSignature().
   *
   * If you do not want to reimplement this function for whatever reason,
   * you can also use a default object  or a @ref DCOPObjectProxy.
   *
   * @param the normalized function signature
   * @param data the received data
   * @param replyType write the reply type in this string
   * @param replyData write the reply data in this array
   * @return true if successful, false otherwise
   * @see setDefaultObject()
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData);

  /**
   * Delays the reply of the current function call
   * until @ref endTransaction() is called.
   *
   * This allows a server to queue requests.
   *
   * Note: Should be called from inside @ref process() only!
   * @see endTransaction()
   */
  DCOPClientTransaction *beginTransaction( );

  /**
   * Sends the delayed reply of a function call.
   * @param t the transaction as received from @ref beginTransaction()
   * @param replyType write the reply type in this string
   * @param replyData write the reply data in this array
   * @see beginTransaction()
   */
  void endTransaction( DCOPClientTransaction *t, QCString& replyType, QByteArray &replyData);

  /**
   * Test whether the current function call is delayed.
   *
   * Note: Should be called from inside @ref process() only!
   * @return The ID of the current transaction or
   *         0 if no transaction is going on.
   * @see process()
   * @see beginTransaction()
   */
  Q_INT32 transactionId() const;

  /**
   * Checks whether @p remApp is registered with the DCOP server.
   * @param remApp the id of the remote application
   * @return true if the remote application is registered, otherwise @p false.
   */
  bool isApplicationRegistered( const QCString& remApp);

  /**
   * Retrieves the list of all currently registered applications
   * from dcopserver.
   * @return a list of all regietered applications
   */
  QCStringList registeredApplications();

  /**
   * Retrieves the list of objects of the remote application @p remApp.
   * @param tremAPp he id of the application
   * @param ok if not null, the function sets @p ok to true if successful
   *           and false if an error occurred
   * @return the list of object ids
   */
  QCStringList remoteObjects( const QCString& remApp, bool *ok = 0 );

  /**
   * Retrieves the list of interfaces of the remote object @p remObj
   * of application @p remApp.
   * @param remApp the id of the application
   * @param remObj the id of the object
   * @param ok if not null, the function sets @p ok to true if successful
   *           and false if an error occurred
   * @return the list of interfaces
  */
  QCStringList remoteInterfaces( const QCString& remApp, const QCString& remObj , bool *ok = 0 );

  /**
   * Retrieves the list of functions of the remote object @p remObj
   * of application @p remApp
   * @param remApp the id of the application
   * @param remObj the id of the object
   * @param ok if not null, the function sets @p ok to true if successful
   *           and false if an error occurred
   * @return the list of function ids
  */
  QCStringList remoteFunctions( const QCString& remApp, const QCString& remObj , bool *ok = 0 );

  /**
   * @internal
   * Receives a DCOPSend or DCOPCall message from the server.
   *
   * @param app The application the message was intended for.  Should be
   *        equal to our appId that we passed when the DCOPClient was
   *        created.
   * @param obj The name of the object to pass the data on to.
   * @param fun The name of the function in the object to call.
   * @param data The arguments for the function.
   * @return true if successful, false otherwise
   */
  bool receive(const QCString &app, const QCString &obj,
	       const QCString &fun, const QByteArray& data,
	       QCString& replyType, QByteArray &replyData);

  /**
   * @internal
   * Receives a @p DCOPFind message from the server.
   *
   * @param app The application the message was intended for.  Should be
   *        equal to our appId that we passed when the DCOPClient was
   *        created.
   * @param obj The name of the object to pass the data on to.
   * @param fun The name of the function in the object to call.
   * @param data The arguments for the function.
   */
  bool find(const QCString &app, const QCString &obj,
	    const QCString &fun, const QByteArray& data,
	    QCString& replyType, QByteArray &replyData);

  /**
   * Normalizes the function signature @p fun.
   *
   * A normalized signature doesn't contain any unnecessary whitespace
   * anymore. The remaining whitespace consists of single blanks only (0x20).
   *
   * Example for a normalized signature:
   * <pre>
   *   "someFunction(QString,int)"
   * </pre>
   *
   * When using @ref send() or @ref call(), normalization is done
   * automatically for you.
   *
   * @param fun the function signature to normalize
   * @return the normalized function
   */
  static QCString normalizeFunctionSignature( const QCString& fun );


  /**
   * Returns the appId of the last application that talked to us.
   * @return the application id of the last application that send a message
   *         to this client
   */
  QCString senderId() const;


   /**
    * Installs object @p objId as application-wide default object.
    *
    * All app-wide messages that have not been processed by the dcopclient
    * will be send further to @p objId.
    * @param objId the id of the new default object
    */
  void setDefaultObject( const QCString& objId );

    /**
     * Returns the current default object or an empty string if no object is
     * installed as default object.
     *
     * A default object receives application-wide messages that have not
     * been processed by the DCOPClient itself.
    * @return the id of the new default object
     */
  QCString defaultObject() const;

  /**
   * Enables / disables the @ref applicationRegistered() /
   * @ref applicationRemoved() signals.
   * Note that a counter is maintained about how often this method
   * was called. If this method is called twice with @p enabled set to
   * true, notifications will be enabled until it was called with
   * @p enabled set to false as often.
   *
   * They are disabled by default.  
   * @param enabled true to enable notifications, false to disable
   */
  void setNotifications( bool enabled );

  /**
   * Tells the dcopserver to treat the client as daemon client, not
   * as regular client.
   * If the number of regular clients drops down to zero, the
   * dcopserver will emit a KDE termination signal after 10
   * seconds.
   * @param daemonMode true to enable daemon mode, false to disable
   */
  void setDaemonMode( bool daemonMode );

  /**
   * Returns the application's main dcop client. The main client can
   * be used by objects that do not have any specific access to a dcop
   * client. In KDE applications, the main client usually is the same
   * as KAppliction::dcopClient().
   * @return the application's main dcop client
   */
  static DCOPClient* mainClient();

 /**
   * Sets the application's main dcop client. The main client can
   * be used by objects that do not have any specific access to a dcop
   * client. In KDE applications, the main client usually is the same
   * as KAppliction::dcopClient().
   * @param mainClient the new main dcop client
   */
  static void setMainClient( DCOPClient* mainClient);

  /**
    * @internal Do not use.
    */
  static void emergencyClose();

  /**
    * @internal Do not use.
    *
    * Provides information about the last DCOP call for debugging purposes.
    */
  static const char *postMortemSender();
  /// @internal
  static const char *postMortemObject();
  /// @internal
  static const char *postMortemFunction();

  /**
    * File with information how to reach the dcopserver.
    * @param hostname Hostname to use, if empty current hostname of
    * the system is used.
    * @return Filename that contains information how to contact the
    * DCOPserver.
    */
  static QCString dcopServerFile(const QCString &hostname=0);

  /**
   * @deprecated
   * For backwards compatibility with KDE 2.x
   */
  static QCString dcopServerFileOld(const QCString &hostname=0);

signals:
  /**
   * Indicates that the application @p appId has been registered with
   * the server we are attached to.
   *
   * You need to call @ref setNotifications() first, to tell the DCOP server
   * that you want to get these events.
   * @param appId the id of the new application
   */
  void applicationRegistered( const QCString& appId );
  /**
   * Indicates that the formerly registered application @p appId has
   * been removed.
   *
   * You need to call @ref setNotifications() first, to tell the
   * DCOP server that you want to get these events.
   * @param appId the id of the removed application
   */
  void applicationRemoved( const QCString& appId );

  /**
   * Indicates that the process of establishing DCOP communications failed
   * in some manner.
   *
   *  Usually attached to a dialog box or some other visual
   * aid.
   * @param msg the message tha contains further information
   */
  void attachFailed(const QString &msg);

  /**
   * Indicates that user input shall be blocked or released,
   * depending on the argument.
   *
   * The signal is emitted whenever the client has to wait too long
   * (i.e. more than 1/10 of a second) for an answer to a
   * synchronous call. In that case, it will enter a local event
   * loop to keep the GUI updated until finally an answer arrives.
   *
   * In KDE, the @ref KApplication object connects to this signal to be
   * able to block any user input (i.e. mouse and key events) while
   * we are waiting for an answer. If we did not do this, the
   * application might end up in an illegal state, as a keyboard
   * shortcut or a mouse action might cause another dcop call to be
   * issued.
   * @param block true to block user input, false otherwise
   */
  void blockUserInput( bool block );

public slots:

protected slots:
  /**
   * Process data from the socket.
   * @param socknum the fd of the socket
   */
  void processSocketData(int socknum);

private slots:
  void processPostedMessagesInternal();

protected:

private:

  bool attachInternal( bool registerAsAnonymous = TRUE );

  bool callInternal(const QCString &remApp, const QCString &remObj,
	    const QCString &remFun, const QByteArray &data,
	    QCString& replyType, QByteArray &replyData,
	    bool useEventLoop, int minor_opcode);
protected:
  virtual void virtual_hook( int id, void* data );
private:
  DCOPClientPrivate *d;
};

#endif
