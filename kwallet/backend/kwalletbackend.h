/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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


#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include "kwalletentry.h"


namespace KWallet {

class Backend {
	public:
		Backend(const QString& name = "kdewallet");
		~Backend();
		
		// Open and unlock the wallet.
		int open(const QByteArray& password);
		
		// Close and lock the wallet (saving changes).
		int close(const QByteArray& password);

		// Close the wallet, losing any changes.
		int close();

		// Returns true if the current wallet is open.
		bool isOpen() const;

		// Returns the current wallet name.
		const QString& walletName() const;

		// The list of folders.
		QStringList folderList() const;

		// Force creation of a folder.
		bool createFolder(const QString& f);

		// Change the folder.
		void setFolder(const QString& f) { _folder = f; }

		// Current folder.  If empty, it's the global folder.
		const QString& folder() const { return _folder; }

		// Does it have this folder?
		bool hasFolder(const QString& f) const { return _entries.contains(f); }

		// Look up an entry.  Returns null if it doesn't exist.
		Entry *readEntry(const QString& key);

		// Store an entry.
		void writeEntry(Entry *e);

		// Does this folder contain this entry?
		bool hasEntry(const QString& key) const;

		// Returns true if the entry was removed
		bool removeEntry(const QString& key);

		// Returns true if the folder was removed
		bool removeFolder(const QString& f);

		// The list of entries in this folder.
		QStringList entryList() const;

		int ref() { return ++_ref; }

		int deref() { return --_ref; }

		int refCount() const { return _ref; }

		static bool exists(const QString& wallet);

	private:
		class BackendPrivate;
		BackendPrivate *d;
		QString _name;
		bool _open;
		QString _folder;
		int _ref;
		// Map Folder->Entries
		typedef QMap< QString, Entry* > EntryMap;
		typedef QMap< QString, EntryMap > FolderMap;
		FolderMap _entries;
};

}

#endif

