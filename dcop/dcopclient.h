#ifndef _DCOPCLIENT_H
#define _DCOPCLIENT_H

#include <qobject.h>
#include <qstring.h>

class DCOPClientPrivate;

class DCOPClient : public QObject
{
  Q_OBJECT

 public:
  /**
   * Creates a new DCOP client, but does not attach to any server.
   * @param appId is a UNIQUE application/program id that the server
   * will use to associate requests with.
   */
  DCOPClient(const QString &appId);

  /**
   * clean up any open connections and dynamic data.
   */
  virtual ~DCOPClient();

  /**
   * specify the address of a server to use upon attaching.
   * if no server address is ever specified, attach will try its best to
   * find the server anyway.
   */
  static void setServerAddress(const QString &addr);

  /**
   * Attach to the DCOP server.
   * @return true if attaching was successful.
   */
  bool attach();
  
  /**
   * Detach from the DCOP server.
   */
  bool detach();

  /**
   * return the socket over which DCOP is communicating with the server.
   */
  int socket() const;

  /**
   * send a data block to the server.
   * @param remApp the remote application id.
   * @param remObj the name of the remote object.
   * @param remFun the remote function in the specified object to call.
   * @param data the data to provide to the remote function.
   *
   * @return whether or not the server was able to accept the send.
   */
  bool send(const QString &remApp, const QString &remObj, 
	    const QString &remFun, const QByteArray &data);

  /**
   * This function acts exactly the same as the above, but the data
   * parameter can be specified as a QString for convenience.
   */
  bool send(const QString &remApp, const QString &remObj, 
	    const QString &remFun, const QString &data);

  /**
   * performs a synchronous send and receive.
   */
  bool call(const QString &remApp, const QString &remObj,
	    const QString &remFun, const QByteArray &data,
	    QByteArray &replyData);


  /**
   * override to handle app-wide function calls unassociated w/an object.
   */
  virtual bool process(const QString &fun, const QByteArray &data,
		       QByteArray &replyData);

  /**
   * receive a piece of data from the server.
   * @param app the application the data was intended for.  Should be
   *        equal to our appId that we passed when the DCOPClient was
   *        created.
   * @param obj the name of the object to pass the data on to.
   * @param fun the name of the function in the object to call.
   * @param data the arguments for the function.
   * @internal
   */
  virtual bool receive(const QString &app, const QString &obj, 
		       const QString &fun, const QByteArray& data,
		       QByteArray &replyData);

  /**
   * ping a remote application to see if it is running.
   * @return true if the remote application is running, otherwise false.
   */
  bool ping(QString remApp);

 public slots:

 protected slots:
  void processSocketData(int socknum);

 protected:

 private:
  /**
   * this is only public so that ICE callbacks work.  Don't touch.
   */
  DCOPClientPrivate *d;

};

#endif
