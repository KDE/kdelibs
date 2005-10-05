/*
	kimproxy.h

	IM service library for KDE

	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIMPROXY_H
#define KIMPROXY_H

#include <q3dict.h>
#include <qmap.h>
#include <q3ptrdict.h>
#include <qstringlist.h>
#include <q3cstring.h>
#include <qpixmap.h>

#define IM_SERVICE_TYPE "DCOP/InstantMessenger"
#define IM_CLIENT_PREFERENCES_FILE "default_components"
#define IM_CLIENT_PREFERENCES_SECTION "InstantMessenger"
#define IM_CLIENT_PREFERENCES_ENTRY "imClient"

#include "kimproxyiface.h"

class DCOPClient;
class KIMIface_stub;
class KURL;
class ContactPresenceListCurrent;

/** FIXME: remove for KDE4, binary compability again. */
typedef QMap<Q3CString, int> AppPresence; 		// appId->presence; contains all applications' ideas of a user's presence
typedef Q3Dict<AppPresence> PresenceMap;			// uid->AppPresence; contains a AppPresences for all users
/** FIXME: remove presenceMap and call this presenceMap in KDE4.  This hack is for binary compatibility */
typedef QMap<QString, ContactPresenceListCurrent> PresenceStringMap;

/**
 * This class provides an easy-to-use interface to any instant messengers or chat programs
 * that you have installed that implement KIMIface
 *
 * It works simultaneously with any running programs that implement the ServiceType DCOP/InstantMessenger
 * If a UID is reachable with more than one program, KIMProxy aggregates the available information and presents
 * the 'best' presence.  For example, for a contact who can be seen to be Away in IRC on program A but Online using
 * ICQ on program B, the information from program B will be used.  KIMProxy is designed for simple information in
 * a wide number of cases, not for detailed messaging.
 *
 * Most operations work in terms of uids belonging to KABC::Addressee, but use of the
 * address book is optional.  You can get a list of known contacts with imAddresseeUids
 * and then check their presence using the various accessor methods presenceString, presenceNumeric
 * and display the IM programs' display names for them using displayName.
 *
 * To use, just get an instance using @ref instance.
 *
 * @since 3.3
 * @author Will Stephenson <lists@stevello.free-online.co.uk>
 */
class KIMPROXY_EXPORT KIMProxy : public QObject, virtual public KIMProxyIface
{
	Q_OBJECT
	struct Private;

	template<class> friend class KStaticDeleter;
	~KIMProxy();

	public:
		/**
		 * Obtain an instance of KIMProxy.
		 * Note, if you share this DCOPClient with your own app,
		 * that kimproxy uses DCOPClient::setNotifications() to make sure
		 * it updates its information when the IM application it is interfacing to
		 * exits.
		 * @param client your app's DCOP client.
		 * @return The singleton instance of this class.
		 */
		static KIMProxy * instance( DCOPClient * client );

		/**
		 * Get the proxy ready to connect
		 * Discover any running preferred IM clients and set up stubs for it
		 * @return whether the proxy is ready to use.  False if there are no apps running.
		 */
		bool initialize();

		/**
		 * Obtain a list of IM-contactable entries in the KDE
		 * address book.
		 * @return a list of KABC uids.
		 */
		QStringList allContacts();

		/**
		 * Obtain a list of  KDE address book entries who are
		 * currently reachable.
		 * @return a list of KABC uids who can receive a message, even if online.
		 */
		QStringList reachableContacts();

		/**
		 * Obtain a list of  KDE address book entries who are
		 * currently online.
		 * @return a list of KABC uids who are online with unspecified presence.
		 */
		QStringList onlineContacts();

		/**
		 * Obtain a list of  KDE address book entries who may
		 * receive file transfers.
		 * @return a list of KABC uids capable of file transfer.
		 */
		QStringList fileTransferContacts();

		/**
		 * Confirm if a given KABC uid is known to KIMProxy
		 * @param uid the KABC uid you are interested in.
		 * @return whether one of the chat programs KIMProxy talks to knows of this KABC uid.
		 */
		bool isPresent( const QString& uid );

		/**
		 * Obtain the IM app's idea of the contact's display name
		 * Useful if KABC lookups may be too slow
		 * @param uid the KABC uid you are interested in.
		 * @return The corresponding display name.
		 */
		QString displayName( const QString& uid );

		/**
		 * Obtain the IM presence as a number (see KIMIface) for the specified addressee
		 * @param uid the KABC uid you want the presence for.
		 * @return a numeric representation of presence - currently one of 0 (Unknown), 1 (Offline), 2 (Connecting), 3 (Away), 4 (Online)
		 */
		int presenceNumeric( const QString& uid );

		/**
		 * Obtain the IM presence as a i18ned string for the specified addressee
		 * @param uid the KABC uid you want the presence for.
		 * @return the i18ned string describing presence.
		 */
		QString presenceString( const QString& uid );

		/**
		 * Obtain the icon representing IM presence for the specified addressee
		 * @param uid the KABC uid you want the presence for.
		 * @return a pixmap representing the uid's presence.
		 */
		QPixmap presenceIcon( const QString& uid );

