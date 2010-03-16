// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>
   Copyright (C) 2008 Norbert Frese <nf2@scheinwelt.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkmanager.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>
#include <QtDBus/QtDBus>
#include <QtGui/QApplication>

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkimporter.h"
#include "kbookmarkdialog.h"
#include "kbookmarkmanageradaptor_p.h"

#define BOOKMARK_CHANGE_NOTIFY_INTERFACE "org.kde.KIO.KBookmarkManager"

class KBookmarkManagerList : public QList<KBookmarkManager *>
{
public:
    ~KBookmarkManagerList() {
        qDeleteAll( begin() , end() ); // auto-delete functionality
    }

    QReadWriteLock lock;
};

K_GLOBAL_STATIC(KBookmarkManagerList, s_pSelf)

class KBookmarkMap : private KBookmarkGroupTraverser {
public:
    KBookmarkMap() : m_mapNeedsUpdate(true) {}
    void setNeedsUpdate() { m_mapNeedsUpdate = true; }
    void update(KBookmarkManager*);
    QList<KBookmark> find( const QString &url ) const
    { return m_bk_map.value(url); }
private:
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &) { ; }
    virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
    typedef QList<KBookmark> KBookmarkList;
    QMap<QString, KBookmarkList> m_bk_map;
    bool m_mapNeedsUpdate;
};

void KBookmarkMap::update(KBookmarkManager *manager)
{
    if (m_mapNeedsUpdate) {
        m_mapNeedsUpdate = false;

        m_bk_map.clear();
        KBookmarkGroup root = manager->root();
        traverse(root);
    }
}

void KBookmarkMap::visit(const KBookmark &bk)
{
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.internalElement().attribute("href")].append(bk);
    }
}

// #########################
// KBookmarkManager::Private
class KBookmarkManager::Private
{
public:
    Private(bool bDocIsloaded, const QString &dbusObjectName = QString())
      : m_doc("xbel")
      , m_dbusObjectName(dbusObjectName)
      , m_docIsLoaded(bDocIsloaded)
      , m_update(false)
      , m_browserEditor(false)
      , m_typeExternal(false)
      , m_kDirWatch(0)
    {}

    ~Private() {
        delete m_kDirWatch;
    }

    mutable QDomDocument m_doc;
    mutable QDomDocument m_toolbarDoc;
    QString m_bookmarksFile;
    QString m_dbusObjectName;
    mutable bool m_docIsLoaded;
    bool m_update;

    bool m_browserEditor;
    QString m_editorCaption;

    bool m_typeExternal;
    KDirWatch * m_kDirWatch;  // for external bookmark files

    KBookmarkMap m_map;
};

// ################
// KBookmarkManager

static KBookmarkManager* lookupExisting(const QString& bookmarksFile)
{
    for ( KBookmarkManagerList::ConstIterator bmit = s_pSelf->constBegin(), bmend = s_pSelf->constEnd();
          bmit != bmend; ++bmit ) {
        if ( (*bmit)->path() == bookmarksFile )
            return *bmit;
    }
    return 0;
}


KBookmarkManager* KBookmarkManager::managerForFile( const QString& bookmarksFile, const QString& dbusObjectName )
{
    KBookmarkManager* mgr(0);
    {
        QReadLocker readLock(&s_pSelf->lock);
        mgr = lookupExisting(bookmarksFile);
        if (mgr) {
            return mgr;
        }
    }

    QWriteLocker writeLock(&s_pSelf->lock);
    mgr = lookupExisting(bookmarksFile);
    if (mgr) {
        return mgr;
    }

    mgr = new KBookmarkManager( bookmarksFile, dbusObjectName );
    s_pSelf->append( mgr );
    return mgr;
}

KBookmarkManager* KBookmarkManager::managerForExternalFile( const QString& bookmarksFile )
{
    KBookmarkManager* mgr(0);
    {
        QReadLocker readLock(&s_pSelf->lock);
        mgr = lookupExisting(bookmarksFile);
        if (mgr) {
            return mgr;
        }
    }

    QWriteLocker writeLock(&s_pSelf->lock);
    mgr = lookupExisting(bookmarksFile);
    if (mgr) {
        return mgr;
    }

    mgr = new KBookmarkManager( bookmarksFile );
    s_pSelf->append( mgr );
    return mgr;
}


