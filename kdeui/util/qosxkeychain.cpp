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

#include <cassert>
#include <sys/param.h>

#include <QtGui/QApplication>
#include <QtCore/QtCore>
#include <QtCore/QPointer>
#include <QtGui/QWidget>

#include "kwallet.h"
#include <kdebug.h>
using namespace KWallet;
#include "qosxkeychain.h"

#include <CoreServices/CoreServices.h>

//! Define INTERNET_TOO=1 in order to build read-access to the kSecInternetPasswordItemClass items
#define INTERNET_TOO    0

// #undef kWarning
// #undef kDebug
// #define kWarning    qWarning
// #define kDebug      qDebug

// returns the textual representation of a FourCharCode (e.g. 'JPEG')
static QString OSTStr( FourCharCode etype )
{   union OSTStr {
        struct {
            char startquote;
            uint32_t four;
            char endquote;
        } __attribute__ ((packed)) value;
        char representation[7];
    }  __attribute__ ((packed)) ltype;
    ltype.value.four = EndianU32_BtoN(etype);
    ltype.representation[0] = ltype.representation[5] = '\'';
    ltype.representation[6] = '\0';
    return QString::fromAscii(ltype.representation);
}

static SecKeychainRef defaultChain()
{   QString errMsg;
    SecKeychainRef keychain;
    if( isError( SecKeychainCopyDefault(&keychain), &errMsg ) ){
        kWarning() << "Could not retrieve reference to default keychain:"  << qPrintable(errMsg);
        keychain = NULL;
    }
    return keychain;
}

/*! Return a name for @p keychain, and possibly the full path to its file
 * The name  will be the equivalent of the `basename path .keychain` shell
 * command.
 */
static QString keyChainName( SecKeychainRef keychain, QString *path=NULL )
{   QFileInfo keyFile;
    QString p = OSXKeychain::Path(keychain);
    int ext = p.lastIndexOf(".keychain");
    keyFile = QFileInfo( ((ext > 0)? p.left(ext) : p) );
    if( path ){
        *path = QString(p);
    }
    return keyFile.fileName();
}

/*! Open an OS X keychain with name @p n.
 * OS X keychains can be created without a full path (say, "kdewallet"), in which case they
 * are stored e.g. as ~/Library/Keychains/kdewallet . However, opening a preexisting keychain like "login"
 * without using the full path seems to fail even if e.g. ~/Library/Keychains/login exists.
 * We try to work around that issue by matching @p n against the known keychain names.
 */
static OSStatus openKeychain( const QString &n, SecKeychainRef *keychain )
{   OSStatus err;
    CFArrayRef list = NULL;

    *keychain = NULL;
    err = SecKeychainCopySearchList( &list );
    if( !err && list ){
        CFIndex len = CFArrayGetCount(list), i;
        for( i = 0 ; i < len && !*keychain ; ++i ){
            SecKeychainRef kr = (SecKeychainRef) CFArrayGetValueAtIndex( list, i );
            QString path, name = keyChainName( kr, &path );
            if( name == n ){
                // a hit, try to open it!
                err = SecKeychainOpen( path.toUtf8(), keychain );
                if( err ){
                    kWarning() << "openKeychain(" << n << ") error" << err << "opening matching" << path;
                }
                else{
                    kDebug() << "openKeychain(" << n << ") opened matching" << path;
                }
            }
        }
        CFRelease(list);
    }
    if( !*keychain ){
        err = SecKeychainOpen( n.toUtf8(), keychain );
    }
    // we actually need to query the keychain's status to know if we succeeded
    // in opening an existing keychain!
    if( !err ){
        SecKeychainStatus status;
        err = SecKeychainGetStatus( *keychain, &status );
    }
    return err;
}

