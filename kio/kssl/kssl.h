/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2005 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KSSL_H
#define _KSSL_H

#include <ksslsettings.h>
#include <ksslpeerinfo.h>
#include <ksslconnectioninfo.h>

class QIODevice;
class KSSLPrivate;
class KSSLCertificate;
class KSSLPKCS12;
class KSSLSession;

/**
 * KDE SSL Wrapper Class
 *
 * This class implements KDE's SSL support by wrapping OpenSSL.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KExtendedSocket, TCPSlaveBase
 * @short KDE SSL Class
 */
class KIO_EXPORT KSSL {
public:
	/**
	 *  Construct a KSSL object
	 *
	 *  @param init Set this to false if you do not want this class to
	 *         immediately initialize OpenSSL.
	 */
	KSSL(bool init = true);

	/**
	 *  Destroy this KSSL object
	 *
	 *  Does not close any socket.
	 */
	~KSSL();

	/**
	 *  Determine if SSL is available and works.
	 *
	 *  @return true is SSL is available and usable
	 */
	static bool doesSSLWork();

	/**
	 *  Initialize OpenSSL.
	 *
	 *  @return true on success
	 *
	 *  This will do nothing if it is already initialized.
	 *  @see reInitialize
	 */
	bool initialize();

	/**
	 *  This is used for applicationss which do STARTTLS or something
	 *  similar. It creates a TLS method regardless of the user's settings.
	 *
	 *  @return true if TLS is successfully initialized
	 */
	bool TLSInit();

	/**
	 *  Set an SSL session to use.  This deep copies the session so it
	 *  doesn't have to remain valid.  You need to call it after calling
	 *  initialize or reInitialize.  The ID is cleared in close().
	 *
	 *  @param session A valid session to reuse.  If 0L, it will clear the
	 *                 session ID in memory.
	 *
	 *  @return true on success
	 */
	bool setSession(const KSSLSession *session);

	/**
	 *  Close the SSL session.
	 */
	void close();

	/**
	 *  Reinitialize OpenSSL.
	 *
	 *  @return true on success
	 *
	 *  This is not generally needed unless you are reusing the KSSL object
	 *  for a new session.
	 *  @see initialize
	 */
	bool reInitialize();

	/**
	 *  Trigger a reread of KSSL configuration and reInitialize() KSSL.
	 *
	 *  @return true on successful reinitalizations
	 *
	 *  If you setAutoReconfig() to false, then this will simply
	 * reInitialize() and not read in the new configuration.
	 *  @see setAutoReconfig
	 */
	bool reconfig();

	/**
	 *  Enable or disable automatic reconfiguration on initialize().
	 *
	 *  @param ar Set to false in order to disable auto-reloading of the
	 *         KSSL configuration during initialize().
	 *
	 *  By default, KSSL will read its configuration on initialize().  You
	 *  might want to disable this for performance reasons.
	 */
	void setAutoReconfig(bool ar);

	/**
	 *  This will reseed the pseudo-random number generator with the EGD
	 *  (entropy gathering daemon) if the EGD is configured and enabled.
	 *  You don't need to call this yourself normally.
	 *
	 *  @return 0 on success
	 */
	int seedWithEGD();

	/**
	 *  Set a new KSSLSettings instance as the settings. This deletes the
	 *  current instance of KSSLSettings.
	 *
	 *  @param settings A new, valid settings object.
	 *
	 *  @return true on success
	 */
	bool setSettings(KSSLSettings *settings);

	/**
	 *  One is built by the constructor, so this will only return a NULL
	 *  pointer if you set one with setSettings().
	 *
	 *  @return the current settings instance
	 */
	KSSLSettings * settings() { return m_cfg; }

	/**
	 *  Use this to set the certificate to send to the server.
	 *  Do NOT delete the KSSLPKCS12 object until you are done with the
	 *  session. It is not defined when KSSL will be done with this.
	 *
	 *  @param pkcs the valid PKCS#12 object to send.
	 *
	 *  @return true if the certificate was properly set to the session.
	 */
	bool setClientCertificate(KSSLPKCS12 *pkcs);

	/**
	 *  Set the peer hostname to be used for certificate verification.
	 *
	 *  @param realHost the remote hostname as the user believes to be
	 *         connecting to
	 */
	void setPeerHost(const QString& realHost = QString());

	/**
	 *  Connect the SSL session to the remote host using the provided
	 *  socket descriptor.
	 *
	 *  @param sock the socket descriptor to connect with.  This must be
	 *         an already connected socket.
	 *  @return 1 on success, 0 on error setting the file descriptor,
	 *          -1 on other error.
	 */
	int connect(int sock);
        int connect(QIODevice* sock);

	/**
	 *  Connect the SSL session to the remote host using the provided
	 *  socket descriptor.  This is for use with an SSL server application.
	 *
	 *  @param sock the socket descriptor to connect with.  This must be
	 *         an already connected socket.
	 *  @return 1 on success, 0 on error setting the file descriptor,
	 *          -1 on other error.
	 */
	int accept(int sock);
        int accept(QIODevice* sock);

	/**
	 *  Read data from the remote host via SSL.
	 *
	 *  @param buf the buffer to read the data into.
	 *  @param len the maximum length of data to read.
	 *  @return the number of bytes read, 0 on an exception, or -1 on error.
	 */
	int read(char *buf, int len);

	/**
	 *  Peek at available data from the remote host via SSL.
	 *
	 *  @param buf the buffer to read the data into.
	 *  @param len the maximum length of data to read.
	 *  @return the number of bytes read, 0 on an exception, or -1 on error.
	 */
	int peek(char *buf, int len);

	/**
	 *  Write data to the remote host via SSL.
	 *
	 *  @param buf the buffer to read the data from.
	 *  @param len the length of data to send from the buffer.
	 *  @return the number of bytes written, 0 on an exception,
	 *          or -1 on error.
	 */
	int write(const char *buf, int len);

	/**
	 *  Determine if data is waiting to be read.
	 *
	 *  @return -1 on error, 0 if no data is waiting, > 0 if data is waiting.
	 */
	int pending();

	/**
	 *  Obtain a reference to the connection information.
	 *
	 *  @return a reference to the connection information,
	 *          valid after connected
	 *  @see KSSLConnectionInfo
	 */
	KSSLConnectionInfo& connectionInfo();

	/**
	 *  Obtain a reference to the information about the peer.
	 *
	 *  @return a reference to the peer information,
	 *          valid after connected
	 *  @see KSSLPeerInfo
	 */
	KSSLPeerInfo& peerInfo();

	/**
	 *  Obtain a pointer to the session information.
	 *
	 *  @return a pointer to the session information.
	 *          This is valid after connected, while connected.
	 *          It is deleted by the KSSL object which returns it.
	 *          May return 0L if no valid session exists.
	 *  @see KSSLSession
	 */
	const KSSLSession* session() const;

	/**
	 *  Determine if we are currently reusing an SSL session ID.
	 *
	 *  @return true if we are reusing a session ID.
	 */
	bool reusingSession() const;

private:
	static bool m_bSSLWorks;
	bool m_bInit;
	bool m_bAutoReconfig;
	KSSLSettings *m_cfg;
	KSSLConnectionInfo m_ci;
	KSSLPeerInfo m_pi;

	KSSLPrivate *d;

	void setConnectionInfo();
	void setPeerInfo();
};


#endif