// principally used for filtered toolbars
KBookmarkManager* KBookmarkManager::createTempManager()
{
    KBookmarkManager* mgr = new KBookmarkManager();
    s_pSelf->append( mgr );
    return mgr;
}

#define PI_DATA "version=\"1.0\" encoding=\"UTF-8\""

static QDomElement createXbelTopLevelElement(QDomDocument & doc)
{
    QDomElement topLevel = doc.createElement("xbel");
    topLevel.setAttribute("xmlns:mime", "http://www.freedesktop.org/standards/shared-mime-info");
    topLevel.setAttribute("xmlns:bookmark", "http://www.freedesktop.org/standards/desktop-bookmarks");
    topLevel.setAttribute("xmlns:kdepriv", "http://www.kde.org/kdepriv");
    doc.appendChild( topLevel );
    doc.insertBefore( doc.createProcessingInstruction( "xml", PI_DATA), topLevel );
    return topLevel;
}

KBookmarkManager::KBookmarkManager( const QString & bookmarksFile, const QString & dbusObjectName)
 : d(new Private(false, dbusObjectName))
{
    if(dbusObjectName.isNull()) // get dbusObjectName from file
        if ( QFile::exists(d->m_bookmarksFile) )
            parse(); //sets d->m_dbusObjectName

    init( "/KBookmarkManager/"+d->m_dbusObjectName );

    d->m_update = true;

    Q_ASSERT( !bookmarksFile.isEmpty() );
    d->m_bookmarksFile = bookmarksFile;

    if ( !QFile::exists(d->m_bookmarksFile) )
    {
        QDomElement topLevel = createXbelTopLevelElement(d->m_doc);
        topLevel.setAttribute("dbusName", dbusObjectName);
        d->m_docIsLoaded = true;
    }
}

KBookmarkManager::KBookmarkManager(const QString & bookmarksFile)
    : d(new Private(false))
{
    // use KDirWatch to monitor this bookmarks file
    d->m_typeExternal = true;
    d->m_update = true;

    Q_ASSERT( !bookmarksFile.isEmpty() );
    d->m_bookmarksFile = bookmarksFile;

    if ( !QFile::exists(d->m_bookmarksFile) )
    {
        createXbelTopLevelElement(d->m_doc);
    }
    else
    {
        parse();
    }
    d->m_docIsLoaded = true;

    // start KDirWatch
    d->m_kDirWatch = new KDirWatch;
    d->m_kDirWatch->addFile(d->m_bookmarksFile);
    QObject::connect( d->m_kDirWatch, SIGNAL(dirty(const QString&)),
            this, SLOT(slotFileChanged(const QString&)));
    QObject::connect( d->m_kDirWatch, SIGNAL(created(const QString&)),
            this, SLOT(slotFileChanged(const QString&)));
    QObject::connect( d->m_kDirWatch, SIGNAL(deleted(const QString&)),
            this, SLOT(slotFileChanged(const QString&)));
    kDebug(7043) << "starting KDirWatch for " << d->m_bookmarksFile;
}

KBookmarkManager::KBookmarkManager( )
    : d(new Private(true))
{
    init( "/KBookmarkManager/generated" );
    d->m_update = false; // TODO - make it read/write

    createXbelTopLevelElement(d->m_doc);
}

void KBookmarkManager::init( const QString& dbusPath )
{
    // A KBookmarkManager without a dbus name is a temporary one, like those used by importers;
    // no need to register them to dbus
    if ( dbusPath != "/KBookmarkManager/" && dbusPath != "/KBookmarkManager/generated")
    {
        new KBookmarkManagerAdaptor(this);
        QDBusConnection::sessionBus().registerObject( dbusPath, this );

        QDBusConnection::sessionBus().connect(QString(), dbusPath, BOOKMARK_CHANGE_NOTIFY_INTERFACE,
                                    "bookmarksChanged", this, SLOT(notifyChanged(QString,QDBusMessage)));
        QDBusConnection::sessionBus().connect(QString(), dbusPath, BOOKMARK_CHANGE_NOTIFY_INTERFACE,
                                    "bookmarkConfigChanged", this, SLOT(notifyConfigChanged()));
    }
}

