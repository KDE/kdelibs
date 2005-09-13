// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkimporter.h"
#include <kdebug.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <dcopref.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <klocale.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <kstaticdeleter.h>
#include <q3ptrstack.h>

#include "dptrtemplate.h"

class KBookmarkManagerPrivate : public dPtrTemplate<KBookmarkManager, KBookmarkManagerPrivate> {
public:
    QString m_editorCaption;
    bool m_browserEditor;
};
template<> Q3PtrDict<KBookmarkManagerPrivate>* dPtrTemplate<KBookmarkManager, KBookmarkManagerPrivate>::d_ptr = 0;

KBookmarkManagerPrivate* KBookmarkManager::dptr() const {
    return KBookmarkManagerPrivate::d( this );
}

// TODO - clean this stuff up by just using the above dptrtemplate?
Q3PtrList<KBookmarkManager>* KBookmarkManager::s_pSelf;
static KStaticDeleter<Q3PtrList<KBookmarkManager> > sdbm;

class KBookmarkMap : private KBookmarkGroupTraverser {
public:
    KBookmarkMap( KBookmarkManager * );
    void update();
    Q3ValueList<KBookmark> find( const QString &url ) const
    { return m_bk_map[url]; }
private:
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &) { ; }
    virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
    typedef Q3ValueList<KBookmark> KBookmarkList;
    QMap<QString, KBookmarkList> m_bk_map;
    KBookmarkManager *m_manager;
};

static KBookmarkMap *s_bk_map = 0;

KBookmarkMap::KBookmarkMap( KBookmarkManager *manager ) {
    m_manager = manager;
}

void KBookmarkMap::update()
{
    m_bk_map.clear();
    KBookmarkGroup root = m_manager->root();
    traverse(root);
}

void KBookmarkMap::visit(const KBookmark &bk)
{
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.internalElement().attribute("href")].append(bk);
    }
}


KBookmarkManager* KBookmarkManager::managerForFile( const QString& bookmarksFile, bool bImportDesktopFiles )
{
    if ( !s_pSelf ) {
        sdbm.setObject( s_pSelf, new Q3PtrList<KBookmarkManager> );
        s_pSelf->setAutoDelete( true );
    }
    Q3PtrListIterator<KBookmarkManager> it ( *s_pSelf );
    for ( ; it.current() ; ++it )
        if ( it.current()->path() == bookmarksFile )
            return it.current();

    KBookmarkManager* mgr = new KBookmarkManager( bookmarksFile, bImportDesktopFiles );
    s_pSelf->append( mgr );
    return mgr;
}

// principally used for filtered toolbars
KBookmarkManager* KBookmarkManager::createTempManager()
{
    if ( !s_pSelf ) {
        sdbm.setObject( s_pSelf, new Q3PtrList<KBookmarkManager> );
        s_pSelf->setAutoDelete( true );
    }
    KBookmarkManager* mgr = new KBookmarkManager();
    s_pSelf->append( mgr );
    return mgr;
}

#define PI_DATA "version=\"1.0\" encoding=\"UTF-8\""

KBookmarkManager::KBookmarkManager( const QString & bookmarksFile, bool bImportDesktopFiles )
    : DCOPObject(Q3CString("KBookmarkManager-")+bookmarksFile.utf8()), m_doc("xbel"), m_docIsLoaded(false)
{
    m_toolbarDoc.clear();

    m_update = true;
    m_showNSBookmarks = true;

    Q_ASSERT( !bookmarksFile.isEmpty() );
    m_bookmarksFile = bookmarksFile;

    if ( !QFile::exists(m_bookmarksFile) )
    {
        QDomElement topLevel = m_doc.createElement("xbel");
        m_doc.appendChild( topLevel );
        m_doc.insertBefore( m_doc.createProcessingInstruction( "xml", PI_DATA), topLevel );
        if ( bImportDesktopFiles )
            importDesktopFiles();
        m_docIsLoaded = true;
    }

    connectDCOPSignal(0, objId(), "bookmarksChanged(QString)", "notifyChanged(QString)", false);
    connectDCOPSignal(0, objId(), "bookmarkConfigChanged()", "notifyConfigChanged()", false);
}

