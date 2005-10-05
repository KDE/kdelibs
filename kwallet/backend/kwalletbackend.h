/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2004 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef _KWALLETBACKEND_H
#define _KWALLETBACKEND_H

#include <kcodecs.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include "kwalletentry.h"


namespace KWallet {

/**
 * @internal
 */
class MD5Digest : public QByteArray {
	public:
		MD5Digest() : QByteArray(16) {}
		MD5Digest(const KMD5::Digest d) : QByteArray() { duplicate(reinterpret_cast<const char *>(d), 16); }
		virtual ~MD5Digest() {}

		int operator<(const MD5Digest& r) const {
				int i = 0;
				char x, y;
				for (; i < 16; ++i) {
					x = at(i);
					y = r.at(i);
					if (x != y) {
						break;
					}
				}
				if (i < 16 && x < y) {
					return 1;
				}
				return 0;
			}
};


/* @internal
 */
class KDE_EXPORT Backend {
	public:
		Backend(const QString& name = "kdewallet", bool isPath = false);
		~Backend();

		// Open and unlock the wallet.
		int open(const QByteArray& password);

		// Close and lock the wallet (saving changes).
		int close(const QByteArray& password);

		// Close the wallet, losing any changes.
		int close();

		// Write the wallet to disk
		int sync(const QByteArray& password);

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
		
		// Look up a list of entries.  Supports wildcards.
		// You delete the list
		QList<Entry*> readEntryList(const QString& key);

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

		// Rename an entry in this folder.
		int renameEntry(const QString& oldName, const QString& newName);

		int ref() { return ++_ref; }

		int deref() { return --_ref; }

		int refCount() const { return _ref; }

		static bool exists(const QString& wallet);

		bool folderDoesNotExist(const QString& folder) const;

		bool entryDoesNotExist(const QString& folder, const QString& entry) const;

		static QString openRCToString(int rc);

	private:
		class BackendPrivate;
		BackendPrivate *d;
		QString _name;
		QString _path;
		bool _open;
		QString _folder;
		int _ref;
		// Map Folder->Entries
		typedef QMap< QString, Entry* > EntryMap;
		typedef QMap< QString, EntryMap > FolderMap;
		FolderMap _entries;
		typedef QMap<MD5Digest, QList<MD5Digest> > HashMap;
		HashMap _hashes;
};

}

#endif

