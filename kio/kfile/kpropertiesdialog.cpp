/* This file is part of the KDE project

   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2000 David Faure <faure@kde.org>
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
 * kpropertiesdialog.cpp
 * View/Edit Properties of files, locally or remotely
 *
 * some FilePermissionsPropsPlugin-changes by
 *  Henner Zeller <zeller@think.de>
 * some layout management by
 *  Bertrand Leconte <B.Leconte@mail.dotcom.fr>
 * the rest of the layout management, bug fixes, adaptation to libkio,
 * template feature by
 *  David Faure <faure@kde.org>
 * More layout, cleanups, and fixes by
 *  Preston Brown <pbrown@kde.org>
 * Plugin capability, cleanups and port to KDialogBase by
 *  Simon Hausmann <hausmann@kde.org>
 * KDesktopPropsPlugin by
 *  Waldo Bastian <bastian@kde.org>
 */

#include <config.h>
extern "C" {
#include <pwd.h>
#include <grp.h>
#include <time.h>
}
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <qfile.h>
#include <qdir.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kdialog.h>
#include <kdirsize.h>
#include <kdirwatch.h>
#include <kdirnotify_stub.h>
#include <kdiskfreesp.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kicondialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kio/chmodjob.h>
#include <kio/renamedlg.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kmimetype.h>
#include <kmountpoint.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kcompletion.h>
#include <klineedit.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kmetaprops.h>
#include <kprocess.h>
#include <krun.h>
#include <klistview.h>
#include "kfilesharedlg.h"

#include "kpropertiesdesktopbase.h"
#include "kpropertiesdesktopadvbase.h"
#include "kpropertiesmimetypebase.h"

#include "kpropertiesdialog.h"

mode_t KFilePermissionsPropsPlugin::fperm[3][4] = {
        {S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID},
        {S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID},
        {S_IROTH, S_IWOTH, S_IXOTH, S_ISVTX}
    };

class KPropertiesDialog::KPropertiesDialogPrivate
{
public:
  KPropertiesDialogPrivate()
  {
    m_aborted = false;
  }
  ~KPropertiesDialogPrivate()
  {
  }
  bool m_aborted:1;
};

