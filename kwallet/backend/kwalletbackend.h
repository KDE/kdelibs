/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef _KWALLETBACKEND_H
#define _KWALLETBACKEND_H


#include <qcstring.h>
#include <qstring.h>


class KWalletBackend {
	public:
		KWalletBackend(QString name = "kdewallet");
		~KWalletBackend();
		
		int unlock(QByteArray& password);
		
		int lock(QByteArray& password);

		// Returns true if the current wallet is open
		bool isOpen();

		// Returns the current wallet name
		const QString& walletName();

		// Changes to a new wallet "name"
		// returns false if it cannot change (ie another wallet is open)
		bool changeWallet(QString name);

		// add
		// remove

	protected:

	private:
		class KWalletBackendPrivate;
		KWalletBackendPrivate *d;
		QString _name;
		bool _open;
};


#endif