static OSStatus basicWriteItem( const QByteArray *serviceName, const QByteArray &accountName, const QByteArray &value,
                               const SecKeychainRef keychain, SecKeychainItemRef *itemRef=NULL )
{   OSStatus err;
    QString errMsg;
    if( serviceName ){
        err = SecKeychainAddGenericPassword( keychain, serviceName->size(), serviceName->constData(),
                                                     accountName.size(), accountName.constData(),
                                                     value.size(), value.constData(), itemRef );
    }
    else{
        err = SecKeychainAddGenericPassword( keychain, 0, NULL,
                                                     accountName.size(), accountName.constData(),
                                                     value.size(), value.constData(), itemRef );
    }
    if( err != errSecDuplicateItem && isError( err, &errMsg ) ){
        kWarning() << "Could not store password in keychain: " << qPrintable(errMsg);
    }
    return err;
}

OSXKeychain::OSXKeychain()
    : name("default")
{ QString errMsg;
    keyChainRef = defaultChain();
    if( keyChainRef ){
        keyChainPath = OSXKeychain::Path(keyChainRef);
        kDebug() << "Retrieved reference to default keychain" << (void*) keyChainRef << "in " << keyChainPath;
        name = keyChainName(keyChainRef);
        isDefaultKeychain = true;
    }
    else{
        keyChainPath = QString::fromUtf8("<undefined>");
    }
    serviceList.clear();
    serviceList.append("");
}

OSXKeychain::OSXKeychain(const QString &n)
    : name(n)
{   QString errMsg;
    OSStatus err = openKeychain( n, &keyChainRef );

    if( err == errSecNoSuchKeychain ){
        kWarning() << "Keychain '" << n << "' does not exist: attempting to create it";
        err = SecKeychainCreate( n.toUtf8(), 0, NULL, true, NULL, &keyChainRef );
        isKDEChain = true;
    }

    if( isError( err, &errMsg ) ){
        // the protocol cannot handle failure to open a keychain, so we have to return the default.
        keyChainRef = defaultChain();
        kWarning() << "Error opening keychain '" << n << "' (falling back to default keychain): " << qPrintable(errMsg);
        name = keyChainName(keyChainRef);
        isDefaultKeychain = true;
    }
    else{
        isDefaultKeychain = false;
    }

    if( keyChainRef ){
        keyChainPath = OSXKeychain::Path(keyChainRef);
        kDebug() << "Retrieved reference to keychain" << name << (void*) keyChainRef << "in " << keyChainPath;
    }
    else{
        keyChainPath = QString::fromUtf8("<undefined>");
    }
    serviceList.clear();
    serviceList.append("");
}

void OSXKeychain::close()
{
    if( keyChainRef ){
        CFRelease(keyChainRef);
        keyChainRef = NULL;
    }
}

OSXKeychain::~OSXKeychain()
{
    close();
}

OSStatus OSXKeychain::renameItem(const QString &currentKey, const QString &newKey)
{   OSStatus err;
    SecKeychainItemRef itemRef = NULL;
    err = ReadItem( currentKey, NULL, keyChainRef, &itemRef, this );
    if( !err && itemRef ){
        const QByteArray accountName( newKey.toUtf8() );
        // store the new key in the account and label attributes
        SecKeychainAttribute attr[] = { { kSecAccountItemAttr, accountName.size(), (void*) accountName.constData() },
                                        { kSecLabelItemAttr, accountName.size(), (void*) accountName.constData() } };
        SecKeychainAttributeList attrList = { 2, &attr[0] };
        QString errMsg;
        if( isError( (err = SecKeychainItemModifyAttributesAndData( itemRef, &attrList, 0, NULL )), &errMsg ) ){
            kWarning() << "OSXKeychain::renameItem(" << currentKey << ") couldn't change name & label to" << accountName
            << ":" << err << "=" << qPrintable(errMsg);
        }
        CFRelease(itemRef);
    }
    return err;
}

#pragma mark ========= static member functions =========

OSStatus OSXKeychain::KeychainList(QStringList &theList)
{   CFArrayRef list = NULL;
    OSStatus err = SecKeychainCopySearchList( &list );
    theList.clear();
    if( !err && list ){
        CFIndex len = CFArrayGetCount(list), i;
        for( i = 0 ; i < len ; ++i ){
            SecKeychainRef keychain = (SecKeychainRef) CFArrayGetValueAtIndex( list, i );
            QString name = keyChainName(keychain);
            if( name.size() > 0 ){
                theList.append(name);
            }
        }
        CFRelease(list);
    }
    return err;
}