void KBookmarkManager::slotFileChanged(const QString& path)
{
    if (path == d->m_bookmarksFile)
    {
        kDebug(7043) << "file changed (KDirWatch) " << path ;
        // Reparse
        parse();
        // Tell our GUI
        // (emit where group is "" to directly mark the root menu as dirty)
        emit changed( "", QString() );
    }
}

KBookmarkManager::~KBookmarkManager()
{
    if (!s_pSelf.isDestroyed()) {
        s_pSelf->removeAll(this);
    }

    delete d;
}

void KBookmarkManager::setUpdate( bool update )
{
    d->m_update = update;
}

QDomDocument KBookmarkManager::internalDocument() const
{
    if(!d->m_docIsLoaded)
    {
        parse();
        d->m_toolbarDoc.clear();
    }
    return d->m_doc;
}


void KBookmarkManager::parse() const
{
    d->m_docIsLoaded = true;
    //kDebug(7043) << "KBookmarkManager::parse " << d->m_bookmarksFile;
    QFile file( d->m_bookmarksFile );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kWarning() << "Can't open " << d->m_bookmarksFile;
        return;
    }
    d->m_doc = QDomDocument("xbel");
    d->m_doc.setContent( &file );

    if ( d->m_doc.documentElement().isNull() )
    {
        kWarning() << "KBookmarkManager::parse : main tag is missing, creating default " << d->m_bookmarksFile;
        QDomElement element = d->m_doc.createElement("xbel");
        d->m_doc.appendChild(element);
    }

    QDomElement docElem = d->m_doc.documentElement();

    QString mainTag = docElem.tagName();
    if ( mainTag != "xbel" )
        kWarning() << "KBookmarkManager::parse : unknown main tag " << mainTag;

    if(d->m_dbusObjectName.isNull())
    {
        d->m_dbusObjectName = docElem.attribute("dbusName");
    }
    else if(docElem.attribute("dbusName") != d->m_dbusObjectName)
    {
        docElem.setAttribute("dbusName", d->m_dbusObjectName);
        save();
    }

    QDomNode n = d->m_doc.documentElement().previousSibling();
    if ( n.isProcessingInstruction() )
    {
        QDomProcessingInstruction pi = n.toProcessingInstruction();
        pi.parentNode().removeChild(pi);
    }

    QDomProcessingInstruction pi;
    pi = d->m_doc.createProcessingInstruction( "xml", PI_DATA );
    d->m_doc.insertBefore( pi, docElem );

    file.close();

    d->m_map.setNeedsUpdate();
}

bool KBookmarkManager::save( bool toolbarCache ) const
{
    return saveAs( d->m_bookmarksFile, toolbarCache );
}

bool KBookmarkManager::saveAs( const QString & filename, bool toolbarCache ) const
{
    kDebug(7043) << "KBookmarkManager::save " << filename;

    // Save the bookmark toolbar folder for quick loading
    // but only when it will actually make things quicker
    const QString cacheFilename = filename + QLatin1String(".tbcache");
    if(toolbarCache && !root().isToolbarGroup())
    {
        KSaveFile cacheFile( cacheFilename );
        if ( cacheFile.open() )
        {
            QString str;
            QTextStream stream(&str, QIODevice::WriteOnly);
            stream << root().findToolbar();
            const QByteArray cstr = str.toUtf8();
            cacheFile.write( cstr.data(), cstr.length() );
            cacheFile.finalize();
        }
    }
    else // remove any (now) stale cache
    {
        QFile::remove( cacheFilename );
    }

    KSaveFile file( filename );
    if ( file.open() )
    {
        file.simpleBackupFile( file.fileName(), QString(), ".bak" );
        QTextStream stream(&file);
        stream.setCodec( QTextCodec::codecForName( "UTF-8" ) );
        stream << internalDocument().toString();
        stream.flush();
        if ( file.finalize() )
        {
            return true;
        }
    }

    static int hadSaveError = false;
    file.abort();
    if ( !hadSaveError ) {
        QString error = i18n("Unable to save bookmarks in %1. Reported error was: %2. "
                             "This error message will only be shown once. The cause "
                             "of the error needs to be fixed as quickly as possible, "
                             "which is most likely a full hard drive.",
                         filename, file.errorString());
        if (qApp->type() != QApplication::Tty)
            KMessageBox::error( QApplication::activeWindow(), error );
        else
            kError() << error << endl;
    }
    hadSaveError = true;
    return false;
}

