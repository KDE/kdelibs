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

#ifndef DCOPCLIENT_H
#define DCOPCLIENT_H

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <dcoptypes.h>
#include <kdatastream.h> // needed for proper bool marshalling
#include <kdelibs_export.h>

class DCOPObjectProxy;
class DCOPClientPrivate;
class DCOPClientTransaction;

/**
 * Inter-process communication and remote procedure calls
 * for KDE applications.
 *
 * This class provides IPC and RPC for KDE applications.  Usually you
 * will not have to instantiate one yourself because KApplication
 * contains a method to return a pointer to a DCOPClient object which
 * can be used for your whole application.
 *
 * Before being able to send or receive any DCOP messages, you will have
 * to attach your client object to the DCOP server, and then register
 * your application with a specific name. See attach()
 * and registerAs() for
 * more information.
 *
 * Data to be sent should be serialized into a QDataStream which was
 * initialized with the QByteArray that you actually intend to send
 * the data in.  An example of how you might do this:
 *
 * \code
 *   QByteArray data;
 *   QDataStream arg(data, QIODevice::WriteOnly);
 *   arg.setVersion(QDataStream::Qt_3_1);
 *   arg << QString("This is text I am serializing");
 *   client->send("someApp", "someObject", "someFunction(QString)", data);
 * \endcode
 *
 * @see KApplication::dcopClient()
 * @author Preston Brown <pbrown@kde.org>, Matthias Ettrich <ettrich@kde.org>
 */
class DCOP_EXPORT DCOPClient : public QObject
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
  static void setServerAddress(const QByteArray &addr);

  /**
   * Attaches to the DCOP server.
   *
   * If the connection was already attached,
   * the connection will be re-established with the current server address.
   *
   * Naturally, only attached application can use DCOP services.
   *
   * If a QApplication object exists then client registers itself as
   * QApplication::name() + "-" + \<pid\>.
   * If no QApplication object exists the client registers itself as
   * "anonymous".
   *
   * If you want to register differently, you should use registerAs()
   * instead.
   *
   * @return true if attaching was successful.
   */
  bool attach();

  /**
   * Internal function for KUniqueApplication to register the DCOPClient
   * with the application in case the application didn't exist at the
   * time the DCOPClient was created.
   * @internal
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
   * @since 3.0.2
   */
  bool qtBridgeEnabled() const; 
  /**
   * Specify whether Qt objects of the application should be accessible
   * via DCOP.
   * By default the DCOP - Qt bridge is enabled.
   * @param b true to make Qt objects accessible over DCOP
   * @since 3.0.2
   */
  void setQtBridgeEnabled(bool b);

  /**
   * Registers at the DCOP server.
   *
   * If the application was already registered,
   * the registration will be re-done with the new appId.
   *
   * @p appId is a @p unique application/program id that the server
   * will use to associate requests with. If there is already an application
   * registered with the same name, the server will add a number to the
   * id to unify it. If @p addPID is true, the PID of the current process
   * will be added to id.
   *
   * Registration is necessary if you want to allow other clients to talk
   * to you.  They can do so using your @p appId as first parameter
   * for send() or call(). If you just want to talk to
   *  other clients, you
   * do not need to register at the server. In that case attach() is
   * enough.
   * It will implicitly register you as "anonymous".
   *
   * @param appId the id of the application
   * @param addPID true to add the process id
   * @return The actual @p appId used for the registration or a null string
   * if the registration wasn't successful.
   */
  DCOPCString registerAs( const DCOPCString &appId, bool addPID = true );

  /**
   * Returns whether or not the client is registered at the server.
   * @return true if registered at the server
   */
  bool isRegistered() const;

  /**
   * Returns the current app id or a null string if the application
   * hasn't yet been registered.
   * @return the application id, or QString::null if not registered
   */
  DCOPCString appId() const;

  /**
   * Returns the socket fd that is used for communication with the server.
   * @return The socket over which DCOP is communicating with the server.
   */
  int socket() const;

  /**
   * Temporarily suspends processing of DCOP events.
   * This can be useful if you need to show e.g. a dialog before
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
  bool send(const DCOPCString &remApp, const DCOPCString &remObj,
	    const DCOPCString &remFun, const QByteArray &data);

  /**
   * This function acts exactly the same as the above, but the data
   * parameter can be specified as a QString for convenience.
   *
   * @param remApp The remote application id.
   * @param remObj The name of the remote object.
   * @param remFun The remote function in the specified object to call.
   * @param data The data to provide to the remote function.
   *
   * @return Whether or not the server was able to accept the send.
   */
  bool send(const DCOPCString &remApp, const DCOPCString &remObj,
	    const DCOPCString &remFun, const QString &data);

  /**
   * Performs a synchronous send and receive.
   *
   *  The parameters are the same as for send, with the exception of
   *  another QByteArray being provided for results to be
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
   * @param timeout timeout for the call in miliseconds, or -1 for no timeout
   * @return true if successful, false otherwise
   *
   * @since 3.2
   *
   * @see send()
   */
  bool call(const DCOPCString &remApp, const DCOPCString &remObj,
	    const DCOPCString &remFun, const QByteArray &data,
	    DCOPCString& replyType, QByteArray &replyData,
	    bool useEventLoop=false, int timeout=-1);

  /**
   * Performs a asynchronous send with receive callback.
   *
   *  The first four parameters are the same as for send.
   *
   * @p callBackObj and @p callBackSlot specify a call back
   * slot that is called when an answer is received.
   *
   * The slot should have the following signature:
   * callBackSlot(int callId, const DCOPCString& replyType,
   *              const QByteArray &replyData);
   *
   *
   * @param remApp the remote application's id
   * @param remObj the remote object id
   * @param remFun the remote function id
   * @param data the data to send
   * @param callBackObj object to call back
   * @param callBackSlot slot to call back
   * @return 0 on failure, on success a callId > 0 is
   * returned that will be passed as first argument of
   * the result call back
   *
   * @see send()
   * @see callback()
   *
   * @since 3.2
   */
  int callAsync(const DCOPCString &remApp, const DCOPCString &remObj,
                const DCOPCString &remFun, const QByteArray &data,
                QObject *callBackObj, const char *callBackSlot);

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
   * @param timeout timeout for the call in miliseconds, or -1 for no timeout
   * @return true is returned when an object was found for which @p remFun
   *         returned true. If no such object is the function returns false.
   *
   * @since 3.2
   *
   * @see send()
   */
  bool findObject(const DCOPCString &remApp, const DCOPCString &remObj,
	    const DCOPCString &remFun, const QByteArray &data,
	    DCOPCString &foundApp, DCOPCString &foundObj,
	    bool useEventLoop=false, int timeout=-1);

  /**
   * Emits @p signal as DCOP signal from object @p object with @p data as
   * arguments.
   */
  void emitDCOPSignal( const DCOPCString &object, const DCOPCString &signal,
                       const QByteArray &data);

  /** 
   * For backwards compatibility, like emitDCOPSignal( const DCOPCString&, 
   * const DCOPCString &, const QByteArray &) with an anonymous object.
   */
  void emitDCOPSignal( const DCOPCString &signal, const QByteArray &data);

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
  bool connectDCOPSignal( const DCOPCString &sender, const DCOPCString &senderObj,
                          const DCOPCString &signal,
                          const DCOPCString &receiverObj, const DCOPCString &slot,
                          bool Volatile);

  /**
   * @deprecated
   * For backwards compatibility
   */
  bool connectDCOPSignal( const DCOPCString &sender,      const DCOPCString &signal,
                          const DCOPCString &receiverObj, const DCOPCString &slot,
                          bool Volatile) KDE_DEPRECATED;

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
  bool disconnectDCOPSignal( const DCOPCString &sender, const DCOPCString &senderObj,
                          const DCOPCString &signal,
                          const DCOPCString &receiverObj, const DCOPCString &slot);

  /**
   * @deprecated
   * For backwards compatibility
   */
  bool disconnectDCOPSignal( const DCOPCString &sender,      const DCOPCString &signal,
                             const DCOPCString &receiverObj, const DCOPCString &slot) KDE_DEPRECATED;

  /**
   * Reimplement this function to handle app-wide function calls unassociated w/an object.
   *
   * Note that @p fun is normalized. See normalizeFunctionSignature().
   *
   * If you do not want to reimplement this function for whatever reason,
   * you can also use a default object  or a DCOPObjectProxy.
   *
   * @param fun the normalized function signature
   * @param data the received data
   * @param replyType write the reply type in this string
   * @param replyData write the reply data in this array
   * @return true if successful, false otherwise
   * @see setDefaultObject()
   */
  virtual bool process(const DCOPCString &fun, const QByteArray &data,
		       DCOPCString& replyType, QByteArray &replyData);

  /**
   * Delays the reply of the current function call
   * until endTransaction() is called.
   *
   * This allows a server to queue requests.
   *
   * Note: Should be called from inside process() only!
   * @see endTransaction()
   */
  DCOPClientTransaction *beginTransaction( );

  /**
   * Sends the delayed reply of a function call.
   * @param t the transaction as received from beginTransaction()
   * @param replyType write the reply type in this string
   * @param replyData write the reply data in this array
   * @see beginTransaction()
   */
  void endTransaction( DCOPClientTransaction *t, DCOPCString& replyType, QByteArray &replyData);

  /**
   * Test whether the current function call is delayed.
   *
   * Note: Should be called from inside process() only!
   * @return The ID of the current transaction or
   *         0 if no transaction is going on.
   * @see process()
   * @see beginTransaction()
   */
  qint32 transactionId() const;

  /**
   * Checks whether @p remApp is registered with the DCOP server.
   * @param remApp the id of the remote application
   * @return true if the remote application is registered, otherwise @p false.
   */
  bool isApplicationRegistered( const DCOPCString& remApp);

  /**
   * Retrieves the list of all currently registered applications
   * from dcopserver.
   * @return a list of all regietered applications
   */
  DCOPCStringList registeredApplications();

  /**
   * Retrieves the list of objects of the remote application @p remApp.
   * @param remApp he id of the application
   * @param ok if not null, the function sets @p ok to true if successful
   *           and false if an error occurred
   * @return the list of object ids
   */
  DCOPCStringList remoteObjects( const DCOPCString& remApp, bool *ok = 0 );

  /**
   * Retrieves the list of interfaces of the remote object @p remObj
   * of application @p remApp.
   * @param remApp the id of the application
   * @param remObj the id of the object
   * @param ok if not null, the function sets @p ok to true if successful
   *           and false if an error occurred
   * @return the list of interfaces
  */
  DCOPCStringList remoteInterfaces( const DCOPCString& remApp, const DCOPCString& remObj , bool *ok = 0 );

  /**
   * Retrieves the list of functions of the remote object @p remObj
   * of application @p remApp
   * @param remApp the id of the application
   * @param remObj the id of the object
   * @param ok if not null, the function sets @p ok to true if successful
   *           and false if an error occurred
   * @return the list of function ids
  */
  DCOPCStringList remoteFunctions( const DCOPCString& remApp, const DCOPCString& remObj , bool *ok = 0 );

  /**
   * Receives a DCOPSend or DCOPCall message from the server.
   *
   * @param app The application the message was intended for.  Should be
   *        equal to our appId that we passed when the DCOPClient was
   *        created.
   * @param obj The name of the object to pass the data on to.
   * @param fun The name of the function in the object to call.
   * @param data The arguments for the function.
   * @param replyType write the reply type in this string
   * @param replyData write the reply data in this array
   * @return true if successful, false otherwise
   * @internal
   */
  bool receive(const DCOPCString &app, const DCOPCString &obj,
	       const DCOPCString &fun, const QByteArray& data,
	       DCOPCString& replyType, QByteArray &replyData);

  /**
   * Receives a @p DCOPFind message from the server.
   *
   * @param app The application the message was intended for.  Should be
   *        equal to our appId that we passed when the DCOPClient was
   *        created.
   * @param obj The name of the object to pass the data on to.
   * @param fun The name of the function in the object to call.
   * @param data The arguments for the function.
   * @param replyType write the reply type in this string
   * @param replyData write the reply data in this array
   * @internal
   */
  bool find(const DCOPCString &app, const DCOPCString &obj,
	    const DCOPCString &fun, const QByteArray& data,
	    DCOPCString& replyType, QByteArray &replyData);

  /**
   * Normalizes the function signature @p fun.
   *
   * A normalized signature doesn't contain any unnecessary whitespace
   * anymore. The remaining whitespace consists of single blanks only (0x20).
   *
   * Example for a normalized signature:
   * \code
   *   "someFunction(QString,int)"
   * \endcode
   *
   * When using send() or call(), normalization is done
   * automatically for you.
   *
   * @param fun the function signature to normalize
   * @return the normalized function
   */
  static DCOPCString normalizeFunctionSignature( const DCOPCString& fun );


  /**
   * Returns the appId of the last application that talked to us.
   * @return the application id of the last application that send a message
   *         to this client
   */
  DCOPCString senderId() const;


   /**
    * Installs object @p objId as application-wide default object.
    *
    * All app-wide messages that have not been processed by the dcopclient
    * will be send further to @p objId.
    * @param objId the id of the new default object
    */
  void setDefaultObject( const DCOPCString& objId );

    /**
     * Returns the current default object or an empty string if no object is
     * installed as default object.
     *
     * A default object receives application-wide messages that have not
     * been processed by the DCOPClient itself.
    * @return the id of the new default object
     */
  DCOPCString defaultObject() const;

  /**
   * Enables / disables the applicationRegistered() /
   * applicationRemoved() signals.
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
   * Switch to priority call mode.
   * @internal
   */
  void setPriorityCall(bool);

  /**
   * Returns the application's main dcop client. The main client can
   * be used by objects that do not have any specific access to a dcop
   * client. In KDE applications, the main client usually is the same
   * as KApplication::dcopClient().
   * @return the application's main dcop client
   */
  static DCOPClient* mainClient();

 /**
   * Sets the application's main dcop client. The main client can
   * be used by objects that do not have any specific access to a dcop
   * client. In KDE applications, the main client usually is the same
   * as KApplication::dcopClient().
   * @param mainClient the new main dcop client
   */
  static void setMainClient( DCOPClient* mainClient);

  /**
   * Look for the given client only in this process. This can be used
   * to check whether a given client (by name) is running in the same
   * process or in another one.
   */
  static DCOPClient* findLocalClient( const DCOPCString &_appId );

  /**
    * Danger will Robinson!
    * @internal Do not use.
    */
  static void emergencyClose();

  /**
    * Provides information about the last DCOP call for debugging purposes.
    * @internal Do not use.
    */
  static const char *postMortemSender();
  /** Information about last DCOP call. @internal */
  static const char *postMortemObject();
  /** Information about last DCOP call. @internal */
  static const char *postMortemFunction();

  /**
    * File with information how to reach the dcopserver.
    * @param hostname Hostname to use, if empty current hostname of
    * the system is used.
    * @return Filename that contains information how to contact the
    * DCOPserver.
    */
  static QByteArray dcopServerFile(const QByteArray &hostname=0);

  /**
   * @deprecated
   * For backwards compatibility with KDE 2.x
   * // KDE4 remove
   */
  static QByteArray dcopServerFileOld(const QByteArray &hostname=0) KDE_DEPRECATED;

  /**
   * Return the path of iceauth or an empty string if not found.
   */
  static QByteArray iceauthPath();

