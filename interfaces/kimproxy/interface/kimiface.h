/*
    kimiface.h - KDE Instant Messenger DCOP Interface
	
			  (c) 2004    Will Stephenson	 	 <lists@stevello.free-online.co.uk>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KIMIFACE_H
#define KIMIFACE_H

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
// ACCESSORS
// contact list
	virtual QStringList allContacts() = 0;
	virtual QStringList reachableContacts() = 0;
	virtual QStringList onlineContacts() = 0;
	virtual QStringList fileTransferContacts() = 0;
	
// individual
	virtual bool isPresent( const QString & uid ) = 0;
	virtual QString presenceString( const QString & uid ) = 0;
	virtual int presenceStatus( const QString & uid ) = 0;
	virtual bool canReceiveFiles( const QString & uid ) = 0;
	virtual bool canRespond( const QString & uid ) = 0;
	virtual QString locate( const QString & contactId, const QString & protocol ) = 0;
// metadata
	virtual QPixmap icon( const QString & uid ) = 0;
	virtual QString context( const QString & uid ) = 0;
// App capabilities
	virtual QStringList protocols() = 0;
	
// ACTORS
	/**
	 * Message a contact by their uid, aka their uid in KABC.
	 */
	virtual void messageContact( const QString &uid, const QString& message ) = 0;
	
	/**
	 * Open a chat to a contact, and optionally set some initial text
	 */
	virtual void messageNewContact( const QString &contactId, const QString &protocol ) = 0;

	/**
	 * Message a contact by their uid, aka their uid in KABC.
	 */
	virtual void chatWithContact( const QString &uid ) = 0;

	/**
	 * Send a file to the contact
	 */
	virtual void sendFile(const QString &uid, const KURL &sourceURL,
		const QString &altFileName = QString::null, uint fileSize = 0) = 0;

// MUTATORS
// Contact list
	virtual bool addContact( const QString &contactId, const QString &protocol ) = 0;
// SIGNALS
k_dcop_signals:
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

