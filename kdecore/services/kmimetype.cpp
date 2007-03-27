/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                2000-2007 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmimetype.h"
#include "kmimetype_p.h"
#include "kmimetypefactory.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kde_file.h>
#include <klocale.h>
#include <kmessage.h>
#include <kprotocolinfo.h>
#include <kprotocolinfofactory.h>
#include <kstandarddirs.h>
#include <ksycoca.h>
#include <kurl.h>

#include <qset.h>
#include <qstring.h>
#include <qfile.h>
#include <QtDBus/QtDBus>
#include <QBuffer>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

template class KSharedPtr<KMimeType>;

static KMimeType::Ptr s_pDefaultMimeType;

static void errorMissingMimeType( const QString& _type )
{
    KMessage::message( KMessage::Error, i18n( "Could not find mime type\n%1", _type ) );
}

static QString iconForMime( const QString& mime )
{
    QString icon = mime;
    int slashindex = icon.indexOf( QLatin1Char( '/' ) );
    if ( slashindex != -1 ) {
        icon[ slashindex ] = QLatin1Char( '-' );
    }
    return icon;
}

void KMimeTypePrivate::loadInternal( QDataStream& _str )
{
    _str >> m_lstPatterns >> m_parentMimeType;
}

/**
 * This function makes sure that the default mime type exists.
 * Not file-static because it needs access to the private KMimeType constructor...
 */
void KMimeType::buildDefaultType()
{
    assert ( !s_pDefaultMimeType );
    // Try to find the default type
    KMimeType::Ptr mime = KMimeTypeFactory::self()->
                          findMimeTypeByName( KMimeType::defaultMimeType() );

    if (mime)
    {
        s_pDefaultMimeType = mime;
    }
    else
    {
        QString defaultMimeType = KMimeType::defaultMimeType();
        errorMissingMimeType( defaultMimeType );
        QString sDefaultMimeType = KGlobal::dirs()->resourceDirs("xdgdata-mime").first()+defaultMimeType+".xml";
        s_pDefaultMimeType = new KMimeType( sDefaultMimeType, defaultMimeType, "mime" );
    }
}

KMimeType::Ptr KMimeType::defaultMimeTypePtr()
{
    if ( !s_pDefaultMimeType ) // we need a default type first
        buildDefaultType();
    return s_pDefaultMimeType;
}

/**
 * This function makes sure that vital mime types are installed.
 */
void KMimeType::checkEssentialMimeTypes()
{
    static bool s_bChecked = false;

    if ( s_bChecked ) // already done
        return;
    if ( !s_pDefaultMimeType ) // we need a default type first
        KMimeType::buildDefaultType();

  s_bChecked = true; // must be done before building mimetypes

  // No Mime-Types installed ?
  // Lets do some rescue here.
  if ( !KMimeTypeFactory::self()->checkMimeTypes() )
  {
    KMessage::message( KMessage::Error, i18n( "No mime types installed." ) );
    return; // no point in going any further
  }

  if ( !KMimeType::mimeType( "inode/directory" ) )
    errorMissingMimeType( "inode/directory" );
  //if ( !KMimeType::mimeType( "inode/directory-locked" ) )
  //  errorMissingMimeType( "inode/directory-locked" );
  if ( !KMimeType::mimeType( "inode/blockdevice" ) )
    errorMissingMimeType( "inode/blockdevice" );
  if ( !KMimeType::mimeType( "inode/chardevice" ) )
    errorMissingMimeType( "inode/chardevice" );
  if ( !KMimeType::mimeType( "inode/socket" ) )
    errorMissingMimeType( "inode/socket" );
  if ( !KMimeType::mimeType( "inode/fifo" ) )
    errorMissingMimeType( "inode/fifo" );
  if ( !KMimeType::mimeType( "application/x-shellscript" ) )
    errorMissingMimeType( "application/x-shellscript" );
  if ( !KMimeType::mimeType( "application/x-executable" ) )
    errorMissingMimeType( "application/x-executable" );
  if ( !KMimeType::mimeType( "application/x-desktop" ) )
    errorMissingMimeType( "application/x-desktop" );
}

KMimeType::Ptr KMimeType::mimeType( const QString& _name, FindByNameOption options )
{
    return KMimeTypeFactory::self()->findMimeTypeByName( _name, options );
}

KMimeType::List KMimeType::allMimeTypes()
{
    return KMimeTypeFactory::self()->allMimeTypes();
}

bool KMimeType::isBufferBinaryData(const QByteArray& data)
{
    // Check the first 32 bytes (see shared-mime spec)
    const char* p = data.data();
    const int end = qMin(32, data.size());
    for (int i = 0; i < end; ++i) {
        if (p[i] < 32) // ASCII control character
            return true;
    }
    return false;
}

