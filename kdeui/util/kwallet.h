/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
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


#ifndef _KWALLET_H
#define _KWALLET_H

#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtGui/qwindowdefs.h> // krazy:exclude=includes (for WId)

#include <kdeui_export.h>


class QDBusError;

namespace KWallet {

/**
 * KDE Wallet
 *
 * This class implements a generic system-wide Wallet for KDE.  This is the
 * ONLY public interface.
 *
 * @author George Staikos <staikos@kde.org>
 * @short KDE Wallet Class
 */
class KDEUI_EXPORT Wallet : public QObject
{
	Q_OBJECT
	protected:
		/**
		 *  Construct a KWallet object.
		 *  @internal
		 *  @param handle The handle for the wallet.
		 *  @param name The name of the wallet.
		 */
		Wallet(int handle, const QString& name);
		/**
		 *  Copy a KWallet object.
		 *  @internal
		 */
		Wallet(const Wallet&);

	public:
		enum EntryType { Unknown=0, Password, Stream, Map, Unused=0xffff };

		/**
		 *  Destroy a KWallet object.  Closes the wallet.
		 */
		virtual ~Wallet();

		/**
		 *  List all the wallets available.
		 *  @return Returns a list of the names of all wallets that are
		 *          open.
		 */
		static QStringList walletList();

		/**
		 *  Determine if the KDE wallet is enabled.  Normally you do
		 *  not need to use this because openWallet() will just fail.
		 *  @return Returns true if the wallet enabled, else false.
		 */
		static bool isEnabled();

		/**
		 *  Determine if the wallet @p name is open by any application.
		 *  @param name The name of the wallet to check.
		 *  @return Returns true if the wallet is open, else false.
		 */
		static bool isOpen(const QString& name);

		/**
		 *  Close the wallet @p name.  The wallet will only be closed
		 *  if it is open but not in use (rare), or if it is forced
		 *  closed.
		 *  @param name The name of the wallet to close.
		 *  @param force Set true to force the wallet closed even if it
		 *               is in use by others.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		static int closeWallet(const QString& name, bool force);

		/**
		 *  Delete the wallet @p name.  The wallet will be forced closed
		 *  first.
		 *  @param name The name of the wallet to delete.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		static int deleteWallet(const QString& name);

		/**
		 *  Disconnect the application @p app from @p wallet.
		 *  @param wallet The name of the wallet to disconnect.
		 *  @param app The name of the application to disconnect.
		 *  @return Returns true on success, false on error.
		 */
		static bool disconnectApplication(const QString& wallet, const QString& app);

		enum OpenType { Synchronous=0, Asynchronous, Path, OpenTypeUnused=0xff };

		/**
		 *  Open the wallet @p name.  The user will be prompted to
		 *  allow your application to open the wallet, and may be
		 *  prompted for a password.  You are responsible for deleting
		 *  this object when you are done with it.
		 *  @param name The name of the wallet to open.
		 *  @param ot    If Asynchronous, the call will return
		 *               immediately with a non-null pointer to an
		 *               invalid wallet.  You must immediately connect
		 *               the walletOpened() signal to a slot so that
		 *               you will know when it is opened, or when it
		 *               fails.
		 *  @param w The window id to associate any dialogs with. You can pass
		 *           0 if you don't have a window the password dialog should
		 *           associate with.
		 *  @return Returns a pointer to the wallet if successful,
		 *          or a null pointer on error or if rejected.
		 */
		static Wallet* openWallet(const QString& name, WId w, OpenType ot = Synchronous);

		/**
		 *  List the applications that are using the wallet @p wallet.
		 *  @param wallet The wallet to query.
		 *  @return Returns a list of all DCOP application IDs using
		 *          the wallet.
		 */
		static QStringList users(const QString& wallet);

		/**
		 *  The name of the wallet used to store local passwords.
		 */
		static const QString LocalWallet();

		/**
		 *  The name of the wallet used to store network passwords.
		 */
		static const QString NetworkWallet();

		/**
		 *  The standardized name of the password folder.
		 *  It is automatically created when a wallet is created, but
		 *  the user may still delete it so you should check for its
		 *  existence and recreate it if necessary and desired.
		 */
		static const QString PasswordFolder();

		/**
		 *  The standardized name of the form data folder.
		 *  It is automatically created when a wallet is created, but
		 *  the user may still delete it so you should check for its
		 *  existence and recreate it if necessary and desired.
		 */
		static const QString FormDataFolder();

		/**
		 *  Request to the wallet service to change the password of
		 *  the wallet @p name.
		 *  @param name The the wallet to change the password of.
		 *  @param w The window id to associate any dialogs with. You can pass
		 *           0 if you don't have a window the password dialog should
		 *           associate with.
		 */
		static void changePassword(const QString& name, WId w);

