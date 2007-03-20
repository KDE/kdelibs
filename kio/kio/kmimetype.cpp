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
#include "kmimetypefactory.h"
#include "kprotocolmanager.h"
#include <kde_file.h>

#include <kconfiggroup.h>
#include <kmessageboxwrapper.h>

#include <kdebug.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <ksycoca.h>

#include <qset.h>
#include <qstring.h>
#include <qfile.h>
#include <QtDBus/QtDBus>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <QBuffer>

template class KSharedPtr<KMimeType>;

static KMimeType::Ptr s_pDefaultMimeType;

static void errorMissingMimeType( const QString& _type )
{
    KMessageBoxWrapper::sorry(
        0, i18n( "Could not find mime type\n%1", _type ) );
}

// KDE4 TODO: when we are in kdecore, share d pointer with KServiceType
class KMimeType::Private
{
public:
    QStringList m_lstPatterns;
    QString m_parentMimeType;
    void loadInternal( QDataStream& _str );
};

void KMimeType::Private::loadInternal( QDataStream& _str )
{
    // kDebug(7009) << "KMimeType::load( QDataStream& ) : loading list of patterns" << endl;
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
    KMessageBoxWrapper::error( 0L, i18n( "No mime types installed." ) );
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

static bool isBinaryData(const QByteArray& data)
{
    // Check the first 32 bytes (see shared-mime spec)
    const char* p = data.data();
    for (int i = 0; i < qMin(32, data.size()); ++i) {
        if (p[i] < 32) // ASCII control character
            return true;
    }
    return false;
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

    if ( device ) {
        if ( is_local_file && (mode == 0 || mode == (mode_t)-1) ) {
            KDE_struct_stat buff;
            if ( KDE_stat( QFile::encodeName(path), &buff ) != -1 )
                mode = buff.st_mode;
        }
    }

    // Look at mode_t first
    if ( S_ISDIR( mode ) ) {
        // KDE4 TODO: use an overlay instead
#if 0
        // Special hack for local files. We want to see whether we
        // are allowed to enter the directory
        if ( is_local_file )
        {
            if ( access( QFile::encodeName(path), R_OK ) == -1 )
                return mimeType( "inode/directory-locked" );
        }
#endif
        return mimeType( "inode/directory" );
    }
    if ( S_ISCHR( mode ) )
        return mimeType( "inode/chardevice" );
    if ( S_ISBLK( mode ) )
        return mimeType( "inode/blockdevice" );
    if ( S_ISFIFO( mode ) )
        return mimeType( "inode/fifo" );
    if ( S_ISSOCK( mode ) )
        return mimeType( "inode/socket" );
    // remote executable file? stop here (otherwise findFromContent can do that better for local files)
    if ( !is_local_file && S_ISREG( mode ) && ( mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
        return mimeType( "application/x-executable" );

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
        KMimeType::Ptr mime = KMimeTypeFactory::self()->findFromContent(
            device, KMimeTypeFactory::LowPriorityRules, accuracy, beginning );
        if (mime)
            return mime;

        // Nothing worked, check if the file contents looks like binary or text
        if (!::isBinaryData(beginning)) {
            if (accuracy)
                *accuracy = 5;
            return KMimeType::mimeType("text/plain");
        }

    } else { // Not a local file, or no magic allowed, find a fallback from the protocol
        if (accuracy)
            *accuracy = 10;
        QString def = KProtocolManager::defaultMimetype( _url );
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
                if ( KProtocolManager::supportsListing( _url ) )
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
    return KMimeTypeFactory::self()->findFromContent(
        &device, KMimeTypeFactory::AllRules, accuracy );
}

bool KMimeType::isBinaryData( const QString &fileName )
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false; // err, whatever
    const QByteArray data = file.read(32);
    return ::isBinaryData(data);
}

KMimeType::KMimeType( const QString & fullpath, const QString& name,
                      const QString& comment )
  : KServiceType( fullpath, name, comment ), d(new Private)
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

KMimeType::KMimeType( QDataStream& _str, int offset )
    : KServiceType( _str, offset ), d(new Private)
{
    d->loadInternal( _str ); // load our specific stuff
}

void KMimeType::load( QDataStream& _str )
{
    KServiceType::load( _str );
    d->loadInternal( _str );
}

void KMimeType::save( QDataStream& _str )
{
    KServiceType::save( _str );
    // Warning adding/removing fields here involves a binary incompatible change - update version
    // number in ksycoca.h
    _str << d->m_lstPatterns << d->m_parentMimeType;
}

QVariant KMimeType::property( const QString& _name ) const
{
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
    return d->m_parentMimeType;
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


/*******************************************************
 *
 * KFolderType
 *
 ******************************************************/

QString KFolderType::icon( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeType::iconName( _url );

  KUrl u( _url );
  u.addPath( ".directory" );

  QString icon;
  // using KStandardDirs as this one checks for path being
  // a file instead of a directory
  if ( KStandardDirs::exists( u.path() ) )
  {
    KDesktopFile cfg( u.path() );
    KConfigGroup group = cfg.desktopGroup();
    icon = group.readEntry( "Icon" );
    QString empty_icon = group.readEntry( "EmptyIcon" );

    if ( !empty_icon.isEmpty() )
    {
      bool isempty = false;
      DIR *dp = 0L;
      struct dirent *ep;
      dp = opendir( QFile::encodeName(_url.path()) );
      if ( dp )
      {
        QSet<QByteArray> entries;
        // Note that readdir isn't guaranteed to return "." and ".." first (#79826)
        ep=readdir( dp ); if ( ep ) entries.insert( ep->d_name );
        ep=readdir( dp ); if ( ep ) entries.insert( ep->d_name );
        if ( (ep=readdir( dp )) == 0L ) // third file is NULL entry -> empty directory
          isempty = true;
        else {
          entries.insert( ep->d_name );
          if ( readdir( dp ) == 0 ) { // only three
            // check if we got "." ".." and ".directory"
            isempty = entries.contains( "." ) &&
                      entries.contains( ".." ) &&
                      entries.contains( ".directory" );
          }
        }
        if (!isempty && !strcmp(ep->d_name, ".directory"))
          isempty = (readdir(dp) == 0L);
        closedir( dp );
      }

      if ( isempty )
        return empty_icon;
    }
  }

  if ( icon.isEmpty() )
    return KMimeType::iconName( _url );

  if ( icon.startsWith( "./" ) ) {
    // path is relative with respect to the location
    // of the .directory file (#73463)
    KUrl v( _url );
    v.addPath( icon.mid( 2 ) );
    icon = v.path();
  }

  return icon;
}

QString KFolderType::comment( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeType::comment( _url );

    KUrl u( _url );
    u.addPath( ".directory" );

    const KDesktopFile cfg( u.path() );
    QString comment = cfg.readComment();
    if ( comment.isEmpty() )
        return KMimeType::comment( _url );

    return comment;
}

const QString & KMimeType::defaultMimeType()
{
    static const QString & s_strDefaultMimeType =
        KGlobal::staticQString( "application/octet-stream" );
    return s_strDefaultMimeType;
}

QString KMimeType::iconName() const
{
    return name();
}

QString KMimeType::iconName( const KUrl& ) const
{
    return name();
}

const QStringList& KMimeType::patterns() const
{
    return d->m_lstPatterns;
}

void KMimeType::addPattern(const QString& pattern)
{
    d->m_lstPatterns.append(pattern);
}

void KMimeType::setParentMimeType(const QString& parent)
{
    d->m_parentMimeType = parent;
}

void KMimeType::virtual_hook( int id, void* data )
{ KServiceType::virtual_hook( id, data ); }

void KFolderType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }
