/* This file is part of the KDE project
 *
 * Copyright (C) 2002 George Staikos <staikos@kde.org>
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


#ifndef _KWALLET_H
#define _KWALLET_H


#include <qcstring.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qmap.h>
#include "kwalletentry.h"


class DCOPClient;

namespace KWallet {

class Wallet {
	public:
		Wallet(const QString& name = "kdewallet");
		~Wallet();
		
		// Open and unlock the wallet
		int open(const QByteArray& password);
		
		// Close and lock the wallet
		int close(const QByteArray& password);

		// Returns true if the current wallet is open
		bool isOpen();

		// Returns the current wallet name
		const QString& walletName() const;

		// Changes to a new wallet "name"
		// returns false if it cannot change (ie another wallet is open)
		bool changeWallet(const QString& name);

		// Not sure if using a reference here makes sense....
		const QPtrList<Entry>& getEntriesByType(const QString& type) const;

		const QStringList getTypeList() const;

	private:
		class WalletPrivate;
		WalletPrivate *d;
		QString _name;
		bool _open;
		DCOPClient *_dcopClient;
};

};

#endif