QString OSXKeychain::Path(const SecKeychainRef keychain)
{   char pathName[MAXPATHLEN];
    UInt32 plen = MAXPATHLEN;
    if( SecKeychainGetPath( (keychain)? keychain : OSXKeychain().reference(), &plen, pathName ) == errSecSuccess ){
        return QString::fromUtf8(pathName);
    }
    else{
        return QString();
    }
}

bool OSXKeychain::IsOpen(const SecKeychainRef keychain)
{   bool isOpen = false;
    SecKeychainStatus status;
    QString errMsg;
    if( isError( SecKeychainGetStatus( keychain, &status ), &errMsg ) ){
        if( keychain ){
            kDebug() << "Could not get the status of keychain" << OSXKeychain::Path(keychain) << ":"  << qPrintable(errMsg);
        }
        else{
            kWarning() << "Could not get the default keychain's status:"  << qPrintable(errMsg);
        }
    }
    else{
        if( (status & kSecUnlockStateStatus) && (status & kSecReadPermStatus) ){
            isOpen = true;
        }
        else{
            kDebug() << "Keychain" << OSXKeychain::Path(keychain) << " has status" << status;
        }
    }
    return isOpen;
}

bool OSXKeychain::IsOpen(const QString &walletName)
{   SecKeychainRef keychain = NULL;
    OSStatus err = openKeychain( walletName.toUtf8(), &keychain );
    bool ret = false;
    if( !err && keychain ){
        ret = IsOpen(keychain);
        CFRelease(keychain);
    }
    return ret;
}

OSStatus OSXKeychain::UnLock(const SecKeychainRef keychain)
{   QString errMsg;
    OSStatus err;
    err = SecKeychainUnlock( keychain, 0, NULL, false );
    if( isError( err, &errMsg ) ){
        if( keychain ){
            kDebug() << "Could not unlock the keychain at '" << OSXKeychain::Path(keychain) << "': " << qPrintable(errMsg);
        }
        else{
            kDebug() << "Could not unlock the default keychain:"  << qPrintable(errMsg);
        }
    }
    return err;
}

OSStatus OSXKeychain::Lock(const SecKeychainRef keychain)
{   QString errMsg;
    OSStatus err;
    if( keychain ){
        err = SecKeychainLock(keychain);
        if( isError( err, &errMsg ) ){
            kDebug() << "Could not lock the keychain at '" << OSXKeychain::Path(keychain) << "': " << qPrintable(errMsg);
        }
    }
    else{
        err = SecKeychainLockAll();
        if( isError( err, &errMsg ) ){
            kDebug() << "Could not lock all keychains:" << qPrintable(errMsg);
        }
    }
    return err;
}

OSStatus OSXKeychain::Lock(const QString &walletName)
{   SecKeychainRef keychain = NULL;
    OSStatus err = openKeychain( walletName, &keychain );
    if( !err && keychain ){
        err = Lock(keychain);
	   CFRelease(keychain);
    }
    return err;
}

/** use the keychain search functions to find the first matching item, if any, @return returning True if found.
 The OS X error code is returned through @p errReturn when not NULL, the item itself through @p itemRef.
 This reference will have to be released with CFRelease() when done with it (when @p itemRef==NULL the
 function does this release itself).
 */
