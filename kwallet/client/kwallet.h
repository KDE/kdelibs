/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2003 George Staikos <staikos@kde.org>
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

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <dcopobject.h>

class DCOPRef;

namespace KWallet {

class Wallet : public QObject, public DCOPObject {
	K_DCOP
	Q_OBJECT
	protected:
		Wallet(int handle, const QString& name);
		Wallet(const Wallet&);

	public:
		enum EntryType { Unknown=0, Password, Stream, Map };

		virtual ~Wallet();
		
		static Wallet* openWallet(const QString& name);

		static const QString LocalWallet();
		static const QString NetworkWallet();

		static const QString PasswordFolder;
		static const QString FormDataFolder;

		virtual int lockWallet();

		virtual const QString& walletName() const;

		virtual bool isOpen() const;

		virtual void requestChangePassword();

		// Folder management functions
		virtual QStringList folderList();

		virtual bool hasFolder(const QString& f);

		virtual bool setFolder(const QString& f);

		virtual bool removeFolder(const QString& f);

		virtual const QString& currentFolder() const;

		// Entry management functions
		virtual int readEntry(const QString& key, QByteArray& value);

		virtual int readMap(const QString& key, QMap<QString,QString>& value);

		virtual int readPassword(const QString& key, QString& value);

		virtual int writeEntry(const QString& key, const QByteArray& value);

		virtual int writeMap(const QString& key, const QMap<QString,QString>& value);

		virtual int writePassword(const QString& key, const QString& value);

		virtual bool hasEntry(const QString& key);

		virtual int removeEntry(const QString& key);

		virtual EntryType entryType(const QString& key);

	signals:
		void walletClosed();
		void folderRemoved(const QString& folder);

	private:
	k_dcop:
		ASYNC slotWalletClosed(int handle);

	private:
		class WalletPrivate;
		WalletPrivate *d;
		QString _name;
		QString _folder;
		int _handle;
		DCOPRef *_dcopRef;

	protected:
		virtual void virtual_hook(int id, void *data);
};

};

#endif

