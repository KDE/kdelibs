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

class QIODevice;
class KSSLPrivate;
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
	KSSLSettings * settings();

private:
	static bool m_bSSLWorks;
	bool m_bInit;
	bool m_bAutoReconfig;
	KSSLSettings *m_cfg;

	KSSLPrivate *d;
};


#endif