Q_SIGNALS:
  /**
   * Indicates that the application @p appId has been registered with
   * the server we are attached to.
   *
   * You need to call setNotifications() first, to tell the DCOP server
   * that you want to get these events.
   * @param appId the id of the new application
   */
  void applicationRegistered( const QByteArray& appId );
  /**
   * Indicates that the formerly registered application @p appId has
   * been removed.
   *
   * You need to call setNotifications() first, to tell the
   * DCOP server that you want to get these events.
   * @param appId the id of the removed application
   */
  void applicationRemoved( const QByteArray& appId );

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
   * In KDE, the KApplication object connects to this signal to be
   * able to block any user input (i.e. mouse and key events) while
   * we are waiting for an answer. If we did not do this, the
   * application might end up in an illegal state, as a keyboard
   * shortcut or a mouse action might cause another dcop call to be
   * issued.
   * @param block true to block user input, false otherwise
   */
  void blockUserInput( bool block );

  /**
   * Signal used for callbacks of async calls.
   * This signal is automatically connected to the call back
   * slot specified in the async call.
   * @see callAsync()
   * @since 3.2
   * @internal
   */
  void callBack(int, const DCOPCString&, const QByteArray &);

public Q_SLOTS:

protected Q_SLOTS:
  /**
   * Process data from the socket.
   * @param socknum the fd of the socket
   */
  void processSocketData(int socknum);

private Q_SLOTS:
  void processPostedMessagesInternal();
  void asyncReplyReady();
  void eventLoopTimeout();

public:
  class ReplyStruct;

  /**
   * Process an async reply in @p replyStruct.
   * @internal
   **/
  void handleAsyncReply(ReplyStruct *replyStruct);

private:

  bool isLocalTransactionFinished(qint32 id, DCOPCString &replyType, QByteArray &replyData);

  bool attachInternal( bool registerAsAnonymous = true );

  bool callInternal(const DCOPCString &remApp, const DCOPCString &remObj,
	    const DCOPCString &remFun, const QByteArray &data,
	    DCOPCString& replyType, QByteArray &replyData,
	    bool useEventLoop, int timeout, int minor_opcode);


  bool callInternal(const DCOPCString &remApp, const DCOPCString &remObjId,
            const DCOPCString &remFun, const QByteArray &data,
            ReplyStruct *replyStruct,
            bool useEventLoop, int timeout, int minor_opcode);

protected:
  /** Standard hack for adding virtuals later. @internal */
  virtual void virtual_hook( int id, void* data );
private:
  DCOPClientPrivate *d;
};

#endif