bool OSXKeychain::HasItem(const QString &key,
                     const SecKeychainRef keychain, OSStatus *errReturn, SecKeychainItemRef *itemRef)
{   const QByteArray accountName( key.toUtf8() );
    OSStatus err;
    SecKeychainSearchRef searchRef;
    SecKeychainAttribute attrs = { kSecAccountItemAttr, accountName.size(), (void*) accountName.constData() };
    SecKeychainAttributeList attrList = { 1, &attrs };
    err = SecKeychainSearchCreateFromAttributes( keychain, kSecGenericPasswordItemClass,
                                                (const SecKeychainAttributeList*) &attrList, &searchRef );
    const CFReleaser<SecKeychainSearchRef> releaseSR(searchRef);
    bool found;
    SecKeychainItemRef item;
    QString errMsg;
    if( err ){
        found = false;
        errMsg = errorString(err);
        kDebug() << "OSXKeychain::HasItem(" << key << "," << (void*) keychain << "): SecKeychainSearchCreateFromAttributes failed";
    }
    else{
	    if( !(err = SecKeychainSearchCopyNext( searchRef, &item )) ){
	        found = true;
	        if( itemRef ){
	            *itemRef = item;
	        }
	        else if( item ){
	            CFRelease(item);
	        }
	        errMsg = QString();
	    }
	    else{
	        found = false;
	        errMsg = errorString(err);
	    }
	    if( errReturn ){
	        *errReturn = err;
	    }
    }
    kDebug() << ((found)? "Found" : "Did not find") << "item '" << key << "' in keychain " << (void*) keychain << ", error=" << err << " " << qPrintable(errMsg);
    return found;
}

OSStatus OSXKeychain::ReadItem(const QString &key, QByteArray *value,
                          const SecKeychainRef keychain, SecKeychainItemRef *itemRef, OSXKeychain *osxKeyChain)
{   const QByteArray accountName( key.toUtf8() );
    UInt32 passwordSize = 0;
    void* passwordData = 0;
    QString errMsg;
    SecKeychainItemRef theItem;
    OSStatus err = SecKeychainFindGenericPassword( keychain, 0, NULL,
                                                  accountName.size(), accountName.constData(),
                                                  &passwordSize, &passwordData, &theItem );
    if( isError( err, &errMsg ) ){
        kDebug() << "Error" << err << "retrieving password for '" << accountName << "' :" << qPrintable(errMsg);
#if INTERNET_TOO
        if( SecKeychainFindInternetPassword( keychain, 0, NULL,
                                                      0, NULL,
                                                      accountName.size(), accountName.constData(),
                                                      0, NULL, 0,
                                                      kSecProtocolTypeAny, kSecAuthenticationTypeDefault,
                                                      &passwordSize, &passwordData, &theItem ) ){
            // just to be sure:
            theItem = NULL;
        }
        else{
            err = 0;
            errMsg = QString();
        }
#else
        theItem = NULL;
#endif
    }
    if( !err && theItem ){
        if( value ){
            *value = QByteArray( reinterpret_cast<const char*>( passwordData ), passwordSize );
        }
        SecKeychainItemFreeContent( NULL, passwordData );
        if( osxKeyChain && osxKeyChain->isKDEChain ){
            SecKeychainAttribute attr = { kSecServiceItemAttr, 0, NULL };
            SecKeychainAttributeList attrList = { 1, &attr };
            UInt32 len = 0;
            // try to fetch the item's ServiceItem attribute
            if( !SecKeychainItemCopyContent( theItem, NULL, &attrList, &len, NULL ) ){
                if( attr.length > 0 ){
                    osxKeyChain->lastReadService.clear();
                    osxKeyChain->lastReadService = QString::fromUtf8( (char*)attr.data, attr.length );
                }
                SecKeychainItemFreeContent( &attrList, NULL );
            }
        }
        if( itemRef ){
            *itemRef = theItem;
        }
        else if( theItem ){
            CFRelease(theItem);
        }
    }
    kDebug() << "OSXKeychain::ReadItem '" << key << "' from keychain " << OSXKeychain::Path(keychain) << ", error=" << err;
    return err;
}