		/**
		 *  This syncs the wallet file on disk with what is in memory.
		 *  You don't normally need to use this.  It happens
		 *  automatically on close.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int sync();

		/**
		 *  This closes and locks the current wallet.  It will
		 *  disconnect all applications using the wallet.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int lockWallet();

		/**
		 *  The name of the current wallet.
		 */
		virtual const QString& walletName() const;

		/**
		 *  Determine if the current wallet is open, and is a valid
		 *  wallet handle.
		 *  @return Returns true if the wallet handle is valid and open.
		 */
		virtual bool isOpen() const;

		/**
		 *  Request to the wallet service to change the password of
		 *  the current wallet.
		 *  @param w The window id to associate any dialogs with. You can pass
		 *           0 if you don't have a window the password dialog should
		 *           associate with.
		 */
		virtual void requestChangePassword(WId w);

		/**
		 *  Obtain the list of all folders contained in the wallet.
		 *  @return Returns an empty list if the wallet is not open.
		 */
		virtual QStringList folderList();

		/**
		 *  Determine if the folder @p f exists in the wallet.
		 *  @param f the name of the folder to check for
		 *  @return Returns true if the folder exists in the wallet.
		 */
		virtual bool hasFolder(const QString& f);

		/**
		 *  Set the current working folder to @p f.  The folder must
		 *  exist, or this call will fail.  Create a folder with
		 *  createFolder().
		 *  @param f the name of the folder to make the working folder
		 *  @return Returns true if the folder was successfully set.
		 */
		virtual bool setFolder(const QString& f);

		/**
		 *  Remove the folder @p f and all its entries from the wallet.
		 *  @param f the name of the folder to remove
		 *  @return Returns true if the folder was successfully removed.
		 */
		virtual bool removeFolder(const QString& f);

		/**
		 *  Created the folder @p f.
		 *  @param f the name of the folder to create
		 *  @return Returns true if the folder was successfully created.
		 */
		virtual bool createFolder(const QString& f);

		/**
		 *  Determine the current working folder in the wallet.
		 *  If the folder name is empty, it is working in the global
		 *  folder, which is valid but discouraged.
		 *  @return Returns the current working folder.
		 */
		virtual const QString& currentFolder() const;

		/**
		 *  Return the list of keys of all entries in this folder.
		 *  @return Returns an empty list if the wallet is not open, or
		 *          if the folder is empty.
		 */
		virtual QStringList entryList();

		/**
		 *  Rename the entry @p oldName to @p newName.
		 *  @param oldName The original key of the entry.
		 *  @param newName The new key of the entry.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int renameEntry(const QString& oldName, const QString& newName);

		/**
		 *  Read the entry @p key from the current folder.
		 *  The entry format is unknown except that it is either a
		 *  QByteArray or a QDataStream, which effectively means that
		 *  it is anything.
		 *  @param key The key of the entry to read.
		 *  @param value A buffer to fill with the value.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int readEntry(const QString& key, QByteArray& value);

		/**
		 *  Read the map entry @p key from the current folder.
		 *  @param key The key of the entry to read.
		 *  @param value A map buffer to fill with the value.
		 *  @return Returns 0 on success, non-zero on error.  Will
		 *          return an error if the key was not originally
		 *          written as a map.
		 */
		virtual int readMap(const QString& key, QMap<QString,QString>& value);

		/**
		 *  Read the password entry @p key from the current folder.
		 *  @param key The key of the entry to read.
		 *  @param value A password buffer to fill with the value.
		 *  @return Returns 0 on success, non-zero on error.  Will
		 *          return an error if the key was not originally
		 *          written as a password.
		 */
		virtual int readPassword(const QString& key, QString& value);

		/**
		 *  Read the entries matching @p key from the current folder.
		 *  The entry format is unknown except that it is either a
		 *  QByteArray or a QDataStream, which effectively means that
		 *  it is anything.
		 *  @param key The key of the entry to read.  Wildcards
		 *             are supported.
		 *  @param value A buffer to fill with the value.  The key in
		 *               the map is the entry key.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		int readEntryList(const QString& key, QMap<QString, QByteArray>& value);

		/**
		 *  Read the map entry @p key from the current folder.
		 *  @param key The key of the entry to read.  Wildcards
		 *             are supported.
		 *  @param value A buffer to fill with the value.  The key in
		 *               the map is the entry key.
		 *  @return Returns 0 on success, non-zero on error.  Will
		 *          return an error if the key was not originally
		 *          written as a map.
		 */
		int readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value);

		/**
		 *  Read the password entry @p key from the current folder.
		 *  @param key The key of the entry to read.  Wildcards
		 *             are supported.
		 *  @param value A buffer to fill with the value.  The key in
		 *               the map is the entry key.
		 *  @return Returns 0 on success, non-zero on error.  Will
		 *          return an error if the key was not originally
		 *          written as a password.
		 */
		int readPasswordList(const QString& key, QMap<QString, QString>& value);