KBookmarkManager::KBookmarkManager( )
    : DCOPObject(Q3CString("KBookmarkManager-generated")), m_doc("xbel"), m_docIsLoaded(true)
{
    m_toolbarDoc.clear(); // strange ;-)

    m_update = false; // TODO - make it read/write
    m_showNSBookmarks = true;

    m_bookmarksFile = QString::null; // AK - check all codepaths for this one

    QDomElement topLevel = m_doc.createElement("xbel");
    m_doc.appendChild( topLevel );
    m_doc.insertBefore( m_doc.createProcessingInstruction( "xml", PI_DATA), topLevel );

    // TODO - enable this via some sort of api and fix the above DCOPObject script somehow
#if 0
    connectDCOPSignal(0, objId(), "bookmarksChanged(QString)", "notifyChanged(QString)", false);
    connectDCOPSignal(0, objId(), "bookmarkConfigChanged()", "notifyConfigChanged()", false);
#endif
}

KBookmarkManager::~KBookmarkManager()
{
    if ( s_pSelf )
        s_pSelf->removeRef( this );
}

void KBookmarkManager::setUpdate( bool update )
{
    m_update = update;
}

const QDomDocument &KBookmarkManager::internalDocument() const
{
    if(!m_docIsLoaded)
    {
        parse();
        m_toolbarDoc.clear();
    }
    return m_doc;
}


void KBookmarkManager::parse() const
{
    m_docIsLoaded = true;
    //kdDebug(7043) << "KBookmarkManager::parse " << m_bookmarksFile << endl;
    QFile file( m_bookmarksFile );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kdWarning() << "Can't open " << m_bookmarksFile << endl;
        return;
    }
    m_doc = QDomDocument("xbel");
    m_doc.setContent( &file );

    QDomElement docElem = m_doc.documentElement();
    if ( docElem.isNull() )
        kdWarning() << "KBookmarkManager::parse : can't parse " << m_bookmarksFile << endl;
    else
    {
        QString mainTag = docElem.tagName();
        if ( mainTag == "BOOKMARKS" )
        {
            kdWarning() << "Old style bookmarks found. Calling convertToXBEL." << endl;
            docElem.setTagName("xbel");
            if ( docElem.hasAttribute( "HIDE_NSBK" ) && m_showNSBookmarks ) // non standard either, but we need it
            {
                docElem.setAttribute( "hide_nsbk", docElem.attribute( "HIDE_NSBK" ) == "1" ? "yes" : "no" );
                docElem.removeAttribute( "HIDE_NSBK" );
            }

            convertToXBEL( docElem );
            save();
        }
        else if ( mainTag != "xbel" )
            kdWarning() << "KBookmarkManager::parse : unknown main tag " << mainTag << endl;

        QDomNode n = m_doc.documentElement().previousSibling();
        if ( n.isProcessingInstruction() )
        {
            QDomProcessingInstruction pi = n.toProcessingInstruction();
            pi.parentNode().removeChild(pi);
        }

        QDomProcessingInstruction pi;
        pi = m_doc.createProcessingInstruction( "xml", PI_DATA );
        m_doc.insertBefore( pi, docElem );
    }

    file.close();
    if ( !s_bk_map )
        s_bk_map = new KBookmarkMap( const_cast<KBookmarkManager*>( this ) );
    s_bk_map->update();
}

void KBookmarkManager::convertToXBEL( QDomElement & group )
{
    QDomNode n = group.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if ( !e.isNull() )
            if ( e.tagName() == "TEXT" )
            {
                e.setTagName("title");
            }
            else if ( e.tagName() == "SEPARATOR" )
            {
                e.setTagName("separator"); // so close...
            }
            else if ( e.tagName() == "GROUP" )
            {
                e.setTagName("folder");
                convertAttribute(e, "ICON","icon"); // non standard, but we need it
                if ( e.hasAttribute( "TOOLBAR" ) ) // non standard either, but we need it
                {
                    e.setAttribute( "toolbar", e.attribute( "TOOLBAR" ) == "1" ? "yes" : "no" );
                    e.removeAttribute( "TOOLBAR" );
                }

                convertAttribute(e, "NETSCAPEINFO","netscapeinfo"); // idem
                bool open = (e.attribute("OPEN") == "1");
                e.removeAttribute("OPEN");
                e.setAttribute("folded", open ? "no" : "yes");
                convertToXBEL( e );
            }
            else
                if ( e.tagName() == "BOOKMARK" )
                {
                    e.setTagName("bookmark"); // so much difference :-)
                    convertAttribute(e, "ICON","icon"); // non standard, but we need it
                    convertAttribute(e, "NETSCAPEINFO","netscapeinfo"); // idem
                    convertAttribute(e, "URL","href");
                    QString text = e.text();
                    while ( !e.firstChild().isNull() ) // clean up the old contained text
                        e.removeChild(e.firstChild());
                    QDomElement titleElem = e.ownerDocument().createElement("title");
                    e.appendChild( titleElem ); // should be the only child anyway
                    titleElem.appendChild( e.ownerDocument().createTextNode( text ) );
                }
                else
                    kdWarning(7043) << "Unknown tag " << e.tagName() << endl;
        n = n.nextSibling();
    }
}