OSStatus OSXKeychain::ItemType(const QString &key, EntryType *entryType,
                          const SecKeychainRef keychain)
{   const QByteArray accountName( key.toUtf8() );
    QString errMsg;
    EntryType etype = (EntryType) 0;
    SecKeychainItemRef itemRef;
#if INTERNET_TOO
    bool isInternetPW = false;
#endif
    OSStatus err = SecKeychainFindGenericPassword( keychain, 0, NULL,
                                                  accountName.size(), accountName.constData(),
                                                  NULL, NULL, &itemRef );
    if( isError( err, &errMsg ) ){
        kDebug() << "Error" << err << "retrieving type for '" << accountName << "' :" << qPrintable(errMsg);
#if INTERNET_TOO
        if( SecKeychainFindInternetPassword( keychain, 0, NULL,
                                            0, NULL,
                                            accountName.size(), accountName.constData(),
                                            0, NULL, 0,
                                            kSecProtocolTypeAny, kSecAuthenticationTypeDefault,
                                            0, NULL, &itemRef ) ){
            // just to be sure:
            itemRef = NULL;
        }
        else{
            isInternetPW = true;
            err = 0;
            errMsg = QString();
        }
#else
        itemRef = NULL;
#endif
    }
    if( itemRef ){
		UInt32 tags[] = { kSecTypeItemAttr };
		UInt32 formats[] = { CSSM_DB_ATTRIBUTE_FORMAT_STRING };
        SecKeychainAttributeInfo attrGet = { 1, tags, formats };
        SecKeychainAttributeList *attrList = NULL;
        err = SecKeychainItemCopyAttributesAndData( itemRef, &attrGet, NULL, &attrList, NULL, NULL );
        if( !err ){
            if( attrList->attr[0].length == sizeof(EntryType) ){
                memcpy( &etype, attrList->attr[0].data, sizeof(EntryType) );
            }
            else if( attrList->attr[0].length ){
                kDebug() << "Error: key" << key << "item type retrieved is of size" << attrList->attr[0].length << "!=" << sizeof(EntryType);
            }
#if INTERNET_TOO
            else if( isInternetPW ){
                // this is just a wild guess ...
                etype = Password;
            }
#endif
            if( entryType ){
                *entryType = etype;
            }
        }
        SecKeychainItemFreeAttributesAndData( attrList, NULL );
        CFRelease(itemRef);
    }
    kDebug() << "OSXKeychain::ItemType '" << key << "' from keychain " << OSXKeychain::Path(keychain) << "=" << OSTStr(etype) << ", error=" << err;
    return err;
}

OSStatus OSXKeychain::RemoveItem(const QString &key, const SecKeychainRef keychain)
{   const QByteArray accountName( key.toUtf8() );
    SecKeychainItemRef itemRef;
    QString errMsg;
    OSStatus result = SecKeychainFindGenericPassword( keychain, 0, NULL,
                                                     accountName.size(), accountName.constData(), NULL, NULL, &itemRef );
    if( isError( result, &errMsg ) ){
        kDebug() << "Could not find entry" << key << ":"  << qPrintable(errMsg);
    }
    else{
        const CFReleaser<SecKeychainItemRef> itemReleaser(itemRef);
        result = SecKeychainItemDelete(itemRef);
        if( isError( result, &errMsg ) ){
            kWarning() << "Could not delete entry" << key << ":"  << qPrintable(errMsg);
        }
    }
    return result;
}