KPropertiesDialog::KPropertiesDialog (KFileItem* item,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed, i18n( "Properties for %1" ).arg(KIO::decodeFileName(item->url().fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal)
{
  d = new KPropertiesDialogPrivate;
  assert( item );
  m_items.append( new KFileItem(*item) ); // deep copy

  m_singleUrl = item->url();
  assert(!m_singleUrl.isEmpty());

  init (modal, autoShow);
}

KPropertiesDialog::KPropertiesDialog (const QString& title,
                                      QWidget* parent, const char* name, bool modal)
  : KDialogBase (KDialogBase::Tabbed, i18n ("Properties for %1").arg(title),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal)
{
  d = new KPropertiesDialogPrivate;

  init (modal, false);
}

KPropertiesDialog::KPropertiesDialog (KFileItemList _items,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(KIO::decodeFileName(_items.first()->url().fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal)
{
  d = new KPropertiesDialogPrivate;

  assert( !_items.isEmpty() );
  m_singleUrl = _items.first()->url();
  assert(!m_singleUrl.isEmpty());

  KFileItemListIterator it ( _items );
  // Deep copy
  for ( ; it.current(); ++it )
      m_items.append( new KFileItem( **it ) );

  init (modal, autoShow);
}

#ifndef KDE_NO_COMPAT
KPropertiesDialog::KPropertiesDialog (const KURL& _url, mode_t /* _mode is now unused */,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(KIO::decodeFileName(_url.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),
  m_singleUrl( _url )
{
  d = new KPropertiesDialogPrivate;

  KIO::UDSEntry entry;

  KIO::NetAccess::stat(_url, entry, parent);

  m_items.append( new KFileItem( entry, _url ) );
  init (modal, autoShow);
}
#endif

KPropertiesDialog::KPropertiesDialog (const KURL& _url,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(KIO::decodeFileName(_url.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),
  m_singleUrl( _url )
{
  d = new KPropertiesDialogPrivate;

  KIO::UDSEntry entry;

  KIO::NetAccess::stat(_url, entry, parent);

  m_items.append( new KFileItem( entry, _url ) );
  init (modal, autoShow);
}

KPropertiesDialog::KPropertiesDialog (const KURL& _tempUrl, const KURL& _currentDir,
                                      const QString& _defaultName,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed,
		 i18n( "Properties for %1" ).arg(KIO::decodeFileName(_tempUrl.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),

  m_singleUrl( _tempUrl ),
  m_defaultName( _defaultName ),
  m_currentDir( _currentDir )
{
  d = new KPropertiesDialogPrivate;

  assert(!m_singleUrl.isEmpty());

  // Create the KFileItem for the _template_ file, in order to read from it.
  m_items.append( new KFileItem( KFileItem::Unknown, KFileItem::Unknown, m_singleUrl ) );
  init (modal, autoShow);
}

void KPropertiesDialog::init (bool modal, bool autoShow)
{
  m_pageList.setAutoDelete( true );
  m_items.setAutoDelete( true );

  insertPages();

  if (autoShow)
    {
      if (!modal)
        show();
      else
        exec();
    }
}

void KPropertiesDialog::showFileSharingPage()
{
    KPropsDlgPlugin *it;

    for ( it=m_pageList.first(); it != 0L; it=m_pageList.next() )
    {
        KFileSharePropsPlugin* plugin = dynamic_cast<KFileSharePropsPlugin*>(it);
        if ( plugin )
        {
            showPage( pageIndex( plugin->page() ) );
            break;
        }
    }
}

void KPropertiesDialog::setFileNameReadOnly( bool ro )
{
    KPropsDlgPlugin *it;

    for ( it=m_pageList.first(); it != 0L; it=m_pageList.next() )
    {
        KFilePropsPlugin* plugin = dynamic_cast<KFilePropsPlugin*>(it);
        if ( plugin ) {
            plugin->setFileNameReadOnly( ro );
            break;
        }
    }
}

void KPropertiesDialog::slotStatResult( KIO::Job * )
{
}

KPropertiesDialog::~KPropertiesDialog()
{
  m_pageList.clear();
  delete d;
}

void KPropertiesDialog::insertPlugin (KPropsDlgPlugin* plugin)
{
  connect (plugin, SIGNAL (changed ()),
           plugin, SLOT (setDirty ()));

  m_pageList.append (plugin);
}

bool KPropertiesDialog::canDisplay( KFileItemList _items )
{
  // TODO: cache the result of those calls. Currently we parse .desktop files far too many times
  return KFilePropsPlugin::supports( _items ) ||
         KFilePermissionsPropsPlugin::supports( _items ) ||
         KDesktopPropsPlugin::supports( _items ) ||
         KBindingPropsPlugin::supports( _items ) ||
         KURLPropsPlugin::supports( _items ) ||
         KDevicePropsPlugin::supports( _items ) ||
         KFileMetaPropsPlugin::supports( _items );
}

void KPropertiesDialog::slotOk()
{
  KPropsDlgPlugin *page;
  d->m_aborted = false;

  KFilePropsPlugin * filePropsPlugin = 0L;
  if ( m_pageList.first()->isA("KFilePropsPlugin") )
    filePropsPlugin = static_cast<KFilePropsPlugin *>(m_pageList.first());

  // If any page is dirty, then set the main one (KFilePropsPlugin) as
  // dirty too. This is what makes it possible to save changes to a global
  // desktop file into a local one. In other cases, it doesn't hurt.
  for ( page = m_pageList.first(); page != 0L; page = m_pageList.next() )
    if ( page->isDirty() && filePropsPlugin )
    {
        filePropsPlugin->setDirty();
        break;
    }

  // Apply the changes in the _normal_ order of the tabs now
  // This is because in case of renaming a file, KFilePropsPlugin will call
  // KPropertiesDialog::rename, so other tab will be ok with whatever order
  // BUT for file copied from templates, we need to do the renaming first !
  for ( page = m_pageList.first(); page != 0L && !d->m_aborted; page = m_pageList.next() )
    if ( page->isDirty() )
    {
      kdDebug( 250 ) << "applying changes for " << page->className() << endl;
      page->applyChanges();
      // applyChanges may change d->m_aborted.
    }
    else
      kdDebug( 250 ) << "skipping page " << page->className() << endl;

  if ( !d->m_aborted && filePropsPlugin )
    filePropsPlugin->postApplyChanges();

  if ( !d->m_aborted )
  {
    emit applied();
    emit propertiesClosed();
    deleteLater();
    accept();
  } // else, keep dialog open for user to fix the problem.
}

void KPropertiesDialog::slotCancel()
{
  emit canceled();
  emit propertiesClosed();

  deleteLater();
  done( Rejected );
}

void KPropertiesDialog::insertPages()
{
  if (m_items.isEmpty())
    return;

  if ( KFilePropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFilePropsPlugin( this );
    insertPlugin (p);
  }

  if ( KFilePermissionsPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFilePermissionsPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KDesktopPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KDesktopPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KBindingPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KBindingPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KURLPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KURLPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KDevicePropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KDevicePropsPlugin( this );
    insertPlugin (p);
  }

  if ( KFileMetaPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KFileMetaPropsPlugin( this );
    insertPlugin (p);
  }


  if ( KFileSharePropsPlugin::supports( m_items ) )
  {

      QString path = m_items.first()->url().path(-1);
      bool isLocal = m_items.first()->url().isLocalFile();
      bool isIntoTrash = isLocal && path.startsWith(KGlobalSettings::trashPath());
      if ( !isIntoTrash )
      {
          KPropsDlgPlugin *p = new KFileSharePropsPlugin( this );
          insertPlugin (p);
      }
  }

  //plugins

  if ( m_items.count() != 1 )
    return;

  KFileItem *item = m_items.first();
  QString mimetype = item->mimetype();

  if ( mimetype.isEmpty() )
    return;

  QString query = QString::fromLatin1(
      "('KPropsDlg/Plugin' in ServiceTypes) and "
      "((not exist [X-KDE-Protocol]) or "
      " ([X-KDE-Protocol] == '%1'  )   )"          ).arg(item->url().protocol());

  kdDebug( 250 ) << "trader query: " << query << endl;
  KTrader::OfferList offers = KTrader::self()->query( mimetype, query );
  KTrader::OfferList::ConstIterator it = offers.begin();
  KTrader::OfferList::ConstIterator end = offers.end();
  for (; it != end; ++it )
  {
    KPropsDlgPlugin *plugin = KParts::ComponentFactory
        ::createInstanceFromLibrary<KPropsDlgPlugin>( (*it)->library().local8Bit().data(),
                                                      this,
                                                      (*it)->name().latin1() );
    if ( !plugin )
        continue;

    insertPlugin( plugin );
  }
}

void KPropertiesDialog::updateUrl( const KURL& _newUrl )
{
  Q_ASSERT( m_items.count() == 1 );
  kdDebug(250) << "KPropertiesDialog::updateUrl (pre)" << _newUrl.url() << endl;
  KURL newUrl = _newUrl;
  emit saveAs(m_singleUrl, newUrl);
  kdDebug(250) << "KPropertiesDialog::updateUrl (post)" << newUrl.url() << endl;

  m_singleUrl = newUrl;
  m_items.first()->setURL( newUrl );
  assert(!m_singleUrl.isEmpty());
  // If we have an Desktop page, set it dirty, so that a full file is saved locally
  // Same for a URL page (because of the Name= hack)
  for ( QPtrListIterator<KPropsDlgPlugin> it(m_pageList); it.current(); ++it )
   if ( it.current()->isA("KExecPropsPlugin") || // KDE4 remove me
        it.current()->isA("KURLPropsPlugin") ||
        it.current()->isA("KDesktopPropsPlugin"))
   {
     //kdDebug(250) << "Setting page dirty" << endl;
     it.current()->setDirty();
     break;
   }
}

void KPropertiesDialog::rename( const QString& _name )
{
  Q_ASSERT( m_items.count() == 1 );
  kdDebug(250) << "KPropertiesDialog::rename " << _name << endl;
  KURL newUrl;
  // if we're creating from a template : use currentdir
  if ( !m_currentDir.isEmpty() )
  {
    newUrl = m_currentDir;
    newUrl.addPath( _name );
  }
  else
  {
    QString tmpurl = m_singleUrl.url();
    if ( tmpurl.at(tmpurl.length() - 1) == '/')
      // It's a directory, so strip the trailing slash first
      tmpurl.truncate( tmpurl.length() - 1);
    newUrl = tmpurl;
    newUrl.setFileName( _name );
  }
  updateUrl( newUrl );
}

void KPropertiesDialog::abortApplying()
{
  d->m_aborted = true;
}

class KPropsDlgPlugin::KPropsDlgPluginPrivate
{
public:
  KPropsDlgPluginPrivate()
  {
  }
  ~KPropsDlgPluginPrivate()
  {
  }

  bool m_bDirty;
};

KPropsDlgPlugin::KPropsDlgPlugin( KPropertiesDialog *_props )
: QObject( _props, 0L )
{
  d = new KPropsDlgPluginPrivate;
  properties = _props;
  fontHeight = 2*properties->fontMetrics().height();
  d->m_bDirty = false;
}

KPropsDlgPlugin::~KPropsDlgPlugin()
{
  delete d;
}

bool KPropsDlgPlugin::isDesktopFile( KFileItem * _item )
{
  // only local files
  if ( !_item->isLocalFile() )
    return false;

  // only regular files
  if ( !S_ISREG( _item->mode() ) )
    return false;

  QString t( _item->url().path() );

  // only if readable
  FILE *f = fopen( QFile::encodeName(t), "r" );
  if ( f == 0L )
    return false;
  fclose(f);

  // return true if desktop file
  return ( _item->mimetype() == "application/x-desktop" );
}

void KPropsDlgPlugin::setDirty( bool b )
{
  d->m_bDirty = b;
}

void KPropsDlgPlugin::setDirty()
{
  d->m_bDirty = true;
}

bool KPropsDlgPlugin::isDirty() const
{
  return d->m_bDirty;
}

void KPropsDlgPlugin::applyChanges()
{
  kdWarning(250) << "applyChanges() not implemented in page !" << endl;
}

///////////////////////////////////////////////////////////////////////////////

class KFilePropsPlugin::KFilePropsPluginPrivate
{
public:
  KFilePropsPluginPrivate()
  {
    dirSizeJob = 0L;
    dirSizeUpdateTimer = 0L;
    m_lined = 0;
  }
  ~KFilePropsPluginPrivate()
  {
    if ( dirSizeJob )
      dirSizeJob->kill();
  }

  KDirSize * dirSizeJob;
  QTimer *dirSizeUpdateTimer;
  QFrame *m_frame;
  bool bMultiple;
  bool bIconChanged;
  bool bKDesktopMode;
  QLabel *m_freeSpaceLabel;
  QString mimeType;
  KLineEdit* m_lined;
};

KFilePropsPlugin::KFilePropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KFilePropsPluginPrivate;
  d->bMultiple = (properties->items().count() > 1);
  d->bIconChanged = false;
  d->bKDesktopMode = (QCString(qApp->name()) == "kdesktop"); // nasty heh?
  kdDebug(250) << "KFilePropsPlugin::KFilePropsPlugin bMultiple=" << d->bMultiple << endl;

  // We set this data from the first item, and we'll
  // check that the other items match against it, resetting when not.
  bool isLocal = properties->kurl().isLocalFile();
  KFileItem * item = properties->item();
  bool bDesktopFile = isDesktopFile(item);
  mode_t mode = item->mode();
  bool hasDirs = item->isDir() && !item->isLink();
  bool hasRoot = isLocal && properties->kurl().path() == QString::fromLatin1("/");
  QString iconStr = KMimeType::iconForURL(properties->kurl(), mode);
  QString directory = properties->kurl().directory();
  QString protocol = properties->kurl().protocol();
  QString mimeComment = item->mimeComment();
  d->mimeType = item->mimetype();
  KIO::filesize_t totalSize = item->size();
  QString magicMimeComment;
  if ( isLocal ) {
      KMimeType::Ptr magicMimeType = KMimeType::findByFileContent( properties->kurl().path() );
      if ( magicMimeType->name() != KMimeType::defaultMimeType() )
          magicMimeComment = magicMimeType->comment();
  }

  // Those things only apply to 'single file' mode
  QString filename = QString::null;
  bool isTrash = false;
  bool isIntoTrash = false;
  bool isDevice = false;
  m_bFromTemplate = false;

  // And those only to 'multiple' mode
  uint iDirCount = S_ISDIR(mode) ? 1 : 0;
  uint iFileCount = 1-iDirCount;

  d->m_frame = properties->addPage (i18n("&General"));

  QVBoxLayout *vbl = new QVBoxLayout( d->m_frame, 0,
                                      KDialog::spacingHint(), "vbl");
  QGridLayout *grid = new QGridLayout(0, 3); // unknown rows
  grid->setColStretch(0, 0);
  grid->setColStretch(1, 0);
  grid->setColStretch(2, 1);
  grid->addColSpacing(1, KDialog::spacingHint());
  vbl->addLayout(grid);
  int curRow = 0;

  if ( !d->bMultiple )
  {
    // Extract the file name only
    filename = properties->defaultName();
    if ( filename.isEmpty() ) // no template
      filename = properties->kurl().fileName();
    else
    {
      m_bFromTemplate = true;
      setDirty(); // to enforce that the copy happens
    }

    bool isDesktopFile = KDesktopPropsPlugin::supports(properties->items());
    if ( d->bKDesktopMode && isDesktopFile ) {
        KDesktopFile config( properties->kurl().path(), true /* readonly */ );
        if ( config.hasKey( "Name" ) ) {
            filename = config.readName();
        }
    }

    oldName = filename;

    // Make it human-readable (%2F => '/', ...)
    filename = KIO::decodeFileName( filename );

    QString path;

    if ( !m_bFromTemplate ) {
      QString tmp = properties->kurl().path( 1 );
      // is it the trash bin ?
      if ( isLocal )
      {
          if ( tmp == KGlobalSettings::trashPath())
              isTrash = true;
          if ( tmp.startsWith(KGlobalSettings::trashPath()))
              isIntoTrash = true;
      }
      if ( properties->kurl().protocol().find("device", 0, false)==0)
            isDevice = true;
      // Extract the full name, but without file: for local files
      if ( isLocal )
        path = properties->kurl().path();
      else
        path = properties->kurl().prettyURL();
    } else {
      path = properties->currentDir().path(1) + properties->defaultName();
      directory = properties->currentDir().prettyURL();
    }

    if (KExecPropsPlugin::supports(properties->items()) || // KDE4 remove me
        isDesktopFile ||
        KBindingPropsPlugin::supports(properties->items())) {

      determineRelativePath( path );

    }

  }
  else
  {
    // Multiple items: see what they have in common
    KFileItemList items = properties->items();
    KFileItemListIterator it( items );
    for ( ++it /*no need to check the first one again*/ ; it.current(); ++it )
    {
      KURL url = (*it)->url();
      kdDebug(250) << "KFilePropsPlugin::KFilePropsPlugin " << url.prettyURL() << endl;
      // The list of things we check here should match the variables defined
      // at the beginning of this method.
      if ( url.isLocalFile() != isLocal )
        isLocal = false; // not all local
      if ( bDesktopFile && isDesktopFile(*it) != bDesktopFile )
        bDesktopFile = false; // not all desktop files
      if ( (*it)->mode() != mode )
        mode = (mode_t)0;
      if ( KMimeType::iconForURL(url, mode) != iconStr )
        iconStr = "kmultiple";
      if ( url.directory() != directory )
        directory = QString::null;
      if ( url.protocol() != protocol )
        protocol = QString::null;
      if ( !mimeComment.isNull() && (*it)->mimeComment() != mimeComment )
        mimeComment = QString::null;
      if ( isLocal && !magicMimeComment.isNull() ) {
          KMimeType::Ptr magicMimeType = KMimeType::findByFileContent( url.path() );
          if ( magicMimeType->comment() != magicMimeComment )
              magicMimeComment = QString::null;
      }

      if ( isLocal && url.path() == QString::fromLatin1("/") )
        hasRoot = true;
      if ( (*it)->isDir() && !(*it)->isLink() )
      {
        iDirCount++;
        hasDirs = true;
      }
      else
      {
        iFileCount++;
        totalSize += (*it)->size();
      }
    }
  }

  if (!isLocal && !protocol.isEmpty())
  {
    directory += ' ';
    directory += '(';
    directory += protocol;
    directory += ')';
  }

  if ( !isDevice && !isIntoTrash && (bDesktopFile || S_ISDIR(mode)) && !d->bMultiple /*not implemented for multiple*/ )
  {
    KIconButton *iconButton = new KIconButton( d->m_frame );
    iconButton->setFixedSize(70, 70);
    iconButton->setStrictIconSize(false);
    // This works for everything except Device icons on unmounted devices
    // So we have to really open .desktop files
    QString iconStr = KMimeType::findByURL( properties->kurl(),
                                            mode )->icon( properties->kurl(),
                                                          isLocal );
    if ( bDesktopFile && isLocal )
    {
      KDesktopFile config( properties->kurl().path(), true );
      config.setDesktopGroup();
      iconStr = config.readEntry( "Icon" );
      if ( config.hasDeviceType() )
	iconButton->setIconType( KIcon::Desktop, KIcon::Device );
      else
	iconButton->setIconType( KIcon::Desktop, KIcon::Application );
    } else
      iconButton->setIconType( KIcon::Desktop, KIcon::FileSystem );
    iconButton->setIcon(iconStr);
    iconArea = iconButton;
    connect( iconButton, SIGNAL( iconChanged(QString) ),
             this, SLOT( slotIconChanged() ) );
  } else {
    QLabel *iconLabel = new QLabel( d->m_frame );
    iconLabel->setFixedSize(70, 70);
    iconLabel->setPixmap( DesktopIcon( iconStr ) );
    iconArea = iconLabel;
  }
  grid->addWidget(iconArea, curRow, 0, AlignLeft);

  if (d->bMultiple || isTrash || isIntoTrash || isDevice || filename == QString::fromLatin1("/"))
  {
    QLabel *lab = new QLabel(d->m_frame );
    if ( d->bMultiple )
      lab->setText( KIO::itemsSummaryString( iFileCount + iDirCount, iFileCount, iDirCount, 0, false ) );
    else
      lab->setText( filename );
    nameArea = lab;
  } else
  {
    d->m_lined = new KLineEdit( d->m_frame );
    d->m_lined->setText(filename);
    nameArea = d->m_lined;
    d->m_lined->setFocus();
    connect( d->m_lined, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( nameFileChanged(const QString & ) ) );
  }

  grid->addWidget(nameArea, curRow++, 2);

  KSeparator* sep = new KSeparator( KSeparator::HLine, d->m_frame);
  grid->addMultiCellWidget(sep, curRow, curRow, 0, 2);
  ++curRow;

  QLabel *l;
  if ( !mimeComment.isEmpty() && !isDevice && !isIntoTrash)
  {
    l = new QLabel(i18n("Type:"), d->m_frame );

    grid->addWidget(l, curRow, 0);

    QHBox *box = new QHBox(d->m_frame);
    l = new QLabel(mimeComment, box );

    QPushButton *button = new QPushButton(box);

    QIconSet iconSet = SmallIconSet(QString::fromLatin1("configure"));
    QPixmap pixMap = iconSet.pixmap( QIconSet::Small, QIconSet::Normal );
    button->setIconSet( iconSet );
    button->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
    QToolTip::add(button, i18n("Edit file type"));

    connect( button, SIGNAL( clicked() ), SLOT( slotEditFileType() ));


    grid->addWidget(box, curRow++, 2);
  }

  if ( !magicMimeComment.isEmpty() && magicMimeComment != mimeComment )
  {
    l = new QLabel(i18n("Contents:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new QLabel(magicMimeComment, d->m_frame );
    grid->addWidget(l, curRow++, 2);
  }

  if ( !directory.isEmpty() )
  {
    l = new QLabel( i18n("Location:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new KSqueezedTextLabel( d->m_frame );
    l->setText( directory );
    grid->addWidget(l, curRow++, 2);
  }

  l = new QLabel(i18n("Size:"), d->m_frame );
  grid->addWidget(l, curRow, 0);

  m_sizeLabel = new QLabel( d->m_frame );
  grid->addWidget( m_sizeLabel, curRow++, 2 );

  if ( !hasDirs ) // Only files [and symlinks]
  {
    m_sizeLabel->setText(QString::fromLatin1("%1 (%2)").arg(KIO::convertSize(totalSize))
			 .arg(KGlobal::locale()->formatNumber(totalSize, 0)));
    m_sizeDetermineButton = 0L;
    m_sizeStopButton = 0L;
  }
  else // Directory
  {
    QHBoxLayout * sizelay = new QHBoxLayout(KDialog::spacingHint());
    grid->addLayout( sizelay, curRow++, 2 );

    // buttons
    m_sizeDetermineButton = new QPushButton( i18n("Calculate"), d->m_frame );
    m_sizeStopButton = new QPushButton( i18n("Stop"), d->m_frame );
    connect( m_sizeDetermineButton, SIGNAL( clicked() ), this, SLOT( slotSizeDetermine() ) );
    connect( m_sizeStopButton, SIGNAL( clicked() ), this, SLOT( slotSizeStop() ) );
    sizelay->addWidget(m_sizeDetermineButton, 0);
    sizelay->addWidget(m_sizeStopButton, 0);
    sizelay->addStretch(10); // so that the buttons don't grow horizontally

    // auto-launch for local dirs only, and not for '/'
    if ( isLocal && !hasRoot )
    {
      m_sizeDetermineButton->setText( i18n("Refresh") );
      slotSizeDetermine();
    }
    else
      m_sizeStopButton->setEnabled( false );
  }

  if ( isLocal )
  {
      QString mountPoint = KIO::findPathMountPoint( properties->item()->url().path() );

      if (mountPoint != "/")
      {
          l = new QLabel(i18n("Mounted on:"), d->m_frame );
          grid->addWidget(l, curRow, 0);

          l = new KSqueezedTextLabel( mountPoint, d->m_frame );
          grid->addWidget( l, curRow++, 2 );
      }

      l = new QLabel(i18n("Free disk space:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      d->m_freeSpaceLabel = new QLabel( d->m_frame );
      grid->addWidget( d->m_freeSpaceLabel, curRow++, 2 );

      KDiskFreeSp * job = new KDiskFreeSp;
      connect( job, SIGNAL( foundMountPoint( const unsigned long&, const unsigned long&,
					     const unsigned long&, const QString& ) ),
               this, SLOT( slotFoundMountPoint( const unsigned long&, const unsigned long&,
						const unsigned long&, const QString& ) ) );
      job->readDF( mountPoint );
  }

  if (!d->bMultiple && item->isLink()) {
    l = new QLabel(i18n("Points to:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new QLabel(item->linkDest(), d->m_frame );
    grid->addWidget(l, curRow++, 2);
  }

  if (!d->bMultiple) // Dates for multiple don't make much sense...
  {
    sep = new KSeparator( KSeparator::HLine, d->m_frame);
    grid->addMultiCellWidget(sep, curRow, curRow, 0, 2);
    ++curRow;

    QDateTime dt;
    time_t tim = item->time(KIO::UDS_CREATION_TIME);
    if ( tim )
    {
      l = new QLabel(i18n("Created:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      dt.setTime_t( tim );
      l = new QLabel(KGlobal::locale()->formatDateTime(dt), d->m_frame );
      grid->addWidget(l, curRow++, 2);
    }

    tim = item->time(KIO::UDS_MODIFICATION_TIME);
    if ( tim )
    {
      l = new QLabel(i18n("Modified:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      dt.setTime_t( tim );
      l = new QLabel(KGlobal::locale()->formatDateTime(dt), d->m_frame );
      grid->addWidget(l, curRow++, 2);
    }

    tim = item->time(KIO::UDS_ACCESS_TIME);
    if ( tim )
    {
      l = new QLabel(i18n("Accessed:"), d->m_frame );
      grid->addWidget(l, curRow, 0);

      dt.setTime_t( tim );
      l = new QLabel(KGlobal::locale()->formatDateTime(dt), d->m_frame );
      grid->addWidget(l, curRow++, 2);
    }
  }
  vbl->addStretch(1);
}

// QString KFilePropsPlugin::tabName () const
// {
//   return i18n ("&General");
// }

void KFilePropsPlugin::setFileNameReadOnly( bool ro )
{
  if ( d->m_lined )
    d->m_lined->setReadOnly( ro );
}

void KFilePropsPlugin::slotEditFileType()
{
  QString keditfiletype = QString::fromLatin1("keditfiletype");
  KRun::runCommand( keditfiletype + " " + KProcess::quote(d->mimeType),
                    keditfiletype, keditfiletype /*unused*/);
}

void KFilePropsPlugin::slotIconChanged()
{
  d->bIconChanged = true;
  emit changed();
}

void KFilePropsPlugin::nameFileChanged(const QString &text )
{
  properties->enableButtonOK(!text.isEmpty());
  emit changed();
}

void KFilePropsPlugin::determineRelativePath( const QString & path )
{
    // now let's make it relative
    QStringList dirs;
    if (KBindingPropsPlugin::supports(properties->items()))
    {
       m_sRelativePath =KGlobal::dirs()->relativeLocation("mime", path);
       if (m_sRelativePath.startsWith("/"))
          m_sRelativePath = QString::null;
    }
    else
    {
       m_sRelativePath =KGlobal::dirs()->relativeLocation("apps", path);
       if (m_sRelativePath.startsWith("/"))
       {
          m_sRelativePath =KGlobal::dirs()->relativeLocation("xdgdata-apps", path);
          if (m_sRelativePath.startsWith("/"))
             m_sRelativePath = QString::null;
          else
             m_sRelativePath = path;
       }
    }
    if ( m_sRelativePath.isEmpty() )
    {
      if (KBindingPropsPlugin::supports(properties->items()))
        kdWarning(250) << "Warning : editing a mimetype file out of the mimetype dirs!" << endl;
    }
}

void KFilePropsPlugin::slotFoundMountPoint( const QString&,
					    unsigned long kBSize,
					    unsigned long /*kBUsed*/,
					    unsigned long kBAvail )
{
    d->m_freeSpaceLabel->setText(
	i18n("Available space out of total partition size (percent used)", "%1 out of %2 (%3% used)")
	.arg(KIO::convertSizeFromKB(kBAvail))
	.arg(KIO::convertSizeFromKB(kBSize))
	.arg( 100 - (int)(100.0 * kBAvail / kBSize) ));
}

// attention: copy&paste below, due to compiler bug
// it doesn't like those unsigned long parameters -- unsigned long& are ok :-/
void KFilePropsPlugin::slotFoundMountPoint( const unsigned long& kBSize,
					    const unsigned long& /*kBUsed*/,
					    const unsigned long& kBAvail,
					    const QString& )
{
    d->m_freeSpaceLabel->setText(
	i18n("Available space out of total partition size (percent used)", "%1 out of %2 (%3% used)")
	.arg(KIO::convertSizeFromKB(kBAvail))
	.arg(KIO::convertSizeFromKB(kBSize))
	.arg( 100 - (int)(100.0 * kBAvail / kBSize) ));
}

void KFilePropsPlugin::slotDirSizeUpdate()
{
    KIO::filesize_t totalSize = d->dirSizeJob->totalSize();
    m_sizeLabel->setText( i18n("Calculating... %1 (%2)")
			  .arg(KIO::convertSize(totalSize))
			  .arg(KGlobal::locale()->formatNumber(totalSize, 0)) );
}

void KFilePropsPlugin::slotDirSizeFinished( KIO::Job * job )
{
  if (job->error())
    m_sizeLabel->setText( job->errorString() );
  else
  {
    KIO::filesize_t totalSize = static_cast<KDirSize*>(job)->totalSize();
    m_sizeLabel->setText( QString::fromLatin1("%1 (%2)")
			  .arg(KIO::convertSize(totalSize))
			  .arg(KGlobal::locale()->formatNumber(totalSize, 0)) );
  }
  m_sizeStopButton->setEnabled(false);
  // just in case you change something and try again :)
  m_sizeDetermineButton->setText( i18n("Refresh") );
  m_sizeDetermineButton->setEnabled(true);
  d->dirSizeJob = 0L;
  delete d->dirSizeUpdateTimer;
  d->dirSizeUpdateTimer = 0L;
}

void KFilePropsPlugin::slotSizeDetermine()
{
  m_sizeLabel->setText( i18n("Calculating...") );
  kdDebug(250) << " KFilePropsPlugin::slotSizeDetermine() properties->item()=" <<  properties->item() << endl;
  kdDebug(250) << " URL=" << properties->item()->url().url() << endl;
  d->dirSizeJob = KDirSize::dirSizeJob( properties->items() );
  d->dirSizeUpdateTimer = new QTimer(this);
  connect( d->dirSizeUpdateTimer, SIGNAL( timeout() ),
           SLOT( slotDirSizeUpdate() ) );
  d->dirSizeUpdateTimer->start(500);
  connect( d->dirSizeJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotDirSizeFinished( KIO::Job * ) ) );
  m_sizeStopButton->setEnabled(true);
  m_sizeDetermineButton->setEnabled(false);
}

void KFilePropsPlugin::slotSizeStop()
{
  if ( d->dirSizeJob )
  {
    m_sizeLabel->setText( i18n("Stopped") );
    d->dirSizeJob->kill();
    d->dirSizeJob = 0;
  }
  if ( d->dirSizeUpdateTimer )
    d->dirSizeUpdateTimer->stop();

  m_sizeStopButton->setEnabled(false);
  m_sizeDetermineButton->setEnabled(true);
}

KFilePropsPlugin::~KFilePropsPlugin()
{
  delete d;
}

bool KFilePropsPlugin::supports( KFileItemList /*_items*/ )
{
  return true;
}

// Don't do this at home
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void KFilePropsPlugin::applyChanges()
{
  if ( d->dirSizeJob )
    slotSizeStop();

  kdDebug(250) << "KFilePropsPlugin::applyChanges" << endl;

  if (nameArea->inherits("QLineEdit"))
  {
    QString n = KIO::encodeFileName(((QLineEdit *) nameArea)->text());
    // Remove trailing spaces (#4345)
    while ( n[n.length()-1].isSpace() )
      n.truncate( n.length() - 1 );
    if ( n.isEmpty() )
    {
      KMessageBox::sorry( properties, i18n("The new file name is empty!"));
      properties->abortApplying();
      return;
    }

    // Do we need to rename the file ?
    kdDebug(250) << "oldname = " << oldName << endl;
    kdDebug(250) << "newname = " << n << endl;
    if ( oldName != n || m_bFromTemplate ) { // true for any from-template file
      KIO::Job * job = 0L;
      KURL oldurl = properties->kurl();
      // Tell properties. Warning, this changes the result of properties->kurl() !
      properties->rename( n );

      // Update also relative path (for apps and mimetypes)
      if ( !m_sRelativePath.isEmpty() )
        determineRelativePath( properties->kurl().path() );

      kdDebug(250) << "New URL = " << properties->kurl().url() << endl;
      kdDebug(250) << "old = " << oldurl.url() << endl;

      // Don't remove the template !!
      if ( !m_bFromTemplate ) // (normal renaming)
        job = KIO::move( oldurl, properties->kurl() );
      else // Copying a template
        job = KIO::copy( oldurl, properties->kurl() );

      connect( job, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotCopyFinished( KIO::Job * ) ) );
      connect( job, SIGNAL( renamed( KIO::Job *, const KURL &, const KURL & ) ),
               SLOT( slotFileRenamed( KIO::Job *, const KURL &, const KURL & ) ) );
      // wait for job
      QWidget dummy(0,0,WType_Dialog|WShowModal);
      qt_enter_modal(&dummy);
      qApp->enter_loop();
      qt_leave_modal(&dummy);
      return;
    }
  }

  // No job, keep going
  slotCopyFinished( 0L );
}

void KFilePropsPlugin::slotCopyFinished( KIO::Job * job )
{
  kdDebug(250) << "KFilePropsPlugin::slotCopyFinished" << endl;
  if (job)
  {
    // allow apply() to return
    qApp->exit_loop();
    if ( job->error() )
    {
        job->showErrorDialog( d->m_frame );
        // Didn't work. Revert the URL to the old one
        properties->updateUrl( static_cast<KIO::CopyJob*>(job)->srcURLs().first() );
        properties->abortApplying(); // Don't apply the changes to the wrong file !
        return;
    }
  }

  assert( properties->item() );
  assert( !properties->item()->url().isEmpty() );
  bool isDesktopFile = KDesktopPropsPlugin::supports(properties->items());

  // Save the file where we can -> usually in ~/.kde/...
  if (KBindingPropsPlugin::supports(properties->items()) && !m_sRelativePath.isEmpty())
  {
    KURL newURL;
    newURL.setPath( locateLocal("mime", m_sRelativePath) );
    properties->updateUrl( newURL );
  }
  else if (isDesktopFile && !m_sRelativePath.isEmpty())
  {
    kdDebug(250) << "KFilePropsPlugin::slotCopyFinished " << m_sRelativePath << endl;
    KURL newURL;
    newURL.setPath( KDesktopFile::locateLocal(m_sRelativePath) );
    kdDebug(250) << "KFilePropsPlugin::slotCopyFinished path=" << newURL.path() << endl;
    properties->updateUrl( newURL );
  }

  if ( d->bKDesktopMode && isDesktopFile ) {
      // Renamed? Update Name field
      if ( oldName != properties->kurl().fileName() || m_bFromTemplate ) {
          KDesktopFile config( properties->kurl().path() );
          QString nameStr = properties->kurl().fileName();
          config.writeEntry( "Name", nameStr );
          config.writeEntry( "Name", nameStr, true, false, true );
      }
  }
}

void KFilePropsPlugin::applyIconChanges()
{
  // handle icon changes - only local files for now
  // TODO: Use KTempFile and KIO::file_copy with overwrite = true
  if (!iconArea->isA("QLabel") && properties->kurl().isLocalFile() && d->bIconChanged) {
    KIconButton *iconButton = (KIconButton *) iconArea;
    QString path;

    if (S_ISDIR(properties->item()->mode()))
    {
      path = properties->kurl().path(1) + QString::fromLatin1(".directory");
      // don't call updateUrl because the other tabs (i.e. permissions)
      // apply to the directory, not the .directory file.
    }
    else
      path = properties->kurl().path();

    // Get the default image
    QString str = KMimeType::findByURL( properties->kurl(),
                                        properties->item()->mode(),
                                        true )->KServiceType::icon();
    // Is it another one than the default ?
    QString sIcon;
    if ( str != iconButton->icon() )
      sIcon = iconButton->icon();
    // (otherwise write empty value)

    kdDebug(250) << "**" << path << "**" << endl;
    QFile f( path );

    // If default icon and no .directory file -> don't create one
    if ( !sIcon.isEmpty() || f.exists() )
    {
        if ( !f.open( IO_ReadWrite ) ) {
          KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not "
				      "have sufficient access to write to <b>%1</b>.</qt>").arg(path));
          return;
        }
        f.close();

        KDesktopFile cfg(path);
        kdDebug(250) << "sIcon = " << (sIcon) << endl;
        kdDebug(250) << "str = " << (str) << endl;
        cfg.writeEntry( "Icon", sIcon );
        cfg.sync();
    }
  }
}

void KFilePropsPlugin::slotFileRenamed( KIO::Job *, const KURL &, const KURL & newUrl )
{
  // This is called in case of an existing local file during the copy/move operation,
  // if the user chooses Rename.
  properties->updateUrl( newUrl );
}

void KFilePropsPlugin::postApplyChanges()
{
  // Save the icon only after applying the permissions changes (#46192)
  applyIconChanges();

  KURL::List lst;
  KFileItemList items = properties->items();
  for ( KFileItemListIterator it( items ); it.current(); ++it )
    lst.append((*it)->url());
  KDirNotify_stub allDirNotify("*", "KDirNotify*");
  allDirNotify.FilesChanged( lst );
}

class KFilePermissionsPropsPlugin::KFilePermissionsPropsPluginPrivate
{
public:
  KFilePermissionsPropsPluginPrivate()
  {
  }
  ~KFilePermissionsPropsPluginPrivate()
  {
  }

  QFrame *m_frame;
  QCheckBox *cbRecursive;
  QLabel *explanationLabel;
  QComboBox *ownerPermCombo, *groupPermCombo, *othersPermCombo;
  QCheckBox *extraCheckbox;
  mode_t partialPermissions;
  KFilePermissionsPropsPlugin::PermissionsMode pmode;
  bool canChangePermissions;
  bool isIrregular;
};

#define UniOwner    (S_IRUSR|S_IWUSR|S_IXUSR)
#define UniGroup    (S_IRGRP|S_IWGRP|S_IXGRP)
#define UniOthers   (S_IROTH|S_IWOTH|S_IXOTH)
#define UniRead     (S_IRUSR|S_IRGRP|S_IROTH)
#define UniWrite    (S_IWUSR|S_IWGRP|S_IWOTH)
#define UniExec     (S_IXUSR|S_IXGRP|S_IXOTH)
#define UniSpecial  (S_ISUID|S_ISGID|S_ISVTX)

// synced with PermissionsTarget
const mode_t KFilePermissionsPropsPlugin::permissionsMasks[3] = {UniOwner, UniGroup, UniOthers};
const mode_t KFilePermissionsPropsPlugin::standardPermissions[4] = { 0, UniRead, UniRead|UniWrite, (mode_t)-1 };

// synced with PermissionsMode and standardPermissions
const char *KFilePermissionsPropsPlugin::permissionsTexts[4][4] = {
  { I18N_NOOP("Forbidden"),
    I18N_NOOP("Can Read"),
    I18N_NOOP("Can Read & Write"),
    0 },
  { I18N_NOOP("Forbidden"),
    I18N_NOOP("Can View Content"),
    I18N_NOOP("Can View & Modify Content"),
    0 },
  { 0, 0, 0, 0}, // no texts for links
  { I18N_NOOP("Forbidden"),
    I18N_NOOP("Can View Content & Read"),
    I18N_NOOP("Can View/Read & Modify/Write"),
    0 }
};


KFilePermissionsPropsPlugin::KFilePermissionsPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KFilePermissionsPropsPluginPrivate;
  d->cbRecursive = 0L;
  grpCombo = 0L; grpEdit = 0;
  usrEdit = 0L;
  QString path = properties->kurl().path(-1);
  QString fname = properties->kurl().fileName();
  bool isLocal = properties->kurl().isLocalFile();
  bool isIntoTrash = isLocal && path.startsWith(KGlobalSettings::trashPath());
  bool isTrash = isLocal && ( properties->kurl().path( 1 ) == KGlobalSettings::trashPath() );
  bool IamRoot = (geteuid() == 0);

  KFileItem * item = properties->item();
  bool isLink = item->isLink();
  bool isDir = item->isDir(); // all dirs
  bool hasDir = item->isDir(); // at least one dir
  permissions = item->permissions(); // common permissions to all files
  d->partialPermissions = permissions; // permissions that only some files have (at first we take everything)
  d->isIrregular = isIrregular(permissions, isDir, isLink);
  strOwner = item->user();
  strGroup = item->group();

  if ( properties->items().count() > 1 )
  {
    // Multiple items: see what they have in common
    KFileItemList items = properties->items();
    KFileItemListIterator it( items );
    for ( ++it /*no need to check the first one again*/ ; it.current(); ++it )
    {
      if (!d->isIrregular)
	d->isIrregular |= isIrregular((*it)->permissions(),
				      (*it)->isDir() == isDir,
				      (*it)->isLink() == isLink);
      if ( (*it)->isLink() != isLink )
        isLink = false;
      if ( (*it)->isDir() != isDir )
        isDir = false;
      hasDir |= (*it)->isDir();
      if ( (*it)->permissions() != permissions )
      {
        permissions &= (*it)->permissions();
        d->partialPermissions |= (*it)->permissions();
      }
      if ( (*it)->user() != strOwner )
        strOwner = QString::null;
      if ( (*it)->group() != strGroup )
        strGroup = QString::null;
    }
  }

  if (isLink)
    d->pmode = PermissionsOnlyLinks;
  else if (isDir)
    d->pmode = PermissionsOnlyDirs;
  else if (hasDir)
    d->pmode = PermissionsMixed;
  else
    d->pmode = PermissionsOnlyFiles;

  // keep only what's not in the common permissions
  d->partialPermissions = d->partialPermissions & ~permissions;

  bool isMyFile = false;

  if (isLocal && !strOwner.isEmpty()) { // local files, and all owned by the same person
    struct passwd *myself = getpwuid( geteuid() );
    if ( myself != 0L )
    {
      isMyFile = (strOwner == QString::fromLocal8Bit(myself->pw_name));
    } else
      kdWarning() << "I don't exist ?! geteuid=" << geteuid() << endl;
  } else {
    //We don't know, for remote files, if they are ours or not.
    //So we let the user change permissions, and
    //KIO::chmod will tell, if he had no right to do it.
    isMyFile = true;
  }

  d->canChangePermissions = (isMyFile || IamRoot) && (!isLink);


  // create GUI

  d->m_frame = properties->addPage(i18n("&Permissions"));

  QBoxLayout *box = new QVBoxLayout( d->m_frame, 0, KDialog::spacingHint() );

  QWidget *l;
  QLabel *lbl;
  QGroupBox *gb;
  QGridLayout *gl;
  QPushButton* pbAdvancedPerm = 0;

  /* Group: Access Permissions */
  gb = new QGroupBox ( 0, Qt::Vertical, i18n("Access Permissions"), d->m_frame );
  gb->layout()->setSpacing(KDialog::spacingHint());
  gb->layout()->setMargin(KDialog::marginHint());
  box->addWidget (gb);

  gl = new QGridLayout (gb->layout(), 7, 2);
  gl->setColStretch(1, 1);

  l = d->explanationLabel = new QLabel( "", gb );
  if (isLink)
    d->explanationLabel->setText(i18n("This file is a link and does not have permissions.",
				      "All files are links and do not have permissions.",
				      properties->items().count()));
  else if (!d->canChangePermissions)
    d->explanationLabel->setText(i18n("Only the owner can change permissions."));
  gl->addMultiCellWidget(l, 0, 0, 0, 1);

  lbl = new QLabel( i18n("O&wner:"), gb);
  gl->addWidget(lbl, 1, 0);
  l = d->ownerPermCombo = new QComboBox(gb);
  lbl->setBuddy(l);
  gl->addWidget(l, 1, 1);
  connect(l, SIGNAL( highlighted(int) ), this, SIGNAL( changed() ));
  QWhatsThis::add(l, i18n("Specifies the actions that the owner is allowed to do."));

  lbl = new QLabel( i18n("Gro&up:"), gb);
  gl->addWidget(lbl, 2, 0);
  l = d->groupPermCombo = new QComboBox(gb);
  lbl->setBuddy(l);
  gl->addWidget(l, 2, 1);
  connect(l, SIGNAL( highlighted(int) ), this, SIGNAL( changed() ));
  QWhatsThis::add(l, i18n("Specifies the actions that the members of the group are allowed to do."));

  lbl = new QLabel( i18n("O&thers:"), gb);
  gl->addWidget(lbl, 3, 0);
  l = d->othersPermCombo = new QComboBox(gb);
  lbl->setBuddy(l);
  gl->addWidget(l, 3, 1);
  connect(l, SIGNAL( highlighted(int) ), this, SIGNAL( changed() ));
  QWhatsThis::add(l, i18n("Specifies the actions that all users, who are neither "
			  "owner nor in the group, are allowed to do."));

  if (!isLink) {
    l = d->extraCheckbox = new QCheckBox(hasDir ?
					 i18n("Only own&er can rename and delete folder content") :
					 i18n("Is &executable"),
					 gb );
    connect( d->extraCheckbox, SIGNAL( clicked() ), this, SIGNAL( changed() ) );
    gl->addWidget(l, 4, 1);
    QWhatsThis::add(l, hasDir ? i18n("Enable this option to allow only the folder's owner to "
				     "delete or rename the contained files and folders. Other "
				     "users can only add new files, which requires the 'Modify "
				     "Content' permission.")
		    : i18n("Enable this option to mark the file as executable. This only makes "
			   "sense for programs and scripts. It is required when you want to "
			   "execute them."));

    QLayoutItem *spacer = new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gl->addMultiCell(spacer, 5, 5, 0, 1);

    pbAdvancedPerm = new QPushButton(i18n("A&dvanced Permissions..."), gb);
    gl->addMultiCellWidget(pbAdvancedPerm, 6, 6, 0, 1, AlignRight);
    connect(pbAdvancedPerm, SIGNAL( clicked() ), this, SLOT( slotShowAdvancedPermissions() ));
  }
  else
    d->extraCheckbox = 0;


  /**** Group: Ownership ****/
  gb = new QGroupBox ( i18n("Ownership"), d->m_frame );
  box->addWidget (gb);

  gl = new QGridLayout (gb, 4, 3, KDialog::marginHint(), KDialog::spacingHint());
  gl->addRowSpacing(0, 10);

  /*** Set Owner ***/
  l = new QLabel( i18n("User:"), gb );
  gl->addWidget (l, 1, 0);

  /* GJ: Don't autocomplete more than 1000 users. This is a kind of random
   * value. Huge sites having 10.000+ user have a fair chance of using NIS,
   * (possibly) making this unacceptably slow.
   * OTOH, it is nice to offer this functionality for the standard user.
   */
  int i, maxEntries = 1000;
  struct passwd *user;
  struct group *ge;

  /* File owner: For root, offer a KLineEdit with autocompletion.
   * For a user, who can never chown() a file, offer a QLabel.
   */
  if (IamRoot && isLocal)
  {
    usrEdit = new KLineEdit( gb );
    KCompletion *kcom = usrEdit->completionObject();
    kcom->setOrder(KCompletion::Sorted);
    setpwent();
    for (i=0; ((user = getpwent()) != 0L) && (i < maxEntries); i++)
      kcom->addItem(QString::fromLatin1(user->pw_name));
    endpwent();
    usrEdit->setCompletionMode((i < maxEntries) ? KGlobalSettings::CompletionAuto :
                               KGlobalSettings::CompletionNone);
    usrEdit->setText(strOwner);
    gl->addWidget(usrEdit, 1, 1);
    connect( usrEdit, SIGNAL( textChanged( const QString & ) ),
             this, SIGNAL( changed() ) );
  }
  else
  {
    l = new QLabel(strOwner, gb);
    gl->addWidget(l, 1, 1);
  }

  /*** Set Group ***/

  QStringList groupList;
  QCString strUser;
  user = getpwuid(geteuid());
  if (user != 0L)
    strUser = user->pw_name;

  setgrent();
  for (i=0; ((ge = getgrent()) != 0L) && (i < maxEntries); i++)
  {
    if (IamRoot)
      groupList += QString::fromLatin1(ge->gr_name);
    else
    {
      /* pick the groups to which the user belongs */
      char ** members = ge->gr_mem;
      char * member;
      while ((member = *members) != 0L) {
        if (strUser == member) {
          groupList += QString::fromLocal8Bit(ge->gr_name);
          break;
        }
        ++members;
      }
    }
  }
  endgrent();

  /* add the effective Group to the list .. */
  ge = getgrgid (getegid());
  if (ge) {
    QString name = QString::fromLatin1(ge->gr_name);
    if (name.isEmpty())
      name.setNum(ge->gr_gid);
    if (groupList.find(name) == groupList.end())
      groupList += name;
  }

  bool isMyGroup = groupList.contains(strGroup);

  /* add the group the file currently belongs to ..
   * .. if its not there already
   */
  if (!isMyGroup)
    groupList += strGroup;

  l = new QLabel( i18n("Group:"), gb );
  gl->addWidget (l, 2, 0);

  /* Set group: if possible to change:
   * - Offer a KLineEdit for root, since he can change to any group.
   * - Offer a QComboBox for a normal user, since he can change to a fixed
   *   (small) set of groups only.
   * If not changeable: offer a QLabel.
   */
  if (IamRoot && isLocal)
  {
    grpEdit = new KLineEdit(gb);
    KCompletion *kcom = new KCompletion;
    kcom->setItems(groupList);
    grpEdit->setCompletionObject(kcom, true);
    grpEdit->setAutoDeleteCompletionObject( true );
    grpEdit->setCompletionMode(KGlobalSettings::CompletionAuto);
    grpEdit->setText(strGroup);
    gl->addWidget(grpEdit, 2, 1);
    connect( grpEdit, SIGNAL( textChanged( const QString & ) ),
             this, SIGNAL( changed() ) );
  }
  else if ((groupList.count() > 1) && isMyFile && isLocal)
  {
    grpCombo = new QComboBox(gb, "combogrouplist");
    grpCombo->insertStringList(groupList);
    grpCombo->setCurrentItem(groupList.findIndex(strGroup));
    gl->addWidget(grpCombo, 2, 1);
    connect( grpCombo, SIGNAL( activated( int ) ),
             this, SIGNAL( changed() ) );
  }
  else
  {
    l = new QLabel(strGroup, gb);
    gl->addWidget(l, 2, 1);
  }

  gl->setColStretch(2, 10);

  // "Apply recursive" checkbox
  if ( hasDir && !isLink && !isIntoTrash )
  {
      d->cbRecursive = new QCheckBox( i18n("Apply changes to all subfolders and their contents"), d->m_frame );
      connect( d->cbRecursive, SIGNAL( clicked() ), this, SIGNAL( changed() ) );
      box->addWidget( d->cbRecursive );
  }

  updateAccessControls();


  if ( isIntoTrash || isTrash )
  {
      //don't allow to change properties for file into trash
      enableAccessControls(false);
      if ( pbAdvancedPerm)
          pbAdvancedPerm->setEnabled(false);
  }

  box->addStretch (10);
}

void KFilePermissionsPropsPlugin::slotShowAdvancedPermissions() {

  bool isDir = (d->pmode == PermissionsOnlyDirs) || (d->pmode == PermissionsMixed);
  KDialogBase dlg(properties, 0, true, i18n("Advanced Permissions"),
		  KDialogBase::Ok|KDialogBase::Cancel);

  QLabel *l, *cl[3];
  QGroupBox *gb;
  QGridLayout *gl;

  // Group: Access Permissions
  gb = new QGroupBox ( i18n("Access Permissions"), &dlg );
  dlg.setMainWidget(gb);

  gl = new QGridLayout (gb, 6, 6, 15);
  gl->addRowSpacing(0, 10);

  l = new QLabel(i18n("Class"), gb);
  gl->addWidget(l, 1, 0);

  if (isDir)
    l = new QLabel( i18n("Show\nEntries"), gb );
  else
    l = new QLabel( i18n("Read"), gb );
  gl->addWidget (l, 1, 1);
  QString readWhatsThis;
  if (isDir)
    readWhatsThis = i18n("This flag allows viewing the content of the folder.");
  else
    readWhatsThis = i18n("The Read flag allows viewing the content of the file.");
  QWhatsThis::add(l, readWhatsThis);

  if (isDir)
    l = new QLabel( i18n("Write\nEntries"), gb );
  else
    l = new QLabel( i18n("Write"), gb );
  gl->addWidget (l, 1, 2);
  QString writeWhatsThis;
  if (isDir)
    writeWhatsThis = i18n("This flag allows adding, renaming and deleting of files. "
			  "Note that deleting and renaming can be limited using the Sticky flag.");
  else
    writeWhatsThis = i18n("The Write flag allows modifying the content of the file.");
  QWhatsThis::add(l, writeWhatsThis);

  QString execWhatsThis;
  if (isDir) {
    l = new QLabel( i18n("Enter folder", "Enter"), gb );
    execWhatsThis = i18n("Enable this flag to allow entering the folder.");
  }
  else {
    l = new QLabel( i18n("Exec"), gb );
    execWhatsThis = i18n("Enable this flag to allow executing the file as a program.");
  }
  QWhatsThis::add(l, execWhatsThis);
  // GJ: Add space between normal and special modes
  QSize size = l->sizeHint();
  size.setWidth(size.width() + 15);
  l->setFixedSize(size);
  gl->addWidget (l, 1, 3);

  l = new QLabel( i18n("Special"), gb );
  gl->addMultiCellWidget(l, 1, 1, 4, 5);
  QString specialWhatsThis;
  if (isDir)
    specialWhatsThis = i18n("Special flag. Valid for the whole folder, the exact "
			    "meaning of the flag can be seen in the right hand column.");
  else
    specialWhatsThis = i18n("Special flag. The exact meaning of the flag can be seen "
			    "in the right hand column.");
  QWhatsThis::add(l, specialWhatsThis);

  cl[0] = new QLabel( i18n("User"), gb );
  gl->addWidget (cl[0], 2, 0);

  cl[1] = new QLabel( i18n("Group"), gb );
  gl->addWidget (cl[1], 3, 0);

  cl[2] = new QLabel( i18n("Others"), gb );
  gl->addWidget (cl[2], 4, 0);

  l = new QLabel(i18n("Set UID"), gb);
  gl->addWidget(l, 2, 5);
  QString setUidWhatsThis;
  if (isDir)
    setUidWhatsThis = i18n("If this flag is set, the owner of this folder will be "
			   "the owner of all new files.");
  else
    setUidWhatsThis = i18n("If this file is an executable and the flag is set, it will "
			   "be executed with the permissions of the owner.");
  QWhatsThis::add(l, setUidWhatsThis);

  l = new QLabel(i18n("Set GID"), gb);
  gl->addWidget(l, 3, 5);
  QString setGidWhatsThis;
  if (isDir)
    setGidWhatsThis = i18n("If this flag is set, the group of this folder will be "
			   "set for all new files.");
  else
    setGidWhatsThis = i18n("If this file is an executable and the flag is set, it will "
			   "be executed with the permissions of the group.");
  QWhatsThis::add(l, setGidWhatsThis);

  l = new QLabel(i18n("File permission, sets user or group ID on execution", "Sticky"), gb);
  gl->addWidget(l, 4, 5);
  QString stickyWhatsThis;
  if (isDir)
    stickyWhatsThis = i18n("If the Sticky flag is set on a folder, only the owner "
			   "and root can delete or rename files. Otherwise everybody "
			   "with write permissions can do this.");
  else
    stickyWhatsThis = i18n("The Sticky flag on a file is ignored on Linux, but may "
			   "be used on some systems");
  QWhatsThis::add(l, stickyWhatsThis);

  mode_t aPermissions, aPartialPermissions;
  mode_t dummy1, dummy2;

  if (!d->isIrregular) {
    switch (d->pmode) {
    case PermissionsOnlyFiles:
      getPermissionMasks(aPartialPermissions,
			 dummy1,
			 aPermissions,
			 dummy2);
      break;
    case PermissionsOnlyDirs:
    case PermissionsMixed:
      getPermissionMasks(dummy1,
			 aPartialPermissions,
			 dummy2,
			 aPermissions);
      break;
    case PermissionsOnlyLinks:
      aPermissions = UniRead | UniWrite | UniExec | UniSpecial;
      aPartialPermissions = 0;
      break;
    }
  }
  else {
    aPermissions = permissions;
    aPartialPermissions = d->partialPermissions;
  }

  // Draw Checkboxes
  QCheckBox *cba[3][4];
  for (int row = 0; row < 3 ; ++row) {
    for (int col = 0; col < 4; ++col) {
      QCheckBox *cb = new QCheckBox(gb);
      cba[row][col] = cb;
      cb->setChecked(aPermissions & fperm[row][col]);
      if ( aPartialPermissions & fperm[row][col] )
      {
        cb->setTristate();
        cb->setNoChange();
      }
      else if (d->cbRecursive && d->cbRecursive->isChecked())
	cb->setTristate();

      cb->setEnabled( d->canChangePermissions );
      gl->addWidget (cb, row+2, col+1);
      switch(col) {
      case 0:
	QWhatsThis::add(cb, readWhatsThis);
	break;
      case 1:
	QWhatsThis::add(cb, writeWhatsThis);
	break;
      case 2:
	QWhatsThis::add(cb, execWhatsThis);
	break;
      case 3:
	switch(row) {
	case 0:
	  QWhatsThis::add(cb, setUidWhatsThis);
	  break;
	case 1:
	  QWhatsThis::add(cb, setGidWhatsThis);
	  break;
	case 2:
	  QWhatsThis::add(cb, stickyWhatsThis);
	  break;
	}
	break;
      }
    }
  }
  gl->setColStretch(6, 10);

  if (dlg.exec() != KDialogBase::Accepted)
    return;

  mode_t andPermissions = mode_t(~0);
  mode_t orPermissions = 0;
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 4; ++col) {
      switch (cba[row][col]->state())
      {
      case QCheckBox::On:
	orPermissions |= fperm[row][col];
	//fall through
      case QCheckBox::Off:
	andPermissions &= ~fperm[row][col];
	break;
      default: // NoChange
	break;
      }
    }

  d->isIrregular = false;
  KFileItemList items = properties->items();
  for (KFileItemListIterator it(items); it.current(); ++it) {
    if (isIrregular(((*it)->permissions() & andPermissions) | orPermissions,
		    (*it)->isDir(), (*it)->isLink())) {
      d->isIrregular = true;
      break;
    }
  }

  permissions = orPermissions;
  d->partialPermissions = andPermissions;

  emit changed();
  updateAccessControls();
}

// QString KFilePermissionsPropsPlugin::tabName () const
// {
//   return i18n ("&Permissions");
// }

KFilePermissionsPropsPlugin::~KFilePermissionsPropsPlugin()
{
  delete d;
}

bool KFilePermissionsPropsPlugin::supports( KFileItemList /*_items*/ )
{
  return true;
}

// sets a combo box in the Access Control frame
void KFilePermissionsPropsPlugin::setComboContent(QComboBox *combo, PermissionsTarget target,
						  mode_t permissions, mode_t partial) {
  combo->clear();
  if (d->pmode == PermissionsOnlyLinks) {
    combo->insertItem(i18n("Link"));
    combo->setCurrentItem(0);
    return;
  }

  mode_t tMask = permissionsMasks[target];
  int textIndex;
  for (textIndex = 0; standardPermissions[textIndex] != (mode_t)-1; textIndex++)
    if ((standardPermissions[textIndex]&tMask) == (permissions&tMask&(UniRead|UniWrite)))
      break;
  Q_ASSERT(standardPermissions[textIndex] != (mode_t)-1); // must not happen, would be irreglar

  for (int i = 0; permissionsTexts[(int)d->pmode][i]; i++)
    combo->insertItem(i18n(permissionsTexts[(int)d->pmode][i]));

  if (partial & tMask & ~UniExec) {
    combo->insertItem(i18n("Varying (No Change)"));
    combo->setCurrentItem(3);
  }
  else
    combo->setCurrentItem(textIndex);
}

// permissions are irregular if they cant be displayed in a combo box.
bool KFilePermissionsPropsPlugin::isIrregular(mode_t permissions, bool isDir, bool isLink) {
  if (isLink)                             // links are always ok
    return false;

  mode_t p = permissions;
  if (p & (S_ISUID | S_ISGID))  // setuid/setgid -> irregular
    return true;
  if (isDir) {
    p &= ~S_ISVTX;          // ignore sticky on dirs

    // check supported flag combinations
    mode_t p0 = p & UniOwner;
    if ((p0 != 0) && (p0 != (S_IRUSR | S_IXUSR)) && (p0 != UniOwner))
      return true;
    p0 = p & UniGroup;
    if ((p0 != 0) && (p0 != (S_IRGRP | S_IXGRP)) && (p0 != UniGroup))
      return true;
    p0 = p & UniOthers;
    if ((p0 != 0) && (p0 != (S_IROTH | S_IXOTH)) && (p0 != UniOthers))
      return true;
    return false;
  }
  if (p & S_ISVTX) // sticky on file -> irregular
    return true;

  // check supported flag combinations
  mode_t p0 = p & UniOwner;
  bool usrXPossible = !p0; // true if this file could be an executable
  if (p0 & S_IXUSR) {
    if ((p0 == S_IXUSR) || (p0 == (S_IWUSR | S_IXUSR)))
      return true;
    usrXPossible = true;
  }
  else if (p0 == S_IWUSR)
    return true;

  p0 = p & UniGroup;
  bool grpXPossible = !p0; // true if this file could be an executable
  if (p0 & S_IXGRP) {
    if ((p0 == S_IXGRP) || (p0 == (S_IWGRP | S_IXGRP)))
      return true;
    grpXPossible = true;
  }
  else if (p0 == S_IWGRP)
    return true;
  if (p0 == 0)
    grpXPossible = true;

  p0 = p & UniOthers;
  bool othXPossible = !p0; // true if this file could be an executable
  if (p0 & S_IXOTH) {
    if ((p0 == S_IXOTH) || (p0 == (S_IWOTH | S_IXOTH)))
      return true;
    othXPossible = true;
  }
  else if (p0 == S_IWOTH)
    return true;

  // check that there either all targets are executable-compatible, or none
  return (p & UniExec) && !(usrXPossible && grpXPossible && othXPossible);
}

// enables/disabled the widgets in the Access Control frame
void KFilePermissionsPropsPlugin::enableAccessControls(bool enable) {
	d->ownerPermCombo->setEnabled(enable);
	d->groupPermCombo->setEnabled(enable);
	d->othersPermCombo->setEnabled(enable);
	if (d->extraCheckbox)
	  d->extraCheckbox->setEnabled(enable);
        if ( d->cbRecursive )
            d->cbRecursive->setEnabled(enable);
}

// updates all widgets in the Access Control frame
void KFilePermissionsPropsPlugin::updateAccessControls() {
  setComboContent(d->ownerPermCombo, PermissionsOwner,
		  permissions, d->partialPermissions);
  setComboContent(d->groupPermCombo, PermissionsGroup,
		  permissions, d->partialPermissions);
  setComboContent(d->othersPermCombo, PermissionsOthers,
		  permissions, d->partialPermissions);

  switch(d->pmode) {
  case PermissionsOnlyLinks:
    enableAccessControls(false);
    break;
  case PermissionsOnlyFiles:
    enableAccessControls(d->canChangePermissions && !d->isIrregular);
    if (d->canChangePermissions)
      d->explanationLabel->setText(d->isIrregular ?
				   i18n("This file uses advanced permissions",
				      "These files use advanced permissions.",
				      properties->items().count()) : "");
    if (d->partialPermissions & UniExec) {
      d->extraCheckbox->setTristate();
      d->extraCheckbox->setNoChange();
    }
    else {
      d->extraCheckbox->setTristate(false);
      d->extraCheckbox->setChecked(permissions & UniExec);
    }
    break;
  case PermissionsOnlyDirs:
    enableAccessControls(d->canChangePermissions && !d->isIrregular);
    if (d->canChangePermissions)
      d->explanationLabel->setText(d->isIrregular ?
				   i18n("This folder uses advanced permissions.",
				      "These folders use advanced permissions.",
				      properties->items().count()) : "");
    if (d->partialPermissions & S_ISVTX) {
      d->extraCheckbox->setTristate();
      d->extraCheckbox->setNoChange();
    }
    else {
      d->extraCheckbox->setTristate(false);
      d->extraCheckbox->setChecked(permissions & S_ISVTX);
    }
    break;
  case PermissionsMixed:
    enableAccessControls(d->canChangePermissions && !d->isIrregular);
    if (d->canChangePermissions)
      d->explanationLabel->setText(d->isIrregular ?
				   i18n("These files use advanced permissions.") : "");
    break;
    if (d->partialPermissions & S_ISVTX) {
      d->extraCheckbox->setTristate();
      d->extraCheckbox->setNoChange();
    }
    else {
      d->extraCheckbox->setTristate(false);
      d->extraCheckbox->setChecked(permissions & S_ISVTX);
    }
    break;
  }
}

// gets masks for files and dirs from the Access Control frame widgets
void KFilePermissionsPropsPlugin::getPermissionMasks(mode_t &andFilePermissions,
						     mode_t &andDirPermissions,
						     mode_t &orFilePermissions,
						     mode_t &orDirPermissions) {
  andFilePermissions = mode_t(~UniSpecial);
  andDirPermissions = mode_t(~(S_ISUID|S_ISGID));
  orFilePermissions = 0;
  orDirPermissions = 0;
  if (d->isIrregular)
    return;

  mode_t m = standardPermissions[d->ownerPermCombo->currentItem()];
  if (m != (mode_t) -1) {
    orFilePermissions |= m & UniOwner;
    if ((m & UniOwner) &&
	((d->pmode == PermissionsMixed) ||
	 ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->state() == QButton::NoChange))))
      andFilePermissions &= ~(S_IRUSR | S_IWUSR);
    else {
      andFilePermissions &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
      if ((m & S_IRUSR) && (d->extraCheckbox->state() == QButton::On))
	orFilePermissions |= S_IXUSR;
    }

    orDirPermissions |= m & UniOwner;
    if (m & S_IRUSR)
	orDirPermissions |= S_IXUSR;
    andDirPermissions &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
  }

  m = standardPermissions[d->groupPermCombo->currentItem()];
  if (m != (mode_t) -1) {
    orFilePermissions |= m & UniGroup;
    if ((m & UniGroup) &&
	((d->pmode == PermissionsMixed) ||
	 ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->state() == QButton::NoChange))))
      andFilePermissions &= ~(S_IRGRP | S_IWGRP);
    else {
      andFilePermissions &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
      if ((m & S_IRGRP) && (d->extraCheckbox->state() == QButton::On))
	orFilePermissions |= S_IXGRP;
    }

    orDirPermissions |= m & UniGroup;
    if (m & S_IRGRP)
	orDirPermissions |= S_IXGRP;
    andDirPermissions &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
  }

  m = standardPermissions[d->othersPermCombo->currentItem()];
  if (m != (mode_t) -1) {
    orFilePermissions |= m & UniOthers;
    if ((m & UniOthers) &&
	((d->pmode == PermissionsMixed) ||
	 ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->state() == QButton::NoChange))))
      andFilePermissions &= ~(S_IROTH | S_IWOTH);
    else {
      andFilePermissions &= ~(S_IROTH | S_IWOTH | S_IXOTH);
      if ((m & S_IROTH) && (d->extraCheckbox->state() == QButton::On))
	orFilePermissions |= S_IXOTH;
    }

    orDirPermissions |= m & UniOthers;
    if (m & S_IROTH)
	orDirPermissions |= S_IXOTH;
    andDirPermissions &= ~(S_IROTH | S_IWOTH | S_IXOTH);
  }

  if (((d->pmode == PermissionsMixed) || (d->pmode == PermissionsOnlyDirs)) &&
      (d->extraCheckbox->state() != QButton::NoChange)) {
    andDirPermissions &= ~S_ISVTX;
    if (d->extraCheckbox->state() == QButton::On)
      orDirPermissions |= S_ISVTX;
  }
}

void KFilePermissionsPropsPlugin::applyChanges()
{
  mode_t orFilePermissions;
  mode_t orDirPermissions;
  mode_t andFilePermissions;
  mode_t andDirPermissions;

  if (!d->canChangePermissions)
    return;

  if (!d->isIrregular)
    getPermissionMasks(andFilePermissions,
		       andDirPermissions,
		       orFilePermissions,
		       orDirPermissions);
  else {
    orFilePermissions = permissions;
    andFilePermissions = d->partialPermissions;
    orDirPermissions = permissions;
    andDirPermissions = d->partialPermissions;
  }

  QString owner, group;
  if (usrEdit)
    owner = usrEdit->text();
  if (grpEdit)
    group = grpEdit->text();
  else if (grpCombo)
    group = grpCombo->currentText();

  if (owner == strOwner)
      owner = QString::null; // no change

  if (group == strGroup)
      group = QString::null;

  bool recursive = d->cbRecursive && d->cbRecursive->isChecked();
  bool permissionChange = false;

  KFileItemList files, dirs;
  KFileItemList items = properties->items();
  for (KFileItemListIterator it(items); it.current(); ++it) {
    if ((*it)->isDir()) {
      dirs.append(*it);
      if ((*it)->permissions() != (((*it)->permissions() & andDirPermissions) | orDirPermissions))
	permissionChange = true;
    }
    else if ((*it)->isFile()) {
      files.append(*it);
      if ((*it)->permissions() != (((*it)->permissions() & andFilePermissions) | orFilePermissions))
	permissionChange = true;
    }
  }

  if ( !owner.isEmpty() || !group.isEmpty() || recursive || permissionChange)
  {
    KIO::Job * job;
    if (files.count() > 0) {
      job = KIO::chmod( files, orFilePermissions, ~andFilePermissions,
			owner, group, false );
      connect( job, SIGNAL( result( KIO::Job * ) ),
	       SLOT( slotChmodResult( KIO::Job * ) ) );
      // Wait for job
      QWidget dummy(0,0,WType_Dialog|WShowModal);
      qt_enter_modal(&dummy);
      qApp->enter_loop();
      qt_leave_modal(&dummy);
    }
    if (dirs.count() > 0) {
      job = KIO::chmod( dirs, orDirPermissions, ~andDirPermissions,
			owner, group, recursive );
      connect( job, SIGNAL( result( KIO::Job * ) ),
	       SLOT( slotChmodResult( KIO::Job * ) ) );
      // Wait for job
      QWidget dummy(0,0,WType_Dialog|WShowModal);
      qt_enter_modal(&dummy);
      qApp->enter_loop();
      qt_leave_modal(&dummy);
    }
  }
}

void KFilePermissionsPropsPlugin::slotChmodResult( KIO::Job * job )
{
  kdDebug(250) << "KFilePermissionsPropsPlugin::slotChmodResult" << endl;
  if (job->error())
    job->showErrorDialog( d->m_frame );
  // allow apply() to return
  qApp->exit_loop();
}




class KURLPropsPlugin::KURLPropsPluginPrivate
{
public:
  KURLPropsPluginPrivate()
  {
  }
  ~KURLPropsPluginPrivate()
  {
  }

  QFrame *m_frame;
};

KURLPropsPlugin::KURLPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KURLPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("U&RL"));
  QVBoxLayout *layout = new QVBoxLayout(d->m_frame, 0, KDialog::spacingHint());

  QLabel *l;
  l = new QLabel( d->m_frame, "Label_1" );
  l->setText( i18n("URL:") );
  layout->addWidget(l);

  URLEdit = new KURLRequester( d->m_frame, "URL Requester" );
  layout->addWidget(URLEdit);

  QString path = properties->kurl().path();

  QFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  URLStr = config.readPathEntry( "URL" );

  if ( !URLStr.isNull() )
    URLEdit->setURL( URLStr );

  connect( URLEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );

  layout->addStretch (1);
}

KURLPropsPlugin::~KURLPropsPlugin()
{
  delete d;
}

// QString KURLPropsPlugin::tabName () const
// {
//   return i18n ("U&RL");
// }

bool KURLPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;

  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasLinkType();
}

void KURLPropsPlugin::applyChanges()
{
  QString path = properties->kurl().path();

  QFile f( path );
  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", QString::fromLatin1("Link"));
  config.writePathEntry( "URL", URLEdit->url() );
  // Users can't create a Link .desktop file with a Name field,
  // but distributions can. Update the Name field in that case.
  if ( config.hasKey("Name") )
  {
    // ### duplicated from KApplicationPropsPlugin
    QString nameStr = properties->kurl().fileName();
    if ( nameStr.right(8) == QString::fromLatin1(".desktop") )
      nameStr.truncate( nameStr.length() - 8 );
    if ( nameStr.right(7) == QString::fromLatin1(".kdelnk") )
      nameStr.truncate( nameStr.length() - 7 );
    config.writeEntry( "Name", nameStr );
    config.writeEntry( "Name", nameStr, true, false, true );

  }
}


/* ----------------------------------------------------
 *
 * KBindingPropsPlugin
 *
 * -------------------------------------------------- */

class KBindingPropsPlugin::KBindingPropsPluginPrivate
{
public:
  KBindingPropsPluginPrivate()
  {
  }
  ~KBindingPropsPluginPrivate()
  {
  }

  QFrame *m_frame;
};

KBindingPropsPlugin::KBindingPropsPlugin( KPropertiesDialog *_props ) : KPropsDlgPlugin( _props )
{
  d = new KBindingPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("A&ssociation"));
  patternEdit = new KLineEdit( d->m_frame, "LineEdit_1" );
  commentEdit = new KLineEdit( d->m_frame, "LineEdit_2" );
  mimeEdit = new KLineEdit( d->m_frame, "LineEdit_3" );

  QBoxLayout *mainlayout = new QVBoxLayout(d->m_frame, 0, KDialog::spacingHint());
  QLabel* tmpQLabel;

  tmpQLabel = new QLabel( d->m_frame, "Label_1" );
  tmpQLabel->setText(  i18n("Pattern ( example: *.html;*.htm )") );
  tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
  mainlayout->addWidget(tmpQLabel, 1);

  //patternEdit->setGeometry( 10, 40, 210, 30 );
  //patternEdit->setText( "" );
  patternEdit->setMaxLength( 512 );
  patternEdit->setMinimumSize( patternEdit->sizeHint() );
  patternEdit->setFixedHeight( fontHeight );
  mainlayout->addWidget(patternEdit, 1);

  tmpQLabel = new QLabel( d->m_frame, "Label_2" );
  tmpQLabel->setText(  i18n("Mime Type") );
  tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
  mainlayout->addWidget(tmpQLabel, 1);

  //mimeEdit->setGeometry( 10, 160, 210, 30 );
  mimeEdit->setMaxLength( 256 );
  mimeEdit->setMinimumSize( mimeEdit->sizeHint() );
  mimeEdit->setFixedHeight( fontHeight );
  mainlayout->addWidget(mimeEdit, 1);

  tmpQLabel = new QLabel( d->m_frame, "Label_3" );
  tmpQLabel->setText(  i18n("Comment") );
  tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
  mainlayout->addWidget(tmpQLabel, 1);

  //commentEdit->setGeometry( 10, 100, 210, 30 );
  commentEdit->setMaxLength( 256 );
  commentEdit->setMinimumSize( commentEdit->sizeHint() );
  commentEdit->setFixedHeight( fontHeight );
  mainlayout->addWidget(commentEdit, 1);

  cbAutoEmbed = new QCheckBox( i18n("Left click previews"), d->m_frame, "cbAutoEmbed" );
  mainlayout->addWidget(cbAutoEmbed, 1);

  mainlayout->addStretch (10);
  mainlayout->activate();

  QFile f( _props->kurl().path() );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( _props->kurl().path() );
  config.setDesktopGroup();
  QString patternStr = config.readEntry( "Patterns" );
  QString iconStr = config.readEntry( "Icon" );
  QString commentStr = config.readEntry( "Comment" );
  m_sMimeStr = config.readEntry( "MimeType" );

  if ( !patternStr.isEmpty() )
    patternEdit->setText( patternStr );
  if ( !commentStr.isEmpty() )
    commentEdit->setText( commentStr );
  if ( !m_sMimeStr.isEmpty() )
    mimeEdit->setText( m_sMimeStr );
  cbAutoEmbed->setTristate();
  if ( config.hasKey( "X-KDE-AutoEmbed" ) )
      cbAutoEmbed->setChecked( config.readBoolEntry( "X-KDE-AutoEmbed" ) );
  else
      cbAutoEmbed->setNoChange();

  connect( patternEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( commentEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( mimeEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( cbAutoEmbed, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
}

KBindingPropsPlugin::~KBindingPropsPlugin()
{
  delete d;
}

// QString KBindingPropsPlugin::tabName () const
// {
//   return i18n ("A&ssociation");
// }

bool KBindingPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;

  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasMimeTypeType();
}

void KBindingPropsPlugin::applyChanges()
{
  QString path = properties->kurl().path();
  QFile f( path );

  if ( !f.open( IO_ReadWrite ) )
  {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", QString::fromLatin1("MimeType") );

  config.writeEntry( "Patterns",  patternEdit->text() );
  config.writeEntry( "Comment", commentEdit->text() );
  config.writeEntry( "Comment",
		     commentEdit->text(), true, false, true ); // for compat
  config.writeEntry( "MimeType", mimeEdit->text() );
  if ( cbAutoEmbed->state() == QButton::NoChange )
      config.deleteEntry( "X-KDE-AutoEmbed", false );
  else
      config.writeEntry( "X-KDE-AutoEmbed", cbAutoEmbed->isChecked() );
  config.sync();
}

/* ----------------------------------------------------
 *
 * KDevicePropsPlugin
 *
 * -------------------------------------------------- */

class KDevicePropsPlugin::KDevicePropsPluginPrivate
{
public:
  KDevicePropsPluginPrivate()
  {
  }
  ~KDevicePropsPluginPrivate()
  {
  }

  QFrame *m_frame;
  QStringList mountpointlist;
};

KDevicePropsPlugin::KDevicePropsPlugin( KPropertiesDialog *_props ) : KPropsDlgPlugin( _props )
{
  d = new KDevicePropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("De&vice"));

  QStringList devices;
  KMountPoint::List mountPoints = KMountPoint::possibleMountPoints();

  for(KMountPoint::List::ConstIterator it = mountPoints.begin();
      it != mountPoints.end(); ++it)
  {
     KMountPoint *mp = *it;
     QString mountPoint = mp->mountPoint();
     QString device = mp->mountedFrom();
     kdDebug()<<"mountPoint :"<<mountPoint<<" device :"<<device<<" mp->mountType() :"<<mp->mountType()<<endl;

     if (device.startsWith("/") && (mountPoint != "-") &&
         (mountPoint != "none") && !mountPoint.isEmpty())
     {
        devices.append( device + QString::fromLatin1(" (")
                        + mountPoint + QString::fromLatin1(")") );
        m_devicelist.append(device);
        d->mountpointlist.append(mountPoint);
     }
  }

  QGridLayout *layout = new QGridLayout( d->m_frame, 0, 3, 0,
                                        KDialog::spacingHint());
  layout->setColStretch(1, 1);

  QLabel* label;
  label = new QLabel( d->m_frame );
  label->setText( devices.count() == 0 ?
                      i18n("Device (/dev/fd0):") : // old style
                      i18n("Device:") ); // new style (combobox)
  layout->addWidget(label, 0, 0);

  device = new QComboBox( true, d->m_frame, "ComboBox_device" );
  device->insertStringList( devices );
  layout->addWidget(device, 0, 1);
  connect( device, SIGNAL( activated( int ) ),
           this, SLOT( slotActivated( int ) ) );

  readonly = new QCheckBox( d->m_frame, "CheckBox_readonly" );
  readonly->setText(  i18n("Read only") );
  layout->addWidget(readonly, 1, 1);

  label = new QLabel( d->m_frame );
  label->setText( devices.count()==0 ?
                      i18n("Mount point (/mnt/floppy):") : // old style
                      i18n("Mount point:")); // new style (combobox)
  layout->addWidget(label, 2, 0);

  mountpoint = new QLabel( d->m_frame, "LineEdit_mountpoint" );

  layout->addWidget(mountpoint, 2, 1);

  KSeparator* sep = new KSeparator( KSeparator::HLine, d->m_frame);
  layout->addMultiCellWidget(sep, 4, 4, 0, 2);

  unmounted = new KIconButton( d->m_frame );
  unmounted->setFixedSize(70, 70);
  unmounted->setIconType(KIcon::Desktop, KIcon::Device);
  layout->addWidget(unmounted, 5, 0);

  label = new QLabel( i18n("Unmounted Icon"),  d->m_frame );
  layout->addWidget(label, 5, 1);

  layout->setRowStretch(6, 1);

  QString path( _props->kurl().path() );

  QFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  QString deviceStr = config.readEntry( "Dev" );
  QString mountPointStr = config.readEntry( "MountPoint" );
  bool ro = config.readBoolEntry( "ReadOnly", false );
  QString unmountedStr = config.readEntry( "UnmountIcon" );

  device->setEditText( deviceStr );
  if ( !deviceStr.isEmpty() ) {
    // Set default options for this device (first matching entry)
    int index = m_devicelist.findIndex(deviceStr);
    if (index != -1)
    {
      //kdDebug(250) << "found it " << index << endl;
      slotActivated( index );
    }
  }

  if ( !mountPointStr.isEmpty() )
    mountpoint->setText( mountPointStr );

  readonly->setChecked( ro );

  if ( unmountedStr.isEmpty() )
    unmountedStr = KMimeType::mimeType(QString::fromLatin1("application/octet-stream"))->KServiceType::icon(); // default icon

  unmounted->setIcon( unmountedStr );

  connect( device, SIGNAL( activated( int ) ),
           this, SIGNAL( changed() ) );
  connect( device, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( readonly, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( unmounted, SIGNAL( iconChanged( QString ) ),
           this, SIGNAL( changed() ) );

  connect( device, SIGNAL( textChanged( const QString & ) ),
           this, SLOT( slotDeviceChanged() ) );
}

KDevicePropsPlugin::~KDevicePropsPlugin()
{
  delete d;
}

// QString KDevicePropsPlugin::tabName () const
// {
//   return i18n ("De&vice");
// }

void KDevicePropsPlugin::slotActivated( int index )
{
  // Update mountpoint so that it matches the device that was selected in the combo
  device->setEditText( m_devicelist[index] );
  mountpoint->setText( d->mountpointlist[index] );
}

void KDevicePropsPlugin::slotDeviceChanged()
{
  // Update mountpoint so that it matches the typed device
  int index = m_devicelist.findIndex( device->currentText() );
  if ( index != -1 )
    mountpoint->setText( d->mountpointlist[index] );
  else
    mountpoint->setText( QString::null );
}

bool KDevicePropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;
  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasDeviceType();
}

void KDevicePropsPlugin::applyChanges()
{
  QString path = properties->kurl().path();
  QFile f( path );
  if ( !f.open( IO_ReadWrite ) )
  {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient "
				"access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", QString::fromLatin1("FSDevice") );

  config.writeEntry( "Dev", device->currentText() );
  config.writeEntry( "MountPoint", mountpoint->text() );

  config.writeEntry( "UnmountIcon", unmounted->icon() );
  kdDebug(250) << "unmounted->icon() = " << unmounted->icon() << endl;

  config.writeEntry( "ReadOnly", readonly->isChecked() );

  config.sync();
}


/* ----------------------------------------------------
 *
 * KDesktopPropsPlugin
 *
 * -------------------------------------------------- */


KDesktopPropsPlugin::KDesktopPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  QFrame *frame = properties->addPage(i18n("&Application"));
  QVBoxLayout *mainlayout = new QVBoxLayout( frame, 0, KDialog::spacingHint() );

  w = new KPropertiesDesktopBase(frame);
  mainlayout->addWidget(w);

  bool bKDesktopMode = (QCString(qApp->name()) == "kdesktop"); // nasty heh?

  if (bKDesktopMode)
  {
    // Hide Name entry
    w->nameEdit->hide();
    w->nameLabel->hide();
  }

  connect( w->nameEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
  connect( w->genNameEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
  connect( w->commentEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
  connect( w->commandEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );

  connect( w->browseButton, SIGNAL( clicked() ), this, SLOT( slotBrowseExec() ) );
  connect( w->addFiletypeButton, SIGNAL( clicked() ), this, SLOT( slotAddFiletype() ) );
  connect( w->delFiletypeButton, SIGNAL( clicked() ), this, SLOT( slotDelFiletype() ) );
  connect( w->advancedButton, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );

  // now populate the page
  QString path = _props->kurl().path();
  QFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDollarExpansion( false );
  config.setDesktopGroup();
  QString nameStr = config.readEntry( "Name" );
  QString genNameStr = config.readEntry( "GenericName" );
  QString commentStr = config.readEntry( "Comment" );
  QString commandStr = config.readPathEntry( "Exec" );
  m_origCommandStr = commandStr;
  m_terminalBool = config.readBoolEntry( "Terminal" );
  m_terminalOptionStr = config.readEntry( "TerminalOptions" );
  m_suidBool = config.readBoolEntry( "X-KDE-SubstituteUID" );
  m_suidUserStr = config.readEntry( "X-KDE-Username" );
  if( config.hasKey( "StartupNotify" ))
    m_startupBool = config.readBoolEntry( "StartupNotify", true );
  else
    m_startupBool = config.readBoolEntry( "X-KDE-StartupNotify", true );
  m_dcopServiceType = config.readEntry("X-DCOP-ServiceType").lower();

  QStringList mimeTypes = config.readListEntry( "MimeType", ';' );

  if ( nameStr.isEmpty() || bKDesktopMode ) {
    // We'll use the file name if no name is specified
    // because we _need_ a Name for a valid file.
    // But let's do it in apply, not here, so that we pick up the right name.
    setDirty();
  }
  if ( !bKDesktopMode )
    w->nameEdit->setText(nameStr);

  w->genNameEdit->setText( genNameStr );
  w->commentEdit->setText( commentStr );
  w->commandEdit->setText( commandStr );
  w->filetypeList->setAllColumnsShowFocus(true);

  KMimeType::Ptr defaultMimetype = KMimeType::defaultMimeTypePtr();
  for(QStringList::ConstIterator it = mimeTypes.begin();
      it != mimeTypes.end(); )
  {
    KMimeType::Ptr p = KMimeType::mimeType(*it);
    ++it;
    QString preference;
    if (it != mimeTypes.end())
    {
       bool numeric;
       (*it).toInt(&numeric);
       if (numeric)
       {
         preference = *it;
         ++it;
       }
    }
    if (p && (p != defaultMimetype))
    {
       new QListViewItem(w->filetypeList, p->name(), p->comment(), preference);
    }
  }

}

KDesktopPropsPlugin::~KDesktopPropsPlugin()
{
}

void KDesktopPropsPlugin::slotSelectMimetype()
{
  QListView *w = (QListView*)sender();
  QListViewItem *item = w->firstChild();
  while(item)
  {
     if (item->isSelected())
        w->setSelected(item, false);
     item = item->nextSibling();
  }
}

void KDesktopPropsPlugin::slotAddFiletype()
{
  KDialogBase dlg(w, "KPropertiesMimetypes", true,
                  i18n("Add File Type for %1").arg(properties->kurl().fileName()),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);

  dlg.setButtonOKText(i18n("&Add"), i18n("Add the selected file types to\nthe list of supported file types."),
                      i18n("Add the selected file types to\nthe list of supported file types."));

  KPropertiesMimetypeBase *mw = new KPropertiesMimetypeBase(&dlg);

  dlg.setMainWidget(mw);

  {
     mw->listView->setRootIsDecorated(true);
     mw->listView->setSelectionMode(QListView::Extended);
     mw->listView->setAllColumnsShowFocus(true);
     mw->listView->setFullWidth(true);
     mw->listView->setMinimumSize(500,400);

     connect(mw->listView, SIGNAL(selectionChanged()),
             this, SLOT(slotSelectMimetype()));
     connect(mw->listView, SIGNAL(doubleClicked( QListViewItem *, const QPoint &, int )),
             &dlg, SLOT( slotOk()));

     QMap<QString,QListViewItem*> majorMap;
     QListViewItem *majorGroup;
     KMimeType::List mimetypes = KMimeType::allMimeTypes();
     QValueListIterator<KMimeType::Ptr> it(mimetypes.begin());
     for (; it != mimetypes.end(); ++it) {
        QString mimetype = (*it)->name();
        if (mimetype == "application/octet-stream")
           continue;
        int index = mimetype.find("/");
        QString maj = mimetype.left(index);
        QString min = mimetype.mid(index+1);

        QMapIterator<QString,QListViewItem*> mit = majorMap.find( maj );
        if ( mit == majorMap.end() ) {
           majorGroup = new QListViewItem( mw->listView, maj );
           majorGroup->setExpandable(true);
           mw->listView->setOpen(majorGroup, true);
           majorMap.insert( maj, majorGroup );
        }
        else
        {
           majorGroup = mit.data();
        }

        QListViewItem *item = new QListViewItem(majorGroup, min, (*it)->comment());
        item->setPixmap(0, (*it)->pixmap(KIcon::Small, IconSize(KIcon::Small)));
     }
     QMapIterator<QString,QListViewItem*> mit = majorMap.find( "all" );
     if ( mit != majorMap.end())
     {
        mw->listView->setCurrentItem(mit.data());
        mw->listView->ensureItemVisible(mit.data());
     }
  }

  if (dlg.exec() == KDialogBase::Accepted)
  {
     KMimeType::Ptr defaultMimetype = KMimeType::defaultMimeTypePtr();
     QListViewItem *majorItem = mw->listView->firstChild();
     while(majorItem)
     {
        QString major = majorItem->text(0);

        QListViewItem *minorItem = majorItem->firstChild();
        while(minorItem)
        {
           if (minorItem->isSelected())
           {
              QString mimetype = major + "/" + minorItem->text(0);
              KMimeType::Ptr p = KMimeType::mimeType(mimetype);
              if (p && (p != defaultMimetype))
              {
                 mimetype = p->name();
                 bool found = false;
                 QListViewItem *item = w->filetypeList->firstChild();
                 while (item)
                 {
                    if (mimetype == item->text(0))
                    {
                       found = true;
                       break;
                    }
                    item = item->nextSibling();
                 }
                 if (!found)
                    new QListViewItem(w->filetypeList, p->name(), p->comment());
              }
           }
           minorItem = minorItem->nextSibling();
        }

        majorItem = majorItem->nextSibling();
     }

  }
}

void KDesktopPropsPlugin::slotDelFiletype()
{
  delete w->filetypeList->currentItem();
}

void KDesktopPropsPlugin::checkCommandChanged()
{
  if (KRun::binaryName(w->commandEdit->text(), true) !=
      KRun::binaryName(m_origCommandStr, true))
  {
    QString m_origCommandStr = w->commandEdit->text();
    m_dcopServiceType= QString::null; // Reset
  }
}

void KDesktopPropsPlugin::applyChanges()
{
  kdDebug(250) << "KDesktopPropsPlugin::applyChanges" << endl;
  QString path = properties->kurl().path();

  QFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  // If the command is changed we reset certain settings that are strongly
  // coupled to the command.
  checkCommandChanged();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", QString::fromLatin1("Application"));
  config.writeEntry( "Comment", w->commentEdit->text() );
  config.writeEntry( "Comment", w->commentEdit->text(), true, false, true ); // for compat
  config.writeEntry( "GenericName", w->genNameEdit->text() );
  config.writeEntry( "GenericName", w->genNameEdit->text(), true, false, true ); // for compat

  config.writePathEntry( "Exec", w->commandEdit->text() );

  // Write mimeTypes
  QStringList mimeTypes;
  for( QListViewItem *item = w->filetypeList->firstChild();
       item; item = item->nextSibling() )
  {
    QString preference = item->text(2);
    mimeTypes.append(item->text(0));
    if (!preference.isEmpty())
       mimeTypes.append(preference);
  }

  config.writeEntry( "MimeType", mimeTypes, ';' );

  if ( !w->nameEdit->isHidden() ) {
      QString nameStr = w->nameEdit->text();
      config.writeEntry( "Name", nameStr );
      config.writeEntry( "Name", nameStr, true, false, true );
  }

  config.writeEntry("Terminal", m_terminalBool);
  config.writeEntry("TerminalOptions", m_terminalOptionStr);
  config.writeEntry("X-KDE-SubstituteUID", m_suidBool);
  config.writeEntry("X-KDE-Username", m_suidUserStr);
  config.writeEntry("StartupNotify", m_startupBool);
  config.writeEntry("X-DCOP-ServiceType", m_dcopServiceType);
  config.sync();

  // KSycoca update needed?
  QString sycocaPath = KGlobal::dirs()->relativeLocation("apps", path);
  bool updateNeeded = !sycocaPath.startsWith("/");
  if (!updateNeeded)
  {
     sycocaPath = KGlobal::dirs()->relativeLocation("xdgdata-apps", path);
     updateNeeded = !sycocaPath.startsWith("/");
  }
  if (updateNeeded)
     KService::rebuildKSycoca(w);
}


void KDesktopPropsPlugin::slotBrowseExec()
{
  KURL f = KFileDialog::getOpenURL( QString::null,
                                      QString::null, w );
  if ( f.isEmpty() )
    return;

  if ( !f.isLocalFile()) {
    KMessageBox::sorry(w, i18n("Only executables on local file systems are supported."));
    return;
  }

  QString path = f.path();
  KRun::shellQuote( path );
  w->commandEdit->setText( path );
}

void KDesktopPropsPlugin::slotAdvanced()
{
  KDialogBase dlg(w, "KPropertiesDesktopAdv", true,
      i18n("Advanced Options for %1").arg(properties->kurl().fileName()),
      KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);
  KPropertiesDesktopAdvBase *w = new KPropertiesDesktopAdvBase(&dlg);

  dlg.setMainWidget(w);

  // If the command is changed we reset certain settings that are strongly
  // coupled to the command.
  checkCommandChanged();

  // check to see if we use konsole if not do not add the nocloseonexit
  // because we don't know how to do this on other terminal applications
  KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
  QString preferredTerminal = confGroup.readEntry("TerminalApplication",
						  QString::fromLatin1("konsole"));

  bool terminalCloseBool = false;

  if (preferredTerminal == "konsole")
  {
     terminalCloseBool = (m_terminalOptionStr.contains( "--noclose" ) > 0);
     w->terminalCloseCheck->setChecked(terminalCloseBool);
     m_terminalOptionStr.replace( "--noclose", "");
  }
  else
  {
     w->terminalCloseCheck->hide();
  }

  w->terminalCheck->setChecked(m_terminalBool);
  w->terminalEdit->setText(m_terminalOptionStr);
  w->terminalCloseCheck->setEnabled(m_terminalBool);
  w->terminalEdit->setEnabled(m_terminalBool);
  w->terminalEditLabel->setEnabled(m_terminalBool);

  w->suidCheck->setChecked(m_suidBool);
  w->suidEdit->setText(m_suidUserStr);
  w->suidEdit->setEnabled(m_suidBool);
  w->suidEditLabel->setEnabled(m_suidBool);

  w->startupInfoCheck->setChecked(m_startupBool);

  if (m_dcopServiceType == "unique")
    w->dcopCombo->setCurrentItem(2);
  else if (m_dcopServiceType == "multi")
    w->dcopCombo->setCurrentItem(1);
  else if (m_dcopServiceType == "wait")
    w->dcopCombo->setCurrentItem(3);
  else
    w->dcopCombo->setCurrentItem(0);

  // Provide username completion up to 1000 users.
  KCompletion *kcom = new KCompletion;
  kcom->setOrder(KCompletion::Sorted);
  struct passwd *pw;
  int i, maxEntries = 1000;
  setpwent();
  for (i=0; ((pw = getpwent()) != 0L) && (i < maxEntries); i++)
    kcom->addItem(QString::fromLatin1(pw->pw_name));
  endpwent();
  if (i < maxEntries)
  {
    w->suidEdit->setCompletionObject(kcom, true);
    w->suidEdit->setAutoDeleteCompletionObject( true );
    w->suidEdit->setCompletionMode(KGlobalSettings::CompletionAuto);
  }
  else
  {
    delete kcom;
  }

  connect( w->terminalEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( w->terminalCloseCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( w->terminalCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( w->suidCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( w->suidEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( w->startupInfoCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( w->dcopCombo, SIGNAL( highlighted( int ) ),
           this, SIGNAL( changed() ) );

  if ( dlg.exec() == QDialog::Accepted )
  {
    m_terminalOptionStr = w->terminalEdit->text().stripWhiteSpace();
    m_terminalBool = w->terminalCheck->isChecked();
    m_suidBool = w->suidCheck->isChecked();
    m_suidUserStr = w->suidEdit->text().stripWhiteSpace();
    m_startupBool = w->startupInfoCheck->isChecked();

    if (w->terminalCloseCheck->isChecked())
    {
      m_terminalOptionStr.append(" --noclose");
    }

    switch(w->dcopCombo->currentItem())
    {
      case 1:  m_dcopServiceType = "multi"; break;
      case 2:  m_dcopServiceType = "unique"; break;
      case 3:  m_dcopServiceType = "wait"; break;
      default: m_dcopServiceType = "none"; break;
    }
  }
}

bool KDesktopPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;
  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasApplicationType() && kapp->authorize("run_desktop_files") && kapp->authorize("shell_access");
}

void KPropertiesDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KPropsDlgPlugin::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }





/**
 * The following code is obsolete and only kept for binary compatibility
 * To be removed in KDE 4
 */

class KExecPropsPlugin::KExecPropsPluginPrivate
{
public:
  KExecPropsPluginPrivate()
  {
  }
  ~KExecPropsPluginPrivate()
  {
  }

  QFrame *m_frame;
  QCheckBox *nocloseonexitCheck;
};

KExecPropsPlugin::KExecPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KExecPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("E&xecute"));
  QVBoxLayout * mainlayout = new QVBoxLayout( d->m_frame, 0,
      KDialog::spacingHint());

  // Now the widgets in the top layout

  QLabel* l;
  l = new QLabel( i18n( "Comman&d:" ), d->m_frame );
  mainlayout->addWidget(l);

  QHBoxLayout * hlayout;
  hlayout = new QHBoxLayout(KDialog::spacingHint());
  mainlayout->addLayout(hlayout);

  execEdit = new KLineEdit( d->m_frame );
  QWhatsThis::add(execEdit,i18n(
    "Following the command, you can have several place holders which will be replaced "
    "with the actual values when the actual program is run:\n"
    "%f - a single file name\n"
    "%F - a list of files; use for applications that can open several local files at once\n"
    "%u - a single URL\n"
    "%U - a list of URLs\n"
    "%d - the folder of the file to open\n"
    "%D - a list of folders\n"
    "%i - the icon\n"
    "%m - the mini-icon\n"
    "%c - the caption"));
  hlayout->addWidget(execEdit, 1);

  l->setBuddy( execEdit );

  execBrowse = new QPushButton( d->m_frame );
  execBrowse->setText( i18n("&Browse...") );
  hlayout->addWidget(execBrowse);

  // The groupbox about swallowing
  QGroupBox* tmpQGroupBox;
  tmpQGroupBox = new QGroupBox( i18n("Panel Embedding"), d->m_frame );
  tmpQGroupBox->setColumnLayout( 0, Qt::Horizontal );

  mainlayout->addWidget(tmpQGroupBox);

  QGridLayout *grid = new QGridLayout(tmpQGroupBox->layout(), 2, 2);
  grid->setSpacing( KDialog::spacingHint() );
  grid->setColStretch(1, 1);

  l = new QLabel( i18n( "&Execute on click:" ), tmpQGroupBox );
  grid->addWidget(l, 0, 0);

  swallowExecEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(swallowExecEdit, 0, 1);

  l->setBuddy( swallowExecEdit );

  l = new QLabel( i18n( "&Window title:" ), tmpQGroupBox );
  grid->addWidget(l, 1, 0);

  swallowTitleEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(swallowTitleEdit, 1, 1);

  l->setBuddy( swallowTitleEdit );

  // The groupbox about run in terminal

  tmpQGroupBox = new QGroupBox( d->m_frame );
  tmpQGroupBox->setColumnLayout( 0, Qt::Horizontal );

  mainlayout->addWidget(tmpQGroupBox);

  grid = new QGridLayout(tmpQGroupBox->layout(), 3, 2);
  grid->setSpacing( KDialog::spacingHint() );
  grid->setColStretch(1, 1);

  terminalCheck = new QCheckBox( tmpQGroupBox );
  terminalCheck->setText( i18n("&Run in terminal") );
  grid->addMultiCellWidget(terminalCheck, 0, 0, 0, 1);

  // check to see if we use konsole if not do not add the nocloseonexit
  // because we don't know how to do this on other terminal applications
  KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
  QString preferredTerminal = confGroup.readEntry("TerminalApplication",
						  QString::fromLatin1("konsole"));

  int posOptions = 1;
  d->nocloseonexitCheck = 0L;
  if (preferredTerminal == "konsole")
  {
    posOptions = 2;
    d->nocloseonexitCheck = new QCheckBox( tmpQGroupBox );
    d->nocloseonexitCheck->setText( i18n("Do not &close when command exits") );
    grid->addMultiCellWidget(d->nocloseonexitCheck, 1, 1, 0, 1);
  }

  terminalLabel = new QLabel( i18n( "&Terminal options:" ), tmpQGroupBox );
  grid->addWidget(terminalLabel, posOptions, 0);

  terminalEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(terminalEdit, posOptions, 1);

  terminalLabel->setBuddy( terminalEdit );

  // The groupbox about run with substituted uid.

  tmpQGroupBox = new QGroupBox( d->m_frame );
  tmpQGroupBox->setColumnLayout( 0, Qt::Horizontal );

  mainlayout->addWidget(tmpQGroupBox);

  grid = new QGridLayout(tmpQGroupBox->layout(), 2, 2);
  grid->setSpacing(KDialog::spacingHint());
  grid->setColStretch(1, 1);

  suidCheck = new QCheckBox(tmpQGroupBox);
  suidCheck->setText(i18n("Ru&n as a different user"));
  grid->addMultiCellWidget(suidCheck, 0, 0, 0, 1);

  suidLabel = new QLabel(i18n( "&Username:" ), tmpQGroupBox);
  grid->addWidget(suidLabel, 1, 0);

  suidEdit = new KLineEdit(tmpQGroupBox);
  grid->addWidget(suidEdit, 1, 1);

  suidLabel->setBuddy( suidEdit );

  mainlayout->addStretch(1);

  // now populate the page
  QString path = _props->kurl().path();
  QFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDollarExpansion( false );
  config.setDesktopGroup();
  execStr = config.readPathEntry( "Exec" );
  swallowExecStr = config.readPathEntry( "SwallowExec" );
  swallowTitleStr = config.readEntry( "SwallowTitle" );
  termBool = config.readBoolEntry( "Terminal" );
  termOptionsStr = config.readEntry( "TerminalOptions" );
  suidBool = config.readBoolEntry( "X-KDE-SubstituteUID" );
  suidUserStr = config.readEntry( "X-KDE-Username" );

  if ( !swallowExecStr.isNull() )
    swallowExecEdit->setText( swallowExecStr );
  if ( !swallowTitleStr.isNull() )
    swallowTitleEdit->setText( swallowTitleStr );

  if ( !execStr.isNull() )
    execEdit->setText( execStr );

  if ( d->nocloseonexitCheck )
  {
    d->nocloseonexitCheck->setChecked( (termOptionsStr.contains( "--noclose" ) > 0) );
    termOptionsStr.replace( "--noclose", "");
  }
  if ( !termOptionsStr.isNull() )
    terminalEdit->setText( termOptionsStr );

  terminalCheck->setChecked( termBool );
  enableCheckedEdit();

  suidCheck->setChecked( suidBool );
  suidEdit->setText( suidUserStr );
  enableSuidEdit();

  // Provide username completion up to 1000 users.
  KCompletion *kcom = new KCompletion;
  kcom->setOrder(KCompletion::Sorted);
  struct passwd *pw;
  int i, maxEntries = 1000;
  setpwent();
  for (i=0; ((pw = getpwent()) != 0L) && (i < maxEntries); i++)
    kcom->addItem(QString::fromLatin1(pw->pw_name));
  endpwent();
  if (i < maxEntries)
  {
    suidEdit->setCompletionObject(kcom, true);
    suidEdit->setAutoDeleteCompletionObject( true );
    suidEdit->setCompletionMode(KGlobalSettings::CompletionAuto);
  }
  else
  {
    delete kcom;
  }

  connect( swallowExecEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( swallowTitleEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( execEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( terminalEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  if (d->nocloseonexitCheck)
    connect( d->nocloseonexitCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( terminalCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( suidCheck, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed() ) );
  connect( suidEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );

  connect( execBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowseExec() ) );
  connect( terminalCheck, SIGNAL( clicked() ), this,  SLOT( enableCheckedEdit() ) );
  connect( suidCheck, SIGNAL( clicked() ), this,  SLOT( enableSuidEdit() ) );

}

KExecPropsPlugin::~KExecPropsPlugin()
{
  delete d;
}

void KExecPropsPlugin::enableCheckedEdit()
{
  bool checked = terminalCheck->isChecked();
  terminalLabel->setEnabled( checked );
  if (d->nocloseonexitCheck)
    d->nocloseonexitCheck->setEnabled( checked );
  terminalEdit->setEnabled( checked );
}

void KExecPropsPlugin::enableSuidEdit()
{
  bool checked = suidCheck->isChecked();
  suidLabel->setEnabled( checked );
  suidEdit->setEnabled( checked );
}

bool KExecPropsPlugin::supports( KFileItemList _items )
{
  if ( _items.count() != 1 )
    return false;
  KFileItem * item = _items.first();
  // check if desktop file
  if ( !KPropsDlgPlugin::isDesktopFile( item ) )
    return false;
  // open file and check type
  KDesktopFile config( item->url().path(), true /* readonly */ );
  return config.hasApplicationType() && kapp->authorize("run_desktop_files") && kapp->authorize("shell_access");
}

void KExecPropsPlugin::applyChanges()
{
  kdDebug(250) << "KExecPropsPlugin::applyChanges" << endl;
  QString path = properties->kurl().path();

  QFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
				"sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", QString::fromLatin1("Application"));
  config.writePathEntry( "Exec", execEdit->text() );
  config.writePathEntry( "SwallowExec", swallowExecEdit->text() );
  config.writeEntry( "SwallowTitle", swallowTitleEdit->text() );
  config.writeEntry( "Terminal", terminalCheck->isChecked() );
  QString temp = terminalEdit->text();
  if (d->nocloseonexitCheck )
    if ( d->nocloseonexitCheck->isChecked() )
      temp += QString::fromLatin1("--noclose ");
  temp = temp.stripWhiteSpace();
  config.writeEntry( "TerminalOptions", temp );
  config.writeEntry( "X-KDE-SubstituteUID", suidCheck->isChecked() );
  config.writeEntry( "X-KDE-Username", suidEdit->text() );
}


void KExecPropsPlugin::slotBrowseExec()
{
    KURL f = KFileDialog::getOpenURL( QString::null,
                                      QString::null, d->m_frame );
    if ( f.isEmpty() )
        return;

    if ( !f.isLocalFile()) {
        KMessageBox::sorry(d->m_frame, i18n("Only executables on local file systems are supported."));
        return;
    }

    QString path = f.path();
    KRun::shellQuote( path );
    execEdit->setText( path );
}

class KApplicationPropsPlugin::KApplicationPropsPluginPrivate
{
public:
  KApplicationPropsPluginPrivate()
  {
      m_kdesktopMode = QCString(qApp->name()) == "kdesktop"; // nasty heh?
  }
  ~KApplicationPropsPluginPrivate()
  {
  }

  QFrame *m_frame;
  bool m_kdesktopMode;
};

KApplicationPropsPlugin::KApplicationPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KApplicationPropsPluginPrivate;
  d->m_frame = properties->addPage(i18n("&Application"));
  QVBoxLayout *toplayout = new QVBoxLayout( d->m_frame, 0, KDialog::spacingHint());

  QIconSet iconSet;
  QPixmap pixMap;

  addExtensionButton = new QPushButton( QString::null, d->m_frame );
  iconSet = SmallIconSet( "back" );
  addExtensionButton->setIconSet( iconSet );
  pixMap = iconSet.pixmap( QIconSet::Small, QIconSet::Normal );
  addExtensionButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
  connect( addExtensionButton, SIGNAL( clicked() ),
            SLOT( slotAddExtension() ) );

  delExtensionButton = new QPushButton( QString::null, d->m_frame );
  iconSet = SmallIconSet( "forward" );
  delExtensionButton->setIconSet( iconSet );
  delExtensionButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
  connect( delExtensionButton, SIGNAL( clicked() ),
            SLOT( slotDelExtension() ) );

  QLabel *l;

  QGridLayout *grid = new QGridLayout(2, 2);
  grid->setColStretch(1, 1);
  toplayout->addLayout(grid);

  if ( d->m_kdesktopMode )
  {
      // in kdesktop the name field comes from the first tab
      nameEdit = 0L;
  }
  else
  {
      l = new QLabel(i18n("Name:"), d->m_frame, "Label_4" );
      grid->addWidget(l, 0, 0);

      nameEdit = new KLineEdit( d->m_frame, "LineEdit_3" );
      grid->addWidget(nameEdit, 0, 1);
  }

  l = new QLabel(i18n("Description:"),  d->m_frame, "Label_5" );
  grid->addWidget(l, 1, 0);

  genNameEdit = new KLineEdit( d->m_frame, "LineEdit_4" );
  grid->addWidget(genNameEdit, 1, 1);

  l = new QLabel(i18n("Comment:"),  d->m_frame, "Label_3" );
  grid->addWidget(l, 2, 0);

  commentEdit = new KLineEdit( d->m_frame, "LineEdit_2" );
  grid->addWidget(commentEdit, 2, 1);

  l = new QLabel(i18n("File types:"), d->m_frame);
  toplayout->addWidget(l, 0, AlignLeft);

  grid = new QGridLayout(4, 3);
  grid->setColStretch(0, 1);
  grid->setColStretch(2, 1);
  grid->setRowStretch( 0, 1 );
  grid->setRowStretch( 3, 1 );
  toplayout->addLayout(grid, 2);

  extensionsList = new QListBox( d->m_frame );
  extensionsList->setSelectionMode( QListBox::Extended );
  grid->addMultiCellWidget(extensionsList, 0, 3, 0, 0);

  grid->addWidget(addExtensionButton, 1, 1);
  grid->addWidget(delExtensionButton, 2, 1);

  availableExtensionsList = new QListBox( d->m_frame );
  availableExtensionsList->setSelectionMode( QListBox::Extended );
  grid->addMultiCellWidget(availableExtensionsList, 0, 3, 2, 2);

  QString path = properties->kurl().path() ;
  QFile f( path );
  if ( !f.open( IO_ReadOnly ) )
    return;
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  QString commentStr = config.readEntry( "Comment" );
  QString genNameStr = config.readEntry( "GenericName" );

  QStringList selectedTypes = config.readListEntry( "ServiceTypes" );
  // For compatibility with KDE 1.x
  selectedTypes += config.readListEntry( "MimeType", ';' );

  QString nameStr = config.readEntry( QString::fromLatin1("Name") );
  if ( nameStr.isEmpty() || d->m_kdesktopMode ) {
    // We'll use the file name if no name is specified
    // because we _need_ a Name for a valid file.
    // But let's do it in apply, not here, so that we pick up the right name.
    setDirty();
  }

  commentEdit->setText( commentStr );
  genNameEdit->setText( genNameStr );
  if ( nameEdit )
      nameEdit->setText( nameStr );

  selectedTypes.sort();
  QStringList::Iterator sit = selectedTypes.begin();
  for( ; sit != selectedTypes.end(); ++sit ) {
    if ( !((*sit).isEmpty()) )
      extensionsList->insertItem( *sit );
  }

  KMimeType::List mimeTypes = KMimeType::allMimeTypes();
  QValueListIterator<KMimeType::Ptr> it2 = mimeTypes.begin();
  for ( ; it2 != mimeTypes.end(); ++it2 )
    addMimeType ( (*it2)->name() );

  updateButton();

  connect( extensionsList, SIGNAL( highlighted( int ) ),
           this, SLOT( updateButton() ) );
  connect( availableExtensionsList, SIGNAL( highlighted( int ) ),
           this, SLOT( updateButton() ) );

  connect( addExtensionButton, SIGNAL( clicked() ),
           this, SIGNAL( changed() ) );
  connect( delExtensionButton, SIGNAL( clicked() ),
           this, SIGNAL( changed() ) );
  if ( nameEdit )
      connect( nameEdit, SIGNAL( textChanged( const QString & ) ),
               this, SIGNAL( changed() ) );
  connect( commentEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( genNameEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( availableExtensionsList, SIGNAL( selected( int ) ),
           this, SIGNAL( changed() ) );
  connect( extensionsList, SIGNAL( selected( int ) ),
           this, SIGNAL( changed() ) );
}

KApplicationPropsPlugin::~KApplicationPropsPlugin()
{
  delete d;
}

// QString KApplicationPropsPlugin::tabName () const
// {
//   return i18n ("&Application");
// }

void KApplicationPropsPlugin::updateButton()
{
    addExtensionButton->setEnabled(availableExtensionsList->currentItem()>-1);
    delExtensionButton->setEnabled(extensionsList->currentItem()>-1);
}

void KApplicationPropsPlugin::addMimeType( const QString & name )
{
  // Add a mimetype to the list of available mime types if not in the extensionsList

  bool insert = true;

  for ( uint i = 0; i < extensionsList->count(); i++ )
    if ( extensionsList->text( i ) == name )
      insert = false;

  if ( insert )
  {
    availableExtensionsList->insertItem( name );
    availableExtensionsList->sort();
  }
}

bool KApplicationPropsPlugin::supports( KFileItemList _items )
{
  // same constraints as KExecPropsPlugin : desktop file with Type = Application
  return KExecPropsPlugin::supports( _items );
}

void KApplicationPropsPlugin::applyChanges()
{
  QString path = properties->kurl().path();

  QFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not "
				"have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( "Type", QString::fromLatin1("Application"));
  config.writeEntry( "Comment", commentEdit->text() );
  config.writeEntry( "Comment", commentEdit->text(), true, false, true ); // for compat
  config.writeEntry( "GenericName", genNameEdit->text() );
  config.writeEntry( "GenericName", genNameEdit->text(), true, false, true ); // for compat

  QStringList selectedTypes;
  for ( uint i = 0; i < extensionsList->count(); i++ )
    selectedTypes.append( extensionsList->text( i ) );

  config.writeEntry( "MimeType", selectedTypes, ';' );
  config.writeEntry( "ServiceTypes", "" );
  // hmm, actually it should probably be the contrary (but see also typeslistitem.cpp)

  QString nameStr = nameEdit ? nameEdit->text() : QString::null;
  if ( nameStr.isEmpty() ) // nothing entered, or widget not existing at all (kdesktop mode)
  {
    nameStr = properties->kurl().fileName();
    if ( nameStr.right(8) == QString::fromLatin1(".desktop") )
      nameStr.truncate( nameStr.length() - 8 );
    if ( nameStr.right(7) == QString::fromLatin1(".kdelnk") )
      nameStr.truncate( nameStr.length() - 7 );
  }
  config.writeEntry( "Name", nameStr );
  config.writeEntry( "Name", nameStr, true, false, true );

  config.sync();
}

void KApplicationPropsPlugin::slotAddExtension()
{
  QListBoxItem *item = availableExtensionsList->firstItem();
  QListBoxItem *nextItem;

  while ( item )
  {
    nextItem = item->next();

    if ( item->isSelected() )
    {
      extensionsList->insertItem( item->text() );
      availableExtensionsList->removeItem( availableExtensionsList->index( item ) );
    }

    item = nextItem;
  }

  extensionsList->sort();
  updateButton();
}

void KApplicationPropsPlugin::slotDelExtension()
{
  QListBoxItem *item = extensionsList->firstItem();
  QListBoxItem *nextItem;

  while ( item )
  {
    nextItem = item->next();

    if ( item->isSelected() )
    {
      availableExtensionsList->insertItem( item->text() );
      extensionsList->removeItem( extensionsList->index( item ) );
    }

    item = nextItem;
  }

  availableExtensionsList->sort();
  updateButton();
}



#include "kpropertiesdialog.moc"