		/**
		 * Indicate if a given uid can receive files
		 * @param uid the KABC uid you are interested in.
		 * @return Whether the specified addressee can receive files.
		 */
		bool canReceiveFiles( const QString & uid );

		/**
		 * Some media are unidirectional (eg, sending SMS via a web interface).
		 * @param uid the KABC uid you are interested in.
		 * @return Whether the specified addressee can respond.
		 */
		bool canRespond( const QString & uid );

		/**
		 * Get the KABC uid corresponding to the supplied IM address
		 * Protocols should be
		 * @param contactId the protocol specific identifier for the contact, eg UIN for ICQ, screenname for AIM, nick for IRC.
		 * @param protocol the protocol, eg one of "AIMProtocol", "MSNProtocol", "ICQProtocol",
		 * @return a KABC uid or null if none found/
		 */
		QString locate( const QString & contactId, const QString & protocol );

		/**
		 * Get the supplied addressee's current context (home, work, or any).
		 * @param uid the KABC uid you want the context for.
		 * @return A QString describing the context, or null if not supported.
		 */
		QString context( const QString & uid );

		/**
		* Start a chat session with the specified addressee
		* @param uid the KABC uid you want to chat with.
		*/
		void chatWithContact( const QString& uid );

		/**
		 * Send a single message to the specified addressee
		 * Any response will be handled by the IM client as a normal
		 * conversation.
		 * @param uid the KABC uid you want to chat with.
		 * @param message the message to send them.
		 */
		void messageContact( const QString& uid, const QString& message );

		/**
		 * Send the file to the contact
		 * @param uid the KABC uid you are sending to.
		 * @param sourceURL a KURL to send.
		 * @param altFileName an alternate filename describing the file
		 * @param fileSize file size in bytes
		 */
		void sendFile(const QString &uid, const KURL &sourceURL, const QString &altFileName = QString::null, uint fileSize = 0);

		/**
		 * Add a contact to the contact list
		 * @param contactId the protocol specific identifier for the contact, eg UIN for ICQ, screenname for AIM, nick for IRC.
		 * @param protocol the protocol, eg one of "AIMProtocol", "MSNProtocol", "ICQProtocol",
		 * @return whether the add succeeded.  False may signal already present, protocol not supported, or add operation not supported.
		 */
		bool addContact( const QString &contactId, const QString &protocol );

		/**
		 * Are there any compatible instant messaging apps installed?
		 * @return true if there are any apps installed, to see if they are running, @ref initialize instead.
		 */
		bool imAppsAvailable();

		/**
		 * Start the user's preferred IM application
		 * @return whether a preferred app was found.  No guarantee that it started correctly
		 */
		bool startPreferredApp();

		/**
		 * Just exists to let the idl compiler make the DCOP signal for this
		 */
		void contactPresenceChanged( QString uid, DCOPCString appId, int presence );

	public slots:
		void registeredToDCOP( const QByteArray& appId );
        void unregisteredFromDCOP( const QByteArray& appId );
	signals:
		/**
		 * Indicates that the specified UID's presence changed
		 * @param uid the KABC uid whose presence changed.
		 */
		void sigContactPresenceChanged( const QString &uid );

		/**
		 * Indicates that the sources of presence information have changed
		 * so any previously supplied presence info is invalid.
		 */
		void sigPresenceInfoExpired();
	protected:
		/**
		 * Bootstrap our presence data for a newly registered app
		 */
		void pollApp( const DCOPCString & appId );
		/**
		 * Bootstrap our presence data by polling all known apps
		 */
		void pollAll( const QString &uid );

		/**
		 * Update our records with the given data
		 */
		bool updatePresence( const QString &uid, const DCOPCString &appId, int presence );

		/**
		 * Get the name of the user's IM weapon of choice
		 */
		QString preferredApp();

		/**
		 * Get the app stub best able to reach this uid
		 */
		KIMIface_stub * stubForUid( const QString &uid );

		/**
		 * Get the app stub for this protocol.
		 * Take the preferred app first, then any other.
		 */
		KIMIface_stub * stubForProtocol( const QString &protocol );

	private:
		// client stubs used to get presence
		// appId (from DCOP) -> KIMIface_stub
		Q3Dict<KIMIface_stub> m_im_client_stubs;
		// map containing numeric presence and the originating application ID for each KABC uid we know of
		// KABC Uid -> (appId, numeric presence )(AppPresence)
		PresenceMap m_presence_map;
		// cache of the client strings in use by each application
		// dictionary of KIMIface_stub -> map of numeric presence -> string presence
		// FIXME: remove for KDE4 - UNUSED but maintained for binary compatibility in KDE 3.4
		Q3PtrDict<int> m_client_presence_strings;
		Private * d;
		bool m_apps_available;
		bool m_initialized;
		/**
		 * Construct an instance of the proxy library.
		 */
		KIMProxy( DCOPClient * client);
		static KIMProxy * s_instance;
};

#endif