OSStatus OSXKeychain::WriteItem( const QString &key, const QByteArray &value,
						   const SecKeychainRef keychain, SecKeychainItemRef *itemRef, EntryType *entryType, OSXKeychain *osxKeyChain )
{   const QByteArray accountName( key.toUtf8() );
    OSStatus err;
    QString errMsg;
    SecKeychainItemRef theItem = NULL;
    bool saveLabel;
    if( osxKeyChain && osxKeyChain->currentService.size() ){
        const QByteArray serviceName( osxKeyChain->currentService.toUtf8() );
        // save the "GenericPassword" item using the service name, which appears to be the only way to write
        // to the "Where" field shown in the Keychain Utility.
        err = basicWriteItem( &serviceName, accountName, value, keychain, &theItem );
        // the service (folder!) string will also appear on the "Name" field, which however can be changed
        // independently, via the Label attribute.
        saveLabel = true;
    }
    else{
        err = basicWriteItem( NULL, accountName, value, keychain, &theItem );
        saveLabel = false;
    }
    if( err == errSecDuplicateItem ){
        // RJVB: the previous implementation was wrong. errSecDuplicateItem means the write failed because of an existing item.
        // So we have to find that item, and modify it.
        if( !(err = ReadItem( key, NULL, keychain, &theItem )) ){
            err = SecKeychainItemModifyAttributesAndData( theItem, NULL, value.size(), value.constData() );
            if( isError( err, &errMsg ) ){
                kDebug() << "Key '" << key
                    << "'already exists in keychain but error modifying the existing item: " << qPrintable(errMsg);
            }
        }
        if( !err ){
            kDebug() << "Key '" << key << "'already existed in keychain: modified the existing item";
        }
    }
    if( !err && saveLabel ){
        // store the desired text in the label attribute
        SecKeychainAttribute attr = { kSecLabelItemAttr, accountName.size(), (void*) accountName.constData() };
        SecKeychainAttributeList attrList = { 1, &attr };
        QString errMsg;
        if( isError( (err = SecKeychainItemModifyAttributesAndData( theItem, &attrList, 0, NULL )), &errMsg ) ){
            kWarning() << "OSXKeychain::WriteItem(" << key << ") couldn't set the desired name/label" << accountName
                << ":" << err << "=" << qPrintable(errMsg);
        }
    }
    if( !err ){
        EntryType defType = Stream;
        if( !entryType ){
            entryType = &defType;
        }
        SecKeychainAttribute attr = { kSecTypeItemAttr, sizeof(EntryType), (void*) entryType };
        SecKeychainAttributeList attrList = { 1, &attr };
        QString errMsg;
        if( isError( (err = SecKeychainItemModifyAttributesAndData( theItem, &attrList, 0, NULL )), &errMsg ) ){
            kWarning() << "OSXKeychain::WriteItem(" << key << ") couldn't set type to" << OSTStr(*entryType)
                << ":" << qPrintable(errMsg);
        }
    }
    if( itemRef ){
        *itemRef = theItem;
    }
    else if( theItem ){
        CFRelease(theItem);
    }
    kDebug() << "OSXKeychain::WriteItem '" << key << "' to keychain " << (void*) keychain << ", error=" << err;
    return err;
}

OSStatus OSXKeychain::WriteItem( const QString &key, const QByteArray &value,
                                 const QString &comment, const SecKeychainRef keychain, EntryType *entryType, OSXKeychain *osxKeyChain )
{   SecKeychainItemRef itemRef = NULL;
    OSStatus err = WriteItem( key, value, keychain, &itemRef, entryType, osxKeyChain );
    if( !err && itemRef ){
        const QByteArray commentString(comment.toUtf8());
        if( commentString.size() ){
            SecKeychainAttribute attr = { kSecCommentItemAttr, commentString.size(), (void*) commentString.constData() };
            SecKeychainAttributeList attrList = { 1, &attr };
            QString errMsg;
            if( isError( (err = SecKeychainItemModifyAttributesAndData( itemRef, &attrList, 0, NULL )), &errMsg ) ){
                kWarning() << "OSXKeychain::WriteItem(" << key << ") couldn't add comment" << comment
                    << ":" << qPrintable(errMsg);
            }
        }
        CFRelease(itemRef);
    }
    return err;
}