void KBookmarkManager::convertAttribute( QDomElement elem, const QString & oldName, const QString & newName )
{
    if ( elem.hasAttribute( oldName ) )
    {
        elem.setAttribute( newName, elem.attribute( oldName ) );
        elem.removeAttribute( oldName );
    }
}

void KBookmarkManager::importDesktopFiles()
{
    KBookmarkImporter importer( const_cast<QDomDocument *>(&internalDocument()) );
    QString path(KGlobal::dirs()->saveLocation("data", "kfm/bookmarks", true));
    importer.import( path );
    //kdDebug(7043) << internalDocument().toCString() << endl;

    save();
}

bool KBookmarkManager::save( bool toolbarCache ) const
{
    return saveAs( m_bookmarksFile, toolbarCache );
}

bool KBookmarkManager::saveAs( const QString & filename, bool toolbarCache ) const
{
    kdDebug(7043) << "KBookmarkManager::save " << filename << endl;

    // Save the bookmark toolbar folder for quick loading
    // but only when it will actually make things quicker
    const QString cacheFilename = filename + QLatin1String(".tbcache");
    if(toolbarCache && !root().isToolbarGroup())
    {
        KSaveFile cacheFile( cacheFilename );
        if ( cacheFile.status() == 0 )
        {
            QString str;
            QTextStream stream(&str, QIODevice::WriteOnly);
            stream << root().findToolbar();
            Q3CString cstr = str.utf8();
            cacheFile.file()->writeBlock( cstr.data(), cstr.length() );
            cacheFile.close();
        }
    }
    else // remove any (now) stale cache
    {
        QFile::remove( cacheFilename );
    }

    KSaveFile file( filename );
    if ( file.status() == 0 )
    {
        file.backupFile( file.name(), QString::null, ".bak" );
        QByteArray cstr = internalDocument().toByteArray(); // is in UTF8
        file.file()->writeBlock( cstr.data(), cstr.length() );
        if ( file.close() )
            return true;
    }

    static int hadSaveError = false;
    file.abort();
    if ( !hadSaveError ) {
        QString error = i18n("Unable to save bookmarks in %1. Reported error was: %2. "
                             "This error message will only be shown once. The cause "
                             "of the error needs to be fixed as quickly as possible, "
                             "which is most likely a full hard drive.")
                        .arg(filename).arg(strerror(file.status()));
        if (qApp->type() != QApplication::Tty)
            KMessageBox::error( 0L, error );
        else
            kdError() << error << endl;
    }
    hadSaveError = true;
    return false;
}

KBookmarkGroup KBookmarkManager::root() const
{
    return KBookmarkGroup(internalDocument().documentElement());
}

