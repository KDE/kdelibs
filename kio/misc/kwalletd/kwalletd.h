/*
   This file is part of the KDE libraries

   Copyright (c) 2002 George Staikos <staikos@kde.org>

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
#ifndef _KWALLETD_H_
#define _KWALLETD_H_

#include <kded/kdedmodule.h>
#include <qstring.h>
#include "kwalletbackend.h"


class KWalletD : public KDEDModule {
	Q_OBJECT
	K_DCOP
	public:
		KWalletD(const QCString &name);
		virtual ~KWalletD();

	k_dcop:
		// Open and unlock the wallet
		virtual int open(const QString& wallet, const QByteArray& password);

		// Close and lock the wallet
		// Will remain open if others are using it still.
		virtual int close(const QString& wallet, const QByteArray& password);

		// Returns true if the wallet is open
		virtual bool isOpen(const QString& wallet) const;

	private:
		QMap<QString,KWallet::Backend*> _wallets;
		QMap<QString,int> _useCount;
};


#endif
