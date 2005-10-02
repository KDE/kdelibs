/*
    kimiface.h - KDE Instant Messenger DCOP Interface

    Copyright (c) 2004-5 Will Stephenson   <lists@stevello.free-online.co.uk>

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

#ifndef KIMIFACE_H
#define KIMIFACE_H

#include <qpixmap.h>
#include <dcopobject.h>
#include <qstringlist.h>
#include <kurl.h>

/**
 * @brief Generic DCOP interface for KDE instant messenger applications
 *
 * The interface has two parts:
 * - methods to get information about IM-contacts, such as their reachability
 *   or their presence status (if the are online or away, etc)
 * - methods to initiate communication with IM-contacts, e.g. sending messages
 *
 * @note If you are looking for a information about accessing application's
 * that implement this interface, have a look at the KIMProxy class.
 *
 * Contacts are identified using unique identifier strings (UID) used by
 * KABC, the KDE address book library.
 * The UID generation is handled by KABC::Addressee so the your application
 * will either have to access the address book or provide a possibility
 * for associating a contact of your application with an entry of the address
 * book.
 *
 * @note one omission of this interface is the lack of control over the range
 * of values used for protocols' names.
 *
 * If you are implementing this interface, note that your application must
 * have the following information in its desktop file, so that it can be
 * identified as providing KIMIface at runtime:
 * @code
 * X-DCOP-ServiceName=<application-name>
 * ServiceTypes=DCOP/InstantMessenger
 * @endcode
 * and the class implementing KIMIface must pass "KIMIface" to the DCOPObject constructor:
 * @code
 * // just need QObject inheritance and Q_OBJECT if you want signals and slots
 * // no need to use K_DCOP macro again
 *
 * class MyIMIface : public QObject, public KIMIface
 * {
 *     Q_OBJECT
 * public:
 *    MyIMIface(QObject* parent = 0, const char* name) :
 *        DCOPObject("KIMIface"), // <-- passing the interface name as required
 *        QObject(parent, name) {}
 * };
 * @endcode
 *
 * The DCOP part of the interface needs to be processed by the DCOP IDL
 * compiler. The KDE autotools framework will do this automatically, all
 * you have to do is add kimiface.skel and kimiface.stub to the
 * @c SOURCES list in your @c Makefile.am
 *
 * @see KIMProxy
 * @see KABC::AddressBook
 * @see KABC::Addressee
 *
 * @since 3.3
 * @author Will Stephenson <lists@stevello.free-online.co.uk>
 */
class KIMIface : virtual public DCOPObject
{
	K_DCOP

k_dcop:
// ACCESSORS
// contact list
	/**
	 * @brief Obtain a list of IM-contacts that are known to the application
	 *
	 * Return a list of KABC UIDs of all the contacts you have such IDs for.
	 *
	 * @return a list of KABC UIDs known to the application
	 *
	 * @see reachableContacts()
	 * @see onlineContacts()
	 * @see fileTransferContacts()
	 * @see isPresent()
	 * @see KABC::Addressee::uid()
	 */
	virtual QStringList allContacts() = 0;

	/**
	 * @brief Obtain a list of IM-contacts that are currently reachable
	 *
	 * Return a list of KABC UIDs of the contacts that are reachable in the
	 * sense that you are connected to the IM-service they are
	 * associated with.
	 *
	 * For example if your application supports ICQ and AIM and the ICQ account is
	 * active but the AIM account isn't, return just the ICQ contacts.
	 *
	 * @return a list of KABC UIDs who can receive a message, even if offline
	 *
	 * @see allContacts()
	 * @see onlineContacts()
	 * @see fileTransferContacts()
	 * @see messageContact()
	 * @see KABC::Addressee::uid()
	 */
	virtual QStringList reachableContacts() = 0;

