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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kbookmarkmanager.h"
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
#include <qptrstack.h>

QPtrList<KBookmarkManager>* KBookmarkManager::s_pSelf;
static KStaticDeleter<QPtrList<KBookmarkManager> > sdbm;

KBookmarkManager* KBookmarkManager::managerForFile( const QString& bookmarksFile, bool bImportDesktopFiles )
{
    if ( !s_pSelf ) {
        sdbm.setObject( s_pSelf, new QPtrList<KBookmarkManager> );
        s_pSelf->setAutoDelete( true );
    }
    QPtrListIterator<KBookmarkManager> it ( *s_pSelf );
    for ( ; it.current() ; ++it )
        if ( it.current()->path() == bookmarksFile )
            return it.current();

    KBookmarkManager* mgr = new KBookmarkManager( bookmarksFile, bImportDesktopFiles );
    s_pSelf->append( mgr );
    return mgr;
}

KBookmarkManager::KBookmarkManager( const QString & bookmarksFile, bool bImportDesktopFiles )
    : DCOPObject(QCString("KBookmarkManager-")+bookmarksFile.utf8()), m_doc("xbel"), m_docIsLoaded(false)
{
    m_toolbarDoc.clear();

    m_update = true;
    m_showNSBookmarks = true;

    Q_ASSERT( !bookmarksFile.isEmpty() );
    m_bookmarksFile = bookmarksFile;

    if ( !QFile::exists(m_bookmarksFile) )
    {
        // First time we use this class
        QDomElement topLevel = m_doc.createElement("xbel");
        m_doc.appendChild( topLevel );
        if ( bImportDesktopFiles )
            importDesktopFiles();
        m_docIsLoaded = true;
    }
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
    if ( !file.open( IO_ReadOnly ) )
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
    }

    file.close();
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
    //kdDebug(7043) << "KBookmarkManager::save " << filename << endl;

    // Save the bookmark toolbar folder for quick loading 
    // but only when it will actually make things quicker
    const QString cacheFilename = filename + QString::fromLatin1(".tbcache");
    if(toolbarCache && !root().isToolbarGroup())
    {
        KSaveFile cacheFile( cacheFilename );
        if ( cacheFile.status() == 0 )
        {
            QString str;
            QTextStream stream(&str, IO_WriteOnly);
            stream << root().findToolbar();
            QCString cstr = str.utf8();
            cacheFile.file()->writeBlock( cstr.data(), cstr.length() );
            cacheFile.close();
        }
    }
    else // remove any (now) stale cache
    {
        QFile::remove( cacheFilename );
    }

    KSaveFile file( filename );

    if ( file.status() != 0 )
    {
        KMessageBox::error( 0L, i18n("Couldn't save bookmarks in %1. %2").arg(filename).arg(strerror(file.status())) );
        return false;
    }
    QCString cstr = internalDocument().toCString(); // is in UTF8
    file.file()->writeBlock( cstr.data(), cstr.length() );
    if (!file.close())
    {
        KMessageBox::error( 0L, i18n("Couldn't save bookmarks in %1. %2").arg(filename).arg(strerror(file.status())) );
        return false;
    }
    return true;
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
        const QString cacheFilename = m_bookmarksFile + QString::fromLatin1(".tbcache");
        QFileInfo bmInfo(m_bookmarksFile);
        QFileInfo cacheInfo(cacheFilename);
        if (m_toolbarDoc.isNull() &&
            QFile::exists(cacheFilename) &&
            bmInfo.lastModified() < cacheInfo.lastModified())
        {
            kdDebug(7043) << "KBookmarkManager::toolbar reading file" << endl;
            QFile file( cacheFilename );

            if ( file.open( IO_ReadOnly ) )
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

void KBookmarkManager::emitChanged( KBookmarkGroup & group )
{
    save();

    // Tell the other processes too
    //kdDebug(7043) << "KBookmarkManager::emitChanged : broadcasting change " << group.address() << endl;
    QCString objId( "KBookmarkManager-" );
    objId += m_bookmarksFile.utf8();
    DCOPRef( "*", objId ).send( "notifyChanged", group.address() );

    // We do get our own broadcast, so no need for this anymore
    //emit changed( group );
}

void KBookmarkManager::notifyCompleteChange( QString caller ) // DCOP call
{
    if (!m_update) return;

    //kdDebug(7043) << "KBookmarkManager::notifyCompleteChange" << endl;
    // The bk editor tells us we should reload everything
    // Reparse
    parse();
    // Tell our GUI
    // (emit with group == "" to directly mark the root menu as dirty)
    emit changed( "", caller );
    // Also tell specifically about the toolbar - unless it's the root as well
    KBookmarkGroup tbGroup = toolbar();
    if (!tbGroup.isNull() && !tbGroup.groupAddress().isEmpty())
        emit changed( tbGroup.groupAddress(), caller );
}

void KBookmarkManager::notifyChanged( QString groupAddress ) // DCOP call
{
    if (!m_update) return;

    // Reparse (the whole file, no other choice)
    // Of course, if we are the emitter this is a bit stupid....
    parse();

    //kdDebug(7043) << "KBookmarkManager::notifyChanged " << groupAddress << endl;
    //KBookmarkGroup group = findByAddress( groupAddress ).toGroup();
    //Q_ASSERT(!group.isNull());
    emit changed( groupAddress, QString::null );
}

bool KBookmarkManager::showNSBookmarks() const
{
    return showDynamicBookmarks("netscape").show;
}

void KBookmarkManager::setShowNSBookmarks( bool show )
{
    m_showNSBookmarks = show;
    DynMenuInfo info;
    info.type = "netscape";
    info.location = KNSBookmarkImporter::netscapeBookmarksFile();
    info.show = show;
    info.name = QString::null;
    setDynamicBookmarks(info.type, info);
}

void KBookmarkManager::slotEditBookmarks()
{
    KProcess proc;
    proc << QString::fromLatin1("keditbookmarks") << m_bookmarksFile;
    proc.start(KProcess::DontCare);
}

void KBookmarkManager::slotEditBookmarksAtAddress( const QString& address )
{
    KProcess proc;
    proc << QString::fromLatin1("keditbookmarks") 
         << QString::fromLatin1("--address") << address
         << m_bookmarksFile;
    proc.start(KProcess::DontCare);
}

///////

void KBookmarkOwner::openBookmarkURL( const QString& url )
{
  (void) new KRun(url);
}

void KBookmarkOwner::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


class KBookmarkMap : private KBookmarkGroupTraverser {
public:
    KBookmarkMap( KBookmarkManager * );
    void update();
    QValueList<KBookmark> find( const KURL &url ) const;
private:
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &) { ; }
    virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
    typedef QValueList<KBookmark> KBookmarkList;
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

void KBookmarkMap::visit(const KBookmark &bk) {
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.url().url()].append(bk);
    }
}