QString KBookmarkManager::path() const
{
    return d->m_bookmarksFile;
}

KBookmarkGroup KBookmarkManager::root() const
{
    return KBookmarkGroup(internalDocument().documentElement());
}

KBookmarkGroup KBookmarkManager::toolbar()
{
    kDebug(7043) << "KBookmarkManager::toolbar begin";
    // Only try to read from a toolbar cache if the full document isn't loaded
    if(!d->m_docIsLoaded)
    {
        kDebug(7043) << "KBookmarkManager::toolbar trying cache";
        const QString cacheFilename = d->m_bookmarksFile + QLatin1String(".tbcache");
        QFileInfo bmInfo(d->m_bookmarksFile);
        QFileInfo cacheInfo(cacheFilename);
        if (d->m_toolbarDoc.isNull() &&
            QFile::exists(cacheFilename) &&
            bmInfo.lastModified() < cacheInfo.lastModified())
        {
            kDebug(7043) << "KBookmarkManager::toolbar reading file";
            QFile file( cacheFilename );

            if ( file.open( QIODevice::ReadOnly ) )
            {
                d->m_toolbarDoc = QDomDocument("cache");
                d->m_toolbarDoc.setContent( &file );
                kDebug(7043) << "KBookmarkManager::toolbar opened";
            }
        }
        if (!d->m_toolbarDoc.isNull())
        {
            kDebug(7043) << "KBookmarkManager::toolbar returning element";
            QDomElement elem = d->m_toolbarDoc.firstChild().toElement();
            return KBookmarkGroup(elem);
        }
    }

    // Fallback to the normal way if there is no cache or if the bookmark file
    // is already loaded
    QDomElement elem = root().findToolbar();
    if (elem.isNull())
        return root(); // Root is the bookmark toolbar if none has been set.
    else
        return KBookmarkGroup(root().findToolbar());
}

KBookmark KBookmarkManager::findByAddress( const QString & address )
{
    //kDebug(7043) << "KBookmarkManager::findByAddress " << address;
    KBookmark result = root();
    // The address is something like /5/10/2+
    const QStringList addresses = address.split(QRegExp("[/+]"),QString::SkipEmptyParts);
    // kWarning() << addresses.join(",");
    for ( QStringList::const_iterator it = addresses.begin() ; it != addresses.end() ; )
    {
       bool append = ((*it) == "+");
       uint number = (*it).toUInt();
       Q_ASSERT(result.isGroup());
       KBookmarkGroup group = result.toGroup();
       KBookmark bk = group.first(), lbk = bk; // last non-null bookmark
       for ( uint i = 0 ; ( (i<number) || append ) && !bk.isNull() ; ++i ) {
           lbk = bk;
           bk = group.next(bk);
         //kWarning() << i;
       }
       it++;
       //kWarning() << "found section";
       result = bk;
    }
    if (result.isNull()) {
       kWarning() << "KBookmarkManager::findByAddress: couldn't find item " << address;
    }
    //kWarning() << "found " << result.address();
    return result;
 }

void KBookmarkManager::emitChanged()
{
    emitChanged(root());
}


void KBookmarkManager::emitChanged( const KBookmarkGroup & group )
{
    (void) save(); // KDE5 TODO: emitChanged should return a bool? Maybe rename it to saveAndEmitChanged?

    // Tell the other processes too
    // kDebug(7043) << "KBookmarkManager::emitChanged : broadcasting change " << group.address();

    emit bookmarksChanged(group.address());

    // We do get our own broadcast, so no need for this anymore
    //emit changed( group );
}

void KBookmarkManager::emitConfigChanged()
{
    emit bookmarkConfigChanged();
}

void KBookmarkManager::notifyCompleteChange( const QString &caller ) // DBUS call
{
    if (!d->m_update)
        return;

    kDebug(7043) << "KBookmarkManager::notifyCompleteChange";
    // The bk editor tells us we should reload everything
    // Reparse
    parse();
    // Tell our GUI
    // (emit where group is "" to directly mark the root menu as dirty)
    emit changed( "", caller );
}

void KBookmarkManager::notifyConfigChanged() // DBUS call
{
    kDebug() << "reloaded bookmark config!";
    KBookmarkSettings::self()->readSettings();
    parse(); // reload, and thusly recreate the menus
    emit configChanged();
}

