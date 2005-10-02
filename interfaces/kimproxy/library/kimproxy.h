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

#include <qdict.h>
#include <qmap.h>
#include <qptrdict.h>
#include <qstringlist.h>


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
typedef QMap<QCString, int> AppPresence; 		// appId->presence; contains all applications' ideas of a user's presence
typedef QDict<AppPresence> PresenceMap;			// uid->AppPresence; contains a AppPresences for all users
/** FIXME: remove presenceMap and call this presenceMap in KDE4.  This hack is for binary compatibility */
typedef QMap<QString, ContactPresenceListCurrent> PresenceStringMap;

/**
 * @brief Provides access to instant messenger programs which implement KDE's
 *        instant messanger interface KIMIface
 *
 * This class provides an easy-to-use interface to all running instant
 * messengers or chat programs that implement KIMIface.
 *
 * It works simultaneously with any running programs that implement the
 * ServiceType @c DCOP/InstantMessenger.
 *
 * IM-contacts are identified using unique identifier strings (UID) used by
 * KDE's address book framework KABC.
 * However the use of the address book API is optional, KIMProxy provides
 * methods to fetch lists of available contact UIDs from the applications.
 * If a UID is reachable with more than one program, KIMProxy aggregates the
 * available information and presents the 'best' presence.  For example, for a
 * contact who can be seen to be @c Away in @c IRC on program A but @c Online
 * using @c ICQ on program B, the information from program B will be used.
 *
 * KIMProxy is designed for simple information in a wide number of cases, not
 * for detailed messaging.
 *
 * The class is implemented as a singleton, so whenever you need access to
 * one of its methods, just retrieve the single instance using instance().
 * The following code snippet demonstrated how to send a message to all
 * reachable IM-contacts:
 * @code
 * // get proxy instance
 * KIMProxy* proxy = KIMProxy::instance();
 *
 * // check if there are suitable applications reachable
 * if (!proxy->initialize()) return;
 *
 * QString message = "Hi!\nHow are you on this wonderful day?";
 *
 * // iterate over the list of reachable contacts
 * QStringList reachableContacts = proxy->reachableContacts();
 * for (QStringList::const_iterator it = reachableContacts.begin();
 *      it != reachableContacts.end(); ++it)
 * {
       proxy->messageContact(*it, message);
 * }
 * @endcode
 *
 * @note While it is possible to communicate with each of those application's
 * by DCOP using KIMProxy has the advantage of communicating with all
 * applications simultaneously and thus getting an augmented view of
 * availability, capability and presence (online status) of IM-contacts.
 *
 * @see KIMIface
 * @see KABC::AddressBook
 * @see KABC::Addressee
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
		 * @brief Obtain an instance of KIMProxy
		 *
		 * Creates a new instance if it is called for the first time.
		 * Call initialize() to check if there are applications available for
		 * communication.
		 *
		 * @note KIMProxy uses DCOPClient::setNotifications() to make sure
		 * it updates its information about IM applications it interacts with,
		 * e.g. that it gets notified about newly available applications or
		 * about applications exiting
		 *
		 * @param client your application's DCOP client
		 * @return the singleton instance of this class
		 */
		static KIMProxy * instance( DCOPClient * client );

		/**
		 * @brief Get the proxy ready to connect
		 *
		 * Discover any running IM clients and setup communication handlers for
		 * them. The list of available clients is updated automatically
		 * through notification about added or removed DCOP applications as
		 * provided by DCOPClient.
		 *
		 * It is safe to call this method more than once, initialization only
		 * occurs at the first time.
		 *
		 * @return whether the proxy is ready to use.
		 *         @c false if there are no suitable apps running
		 */
		bool initialize();

		/**
		 * @brief Obtain a list of IM-contacts known to IM-applications
		 *
		 * Returns a list of KABC UIDs gathered by merging the lists of
		 * each application the proxy can communicate with.
		 * The method uses cached information to make it fast and not
		 * require actually communicating with the applications every time
		 * it is called.
		 *
		 * @return a list of KABC UIDs known to any IM-application
		 *
		 * @see reachableContacts()
		 * @see onlineContacts()
		 * @see fileTransferContacts()
		 * @see isPresent()
		 * @see KABC::Addressee::uid()
		 */
		QStringList allContacts();

		/**
		 * @brief Obtain a list of IM-contacts that are currently reachable
		 *
		 * Returns a list of KABC UIDs gathered by merging the lists of
		 * each application the proxy can communicate with.
		 *
		 * @return a list of KABC UIDs who can receive a message, even if offline
		 *
		 * @see allContacts()
		 * @see onlineContacts()
		 * @see fileTransferContacts()
		 * @see messageContact()
		 * @see KABC::Addressee::uid()
		 */
		QStringList reachableContacts();

		/**
		 * @brief Obtain a list of IM-contacts that are currently online
		 *
		 * Returns a list of KABC UIDs gathered by merging the lists of
		 * each application the proxy can communicate with.
		 * The method uses cached information to make it fast and not
		 * require actually communicating with the applications every time
		 * it is called.
		 *
		 * @return a list of KABC UIDs who are online with unspecified presence
		 *
		 * @see allContacts()
		 * @see reachableContacts()
		 * @see fileTransferContacts()
		 * @see messageContact()
		 * @see chatWithContact()
		 * @see KABC::Addressee::uid()
		 */
		QStringList onlineContacts();

		/**
		 * @brief Obtain a list of IM-contacts who may receive file transfers
		 *
		 * Returned IM-contacts might not be capable of receiving file
		 * tranfers due to limitations of their clients or because they are
		 * behind a firewall.
		 * KIMProxy doesn't have the possibilities to discover this and will
		 * list them as well.
		 *
		 * Returns a list of KABC UIDs gathered by merging the lists of
		 * each application the proxy can communicate with.
		 *
		 * @return a list of KABC UIDs capable of file transfer
		 *
		 * @see allContacts()
		 * @see reachableContacts()
		 * @see onlineContacts()
		 * @see canReceiveFiles()
		 * @see KABC::Addressee::uid()
		 */
		QStringList fileTransferContacts();

		/**
		 * @brief Confirm if a given contact is known to the proxy
		 *
		 * A contact is known to the proxy if it is know to at least one
		 * IM-application the proxy is communicating with.
		 *
		 * The method uses cached information to make it fast and not
		 * require actually communicating with the applications every time
		 * it is called.
		 *
		 * @param uid the KABC UID you are interested in
		 * @return whether any IM-program knows of this KABC UID
		 *
		 * @see allContacts()
		 * @see presenceString()
		 * @see presenceNumeric()
		 * @see KABC::Addressee::uid()
		 */
		bool isPresent( const QString& uid );

		/**
		 * @brief Obtain the proxy's idea of the contact's display name
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and @c Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * Useful if KABC lookups may be too slow.
		 *
		 * @param uid the KABC UID you are interested in
		 * @return the corresponding display name or QString:null if the
		 *         UID is unknown
		 *
		 * @see isPresent()
		 * @see presenceString()
		 * @see presenceNumeric()
		 * @see KABC::Addressee::uid()
		 */
		QString displayName( const QString& uid );

		/**
		 * @brief Obtain the IM presence as a number for the specified contact
		 *
		 * Returns one of the following values depending on the given contact's
		 * presence:
		 * - 0 - @c Unknown: for contacts where you can not use any of the
		 *   other values
		 *
		 * - 1 - @c Offline: for contacts that are offline, i.e. not connected
		 *   to their IM-service.
		 *
		 * - 2 - @c Connecting
		 *
		 * - 3 - @c Away: for contacts that are connected to their IM-service
		 *   but not @c Online
		 *
		 * - 4 - @c Online
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * The method uses cached information to make it fast and not
		 * require actually communicating with the applications every time
		 * it is called.
		 *
		 * @param uid the KABC UID you want the presence for
		 * @return a numeric representation of presence - currently one of
		 *         0 (Unknown), 1 (Offline), 2 (Connecting), 3 (Away),
		 *         4 (Online). Returns 0 if the given UID is unknown
		 *
		 * @see isPresent()
		 * @see presenceString()
		 * @see presenceIcon()
		 * @see KIMIface::presenceStatus()
		 * @see KABC::Addressee::uid()
		 */
		int presenceNumeric( const QString& uid );

		/**
		 * @brief Obtain the IM presence as a i18ned string for the specified
		 *        contact
		 *
		 * The presence string is one of the following:
		 * - i18n("Unknown")
		 * - i18n("Offline")
		 * - i18n("Connecting")
		 * - i18n("Away")
		 * - i18n("Online")
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * @note The presence string is created based on the numerical
		 *       presence value returned by the applications. It currently
		 *       does not return the presence strings used by the applications.
		 *
		 * The method uses cached information to make it fast and not
		 * require actually communicating with the applications every time
		 * it is called.
		 *
		 * @param uid the KABC UID you want the presence for
		 * @return the i18ned string describing the contact's presence or
		 *         QString::null if the UID is unknown
		 *
		 * @see isPresent()
		 * @see presenceNumeric()
		 * @see presenceIcon()
		 * @see KABC::Addressee::uid()
		 */
		QString presenceString( const QString& uid );

		/**
		 * @brief Obtain the icon representing the IM presence for the
		 *        specified contact
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * @note The presence icon is chosen based on the numerical
		 *       presence value returned by the applications. It currently
		 *       does not return the presence icon used by the applications.
		 *
		 * The method uses cached information to make it fast and not
		 * require actually communicating with the applications every time
		 * it is called.
		 *
		 * @param uid the KABC UID you want the presence icon for
		 * @return a pixmap representing the contact's presence or a null
		 *         pixmap if the contact is unknown. See QPixmap::isNull()
		 *
		 * @see isPresent()
		 * @see presenceString()
		 * @see presenceNumeric()
		 * @see KABC::Addressee::uid()
		 */
		QPixmap presenceIcon( const QString& uid );

		/**
		 * @brief Indicate if a given contact can receive files
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * @param uid the KABC UID you want to the file transfer capability for
		 * @return whether the specified contact can receive files
		 *
		 * @see fileTransferContacts()
		 * @see KABC::Addressee::uid()
		 */
		bool canReceiveFiles( const QString & uid );

		/**
		 * @brief Indicate if a given contact will be able to respond
		 *
		 * Some media are unidirectional (e.g., sending SMS via a web
		 * interface).
		 * This refers to the contact's ability to respond as defined by the
		 * medium, not by their presence.
		 *
		 * Someone may appear offline (SMS has no presence) to you but in fact
		 * be able to respond.
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * @param uid the KABC UID you are interested in
		 * @return whether the specified contact can respond
		 *
		 * @see isPresent()
		 * @see KABC::Addressee::uid()
		 */
		bool canRespond( const QString & uid );

		/**
		 * @brief Obtain the KABC UID corresponding to the given IM address
		 *
		 * @param contactId the protocol specific identifier for the contact,
		 *        e.g. UIN for ICQ, screenname for AIM, nick for IRC
		 * @param protocol the IM protocol/service to check.
		 *        See KIMIface::protocols()
		 * @return the KABC UID for the given contact or @c QString::null if
		 *         not found or either input stream was empty or the protocol
		 *         is not supported
		 *
		 * @see KIMIface::protocols()
		 * @see addContact()
		 * @see isPresent()
		 * @see KABC::Addressee::uid()
		 */
		QString locate( const QString & contactId, const QString & protocol );

		/**
		 * @brief Obtain the given contact's current context (home, work, or
		 *        any)
		 *
		 * Not all IM services/protocols support the concept of contexts. If the
		 * given UID maps to such a service, @c QString::null will be returned
		 *
		 * If the given KABC UID is known to more than one IM-application
		 * the result of the application which has the best presence for the
		 * contact is taken.
		 * For example if a contact is @c Online on ICQ and Away on AIM, the
		 * value returned by the application providing ICQ is taken.
		 *
		 * @param uid the KABC UID you want the context for
		 * @return a string describing the context, or @c QString::null if not
		 *         supported or if the contact is unknown
		 *
		 * @see isPresent()
		 * @see KABC::Addressee::uid()
		 */
		QString context( const QString & uid );

		/**
		 * @brief Start a chat session with the specified contact
		 *
		 * @param uid the KABC UID you want to chat with
		 *
		 * @see messageContact()
		 * @see sendFile()
		 * @see isPresent()
		 * @see reachableContacts()
		 * @see KABC::Addressee::uid()
		 */
		void chatWithContact( const QString& uid );

		/**
		 * @brief Send a single message to the specified contact
		 *
		 * Any response will be handled by the IM client as a normal
		 * conversation.
		 *
		 * @param uid the KABC UID you want to send the message to
		 * @param message the message text to send to the contact
		 *
		 * @see chatWithContact()
		 * @see sendFile()
		 * @see isPresent()
		 * @see reachableContacts()
		 * @see KABC::Addressee::uid()
		 */
		void messageContact( const QString& uid, const QString& message );

		/**
		 * @brief Send a file to the contact
		 *
		 * Initiates a file transfer with the given contact if possible.
		 *
		 * @param uid the KABC UID you want to send to
		 * @param sourceURL a KURL pointing to the file to send
		 * @param altFileName an alternate filename describing the file or a
		 *        description or title
		 * @param fileSize file size in bytes
		 *
		 * @see messageContact()
		 * @see chatWithContact()
		 * @see isPresent()
		 * @see fileTransferContacts()
		 * @see KABC::Addressee::uid()
		 */
		void sendFile(const QString &uid, const KURL &sourceURL,
			 const QString &altFileName = QString::null, uint fileSize = 0);

		/**
		 * @brief Add a new contact given its protocol specific identifier
		 *
		 * @param contactId the protocol specific identifier for the contact
		 *        e.g. UIN for ICQ, screenname for AIM, nick for IRC
		 * @param protocol the IM protocol/service to use.
		 *        See KIMIface:::protocols()
		 * @return whether the add succeeded. @c false may signal already present,
		 *         protocol not supported, or add operation not supported.
		 *
		 * @see locate()
		 * @see KIMIface::protocols()
		 */
		bool addContact( const QString &contactId, const QString &protocol );

		/**
		 * @brief Checks if there are any compatible instant messaging
		 *        applications available
		 *
		 * Available means that they are started and registered with DCOP
		 * and implementing the correct DCOP service.
		 * This information will be updated on the first call to initialize()
		 * and whenever an application registers or unregisters with DCOP,
		 * i.e. the information will be kept up to date.
		 *
		 * @return @c true if there are any apps available
		 */
		bool imAppsAvailable();

		/**
		 * @brief Start the user's preferred IM application
		 * @return whether a preferred app was found. No guarantee that it
		 *         started correctly
		 */
		bool startPreferredApp();

		/**
		 * Just exists to let the IDL compiler make the DCOP signal for this
		 */
		void contactPresenceChanged( QString uid, QCString appId, int presence );

	public slots:
		/**
		 * @brief Updates the proxy's data after a new application registered
		 *        with DCOP
		 *
		 * Checks if the application specified by the given DCOP application
		 * identifier implements the instant messenger service.
		 *
		 * @param appId the DCOP application ID of the newly registered
		 *        application
		 *
		 * @see DCOPClient::applicationRegistered()
		 */
		void registeredToDCOP( const QCString& appId );

		/**
		 * @brief Updates the proxy's data after an application unregistered
		 *        with DCOP
		 *
		 * If the application specified by the given DCOP application
		 * identifier is one of the instant messenger applications of the
		 * proxy, it will remove the presence information it gathered from it
		 * earlier on.
		 *
		 * Emits sigPresenceInfoExpired() to let the using applcation know
		 * its presence related information might need updating.
		 *
		 * @param appId the DCOP application ID of the now unregistered
		 *        application
		 *
		 * @see DCOPClient::applicationRemoved()
		 */
		void unregisteredFromDCOP( const QCString& appId );
	signals:
		/**
		 * @brief Indicates that the specified IM-contact's presence changed
		 *
		 * @param uid the KABC UID whose presence changed
		 *
		 * @see isPresent()
		 * @see presenceNumeric()
		 * @see presenceIcon()
		 * @see KABC::Addressee::uid()
		 */
		void sigContactPresenceChanged( const QString &uid );

		/**
		 * @brief Indicates that presence information obtained earlier on might
		 *        not be valid any longer
		 *
		 * After the sources of presence information have changed so any
		 * previously supplied presence info is invalid.
		 */
		void sigPresenceInfoExpired();
	protected:
		/**
		 * Bootstrap our presence data for a newly registered app
		 */
		void pollApp( const QCString & appId );
		/**
		 * Bootstrap our presence data by polling all known apps
		 */
		void pollAll( const QString &uid );

		/**
		 * Update our records with the given data
		 */
		bool updatePresence( const QString &uid, const QCString &appId, int presence );

		/**
		 * Get the name of the user's IM application of choice
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
		QDict<KIMIface_stub> m_im_client_stubs;
		// map containing numeric presence and the originating application ID for each KABC uid we know of
		// KABC Uid -> (appId, numeric presence )(AppPresence)
		PresenceMap m_presence_map;
		// cache of the client strings in use by each application
		// dictionary of KIMIface_stub -> map of numeric presence -> string presence
		// FIXME: remove for KDE4 - UNUSED but maintained for binary compatibility in KDE 3.4
		QPtrDict<int> m_client_presence_strings;
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

