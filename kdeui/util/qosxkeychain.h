/* This file is part of the KDE project
 *
 * Copyright (C) 2014 René Bertin <rjvbertin@gmail.com>
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

#include <Security/Security.h>
#include <Security/SecKeychain.h>

namespace {
    template <typename T>
    struct CFReleaser {
        explicit CFReleaser( const T& r ) : ref( r ) {}
        ~CFReleaser() { if( ref ){ CFRelease( ref ); } }
        T ref;
    };

    template <typename T>
    struct CPPDeleter {
        explicit CPPDeleter( const T& r ) : ptr( r ) {}
        ~CPPDeleter() { if( ptr ){ delete ptr; } }
        T ptr;
    };

    template <typename T>
    struct CPPArrayDeleter {
        explicit CPPArrayDeleter( const T& r ) : ptr( r ) {}
        ~CPPArrayDeleter() { if( ptr ){ delete[] ptr; } }
        T ptr;
    };

    template <typename T>
    struct CacheOldValue {
        explicit CacheOldValue( T &var, const T newVal )
            : oldVal(var), varRef(var)
        {
            var = newVal;
        }
        ~CacheOldValue()
        {
            varRef = oldVal;
        }
        T oldVal, &varRef;
    };
}

static inline QString asQString( CFStringRef sr )
{   CFIndex len = CFStringGetLength(sr)*2;
    const CPPArrayDeleter<char*> buff(new char[len]);
    if( CFStringGetCString( sr, buff.ptr, len, kCFStringEncodingUTF8 ) ){
        return QString::fromUtf8(buff.ptr); //RJVB: use UTF8
    }
    else if( CFStringGetCString( sr, buff.ptr, len, kCFStringEncodingNonLossyASCII ) ){
        return QString::fromLocal8Bit(buff.ptr);
    }
    else{
        CFStringGetCString( sr, buff.ptr, len, NULL );
        return QString::fromLatin1(buff.ptr);
    }
}

static inline QString errorString( OSStatus s )
{
    const CFReleaser<CFStringRef> ref( SecCopyErrorMessageString( s, NULL ) );
    return asQString( ref.ref );
}

static inline bool isError( OSStatus s, QString *errMsg )
{
    if( errMsg ){
        *errMsg = errorString(s);
    }
    return s != 0;
}

class OSXKeychain
{
private:
    SecKeychainRef keyChainRef;
    QString keyChainPath;
    bool isDefaultKeychain, generateFolderList;

public:
	enum EntryType { Unknown='K\?\?\?', Password='KPWD', Map='KMAP', Stream='KSTR' };
    QString name;
    QString currentService, lastReadService;
    QStringList serviceList;
    bool isKDEChain;

    OSXKeychain();
    OSXKeychain(const QString &name);
    virtual ~OSXKeychain();

    inline SecKeychainRef reference()
    {
        return keyChainRef;
    }
    inline QString &path()
    {
        return keyChainPath;
    }
    inline bool isDefault()
    {
	    return isDefaultKeychain;
    }
    inline bool isOpen()
    {
        return IsOpen(keyChainRef);
    }
    inline OSStatus lock()
    {
        return Lock(keyChainRef);
    }
    inline OSStatus unLock()
    {
        return UnLock(keyChainRef);
    }
    void close();
    inline bool hasItem(const QString &key, OSStatus *errReturn, SecKeychainItemRef *itemRef=NULL)
    {
	    // qDebug() << "OSXKeychain::hasItem(" << key << "): scanning '" << name << "'=" << (void*) keyChainRef;
	    return OSXKeychain::HasItem( key, keyChainRef, errReturn, itemRef );
    }
    inline OSStatus readItem(const QString &key, QByteArray *value, SecKeychainItemRef *itemRef=NULL)
    {
        return ReadItem( key, value, keyChainRef, itemRef, this );
    }
    inline OSStatus itemType(const QString &key, EntryType *entryType)
    {
        return ItemType( key, entryType, keyChainRef );
    }
    inline OSStatus removeItem(const QString &key)
    {
        return RemoveItem( key, keyChainRef );
    }
    inline OSStatus writeItem( const QString &key, const QByteArray &value, EntryType *entryType=NULL )
    {
        return WriteItem( key, value, keyChainRef, NULL, entryType, this );
    }
    inline OSStatus writeItem( const QString &key, const QByteArray &value, const QString &comment,
                               EntryType *entryType=NULL )
    {
        return WriteItem( key, value, comment, keyChainRef, entryType, this );
    }
    inline OSStatus itemList( QStringList &keyList )
    {
        return ItemList( keyChainRef, keyList, this );
    }
    inline QStringList folderList()
    {
        QStringList r;
        CacheOldValue<bool> gFL(generateFolderList, true);
        ItemList( keyChainRef, r, this );
        r.clear();
        return serviceList;
    }
    OSStatus renameItem(const QString &currentKey, const QString &newKey);

#pragma mark ==== class methods aka static member functions ====
    static OSStatus KeychainList(QStringList &theList);
    static QString Path(const SecKeychainRef keychain);
    static bool IsOpen(const SecKeychainRef keychain);
    static bool IsOpen(const QString& name);
    static OSStatus UnLock(const SecKeychainRef keychain);
    static OSStatus Lock(const SecKeychainRef keychain);
    static OSStatus Lock(const QString &walletName);
    /** use the keychain search functions to find the first matching item, if any, returning True if found.
     The OS X error code is returned through @p errReturn when not NULL, the item itself through @p itemRef.
     This reference will have to be released with CFRelease() when done with it (when @p itemRef==NULL the
     function does this release itself).
     */
    static bool HasItem(const QString &key,
                         const SecKeychainRef keychain, OSStatus *errReturn, SecKeychainItemRef *itemRef);
    static OSStatus ReadItem(const QString &key, QByteArray *value,
                              const SecKeychainRef keychain, SecKeychainItemRef *itemRef=NULL, OSXKeychain *osxKeyChain=NULL);
    static OSStatus ItemType(const QString &key, EntryType *entryType,
                               const SecKeychainRef keychain);
    static OSStatus RemoveItem(const QString &key, const SecKeychainRef keychain);
    static OSStatus WriteItem( const QString &key, const QByteArray &value,
                               const SecKeychainRef keychain, SecKeychainItemRef *itemRef=NULL, EntryType *entryType=NULL, OSXKeychain *osxKeyChain=NULL );
    static OSStatus WriteItem( const QString& key, const QByteArray& value,
                               const QString& comment, const SecKeychainRef keychain, EntryType *entryType, OSXKeychain *osxKeyChain=NULL );
    static OSStatus ItemList( const SecKeychainRef keychain, QStringList &keyList, OSXKeychain *osxKeyChain=NULL );
    static OSStatus Destroy( SecKeychainRef *keychain );
    static OSStatus Destroy( const QString &walletName );
};