void KBookmarkManager::notifyChanged( const QString &groupAddress, const QDBusMessage &msg ) // DBUS call
{
    kDebug() << "KBookmarkManager::notifyChanged ( "<<groupAddress<<")";
    if (!d->m_update)
        return;

    // Reparse (the whole file, no other choice)
    // if someone else notified us
    if (msg.service() != QDBusConnection::sessionBus().baseService())
       parse();

    //kDebug(7043) << "KBookmarkManager::notifyChanged " << groupAddress;
    //KBookmarkGroup group = findByAddress( groupAddress ).toGroup();
    //Q_ASSERT(!group.isNull());
    emit changed( groupAddress, QString() );
}

void KBookmarkManager::setEditorOptions( const QString& caption, bool browser )
{
    d->m_editorCaption = caption;
    d->m_browserEditor = browser;
}

void KBookmarkManager::slotEditBookmarks()
{
    QStringList args;
    if ( !d->m_editorCaption.isEmpty() )
       args << QLatin1String("--customcaption") << d->m_editorCaption;
    if ( !d->m_browserEditor )
       args << QLatin1String("--nobrowser");
    if( !d->m_dbusObjectName.isEmpty() )
      args << QLatin1String("--dbusObjectName") << d->m_dbusObjectName;
    args << d->m_bookmarksFile;
    QProcess::startDetached("keditbookmarks", args);
}

void KBookmarkManager::slotEditBookmarksAtAddress( const QString& address )
{
    QStringList args;
    if ( !d->m_editorCaption.isEmpty() )
       args << QLatin1String("--customcaption") << d->m_editorCaption;
    if ( !d->m_browserEditor )
       args << QLatin1String("--nobrowser");
    if( !d->m_dbusObjectName.isEmpty() )
      args << QLatin1String("--dbusObjectName") << d->m_dbusObjectName;
    args << QLatin1String("--address") << address
         << d->m_bookmarksFile;
    QProcess::startDetached("keditbookmarks", args);
}

///////
bool KBookmarkManager::updateAccessMetadata( const QString & url )
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    if ( list.count() == 0 )
        return false;

    for ( QList<KBookmark>::iterator it = list.begin();
          it != list.end(); ++it )
        (*it).updateAccessMetadata();

    return true;
}

void KBookmarkManager::updateFavicon( const QString &url, const QString &faviconurl )
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    for ( QList<KBookmark>::iterator it = list.begin();
          it != list.end(); ++it )
    {
        // TODO - update favicon data based on faviconurl
        //        but only when the previously used icon
        //        isn't a manually set one.
    }
}

KBookmarkManager* KBookmarkManager::userBookmarksManager()
{
     const QString bookmarksFile = KStandardDirs::locateLocal("data", QString::fromLatin1("konqueror/bookmarks.xml"));
     KBookmarkManager* bookmarkManager = KBookmarkManager::managerForFile( bookmarksFile, "konqueror" );
     bookmarkManager->setEditorOptions(KGlobal::caption(), true);
     return bookmarkManager;
}

KBookmarkSettings* KBookmarkSettings::s_self = 0;

void KBookmarkSettings::readSettings()
{
   KConfig config("kbookmarkrc", KConfig::NoGlobals);
   KConfigGroup cg(&config, "Bookmarks");

   // add bookmark dialog usage - no reparse
   s_self->m_advancedaddbookmark = cg.readEntry("AdvancedAddBookmarkDialog", false);

   // this one alters the menu, therefore it needs a reparse
   s_self->m_contextmenu = cg.readEntry("ContextMenuActions", true);
}

KBookmarkSettings *KBookmarkSettings::self()
{
   if (!s_self)
   {
      s_self = new KBookmarkSettings;
      readSettings();
   }
   return s_self;
}

/////////// KBookmarkOwner

bool KBookmarkOwner::enableOption(BookmarkOption action) const
{
    if(action == ShowAddBookmark)
        return true;
    if(action == ShowEditBookmark)
        return true;
    return false;
}

KBookmarkDialog * KBookmarkOwner::bookmarkDialog(KBookmarkManager * mgr, QWidget * parent)
{
    return new KBookmarkDialog(mgr, parent);
}

void KBookmarkOwner::openFolderinTabs(const KBookmarkGroup &)
{

}

#include "kbookmarkmanager.moc"