	/**
	 * @brief Obtain a list of IM-contacts that are currently online
	 *
	 * Return a list of KABC UIDs of the contacts you have any presence
	 * information for that indicates that they are connected to the
	 * IM-service they are associated with.
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
	virtual QStringList onlineContacts() = 0;

	/**
	 * @brief Obtain a list of IM-contacts who may receive file transfers
	 *
	 * Return a list of KABC UIDs of the contacts that are capable of
	 * receiving file transfers based on the IM-service they are associated
	 * with, i.e. if it is technically able to provide this, on their online
	 * state, i.e. can likely not receive files while offline, and perhaps even
	 * information your application has additionally, e.g. a user config that
	 * tells you that the contact is behind a firewall.
	 *
	 * The simplest implementation is to return the same list as
	 * onlineContacts(), provided all the IM-services that are currently used
	 * support it.
	 *
	 * @return a list of KABC UIDs capable of file transfer
	 *
	 * @see allContacts()
	 * @see reachableContacts()
	 * @see onlineContacts()
	 * @see canReceiveFiles()
	 * @see sendFile()
	 * @see KABC::Addressee::uid()
	 */
	virtual QStringList fileTransferContacts() = 0;

// individual
	/**
	 * @brief Confirm if a given contact is known to the IM application
	 *
	 * Check if you can map the given KABC UID to one if the IM-contacts, e.g.
	 * the would be part of the list returned by allContacts()
	 *
	 * @param uid the KABC UID you are interested in
	 * @return whether the program knows of this KABC UID
	 *
	 * @see allContacts()
	 * @see presenceString()
	 * @see presenceStatus()
	 * @see KABC::Addressee::uid()
	 */
	virtual bool isPresent( const QString & uid ) = 0;

	/**
	 * @brief Obtain the IM app's idea of the contact's display name
	 *
	 * Useful if KABC lookups may be too slow. Should return whatever
	 * the application uses in its contact list or similar GUI, e.g.
	 * a nick name, a user configured name string, etc.
	 *
	 * @param uid the KABC UID you are interested in
	 * @return the corresponding display name or QString:null if the
	 *         UID is unknown
	 *
	 * @see isPresent()
	 * @see presenceString()
	 * @see presenceStatus()
	 * @see KABC::Addressee::uid()
	 */
	virtual QString displayName( const QString & uid ) = 0;

	/**
	 * @brief Obtain the IM presence as a i18ned string for the specified
	 *        contact
	 *
	 * Return a translated string your application would use when displaying
	 * the contact's presence, e.g. i18n("Online"), i18n("Away")
	 *
	 * @param uid the KABC UID you want the presence for
	 * @return the i18ned string describing the contact's presence or
	 *         QString::null if the UID is unknown
	 *
	 * @see isPresent()
	 * @see presenceStatus()
	 * @see KABC::Addressee::uid()
	 */
	virtual QString presenceString( const QString & uid ) = 0;

	/**
	 * @brief Obtain the IM presence as a number for the specified contact
	 *
	 * Return one of the following values depending on the given contact's
	 * presence:
	 * - 0 - @c Unknown: for contacts where you can not use any of the other
	 *   values
	 *
	 * - 1 - @c Offline: for contacts that are offline, i.e. not connected to
	 *   their IM-service. If the application itself or the IM-service for the
	 *   given contact is offline return @c Unknown instead
	 *
	 * - 2 - @c Connecting
	 *
	 * - 3 - @c Away: for contacts that are connected to their IM-service but
	 *   not @c Online
	 *
	 * - 4 - @c Online
	 *
	 * @param uid the KABC UID you want the presence for
	 * @return a numeric representation of presence - currently one of
	 *         0 (Unknown), 1 (Offline), 2 (Connecting), 3 (Away), 4 (Online).
	 *         Returns 0 if the given UID is unknown
	 *
	 * @see isPresent()
	 * @see presenceString()
	 * @see KABC::Addressee::uid()
	 */
	virtual int presenceStatus( const QString & uid ) = 0;