static KMimeType::Ptr findFromMode( const QString& path /*only used if is_local_file*/,
                                    mode_t mode /*0 if unknown*/,
                                    bool is_local_file )
{
    if ( is_local_file && (mode == 0 || mode == (mode_t)-1) ) {
        KDE_struct_stat buff;
        if ( KDE_stat( QFile::encodeName(path), &buff ) != -1 )
            mode = buff.st_mode;
    }

    if ( S_ISDIR( mode ) ) {
        // KDE4 TODO: use an overlay instead
#if 0
        // Special hack for local files. We want to see whether we
        // are allowed to enter the directory
        if ( is_local_file )
        {
            if ( access( QFile::encodeName(path), R_OK ) == -1 )
                return KMimeType::mimeType( "inode/directory-locked" );
        }
#endif
        return KMimeType::mimeType( "inode/directory" );
    }
    if ( S_ISCHR( mode ) )
        return KMimeType::mimeType( "inode/chardevice" );
    if ( S_ISBLK( mode ) )
        return KMimeType::mimeType( "inode/blockdevice" );
    if ( S_ISFIFO( mode ) )
        return KMimeType::mimeType( "inode/fifo" );
    if ( S_ISSOCK( mode ) )
        return KMimeType::mimeType( "inode/socket" );
    // remote executable file? stop here (otherwise findFromContent can do that better for local files)
    if ( !is_local_file && S_ISREG( mode ) && ( mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
        return KMimeType::mimeType( "application/x-executable" );

    return KMimeType::Ptr();
}


KMimeType::Ptr KMimeType::findByUrlHelper( const KUrl& _url, mode_t mode,
                                           bool is_local_file,
                                           QIODevice* device,
                                           int* accuracy )
{
    checkEssentialMimeTypes();
    const QString path = _url.path();

    if (accuracy)
        *accuracy = 100;

    // Look at mode first
    KMimeType::Ptr mimeFromMode = findFromMode( path, mode, is_local_file );
    if (mimeFromMode)
        return mimeFromMode;

    // We can optimize some device reading for use by the two findFromContent calls and isBinaryData
    QByteArray beginning;
    if ( device ) {
        if ( !device->isOpen() ) {
            if ( !device->open(QIODevice::ReadOnly) ) {
                device = 0;
            }
        }
        if ( device ) {
            // Most magic rules care about 0 to 256 only, tar magic looks at offset 257, only the broken msoffice magic looks at > 2000...
            // So 256 or 512 is a good choice imho.
            beginning = device->read(512);
        }
    }

    // First try the high-priority magic matches (if we can read the data)
    if ( device ) {
        KMimeType::Ptr mime = KMimeTypeFactory::self()->findFromContent(
            device, KMimeTypeFactory::HighPriorityRules, accuracy, beginning );
        if (mime)
            return mime;
    }

    // Then try to find out by looking at the filename (if there's one)
    const QString fileName( _url.fileName() );
    static const QString& slash = KGlobal::staticQString("/");
    if ( !fileName.isEmpty() && !path.endsWith( slash ) ) {
        KMimeType::Ptr mime( KMimeTypeFactory::self()->findFromPattern( fileName ) );
        if ( mime ) {
            // Found something - can we trust it ? (e.g. don't trust *.pl over HTTP, could be anything)
            if ( is_local_file || _url.hasSubUrl() || // Explicitly trust suburls
                 KProtocolInfo::determineMimetypeFromExtension( _url.protocol() ) ) {
                if (accuracy)
                    *accuracy = 80;
                return mime;
            }
        }
    }

    // Try the low-priority magic matches (if we can read the data)
    if ( device ) {
        return KMimeTypeFactory::self()->findFromContent(
            device, KMimeTypeFactory::LowPriorityRules, accuracy, beginning );
    } else { // Not a local file, or no magic allowed, find a fallback from the protocol
        if (accuracy)
            *accuracy = 10;
        // ## this breaks with proxying; find a way to move proxying info to kdecore's kprotocolinfo?
        KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol( _url.protocol() );
        QString def;
        if (prot)
            def = prot->defaultMimeType();
        if ( !def.isEmpty() && def != defaultMimeType() ) {
            // The protocol says it always returns a given mimetype (e.g. text/html for "man:")
            return mimeType( def );
        }
        if ( path.endsWith( slash ) || path.isEmpty() ) {
            // We have no filename at all. Maybe the protocol has a setting for
            // which mimetype this means (e.g. directory).
            // For HTTP (def==defaultMimeType()) we don't assume anything,
            // because of redirections (e.g. freshmeat downloads).
            if ( def.isEmpty() ) {
                // Assume inode/directory, if the protocol supports listing.
                KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol( _url.protocol() );
                if ( prot && prot->supportsListing() )
                    return mimeType( QLatin1String("inode/directory") );
                else
                    return defaultMimeTypePtr(); // == 'no idea', e.g. for "data:,foo/"
            }
        }
    }

    if (accuracy)
        *accuracy = 0;
    return defaultMimeTypePtr();
}

KMimeType::Ptr KMimeType::findByUrl( const KUrl& url, mode_t mode,
                                     bool is_local_file, bool fast_mode,
                                     int *accuracy )
{
    if ( !is_local_file && url.isLocalFile() )
        is_local_file = true;
    if (is_local_file && !fast_mode) {
        QFile file(url.path());
        return findByUrlHelper(url, mode, is_local_file, &file, accuracy);
    }
    return findByUrlHelper(url, mode, is_local_file, 0, accuracy);
}

KMimeType::Ptr KMimeType::findByPath( const QString& path, mode_t mode,
                                      bool fast_mode, int* accuracy )
{
    KUrl url;
    url.setPath(path);
    return findByUrl(url, mode, true, fast_mode, accuracy);
}

KMimeType::Ptr KMimeType::findByNameAndContent( const QString& name, const QByteArray& data,
                                                mode_t mode, int* accuracy )
{
    KUrl url;
    url.setPath(name);
    QBuffer buffer(const_cast<QByteArray *>(&data));
    return findByUrlHelper(url, mode, false, &buffer, accuracy);
}

QString KMimeType::extractKnownExtension(const QString &fileName)
{
    QString pattern;
    KMimeTypeFactory::self()->findFromPattern( fileName, &pattern );
    if ( !pattern.isEmpty() && pattern.startsWith( "*." ) && pattern.indexOf('*', 2) == -1 )
        return pattern.mid( 2 ); // remove the leading "*."
    return QString();
}

KMimeType::Ptr KMimeType::findByContent( const QByteArray &data, int *accuracy )
{
    QBuffer buffer(const_cast<QByteArray *>(&data));
    return KMimeTypeFactory::self()->findFromContent(
        &buffer, KMimeTypeFactory::AllRules, accuracy );
}

KMimeType::Ptr KMimeType::findByFileContent( const QString &fileName, int *accuracy )
{
    QFile device(fileName);
    // Look at mode first
    KMimeType::Ptr mimeFromMode = findFromMode( fileName, 0, true );
    if (mimeFromMode)
        return mimeFromMode;

    return KMimeTypeFactory::self()->findFromContent(
        &device, KMimeTypeFactory::AllRules, accuracy );
}

bool KMimeType::isBinaryData( const QString &fileName )
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false; // err, whatever
    const QByteArray data = file.read(32);
    return isBufferBinaryData(data);
}

KMimeType::KMimeType( KMimeTypePrivate &dd, const QString & fullpath, const QString& name,
                      const QString& comment )
    : KServiceType( dd, fullpath, name, comment )
{
}

KMimeType::KMimeType( const QString & fullpath, const QString& name,
                      const QString& comment )
    : KServiceType( *new KMimeTypePrivate(this), fullpath, name, comment )
{
}

#if 0
// TODO remove once all converted to xdg mime
void KMimeType::init( KDesktopFile * config )
{
  // TODO: move X-KDE-AutoEmbed to a kde (konqueror?) specific config file
  const KConfigGroup group = config->desktopGroup();

  // Read the X-KDE-AutoEmbed setting and store it in the properties map
  QString XKDEAutoEmbed = QLatin1String("X-KDE-AutoEmbed");
  if ( group.hasKey( XKDEAutoEmbed ) )
    m_mapProps.insert( XKDEAutoEmbed, group.readEntry( XKDEAutoEmbed, false ) );

}
#endif

KMimeType::KMimeType( KMimeTypePrivate &dd, QDataStream& _str, int offset )
    : KServiceType( dd, _str, offset )
{
    Q_D(KMimeType);
    d->loadInternal( _str ); // load our specific stuff
}

KMimeType::KMimeType( QDataStream& _str, int offset )
    : KServiceType( *new KMimeTypePrivate(this), _str, offset )
{
    Q_D(KMimeType);
    d->loadInternal( _str ); // load our specific stuff
}

void KMimeType::load( QDataStream& _str )
{
    Q_D(KMimeType);
    KServiceType::load( _str );
    d->loadInternal( _str );
}

void KMimeType::save( QDataStream& _str )
{
    Q_D(KMimeType);
    KServiceType::save( _str );
    // Warning adding/removing fields here involves a binary incompatible change - update version
    // number in ksycoca.h
    _str << d->m_lstPatterns << d->m_parentMimeType;
}

QVariant KMimeType::property( const QString& _name ) const
{
    Q_D(const KMimeType);
    if ( _name == "Patterns" )
        return QVariant( d->m_lstPatterns );
    if ( _name == "Icon" )
        return QVariant( iconName() );

    return KServiceType::property( _name );
}

QStringList KMimeType::propertyNames() const
{
    QStringList res = KServiceType::propertyNames();
    res.append( "Patterns" );
    res.append( "Icon" );
    return res;
}

KMimeType::~KMimeType()
{
}

QString KMimeType::iconNameForUrl( const KUrl & _url, mode_t mode )
{
    const KMimeType::Ptr mt = findByUrl( _url, mode, _url.isLocalFile(),
                                         false /*HACK*/);
    static const QString& unknown = KGlobal::staticQString("unknown");
    const QString mimeTypeIcon = mt->iconName( _url );
    QString i = mimeTypeIcon;

    // if we don't find an icon, maybe we can use the one for the protocol
    if ( i == unknown || i.isEmpty() || mt == defaultMimeTypePtr()
        // and for the root of the protocol (e.g. trash:/) the protocol icon has priority over the mimetype icon
        || _url.path().length() <= 1 )
    {
        i = favIconForUrl( _url ); // maybe there is a favicon?

        if ( i.isEmpty() )
            i = KProtocolInfo::icon( _url.protocol() );

        // root of protocol: if we found nothing, revert to mimeTypeIcon (which is usually "folder")
        if ( _url.path().length() <= 1 && ( i == unknown || i.isEmpty() ) )
            i = mimeTypeIcon;
    }
    return !i.isEmpty() ? i : unknown;
}

QString KMimeType::favIconForUrl( const KUrl& url )
{
    // this method will be called quite often, so better not read the config
    // again and again.
    static bool useFavIcons = true;
    static bool check = true;
    if ( check ) {
        check = false;
        KConfigGroup cg( KGlobal::config(), "HTML Settings" );
        useFavIcons = cg.readEntry("EnableFavicon", true);
    }

    if ( url.isLocalFile() || !url.protocol().startsWith("http")
         || !useFavIcons )
        return QString();

    QDBusInterface kded( "org.kde.kded", "/modules/favicons", "org.kde.FavIcon" );
    QDBusReply<QString> result = kded.call( "iconForUrl", url.url() );
    return result;              // default is QString()
}

QString KMimeType::parentMimeType() const
{
    Q_D(const KMimeType);

    if (!d->m_parentMimeType.isEmpty())
        return d->m_parentMimeType;
    const QString myName = name();
    const QString myGroup = myName.left(myName.indexOf('/'));
    // All text/* types are subclasses of text/plain.
    if (myGroup == "text" && myName != "text/plain")
        return "text/plain";
    // All real-file mimetypes implicitly derive from application/octet-stream
    if (myGroup != "inode" &&
        // kde extensions
        myGroup != "all" && myGroup != "fonts" && myGroup != "media" && myGroup != "print" && myGroup != "uri"
        && myName != "application/octet-stream")
        return "application/octet-stream";
    return QString();
}

bool KMimeType::is( const QString& mimeTypeName ) const
{
  if (name() == mimeTypeName)
      return true;
  KMimeType::Ptr me = KMimeTypeFactory::self()->findMimeTypeByName(mimeTypeName, KMimeType::ResolveAliases);
  if (me && name() == me->name())
      return true;
  QString st = parentMimeType();
  //if (st.isEmpty()) kDebug(7009)<<"Parent mimetype is empty"<<endl;
  while ( !st.isEmpty() )
  {
      //kDebug(7009)<<"Checking parent mime type: "<<st<<endl;
      KMimeType::Ptr ptr = KMimeType::mimeType( st );
      if (!ptr) return false; //error
      if ( ptr->name() == mimeTypeName )
          return true;
      st = ptr->parentMimeType();
  }
  return false;
}


const QString & KMimeType::defaultMimeType()
{
    static const QString & s_strDefaultMimeType =
        KGlobal::staticQString( "application/octet-stream" );
    return s_strDefaultMimeType;
}

QString KMimeType::iconName() const
{
    return iconForMime( name() );
}

QString KMimeType::iconName( const KUrl& ) const
{
    return iconForMime( name() );
}

const QStringList& KMimeType::patterns() const
{
    Q_D(const KMimeType);
    return d->m_lstPatterns;
}

void KMimeType::addPattern(const QString& pattern)
{
    Q_D(KMimeType);
    d->m_lstPatterns.append(pattern);
}

void KMimeType::setParentMimeType(const QString& parent)
{
    Q_D(KMimeType);
    d->m_parentMimeType = parent;
}

void KMimeType::internalClearData()
{
    Q_D(KMimeType);
    // Clear that data that KBuildMimeTypeFactory is going to refill - and only that data.
    d->m_parentMimeType.clear();
    d->m_lstPatterns.clear();
}

void KMimeType::virtual_hook( int id, void* data )
{ KServiceType::virtual_hook( id, data ); }