QValueList<KBookmark> KBookmarkMap::find( const KURL &url ) const
{
    return m_bk_map.contains(url.url()) 
         ? m_bk_map[url.url()] : QValueList<KBookmark>();
}

void KBookmarkManager::updateAccessMetadata( const QString & url, bool emitSignal )
{
    if (!s_bk_map) 
        s_bk_map = new KBookmarkMap(this);
    s_bk_map->update(); // TODO - should make update only when dirty
    
    QValueList<KBookmark> list = s_bk_map->find(url);
    for ( QValueList<KBookmark>::iterator it = list.begin(); 
          it != list.end(); ++it )
        (*it).updateAccessMetadata();
   
    if (emitSignal)
        emit notifier().updatedAccessMetadata( path(), url );
}

void KBookmarkManager::updateFavicon( const QString &url, const QString &faviconurl, bool emitSignal )
{
    Q_UNUSED(faviconurl);

    if (!s_bk_map) 
        s_bk_map = new KBookmarkMap(this);
    s_bk_map->update(); // TODO - should make update only when dirty
    
    QValueList<KBookmark> list = s_bk_map->find(url);
    for ( QValueList<KBookmark>::iterator it = list.begin(); 
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

#undef DYNMENUCONFIGPREFIX
#define DYNMENUCONFIGPREFIX "DynamicMenu-"

KBookmarkManager::DynMenuInfo KBookmarkManager::showDynamicBookmarks( const QString &id ) const 
{
   KConfig config("kbookmarkrc", false, false);
   config.setGroup("Bookmarks");

   DynMenuInfo info;
   info.show = false;

   if (!config.hasKey("DynamicMenus")) {
      // upgrade path
      if (id == "netscape") {
         info.show = root().internalElement().attribute("hide_nsbk") != "yes";
         info.location = KNSBookmarkImporter::netscapeBookmarksFile();
         info.type = "netscape";
         info.name = i18n("Netscape Bookmarks");
      } // else, no show

   } else {
      // have new version config
      if (config.hasGroup(DYNMENUCONFIGPREFIX + id)) {
         config.setGroup(DYNMENUCONFIGPREFIX + id);
         info.show = config.readBoolEntry("Show");
         info.location = config.readPathEntry("Location");
         info.type = config.readEntry("Type");
         info.name = config.readEntry("Name");
      } // else, no show
   }

   return info;
}

QStringList KBookmarkManager::dynamicBookmarksList() const
{
   KConfig config("kbookmarkrc", false, false);
   config.setGroup("Bookmarks");

   QStringList mlist;
   if (config.hasKey("DynamicMenus"))
      mlist = config.readListEntry("DynamicMenus");
   else 
      mlist << "netscape";

   return mlist;
}

void KBookmarkManager::setDynamicBookmarks(const QString &id, const DynMenuInfo &newMenu) 
{
   KConfig config("kbookmarkrc", false, false);

   // add group unconditionally
   config.setGroup(DYNMENUCONFIGPREFIX + id);
   config.writeEntry("Show", newMenu.show);
   config.writeEntry("Location", newMenu.location);
   config.writeEntry("Type", newMenu.type);
   config.writeEntry("Name", newMenu.name);

   QStringList elist;

   config.setGroup("Bookmarks");
   if (!config.hasKey("DynamicMenus")) {
      if (newMenu.type != "netscape") {
         // update from old xbel method to new rc method
         // though only if not writing the netscape setting
         config.setGroup(DYNMENUCONFIGPREFIX "netscape");
         DynMenuInfo xbelSetting;
         xbelSetting = showDynamicBookmarks("netscape");
         config.writeEntry("Show", xbelSetting.show);
         config.writeEntry("Location", xbelSetting.location);
         config.writeEntry("Type", xbelSetting.type);
         config.writeEntry("Name", xbelSetting.name);
         // maybe could reuse code?
      }
   } else {
      elist = config.readListEntry("DynamicMenus");
   }

   // make sure list includes type
   config.setGroup("Bookmarks");
   if (elist.contains(DYNMENUCONFIGPREFIX + id) < 1) {
      elist << (DYNMENUCONFIGPREFIX + id);
      config.writeEntry("DynamicMenus", elist);
   }

   config.sync();
}

#include "kbookmarkmanager.moc"