KBookmarkGroup KBookmarkManager::toolbar()
{
    kdDebug(7043) << "KBookmarkManager::toolbar begin" << endl;
    // Only try to read from a toolbar cache if the full document isn't loaded
    if(!m_docIsLoaded)
    {
        kdDebug(7043) << "KBookmarkManager::toolbar trying cache" << endl;
        const QString cacheFilename = m_bookmarksFile + QLatin1String(".tbcache");
        QFileInfo bmInfo(m_bookmarksFile);
        QFileInfo cacheInfo(cacheFilename);
        if (m_toolbarDoc.isNull() &&
            QFile::exists(cacheFilename) &&
            bmInfo.lastModified() < cacheInfo.lastModified())
        {
            kdDebug(7043) << "KBookmarkManager::toolbar reading file" << endl;
            QFile file( cacheFilename );

            if ( file.open( QIODevice::ReadOnly ) )
            {
                m_toolbarDoc = QDomDocument("cache");
                m_toolbarDoc.setContent( &file );
                kdDebug(7043) << "KBookmarkManager::toolbar opened" << endl;
            }
        }
        if (!m_toolbarDoc.isNull())
        {
            kdDebug(7043) << "KBookmarkManager::toolbar returning element" << endl;
            QDomElement elem = m_toolbarDoc.firstChild().toElement();
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

KBookmark KBookmarkManager::findByAddress( const QString & address, bool tolerant )
{
    //kdDebug(7043) << "KBookmarkManager::findByAddress " << address << endl;
    KBookmark result = root();
    // The address is something like /5/10/2+
    QStringList addresses = QStringList::split(QRegExp("[/+]"),address);
    // kdWarning() << addresses.join(",") << endl;
    for ( QStringList::Iterator it = addresses.begin() ; it != addresses.end() ; )
    {
       bool append = ((*it) == "+");
       uint number = (*it).toUInt();
       Q_ASSERT(result.isGroup());
       KBookmarkGroup group = result.toGroup();
       KBookmark bk = group.first(), lbk = bk; // last non-null bookmark
       for ( uint i = 0 ; ( (i<number) || append ) && !bk.isNull() ; ++i ) {
           lbk = bk;
           bk = group.next(bk);
         //kdWarning() << i << endl;
       }
       it++;
       int shouldBeGroup = !bk.isGroup() && (it != addresses.end());
       if ( tolerant && ( bk.isNull() || shouldBeGroup ) ) {
          if (!lbk.isNull()) result = lbk;
          //kdWarning() << "break" << endl;
          break;
       }
       //kdWarning() << "found section" << endl;
       result = bk;
    }
    if (result.isNull()) {
       kdWarning() << "KBookmarkManager::findByAddress: couldn't find item " << address << endl;
       Q_ASSERT(!tolerant);
    }
    //kdWarning() << "found " << result.address() << endl;
    return result;
 }

static QString pickUnusedTitle( KBookmarkGroup parentBookmark,
                                const QString &title, const QString &url
) {
    // If this title is already used, we'll try to find something unused.
    KBookmark ch = parentBookmark.first();
    int count = 1;
    QString uniqueTitle = title;
    do
    {
        while ( !ch.isNull() )
        {
            if ( uniqueTitle == ch.text() )
            {
                // Title already used !
                if ( url != ch.url().url() )
                {
                    uniqueTitle = title + QString(" (%1)").arg(++count);
                    // New title -> restart search from the beginning
                    ch = parentBookmark.first();
                    break;
                }
                else
                {
                    // this exact URL already exists
                    return QString::null;
                }
            }
            ch = parentBookmark.next( ch );
        }
    } while ( !ch.isNull() );

    return uniqueTitle;
}

KBookmarkGroup KBookmarkManager::addBookmarkDialog(
                     const QString & _url, const QString & _title,
                     const QString & _parentBookmarkAddress
) {
    QString url = _url;
    QString title = _title;
    QString parentBookmarkAddress = _parentBookmarkAddress;

    if ( url.isEmpty() )
    {
        KMessageBox::error( 0L, i18n("Cannot add bookmark with empty URL."));
        return KBookmarkGroup();
    }

    if ( title.isEmpty() )
        title = url;

    if ( KBookmarkSettings::self()->m_advancedaddbookmark)
    {
        KBookmarkEditDialog dlg( title, url, this, KBookmarkEditDialog::InsertionMode, parentBookmarkAddress );
        if ( dlg.exec() != KDialogBase::Accepted )
            return KBookmarkGroup();
        title = dlg.finalTitle();
        url = dlg.finalUrl();
        parentBookmarkAddress = dlg.finalAddress();
    }

    KBookmarkGroup parentBookmark;
    parentBookmark = findByAddress( parentBookmarkAddress ).toGroup();
    Q_ASSERT( !parentBookmark.isNull() );

    QString uniqueTitle = pickUnusedTitle( parentBookmark, title, url );
    if ( !uniqueTitle.isNull() )
        parentBookmark.addBookmark( this, uniqueTitle, KURL( url ));

    return parentBookmark;
}


void KBookmarkManager::emitChanged( /*KDE4 const*/ KBookmarkGroup & group )
{
    save();

    // Tell the other processes too
    // kdDebug(7043) << "KBookmarkManager::emitChanged : broadcasting change " << group.address() << endl;

    QByteArray data;
    QDataStream ds( &data, QIODevice::WriteOnly );
    ds << group.address();

    emitDCOPSignal("bookmarksChanged(QString)", data);

    // We do get our own broadcast, so no need for this anymore
    //emit changed( group );
}

void KBookmarkManager::emitConfigChanged()
{
    emitDCOPSignal("bookmarkConfigChanged()", QByteArray());
}

void KBookmarkManager::notifyCompleteChange( QString caller ) // DCOP call
{
    if (!m_update) return;

    //kdDebug(7043) << "KBookmarkManager::notifyCompleteChange" << endl;
    // The bk editor tells us we should reload everything
    // Reparse
    parse();
    // Tell our GUI
    // (emit where group is "" to directly mark the root menu as dirty)
    emit changed( "", caller );
}

void KBookmarkManager::notifyConfigChanged() // DCOP call
{
    kdDebug() << "reloaded bookmark config!" << endl;
    KBookmarkSettings::self()->readSettings();
    parse(); // reload, and thusly recreate the menus
}

void KBookmarkManager::notifyChanged( QString groupAddress ) // DCOP call
{
    if (!m_update) return;

    // Reparse (the whole file, no other choice)
    // if someone else notified us
    if (callingDcopClient()->senderId() != DCOPClient::mainClient()->appId())
       parse();

    //kdDebug(7043) << "KBookmarkManager::notifyChanged " << groupAddress << endl;
    //KBookmarkGroup group = findByAddress( groupAddress ).toGroup();
    //Q_ASSERT(!group.isNull());
    emit changed( groupAddress, QString::null );
}

bool KBookmarkManager::showNSBookmarks() const
{
    return KBookmarkMenu::showDynamicBookmarks("netscape").show;
}

void KBookmarkManager::setShowNSBookmarks( bool show )
{
    m_showNSBookmarks = show;
    if (this != userBookmarksManager())
       return;
    KBookmarkMenu::DynMenuInfo info
       = KBookmarkMenu::showDynamicBookmarks("netscape");
    info.show = show;
    KBookmarkMenu::setDynamicBookmarks("netscape", info);
}

void KBookmarkManager::setEditorOptions( const QString& caption, bool browser )
{
    dptr()->m_editorCaption = caption;
    dptr()->m_browserEditor = browser;
}

void KBookmarkManager::slotEditBookmarks()
{
    KProcess proc;
    proc << QLatin1String("keditbookmarks");
    if (!dptr()->m_editorCaption.isNull())
       proc << QLatin1String("--customcaption") << dptr()->m_editorCaption;
    if (!dptr()->m_browserEditor)
       proc << QLatin1String("--nobrowser");
    proc << m_bookmarksFile;
    proc.start(KProcess::DontCare);
}

void KBookmarkManager::slotEditBookmarksAtAddress( const QString& address )
{
    KProcess proc;
    proc << QLatin1String("keditbookmarks")
         << QLatin1String("--address") << address
         << m_bookmarksFile;
    proc.start(KProcess::DontCare);
}

///////

void KBookmarkOwner::openBookmarkURL( const QString& url )
{
  (void) new KRun(KURL( url ),(QWidget*)0);
}

void KBookmarkOwner::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

bool KBookmarkManager::updateAccessMetadata( const QString & url, bool emitSignal )
{
    if (!s_bk_map) {
        s_bk_map = new KBookmarkMap(this);
        s_bk_map->update();
    }

    Q3ValueList<KBookmark> list = s_bk_map->find(url);
    if ( list.count() == 0 )
        return false;

    for ( Q3ValueList<KBookmark>::iterator it = list.begin();
          it != list.end(); ++it )
        (*it).updateAccessMetadata();

    if (emitSignal)
        emit notifier().updatedAccessMetadata( path(), url );

    return true;
}

void KBookmarkManager::updateFavicon( const QString &url, const QString &faviconurl, bool emitSignal )
{
    Q_UNUSED(faviconurl);

    if (!s_bk_map) {
        s_bk_map = new KBookmarkMap(this);
        s_bk_map->update();
    }

    Q3ValueList<KBookmark> list = s_bk_map->find(url);
    for ( Q3ValueList<KBookmark>::iterator it = list.begin();
          it != list.end(); ++it )
    {
        // TODO - update favicon data based on faviconurl
        //        but only when the previously used icon
        //        isn't a manually set one.
    }

    if (emitSignal)
    {
        // TODO
        // emit notifier().updatedFavicon( path(), url, faviconurl );
    }
}

KBookmarkManager* KBookmarkManager::userBookmarksManager()
{
   QString bookmarksFile = locateLocal("data", QLatin1String("konqueror/bookmarks.xml"));
   return KBookmarkManager::managerForFile( bookmarksFile );
}

KBookmarkSettings* KBookmarkSettings::s_self = 0;

void KBookmarkSettings::readSettings()
{
   KConfig config("kbookmarkrc", false, false);
   config.setGroup("Bookmarks");

   // add bookmark dialog usage - no reparse
   s_self->m_advancedaddbookmark = config.readBoolEntry("AdvancedAddBookmarkDialog", false);

   // these three alter the menu, therefore all need a reparse
   s_self->m_contextmenu = config.readBoolEntry("ContextMenuActions", true);
   s_self->m_quickactions = config.readBoolEntry("QuickActionSubmenu", false);
   s_self->m_filteredtoolbar = config.readBoolEntry("FilteredToolbar", false);
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

#include "kbookmarkmanager.moc"