	/**
	 * @brief Indicate if a given contact can receive files
	 *
	 * @param uid the KABC UID you want to the file transfer capability for
	 * @return whether the specified contact can receive files
	 *
	 * @see fileTransferContacts()
	 * @see KABC::Addressee::uid()
	 */
	virtual bool canReceiveFiles( const QString & uid ) = 0;

	/**
	 * @brief Indicate if a given contact will be able to respond
	 *
	 * Some media are unidirectional (e.g., sending SMS via a web interface).
	 * This refers to the contact's ability to respond as defined by the
	 * medium, not by their presence.
	 *
	 * Someone may appear offline (SMS has no presence) to you but in fact be
	 * able to respond.
	 *
	 * @param uid the KABC UID you are interested in
	 * @return whether the specified contact can respond
	 *
	 * @see isPresent()
	 * @see KABC::Addressee::uid()
	 */
	virtual bool canRespond( const QString & uid ) = 0;

	/**
	 * @brief Obtain the KABC UID corresponding to the given IM address
	 *
	 * @param contactId the protocol specific identifier for the contact,
	 *        e.g. UIN for ICQ, screenname for AIM, nick for IRC
	 * @param protocol the IM protocol/service to check. See protocols()
	 * @return the KABC UID for the given contact or @c QString::null if not
	 *         found or either input stream was empty or the protocol is not
	 *         supported
	 *
	 * @see protocols()
	 * @see addContact()
	 * @see isPresent()
	 * @see KABC::Addressee::uid()
	 */
	virtual QString locate( const QString & contactId, const QString & protocol ) = 0;

// metadata
	/**
	 * @brief Obtain the icon representing the IM presence for the specified
	 *        contact
	 *
	 * Return the image the application would use to display a contact's presence.
	 * The size and other properties of the image are currently unspecified.
	 *
	 * @param uid the KABC UID you want the presence icon for
	 * @return a pixmap representing the contact's presence or a null pixmap
	 *         if the contact is unknown. See QPixmap::isNull()
	 *
	 * @see isPresent()
	 * @see presenceString()
	 * @see presenceStatus()
	 * @see KABC::Addressee::uid()
	 */
	virtual QPixmap icon( const QString & uid ) = 0;

	/**
	 * @brief Obtain the given contact's current context (home, work, or any)
	 *
	 * Not all IM services/protocols support the concept of contexts. If the
	 * given UID maps to such a service, just return @c QString::null
	 *
	 * @param uid the KABC UID you want the context for
	 * @return a string describing the context, or @c QString::null if not
	 *         supported or if the contact is unknown
	 *
	 * @see isPresent()
	 * @see KABC::Addressee::uid()
	 */
	virtual QString context( const QString & uid ) = 0;

// App capabilities
	/**
	 * @brief Obtain a list of supported IM services/protocols
	 *
	 * Protocol names are currently of the form "protocol name" + "Protocol"
	 * for example:
	 * - AIMProtocol: AOL instant messenger protocol
	 * - MSNProtocol: Microsoft messanger protocol
	 * - ICQProtocol: AOL (Mirabilis) ICQ protocol
	 * - ....
	 *
	 * The string is currently just an identifier to use with methods such as
	 * locate(), addContact() or messageNewContact()
	 *
	 * @return the set of protocols that the application supports
	 *
	 * @see locate()
	 * @see addContact()
	 * @see messageNewContact
	 */
	virtual QStringList protocols() = 0;

// ACTORS
	/**
	 * @brief Send a single message to the specified contact
	 *
	 * Any response will be handled by the IM client as a normal
	 * conversation.
	 *
	 * Implementations might send the message silently, ask the user for
	 * permission or just prefill the usual message input GUI.
	 *
	 * @note As sending any text could potentially be a breach of the user's
	 * privacy it is recommended to let the user know about it.
	 *
	 * @param uid the KABC UID you want to send the message to
	 * @param message the message text to send to the contact
	 *
	 * @see messageNewContact()
	 * @see chatWithContact()
	 * @see sendFile()
	 * @see isPresent()
	 * @see reachableContacts()
	 * @see KABC::Addressee::uid()
	 */
	virtual void messageContact( const QString &uid, const QString& message ) = 0;

