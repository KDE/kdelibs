/* This file is part of the KDE project
 *
 * Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
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


#ifndef _KWALLETNETWORKPASSWORD_H
#define _KWALLETNETWORKPASSWORD_H

#include "kwalletentry.h"

namespace KWallet {

class NetworkPassword : public Entry {
        protected:
	        // ctor to load a NetworkPassword, used by factory()
		NetworkPassword(const QString &type,
				const Key &key, 
				const Value &value);
	public:
		// TODO: ctor to create a new NetworkPassword
		virtual ~NetworkPassword();
		
		QString url() const;
		QString userName() const;
		QString password() const;

		virtual QString type() const;

		static QPtrList<NetworkPassword> 
			getNetworkPasswords(const QString& url = QString::null, 
					    const QString& username = QString::null);

	private:
		static Entry *factory(const QString &type,
				      const Key &key, 
				      const Value &value);
		static const char *typeName;
		class NetworkPasswordPrivate;
		NetworkPasswordPrivate *d;
		QString _userName;
		QString _password;
};


};

#endif