// returns the kSecAccountItemAttr's of all items in the keychain
OSStatus OSXKeychain::ItemList( SecKeychainRef keychain, QStringList &keyList, OSXKeychain *osxKeyChain )
{   OSStatus err;
    SecKeychainSearchRef searchRef[2];
    bool generateFolderList = ( osxKeyChain && osxKeyChain->isKDEChain && osxKeyChain->generateFolderList );

    keyList.clear();
    if( generateFolderList ){
        osxKeyChain->serviceList.clear();
        if( osxKeyChain->currentService.size() > 0 ){
            osxKeyChain->serviceList.append(osxKeyChain->currentService);
        }
    }

    err = SecKeychainSearchCreateFromAttributes( keychain, kSecGenericPasswordItemClass, NULL, &searchRef[0] );
#if INTERNET_TOO
    if( SecKeychainSearchCreateFromAttributes( keychain, kSecInternetPasswordItemClass, NULL, &searchRef[1] ) ){
        searchRef[1] = NULL;
    }
#else
    searchRef[1] = NULL;
#endif
    SecKeychainItemRef item;
    QString errMsg;
    if( isError(err, &errMsg) ){
        kDebug() << "OSXKeychain::ItemList(" << (void*) keychain << "): SecKeychainSearchCreateFromAttributes failed" << qPrintable(errMsg);
    }
    else{
        for( size_t i = 0 ; i < sizeof(searchRef)/sizeof(SecKeychainSearchRef) && !err ; ++i ){
            if( searchRef[i] ){
                while( !(err = SecKeychainSearchCopyNext( searchRef[i], &item )) ){
                    if( item ){
                        // whether the item will be listed in the keyList we return: by default it is
                        // (better an item shows up multiple times than not at all).
                        bool listItem = true;
                        SecKeychainAttribute attr = { kSecAccountItemAttr, 0, NULL };
                        SecKeychainAttributeList attrList = { 1, &attr };
                        UInt32 len = 0;
                        if( osxKeyChain && osxKeyChain->isKDEChain ){
                            // try to fetch the item's ServiceItem attribute
                            attr.tag = kSecServiceItemAttr;
                            if( !SecKeychainItemCopyContent( item, NULL, &attrList, &len, NULL ) ){
                                QString lbl = QString::fromUtf8( (char*)attr.data, attr.length );
                                // we got a service item attribute, which is where we store the kwallet folder info.
                                // If we disallow empty attributes, keychain items without service item attribute will
                                // appear in each folder that has a non-empty name. In other words, we allow a folder without name.
                                if( generateFolderList ){
                                    // add the "folder" to the list if not already listed
                                    if( !osxKeyChain->serviceList.contains(lbl) ){
                                        osxKeyChain->serviceList.append(lbl);
                                    }
                                }
                                else{
                                    // only list the item if it's in the current "folder"
                                    listItem = (lbl == osxKeyChain->currentService);
                                }
                                SecKeychainItemFreeContent( &attrList, NULL );
                            }
                        }
                        else{
                            // errors retrieving the service item attribute are ignored
                        }
                        if( listItem ){
                            attr.tag = kSecAccountItemAttr;
                            if( !(err = SecKeychainItemCopyContent( item, NULL, &attrList, &len, NULL )) ){
                                if( attr.length > 0 ){
                                    keyList.append(QString::fromUtf8( (char*)attr.data, attr.length ));
                                }
                                SecKeychainItemFreeContent( &attrList, NULL );
                            }
                            else{
                                errMsg = errorString(err);
                                kDebug() << "SecKeychainItemCopyContent returned" << err << "=" << qPrintable(errMsg);
                            }
                        }
                        CFRelease(item);
                    }
                }
                if( err ){
                    errMsg = errorString(err);
                }
                CFRelease(searchRef[i]);
            }
        }
    }
    return err;
}

OSStatus OSXKeychain::Destroy( SecKeychainRef *keychain )
{   OSStatus err = SecKeychainDelete(*keychain);
    QString errMsg;
    if( isError( err, &errMsg ) ){
        kWarning() << "OSXKeychain::Destroy " << (void*) *keychain << ", error " << qPrintable(errMsg);
    }
    else{
        kWarning() << "OSXKeychain::Destroy " << (void*) *keychain << ", error=" << err;
    }
    if( keychain ){
        CFRelease(*keychain);
        *keychain = NULL;
    }
    return err;
}

OSStatus OSXKeychain::Destroy( const QString &walletName )
{   SecKeychainRef keychain;
    OSStatus err = openKeychain( walletName, &keychain );
    if( !err && keychain ){
        err = Destroy(&keychain);
    }
    return err;
}