	/**
	 * @brief Send a single message to a contact given only its protocol
	 *        specific identifier
	 *
	 * This could be used to send a message without having to know the KABC UID
	 * of the contact or without having to add it first.
	 * 
	 * @param contactId the protocol specific identifier for the contact,
	 *        e.g. UIN for ICQ, screenname for AIM, nick for IRC
	 * @param protocol the IM protocol/service to check. See protocols()
	 *
	 * @see messageContact()
	 * @see chatWithContact()
	 * @see sendFile()
	 * @see locate()
	 * @see protocols()
	 * @see addContact()
	 */
	virtual void messageNewContact( const QString &contactId, const QString &protocol ) = 0;

	/**
	 * @brief Start a chat session with the specified contact
	 *
	 * Applications that do not support a chat mode or when the IM-service
	 * of the given contact does not support it, this can also open
	 * a normal message input GUI.
	 *
	 * @param uid the KABC UID you want to chat with
	 *
	 * @see messageContact()
	 * @see messageNewContact()
	 * @see sendFile()
	 * @see isPresent()
	 * @see reachableContacts()
	 * @see KABC::Addressee::uid()
	 */
	virtual void chatWithContact( const QString &uid ) = 0;

	/**
	 * @brief Send a file to the contact
	 *
	 * Initiates a file transfer with the given contact if possible.
	 *
	 * Implementations might start the transfer right away, ask the user's
	 * permission or just prefill the usual file transfer GUI.
	 *
	 * @note As sending any file could potentially be a breach of the user's
	 * privacy it is recommended to let the user know about it.
	 *
	 * @param uid the KABC UID you want to send to
	 * @param sourceURL a KURL pointing to the file to send
	 * @param altFileName an alternate filename describing the file or a
	 *        description or title
	 * @param fileSize file size in bytes
	 *
	 * @see messageContact()
	 * @see messageNewContact()
	 * @see chatWithContact()
	 * @see isPresent()
	 * @see fileTransferContacts()
	 * @see KABC::Addressee::uid()
	 */
	virtual void sendFile(const QString &uid, const KURL &sourceURL,
		const QString &altFileName = QString::null, uint fileSize = 0) = 0;

// MUTATORS
// Contact list
	/**
	 * @brief Add a new contact given its protocol specific identifier
	 *
	 * Implementations might add the contact silently, including sending an
	 * authorization request if necessary, ask the user for confirmation or
	 * just prefill the usual contact addingGUI.
	 *
	 * @param contactId the protocol specific identifier for the contact
	 *        e.g. UIN for ICQ, screenname for AIM, nick for IRC
	 * @param protocol the IM protocol/service to use. See protocols()
	 * @return whether the add succeeded. @c false may signal already present,
	 *         protocol not supported, or add operation not supported.
	 *
	 * @see locate()
	 * @see protocols()
	 * @see messageNewContact()
	 */
	virtual bool addContact( const QString &contactId, const QString &protocol ) = 0;

// SIGNALS
k_dcop_signals:
	/**
	 * @brief Indicates that a contact's presence has changed
	 *
	 * Notifies connected DCOP receivers about a change in a contact's
	 * presence.
	 *
	 * Implementations just have to call this method with the appropriate
	 * values to get the DCOP signal emitted.
	 *
	 * @param uid the KABC UID whose presence changed
	 * @param appId the DCOP application ID of the program the signal
	 *        originates from
	 * @param presence the new presence's numeric value. See presenceStatus()
	 *
	 * @see presenceStatus()
	 * @see KABC::Addressee::uid()
	 * @see DCOPClient::appId()
	 */
	void contactPresenceChanged( QString uid, QCString appId, int presence );
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

