/*
    kimiface.h - KDE Instant Messenger DCOP Interface
	
	Refactored out of
	
	kopeteiface.h - Kopete DCOP Interface

    Copyright (c) 2002 by Hendrik vom Lehn       <hennevl@hennevl.de>
			  (c) 2004    Will Stephenson	 	 <lists@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003     by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIMIface_h
#define KIMIface_h

#include <qpixmap.h>
#include <dcopobject.h>
#include <qstringlist.h>
#include <kurl.h>

/**
 * Generic DCOP interface for KDE instant messenger applications
 */
class KIMIface : virtual public DCOPObject
{
	K_DCOP

/*public:
	KIMIface();
*/
k_dcop:
// ACCESSOR METHODS
// classes of contacts
	virtual QStringList contacts() = 0;
	virtual QStringList reachableContacts() = 0;
	virtual QStringList onlineContacts() = 0;
	virtual QStringList fileTransferContacts() = 0;

	/**
	 * Get a list of KABC uids who use IM.  The list contains no status info
	 * @return list of KABC uids
	 */
	virtual QStringList imAddresseeUids() = 0;

// all contacts' status
	// FIXME: Do we *need* this one? Sounds error prone to me, because
	// nicknames can contain parentheses too.
	// Better add a contactStatus( const QString id ) I'd say - Martijn
	virtual QStringList contactsStatus() = 0;

// individual status
	/**
	 * Describe the status of a contact by their metaContactId,
	 * aka their uid in KABC.
	 * @return i18n'ed string describing online status
	 */
	virtual QString onlineStatus( const QString &metaContactId ) = 0;

	/**
	 * Describe the status of a contact by their metaContactId,
	 * aka their uid in KABC.
	 * @return int representing online status -1 = unknown contact, 0 = unknown status, 1 = Offline, 2 = Connecting, 3 = Away, 4 = Online
	 */
	virtual int onlineStatusNumeric( const QString &metaContactId ) = 0;

	/** 
	 * Get the current icon for a contact
	 * @param UID the UID you want an icon for
	 * @return the icon
	 */
	virtual QPixmap statusIcon ( const QString &metaContactId ) = 0;
	
// individual info
	/** 
	 * Get the display name for a contact
	 * Probably not needed.  If we know it is in KABC, we can look it up there, unless we're a bash script and KABC has no DCOP interface...
	 * @param UID the contact you want the name for
	 * @return the display name
	 */
	virtual QString displayName( const QString &metaContactId ) = 0;
	 
// available IM accounts
	/**
	 * return a list of alls accounts.
	 * form: XXXProtocol||AccountId
	 */
	virtual QStringList accounts() = 0;

// protocols usable for file transfer to a contact
	virtual QStringList contactFileProtocols(const QString &displayName) = 0 ;

// ACTOR METHODS	
	/**
	 * Open a chat to a contact, and optionally set some initial text
	 */
	virtual void messageContact( const QString &displayName, const QString &messageText = QString::null ) = 0;

	/**
	 * Message a contact by their metaContactId, aka their uid in KABC.
	 */
	virtual void chatContactById( const QString &metaContactId ) = 0;

	/**
	 * Message a contact by their metaContactId, aka their uid in KABC.
	 */
	virtual void messageContactById( const QString &metaContactId, const QString& message ) = 0;
	
	/**
	 * Send the file to the contact
	 */
	virtual void sendFileToId(const QString &metaContactId, const KURL &sourceURL,
		const QString &altFileName = QString::null, uint fileSize = 0) = 0;

	/*void sendFile(const QString &displayName, const KURL &sourceURL,
		const QString &altFileName = QString::null, uint fileSize = 0) = 0;*/

// MUTATOR METHODS
// contact list control	
	/**
	 * Adds a contact with the specified params.
	 *
	 * @param protocolName The name of the protocol this contact is for ("ICQ", etc)
	 * @param accountId The account ID to add the contact to
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayName of the contact (may equal userId for some protocols
	 * @param groupName The name of the group to add the contact to
	 * @return Weather or not the contact was added successfully
	 */
	virtual bool addContact( const QString &protocolName, const QString &accountId, const QString &contactId,
		const QString &displayName, const QString &groupName = QString::null ) = 0;

// connection control
	/**
	 * connect a given account in the given protocol
	 */
	virtual void connect(const QString &protocolName, const QString &accountId) = 0;
	/**
	 * disconnect a given account in the given protocol
	 */
	virtual void disconnect(const QString &protocolName, const QString &accountId) = 0;

	/**
	 * Ask all accounts to connect
	 */
	virtual void connectAll() = 0;

	/**
	 * Ask all accounts to disconnect
	 */
	virtual void disconnectAll() = 0;

// presence control
	/**
	 * set all account away using the global away function
	 */
	virtual void setAway() = 0;
	/**
	 * set all account away using the global away function
	 * and set an away message.
	 */
	virtual void setAway(const QString &msg) = 0;
	/**
	 * set Available all accountes
	 */
	virtual void setAvailable() = 0;
	/**
	 * set all account away using the auto away funciton.
	 * accounts will return online if activity is detected again
	 */
	virtual void setAutoAway() = 0;

// Kopete specific app control
	/**
	 * load a plugin
	 * the name is the name of the library: example: kopete_msn
	 * but you can ommit the kopete_ prefix
	 */
	virtual bool loadPlugin( const QString& name ) = 0;
	/**
	 * unload a plugin
	 * the name is the name of the library: example: kopete_msn
	 * but you can ommit the kopete_ prefix
	 */
	virtual bool unloadPlugin( const QString& name ) = 0;

k_dcop_signals:
	void contactStatusChanged( const QString &metaContactId);
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

