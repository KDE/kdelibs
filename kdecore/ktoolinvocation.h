/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _KTOOLSINVOKATION_H
#define _KTOOLSINVOKATION_H

// Version macros. Never put this further down.
#include "kdeversion.h"
#include "kdelibs_export.h"

#include "kurl.h"
#include <QObject>
#include <QByteArray>
#include <QStringList>

class DCOPClient;

/**
 *The static memberst (exception is the self() member, have to b called from the QApplication main thread.
 *Calles to members are only allowed if there is a Q(Core)Application object created
 *If you call the members with signal/slot connections across threads, you can't use the return values
 *If a function is called from the wrong thread and it has a return value -1 is returned
 *Investigate if this is really needed or if DCOP/DBUS is threadsafe anyway
 */
class KDECORE_EXPORT KToolInvocation : public QObject
{

  Q_OBJECT
private:
	KToolInvocation(QObject *parent);
	~KToolInvocation();
	static KToolInvocation *s_self;
	DCOPClient *m_dcopClient;
	static DCOPClient *dcopClient();
public:
	static KToolInvocation *self();
public slots:
  /**
   * Invokes the KHelpCenter HTML help viewer from docbook sources.
   *
   * @param anchor      This has to be a defined anchor in your
   *                    docbook sources. If empty the main index
   *                    is loaded
   * @param appname     This allows you to show the help of another
   *                    application. If empty the current name() is
   *                    used
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */

  static void invokeHelp( const QString& anchor = QString::null,
                   const QString& appname = QString::null,
		   const QByteArray& startup_id = "");

  /**
   * Convenience method; invokes the standard email application.
   *
   * @param address The destination address
   * @param subject Subject string. Can be QString::null.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  static void invokeMailer( const QString &address, const QString &subject,const QByteArray& startup_id = "" );

  /**
   * Invokes the standard email application.
   *
   * @param mailtoURL A mailto URL.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param allowAttachments whether attachments specified in mailtoURL should be honoured.
               The default is false; do not honour requests for attachments.
   */
  static void invokeMailer( const KURL &mailtoURL, const QByteArray& startup_id = "" ,  bool allowAttachments = false );

  /**
   * Convenience method; invokes the standard email application.
   *
   * All parameters are optional.
   *
   * @param to          The destination address.
   * @param cc          The Cc field
   * @param bcc         The Bcc field
   * @param subject     Subject string
   * @param body        A string containing the body of the mail (exclusive with messageFile)
   * @param messageFile A file (URL) containing the body of the mail (exclusive with body) - currently unsupported
   * @param attachURLs  List of URLs to be attached to the mail.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  static void invokeMailer(const QString &to, const QString &cc, const QString &bcc,
                    const QString &subject, const QString &body,
                    const QString &messageFile = QString::null, const QStringList &attachURLs = QStringList(),const QByteArray& startup_id = "" );

  /**
   * Invokes the standard browser.
   * Note that you should only do this when you know for sure that the browser can
   * handle the URL (i.e. its mimetype). In doubt, if the URL can point to an image
   * or anything else than directory or HTML, prefer to use new KRun( url ).
   *
   * @param url The destination address
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   */
  static void invokeBrowser( const QString &url, const QByteArray& startup_id = "" );

public:
  /**
   * Returns the DCOP name of the service launcher. This will be something like
   * klaucher_$host_$uid.
   * @return the name of the service launcher
   */
  static QByteArray launcher();

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param _name the name of the service
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByName( const QString& _name, const QString &URL,
                QString *error=0, QByteArray *dcopService=0, int *pid=0, const QByteArray &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param _name the name of the service
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByName( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QByteArray *dcopService=0, int *pid=0, const QByteArray &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param _name the path of the desktop file
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopPath( const QString& _name, const QString &URL,
                QString *error=0, QByteArray *dcopService=0, int *pid = 0, const QByteArray &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param _name the path of the desktop file
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopPath( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QByteArray *dcopService=0, int *pid = 0, const QByteArray &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param _name the desktop name of the service
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopName( const QString& _name, const QString &URL,
                QString *error=0, QByteArray *dcopService=0, int *pid = 0, const QByteArray &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param _name the desktop name of the service
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopName( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QByteArray *dcopService=0, int *pid = 0, const QByteArray &startup_id = "", bool noWait = false );

  /**
   * Starts a program via kdeinit.
   *
   * program name and arguments are converted to according to the
   * local encoding and passed as is to kdeinit.
   *
   * @param name Name of the program to start
   * @param args Arguments to pass to the program
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int kdeinitExec( const QString& name, const QStringList &args=QStringList(),
                QString *error=0, int *pid = 0, const QByteArray& startup_id = "" );

  /**
   * Starts a program via kdeinit and wait for it to finish.
   *
   * Like kdeinitExec(), but it waits till the program is finished.
   * As such it behaves similar to the system(...) function.
   *
   * @param name Name of the program to start
   * @param args Arguments to pass to the program
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int kdeinitExecWait( const QString& name, const QStringList &args=QStringList(),
                QString *error=0, int *pid = 0, const QByteArray& startup_id = "" );

};

#endif