		/**
		 *  Write @p key = @p value as a binary entry to the current
		 *  folder.  Be careful with this, it could cause inconsistency
		 *  in the future since you can put an arbitrary entry type in
		 *  place.
		 *  @param key The key of the new entry.
		 *  @param value The value of the entry.
		 *  @param entryType The type of the entry.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int writeEntry(const QString& key, const QByteArray& value, EntryType entryType);

		/**
		 *  Write @p key = @p value as a binary entry to the current
		 *  folder.
		 *  @param key The key of the new entry.
		 *  @param value The value of the entry.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int writeEntry(const QString& key, const QByteArray& value);

		/**
		 *  Write @p key = @p value as a map to the current folder.
		 *  @param key The key of the new entry.
		 *  @param value The value of the map.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int writeMap(const QString& key, const QMap<QString,QString>& value);

		/**
		 *  Write @p key = @p value as a password to the current folder.
		 *  @param key The key of the new entry.
		 *  @param value The value of the password.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int writePassword(const QString& key, const QString& value);

		/**
		 *  Determine if the current folder has they entry @p key.
		 *  @param key The key to search for.
		 *  @return Returns true if the folder contains @p key.
		 */
		virtual bool hasEntry(const QString& key);

		/**
		 *  Remove the entry @p key from the current folder.
		 *  @param key The key to remove.
		 *  @return Returns 0 on success, non-zero on error.
		 */
		virtual int removeEntry(const QString& key);

		/**
		 *  Determine the type of the entry @p key in this folder.
		 *  @param key The key to look up.
		 *  @return Returns an enumerated type representing the type
		 *          of the entry.
		 */
		virtual EntryType entryType(const QString& key);

		/**
		 *  Determine if a folder does not exist in a wallet.  This
		 *  does not require decryption of the wallet.
		 *  This is a handy optimization to avoid prompting the user
		 *  if your data is certainly not in the wallet.
		 *  @param wallet The wallet to look in.
		 *  @param folder The folder to look up.
		 *  @return Returns true if the folder does NOT exist in the
		 *  wallet, or the wallet does not exist.
		 */
		static bool folderDoesNotExist(const QString& wallet, const QString& folder);

		/**
		 *  Determine if an entry in a folder does not exist in a
		 *  wallet.  This does not require decryption of the wallet.
		 *  This is a handy optimization to avoid prompting the user
		 *  if your data is certainly not in the wallet.
		 *  @param wallet The wallet to look in.
		 *  @param folder The folder to look in.
		 *  @param key The key to look up.
		 *  @return Returns true if the key does NOT exist in the
		 *  wallet, or the folder or wallet does not exist.
		 */
		static bool keyDoesNotExist(const QString& wallet, const QString& folder,
					    const QString& key);

	Q_SIGNALS:
		/**
		 *  Emitted when this wallet is closed.
		 */
		void walletClosed();

		/**
		 *  Emitted when a folder in this wallet is updated.
		 *  @param folder The folder that was updated.
		 */
		void folderUpdated(const QString& folder);

		/**
		 *  Emitted when the folder list is changed in this wallet.
		 */
		void folderListUpdated();

		/**
		 *  Emitted when a folder in this wallet is removed.
		 *  @param folder The folder that was removed.
		 */
		void folderRemoved(const QString& folder);

		/**
		 *  Emitted when a wallet is opened in asynchronous mode.
		 *  @param success True if the wallet was opened successfully.
		 */
		void walletOpened(bool success);

	private Q_SLOTS:
		/**
		 *  @internal
		 *  DBUS slot for signals emitted by the wallet service.
		 */
		void slotWalletClosed(int handle);

		/**
		 *  @internal
		 *  DBUS slot for signals emitted by the wallet service.
		 */
		void slotFolderUpdated(const QString& wallet, const QString& folder);

		/**
		 *  @internal
		 *  DBUS slot for signals emitted by the wallet service.
		 */
		void slotFolderListUpdated(const QString& wallet);

		/**
		 *  @internal
		 *  DBUS slot for signals emitted by the wallet service.
		 */
		void slotApplicationDisconnected(const QString& wallet, const QString& application);

		/**
		 *  @internal
		 *  Callback for kwalletd
		 *  @param tId identifer for the open transaction
		 *  @param handle the wallet's handle
		 */
		void walletAsyncOpened(int tId, int handle);

		/**
		 *  @internal
		 *  DBUS error slot.
		 */
		void emitWalletAsyncOpenError();

		/**
		 *  @internal
		 *  Used to detect when the wallet service dies.
		 */
		void slotServiceOwnerChanged(const QString&,const QString&,const QString&);

	private:
		class WalletPrivate;
		WalletPrivate* const d;

	protected:
		/**
		 *  @internal
		 */
		virtual void virtual_hook(int id, void *data);
};

}

#endif //_KWALLET_H

