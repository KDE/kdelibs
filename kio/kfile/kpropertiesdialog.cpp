/* This file is part of the KDE project

   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2000 David Faure <faure@kde.org>

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
#include <qapplication.h>
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
#include <kfiledialog.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kcompletion.h>
#include <klineedit.h>
#include <kseparator.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kmetaprops.h>
#include <krun.h>
#include "kfilesharedlg.h"

#include "kpropertiesdialog.h"

#ifdef Q_WS_X11
extern "C" {
#include <X11/Xlib.h> // for XSetTransientForHint
}
#endif

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
  bool modal:1;
  bool autoShow:1;
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
  : KDialogBase (KDialogBase::Tabbed, i18n( "Properties for %1" ).arg(KIO::decodeFileName(_items.first()->url().fileName())),
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
  : KDialogBase (KDialogBase::Tabbed, i18n( "Properties for %1" ).arg(KIO::decodeFileName(_url.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),
  m_singleUrl( _url )
{
  d = new KPropertiesDialogPrivate;
  d->modal = modal;
  d->autoShow = autoShow;

  assert(!_url.isEmpty());

  KIO::StatJob * job = KIO::stat( _url );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotStatResult( KIO::Job * ) ) );
}
#endif

KPropertiesDialog::KPropertiesDialog (const KURL& _url,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed, i18n( "Properties for %1" ).arg(KIO::decodeFileName(_url.fileName())),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name, modal),
  m_singleUrl( _url )
{
  d = new KPropertiesDialogPrivate;
  d->modal = modal;
  d->autoShow = autoShow;

  assert(!_url.isEmpty());

  KIO::StatJob * job = KIO::stat( _url );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotStatResult( KIO::Job * ) ) );
}

KPropertiesDialog::KPropertiesDialog (const KURL& _tempUrl, const KURL& _currentDir,
                                      const QString& _defaultName,
                                      QWidget* parent, const char* name,
                                      bool modal, bool autoShow)
  : KDialogBase (KDialogBase::Tabbed, i18n( "Properties for %1" ).arg(KIO::decodeFileName(_tempUrl.fileName())),
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

#ifdef Q_WS_X11 // FIXME(E): Can we do something similar for Qt Embedded?
  // Matthias: let the dialog look like a modal dialog
  if (!modal)
    XSetTransientForHint(qt_xdisplay(), winId(), winId());
#endif

  //  resize( 400, 400 ); // not sure what that's for

  insertPages();

  //kdDebug(250) << "KPropertiesDialog sizeHint " << sizeHint().width() << "x" << sizeHint().height() << endl;
  // This HACK forces KDialogBase to recompute the layout
  // It is necessary for the case where init is not called from the constructor,
  // but from slotStatResult. And I'm way too lazy to look into KDialogBase...
  enableLinkedHelp( true );
  enableLinkedHelp( false );
  resize(sizeHint());

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
  int i=0;

  for ( it=m_pageList.first(); it != 0L; it=m_pageList.next() )
    {
      if(dynamic_cast<KFileSharePropsPlugin*>(it))
   {
     showPage(i);
     break;
   }
      i++;
    }

}

void KPropertiesDialog::slotStatResult( KIO::Job * job )
{
    if (job->error())
    {
        job->showErrorDialog( this );
        delete this;
    }
    else
    {
        KIO::StatJob * statJob = static_cast<KIO::StatJob*>(job);
        m_items.append( new KFileItem( statJob->statResult(), statJob->url() ) );
        init (d->modal, d->autoShow);
    }
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
  return KFilePropsPlugin::supports( _items ) ||
         KFilePermissionsPropsPlugin::supports( _items ) ||
         KExecPropsPlugin::supports( _items ) ||
         KApplicationPropsPlugin::supports( _items ) ||
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

  if ( KExecPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KExecPropsPlugin( this );
    insertPlugin (p);
  }

  if ( KApplicationPropsPlugin::supports( m_items ) )
  {
    KPropsDlgPlugin *p = new KApplicationPropsPlugin( this );
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
    KPropsDlgPlugin *p = new KFileSharePropsPlugin( this );
    insertPlugin (p);
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
  kdDebug(250) << "KPropertiesDialog::updateUrl " << _newUrl.url() << endl;
  m_singleUrl = _newUrl;
  m_items.first()->setURL( _newUrl );
  assert(!m_singleUrl.isEmpty());
  // If we have an Exec page, set it dirty, so that a full file is saved locally
  for ( QPtrListIterator<KPropsDlgPlugin> it(m_pageList); it.current(); ++it )
   if ( it.current()->isA("KExecPropsPlugin") )
   {
     kdDebug(250) << "Setting exec page dirty" << endl;
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
  fontHeight = 2*properties->dialog()->fontMetrics().height();
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
  return ( _item->mimetype() == QString::fromLatin1("application/x-desktop") );
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
  }
  ~KFilePropsPluginPrivate()
  {
    if ( dirSizeJob )
      dirSizeJob->kill();
  }

  KDirSize * dirSizeJob;
  QFrame *m_frame;
  bool bMultiple;
  QLabel *m_freeSpaceLabel;
};

KFilePropsPlugin::KFilePropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KFilePropsPluginPrivate;
  d->bMultiple = (properties->items().count() > 1);
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
  KIO::filesize_t totalSize = item->size();

  // Those things only apply to 'single file' mode
  QString filename = QString::null;
  bool isTrash = false;
  m_bFromTemplate = false;

  // And those only to 'multiple' mode
  uint iDirCount = S_ISDIR(mode) ? 1 : 0;
  uint iFileCount = 1-iDirCount;

  d->m_frame = properties->dialog()->addPage (i18n("&General"));

  QVBoxLayout *vbl = new QVBoxLayout( d->m_frame, KDialog::marginHint(),
                                      KDialog::spacingHint(), "vbl");
  QGridLayout *grid = new QGridLayout(0, 3); // unknown rows
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
    oldName = filename;

    // Make it human-readable (%2F => '/', ...)
    filename = KIO::decodeFileName( filename );

    QString path;

    if ( !m_bFromTemplate ) {
      QString tmp = properties->kurl().path( 1 );
      // is it the trash bin ?
      if ( isLocal && tmp == KGlobalSettings::trashPath())
        isTrash = true;

      // Extract the full name, but without file: for local files
      if ( isLocal )
        path = properties->kurl().path();
      else
        path = properties->kurl().prettyURL();
    } else {
      path = properties->currentDir().path(1) + properties->defaultName();
      directory = properties->currentDir().prettyURL();
    }

    if (KExecPropsPlugin::supports(properties->items()) ||
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
      if ( (*it)->mimeComment() != mimeComment )
        mimeComment = QString::null;
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

  if ( (bDesktopFile || S_ISDIR(mode)) && !d->bMultiple /*not implemented for multiple*/ )
  {
    KIconButton *iconButton = new KIconButton( d->m_frame );
    iconButton->setFixedSize(70, 70);
    iconButton->setStrictIconSize(false);
    iconButton->setIconType(KIcon::Desktop, KIcon::Device);
    // This works for everything except Device icons on unmounted devices
    // So we have to really open .desktop files
    QString iconStr = KMimeType::findByURL( properties->kurl(),
                                            mode )->icon( properties->kurl(),
                                                          isLocal );
    if ( bDesktopFile && isLocal )
    {
      KSimpleConfig config( properties->kurl().path() );
      config.setDesktopGroup();
      iconStr = config.readEntry( QString::fromLatin1("Icon") );
    }
    iconButton->setIcon(iconStr);
    iconArea = iconButton;
    connect( iconButton, SIGNAL( iconChanged(QString) ),
             this, SIGNAL( changed() ) );
  } else {
    QLabel *iconLabel = new QLabel( d->m_frame );
    iconLabel->setFixedSize(70, 70);
    iconLabel->setPixmap( DesktopIcon( iconStr ) );
    iconArea = iconLabel;
  }
  grid->addWidget(iconArea, curRow, 0, AlignLeft);

  if (d->bMultiple || isTrash || filename == QString::fromLatin1("/"))
  {
    QLabel *lab = new QLabel(d->m_frame );
    if ( d->bMultiple )
      lab->setText( KIO::itemsSummaryString( iFileCount + iDirCount, iFileCount, iDirCount, 0, false ) );
    else
      lab->setText( filename );
    nameArea = lab;
  } else
  {
    KLineEdit *lined = new KLineEdit( d->m_frame );
    lined->setText(filename);
    nameArea = lined;
    lined->setFocus();
    connect( lined, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( nameFileChanged(const QString & ) ) );
  }

  grid->addWidget(nameArea, curRow++, 2);

  KSeparator* sep = new KSeparator( KSeparator::HLine, d->m_frame);
  grid->addMultiCellWidget(sep, curRow, curRow, 0, 2);
  ++curRow;

  QLabel *l;
  if ( !mimeComment.isEmpty() )
  {
    l = new QLabel(i18n("Type:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new QLabel(mimeComment, d->m_frame );
    grid->addWidget(l, curRow++, 2);
  }

  if ( !directory.isEmpty() )
  {
    l = new QLabel( i18n("Location:"), d->m_frame );
    grid->addWidget(l, curRow, 0);

    l = new QLabel( d->m_frame );
    l->setText( directory );
    grid->addWidget(l, curRow++, 2);
  }

  l = new QLabel(i18n("Size:"), d->m_frame );
  grid->addWidget(l, curRow, 0);

  m_sizeLabel = new QLabel( d->m_frame );
  grid->addWidget( m_sizeLabel, curRow++, 2 );

  if ( !hasDirs ) // Only files [and symlinks]
  {
    m_sizeLabel->setText(QString::fromLatin1("%1 (%2)").arg(KIO::convertSize(totalSize)).arg(KGlobal::locale()->formatNumber(totalSize, 0)));
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

      l = new QLabel(i18n("Free space on %1:").arg(mountPoint), d->m_frame );
      grid->addWidget(l, curRow, 0);

      d->m_freeSpaceLabel = new QLabel( d->m_frame );
      grid->addWidget( d->m_freeSpaceLabel, curRow++, 2 );

      KDiskFreeSp * job = new KDiskFreeSp;
      connect( job, SIGNAL( foundMountPoint( const unsigned long&, const unsigned long&, const unsigned long&, const QString& ) ),
               this, SLOT( slotFoundMountPoint( const unsigned long&, const unsigned long&, const unsigned long&, const QString& ) ) );
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

    grid = new QGridLayout(0, 3); // unknown # of rows
    grid->setColStretch(2, 1);
    grid->addColSpacing(1, KDialog::spacingHint());
    vbl->addLayout(grid);
    curRow = 0;

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


void KFilePropsPlugin::nameFileChanged(const QString &text )
{
  properties->enableButtonOK(!text.isEmpty());
  changed();
}
void KFilePropsPlugin::determineRelativePath( const QString & path )
{
    m_sRelativePath = "";
    // now let's make it relative
    QStringList dirs;
    if (KBindingPropsPlugin::supports(properties->items()))
      dirs = KGlobal::dirs()->resourceDirs("mime");
    else
      dirs = KGlobal::dirs()->resourceDirs("apps");

    QStringList::ConstIterator it = dirs.begin();
    for ( ; it != dirs.end() && m_sRelativePath.isEmpty(); ++it ) {
      // might need canonicalPath() ...
      if ( path.find( *it ) == 0 ) // path is dirs + relativePath
        m_sRelativePath = path.mid( (*it).length() ); // skip appsdirs
    }
    if ( m_sRelativePath.isEmpty() )
    {
      if (KBindingPropsPlugin::supports(properties->items()))
        kdWarning(250) << "Warning : editing a mimetype file out of the mimetype dirs!" << endl;
      // for Application desktop files, no problem : we can editing a .desktop file anywhere...
    } else
        while ( m_sRelativePath.at(0) == '/' ) m_sRelativePath.remove( 0, 1 );
}

void KFilePropsPlugin::slotFoundMountPoint( const QString&, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail )
{
    d->m_freeSpaceLabel->setText( i18n("Available space out of total partition size (percent used)", "%1/%2 (%3% used)")
                               .arg(KIO::convertSizeFromKB(kBAvail))
                               .arg(KIO::convertSizeFromKB(kBSize))
                               .arg( (int)(100.0 * kBUsed / kBSize) ) );
}

// attention: copy&paste below, due to compiler bug
// it doesn't like those unsigned long parameters -- unsigned long& are ok :-/
void KFilePropsPlugin::slotFoundMountPoint( const unsigned long& kBSize, const unsigned long& kBUsed, const unsigned long& kBAvail, const QString& )
{
    d->m_freeSpaceLabel->setText( i18n("Available space out of total partition size (percent used)", "%1/%2 (%3% used)")
                               .arg(KIO::convertSizeFromKB(kBAvail))
                               .arg(KIO::convertSizeFromKB(kBSize))
                               .arg( (int)(100.0 * kBUsed / kBSize) ) );
}

void KFilePropsPlugin::slotDirSizeFinished( KIO::Job * job )
{
  if (job->error())
    m_sizeLabel->setText( job->errorString() );
  else
  {
    KIO::filesize_t totalSize = static_cast<KDirSize*>(job)->totalSize();
    m_sizeLabel->setText( QString::fromLatin1("%1 (%2)").arg(KIO::convertSize(totalSize)).arg(KGlobal::locale()->formatNumber(totalSize, 0)) );
  }
  m_sizeStopButton->setEnabled(false);
  // just in case you change something and try again :)
  m_sizeDetermineButton->setText( i18n("Refresh") );
  m_sizeDetermineButton->setEnabled(true);
  d->dirSizeJob = 0L;
}

void KFilePropsPlugin::slotSizeDetermine()
{
  m_sizeLabel->setText( i18n("Calculating...") );
  kdDebug(250) << " KFilePropsPlugin::slotSizeDetermine() properties->item()=" <<  properties->item() << endl;
  kdDebug(250) << " URL=" << properties->item()->url().url() << endl;
  d->dirSizeJob = KDirSize::dirSizeJob( properties->items() );
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

  // Save the file where we can -> usually in ~/.kde/...
  if (KBindingPropsPlugin::supports(properties->items()) && !m_sRelativePath.isEmpty())
  {
    KURL newURL;
    newURL.setPath( locateLocal("mime", m_sRelativePath) );
    properties->updateUrl( newURL );
  }
  else if (KExecPropsPlugin::supports(properties->items()) && !m_sRelativePath.isEmpty())
  {
    kdDebug(250) << "KFilePropsPlugin::slotCopyFinished " << m_sRelativePath << endl;
    KURL newURL;
    newURL.setPath( locateLocal("apps", m_sRelativePath) );
    kdDebug(250) << "KFilePropsPlugin::slotCopyFinished path=" << newURL.path() << endl;
    properties->updateUrl( newURL );
  }

  // handle icon changes - only local files for now
  // TODO: Use KTempFile and KIO::file_copy with resume = true
  if (!iconArea->isA("QLabel") && properties->kurl().isLocalFile()) {
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
          KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient access to write to <b>%1</b>.</qt>").arg(path));
          return;
        }
        f.close();

        KDesktopFile cfg(path);
        kdDebug(250) << "sIcon = " << (sIcon) << endl;
        kdDebug(250) << "str = " << (str) << endl;
        cfg.writeEntry( QString::fromLatin1("Icon"), sIcon );
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

  bool IamRoot = (geteuid() == 0);

  KFileItem * item = properties->item();
  bool isLink = item->isLink();
  bool isDir = item->isDir(); // all dirs
  bool hasDir = item->isDir(); // at least one dir
  permissions = item->permissions(); // common permissions to all files
  mode_t partialPermissions = permissions; // permissions that only some files have (at first we take everything)
  strOwner = item->user();
  strGroup = item->group();

  if ( properties->items().count() > 1 )
  {
    // Multiple items: see what they have in common
    KFileItemList items = properties->items();
    KFileItemListIterator it( items );
    for ( ++it /*no need to check the first one again*/ ; it.current(); ++it )
    {
      if ( (*it)->isLink() != isLink )
        isLink = false;
      if ( (*it)->isDir() != isDir )
        isDir = false;
      hasDir |= (*it)->isDir();
      if ( (*it)->permissions() != permissions )
      {
        permissions &= (*it)->permissions();
        partialPermissions |= (*it)->permissions();
      }
      if ( (*it)->user() != strOwner )
        strOwner = QString::null;
      if ( (*it)->group() != strGroup )
        strGroup = QString::null;
    }
  }

  // keep only what's not in the common permissions
  partialPermissions = partialPermissions & ~permissions;

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

  d->m_frame = properties->dialog()->addPage(i18n("&Permissions"));

  QBoxLayout *box = new QVBoxLayout( d->m_frame, KDialog::spacingHint() );

  QLabel *l, *cl[3];
  QGroupBox *gb;
  QGridLayout *gl;

  /* Group: Access Permissions */
  gb = new QGroupBox ( i18n("Access Permissions"), d->m_frame );
  box->addWidget (gb);

  gl = new QGridLayout (gb, 6, 6, 15);
  gl->addRowSpacing(0, 10);

  l = new QLabel(i18n("Class"), gb);
  gl->addWidget(l, 1, 0);

  if (isDir)
    l = new QLabel( i18n("Show\nEntries"), gb );
  else
    l = new QLabel( i18n("Read"), gb );
  gl->addWidget (l, 1, 1);

  if (isDir)
    l = new QLabel( i18n("Write\nEntries"), gb );
  else
    l = new QLabel( i18n("Write"), gb );
  gl->addWidget (l, 1, 2);

  if (isDir)
    l = new QLabel( i18n("Enter directory", "Enter"), gb );
  else
    l = new QLabel( i18n("Exec"), gb );
  // GJ: Add space between normal and special modes
  QSize size = l->sizeHint();
  size.setWidth(size.width() + 15);
  l->setFixedSize(size);
  gl->addWidget (l, 1, 3);

  l = new QLabel( i18n("Special"), gb );
  gl->addMultiCellWidget(l, 1, 1, 4, 5);

  cl[0] = new QLabel( i18n("User"), gb );
  gl->addWidget (cl[0], 2, 0);

  cl[1] = new QLabel( i18n("Group"), gb );
  gl->addWidget (cl[1], 3, 0);

  cl[2] = new QLabel( i18n("Others"), gb );
  gl->addWidget (cl[2], 4, 0);

  l = new QLabel(i18n("Set UID"), gb);
  gl->addWidget(l, 2, 5);

  l = new QLabel(i18n("Set GID"), gb);
  gl->addWidget(l, 3, 5);

  l = new QLabel(i18n("File permission, sets user or group ID on execution", "Sticky"), gb);
  gl->addWidget(l, 4, 5);

  bool enablePage = (isMyFile || IamRoot) && (!isLink);
  /* Draw Checkboxes */
  for (int row = 0; row < 3 ; ++row) {
    for (int col = 0; col < 4; ++col) {
      QCheckBox *cb = new QCheckBox(gb);
      cb->setChecked(permissions & fperm[row][col]);
      if ( partialPermissions & fperm[row][col] )
      {
        cb->setTristate( true );
        cb->setNoChange();
      }
      cb->setEnabled( enablePage );
      permBox[row][col] = cb;
      gl->addWidget (permBox[row][col], row+2, col+1);
      connect( cb, SIGNAL( clicked() ),
               this, SIGNAL( changed() ) );
    }
  }
  gl->setColStretch(6, 10);
  gb->setEnabled( enablePage );

  /**** Group: Ownership ****/
  gb = new QGroupBox ( i18n("Ownership"), d->m_frame );
  box->addWidget (gb);

  gl = new QGridLayout (gb, 4, 3, 15);
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
  if ( hasDir )
  {
      d->cbRecursive = new QCheckBox( i18n("Apply changes to all subdirectories and their contents"), d->m_frame );
      box->addWidget( d->cbRecursive );
      connect( d->cbRecursive, SIGNAL( clicked() ),
               this, SLOT( slotRecursiveClicked() ) );
  }

  box->addStretch (10);

  if (isMyFile)
    cl[0]->setText(i18n("<b>User</b>"));
  else if (isMyGroup)
    cl[1]->setText(i18n("<b>Group</b>"));
  else
    cl[2]->setText(i18n("<b>Others</b>"));
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

void KFilePermissionsPropsPlugin::slotRecursiveClicked()
{
  // If we want to apply permissions recursively, then we didn't
  // show up the right permissions to start with. Files in subdirs might
  // have other flags.... At least, let the user the possibility
  // to set any flag to "unchanged", so that he isn't forced to set +x
  // on all files !
  for (int row = 0;row < 3; ++row)
    for (int col = 0; col < 4; ++col)
      permBox[row][col]->setTristate();
}

void KFilePermissionsPropsPlugin::applyChanges()
{
  mode_t newPermission = 0;
  mode_t permissionMask = 0;
  for (int row = 0;row < 3; ++row)
    for (int col = 0; col < 4; ++col)
    {
      switch (permBox[row][col]->state())
      {
          case QCheckBox::On:
            newPermission |= fperm[row][col];
            //fall through
          case QCheckBox::Off:
            permissionMask |= fperm[row][col];
            break;
          default: // NoChange
            break;
      }
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

  kdDebug(250) << "old permissions : " << QString::number(permissions,8) << endl;
  kdDebug(250) << "new permissions : " << QString::number(newPermission,8) << endl;
  kdDebug(250) << "permissions mask : " << QString::number(permissionMask,8) << endl;
  kdDebug(250) << "url=" << properties->items().first()->url().url() << endl;

  if ( permissions != newPermission || !owner.isEmpty() || !group.isEmpty() )
  {
    KIO::Job * job = KIO::chmod( properties->items(), newPermission, permissionMask,
                                 owner, group,
                                 d->cbRecursive && d->cbRecursive->isChecked() );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotChmodResult( KIO::Job * ) ) );
    // Wait for job
    QWidget dummy(0,0,WType_Dialog|WShowModal);
    qt_enter_modal(&dummy);
    qApp->enter_loop();
    qt_leave_modal(&dummy);
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
};

KExecPropsPlugin::KExecPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KExecPropsPluginPrivate;
  d->m_frame = properties->dialog()->addPage(i18n("E&xecute"));
  QVBoxLayout * mainlayout = new QVBoxLayout( d->m_frame );
  mainlayout->setSpacing( KDialog::spacingHint() );

  // Now the widgets in the top layout

  QLabel* l;
  l = new QLabel( i18n( "Comman&d:" ), d->m_frame );
  mainlayout->addWidget(l);

  QHBoxLayout * hlayout;
  hlayout = new QHBoxLayout(KDialog::spacingHint());
  mainlayout->addLayout(hlayout);

  execEdit = new KLineEdit( d->m_frame );
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

  grid = new QGridLayout(tmpQGroupBox->layout(), 2, 2);
  grid->setSpacing( KDialog::spacingHint() );
  grid->setColStretch(1, 1);

  terminalCheck = new QCheckBox( tmpQGroupBox );
  terminalCheck->setText( i18n("&Run in terminal") );
  grid->addMultiCellWidget(terminalCheck, 0, 0, 0, 1);

  terminalLabel = new QLabel( i18n( "&Terminal options:" ), tmpQGroupBox );
  grid->addWidget(terminalLabel, 1, 0);

  terminalEdit = new KLineEdit( tmpQGroupBox );
  grid->addWidget(terminalEdit, 1, 1);

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
  execStr = config.readEntry( QString::fromLatin1("Exec") );
  swallowExecStr = config.readEntry( QString::fromLatin1("SwallowExec") );
  swallowTitleStr = config.readEntry( QString::fromLatin1("SwallowTitle") );
  termBool = config.readBoolEntry( QString::fromLatin1("Terminal") );
  termOptionsStr = config.readEntry( QString::fromLatin1("TerminalOptions") );
  suidBool = config.readBoolEntry( QString::fromLatin1("X-KDE-SubstituteUID") );
  suidUserStr = config.readEntry( QString::fromLatin1("X-KDE-Username") );

  if ( !swallowExecStr.isNull() )
    swallowExecEdit->setText( swallowExecStr );
  if ( !swallowTitleStr.isNull() )
    swallowTitleEdit->setText( swallowTitleStr );

  if ( !execStr.isNull() )
    execEdit->setText( execStr );
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

// QString KExecPropsPlugin::tabName () const
// {
//   return i18n ("E&xecute");
// }

void KExecPropsPlugin::enableCheckedEdit()
{
  bool checked = terminalCheck->isChecked();
  terminalLabel->setEnabled( checked );
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
  return config.hasApplicationType();
}

void KExecPropsPlugin::applyChanges()
{
  kdDebug(250) << "KExecPropsPlugin::applyChanges" << endl;
  QString path = properties->kurl().path();

  QFile f( path );

  if ( !f.open( IO_ReadWrite ) ) {
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Application"));
  config.writeEntry( QString::fromLatin1("Exec"), execEdit->text() );
  config.writeEntry( QString::fromLatin1("SwallowExec"), swallowExecEdit->text() );
  config.writeEntry( QString::fromLatin1("SwallowTitle"), swallowTitleEdit->text() );
  config.writeEntry( QString::fromLatin1("Terminal"), terminalCheck->isChecked() );
  config.writeEntry( QString::fromLatin1("TerminalOptions"), terminalEdit->text() );
  config.writeEntry( QString::fromLatin1("X-KDE-SubstituteUID"), suidCheck->isChecked() );
  config.writeEntry( QString::fromLatin1("X-KDE-Username"), suidEdit->text() );
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
  d->m_frame = properties->dialog()->addPage(i18n("U&RL"));
  QVBoxLayout * layout = new QVBoxLayout(d->m_frame, KDialog::spacingHint());

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
  URLStr = config.readEntry( QString::fromLatin1("URL") );

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
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Link"));
  config.writeEntry( QString::fromLatin1("URL"), URLEdit->url() );
}

/* ----------------------------------------------------
 *
 * KApplicationPropsPlugin
 *
 * -------------------------------------------------- */

class KApplicationPropsPlugin::KApplicationPropsPluginPrivate
{
public:
  KApplicationPropsPluginPrivate()
  {
  }
  ~KApplicationPropsPluginPrivate()
  {
  }

  QFrame *m_frame;
};

KApplicationPropsPlugin::KApplicationPropsPlugin( KPropertiesDialog *_props )
  : KPropsDlgPlugin( _props )
{
  d = new KApplicationPropsPluginPrivate;
  d->m_frame = properties->dialog()->addPage(i18n("&Application"));
  QVBoxLayout *toplayout = new QVBoxLayout( d->m_frame, KDialog::spacingHint());

  addExtensionButton = new QPushButton( QString::null, d->m_frame );
  addExtensionButton->setPixmap( BarIcon( "back", KIcon::SizeSmall ) );
  connect( addExtensionButton, SIGNAL( clicked() ),
            SLOT( slotAddExtension() ) );

  delExtensionButton = new QPushButton( QString::null, d->m_frame );
  delExtensionButton->setPixmap( BarIcon( "forward", KIcon::SizeSmall ) );
  connect( delExtensionButton, SIGNAL( clicked() ),
            SLOT( slotDelExtension() ) );

  QLabel *l;

  QGridLayout *grid = new QGridLayout(2, 2);
  grid->setColStretch(1, 1);
  toplayout->addLayout(grid);

  l = new QLabel(i18n("Name:"), d->m_frame, "Label_4" );
  grid->addWidget(l, 0, 0);

  nameEdit = new KLineEdit( d->m_frame, "LineEdit_3" );
  grid->addWidget(nameEdit, 0, 1);

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
  QString commentStr = config.readEntry( QString::fromLatin1("Comment") );
  QString genNameStr = config.readEntry( QString::fromLatin1("GenericName") );

  QStringList selectedTypes = config.readListEntry( "ServiceTypes" );
  // For compatibility with KDE 1.x
  selectedTypes += config.readListEntry( "MimeType", ';' );

  QString nameStr = config.readEntry( QString::fromLatin1("Name") );
  if ( nameStr.isEmpty() ) {
    // We'll use the file name if no name is specified
    // because we _need_ a Name for a valid file.
    // But let's do it in apply, not here, so that we pick up the right name.
    setDirty();
  }

  commentEdit->setText( commentStr );
  genNameEdit->setText( genNameStr );
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

  connect( availableExtensionsList, SIGNAL( selected( int ) ),
           this, SIGNAL( changed() ) );
  connect( addExtensionButton, SIGNAL( clicked() ),
           this, SIGNAL( changed() ) );
  connect( delExtensionButton, SIGNAL( clicked() ),
           this, SIGNAL( changed() ) );
  connect( nameEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( commentEdit, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( genNameEdit, SIGNAL( textChanged( const QString & ) ),
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
    addExtensionButton->setEnabled(availableExtensionsList->count()>0);
    delExtensionButton->setEnabled(extensionsList->count()>0);
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
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Application"));
  config.writeEntry( QString::fromLatin1("Comment"), commentEdit->text(), true, false, true );
  config.writeEntry( QString::fromLatin1("GenericName"), genNameEdit->text(), true, false, true );

  QStringList selectedTypes;
  for ( uint i = 0; i < extensionsList->count(); i++ )
    selectedTypes.append( extensionsList->text( i ) );

  config.writeEntry( QString::fromLatin1("MimeType"), selectedTypes, ';' );
  config.writeEntry( QString::fromLatin1("ServiceTypes"), "" );
  // hmm, actually it should probably be the contrary (but see also typeslistitem.cpp)

  QString nameStr = nameEdit->text();
  if ( nameStr.isEmpty() )
  {
    nameStr = properties->kurl().fileName();
    if ( nameStr.right(8) == QString::fromLatin1(".desktop") )
      nameStr.truncate( nameStr.length() - 8 );
    if ( nameStr.right(7) == QString::fromLatin1(".kdelnk") )
      nameStr.truncate( nameStr.length() - 7 );
  }
  config.writeEntry( QString::fromLatin1("Name"), nameStr, true, false, true );

  config.sync();
  f.close();
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
  d->m_frame = properties->dialog()->addPage(i18n("A&ssociation"));
  patternEdit = new KLineEdit( d->m_frame, "LineEdit_1" );
  commentEdit = new KLineEdit( d->m_frame, "LineEdit_2" );
  mimeEdit = new KLineEdit( d->m_frame, "LineEdit_3" );

  QBoxLayout * mainlayout = new QVBoxLayout(d->m_frame, KDialog::spacingHint());
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
  QString patternStr = config.readEntry( QString::fromLatin1("Patterns") );
  QString iconStr = config.readEntry( QString::fromLatin1("Icon") );
  QString commentStr = config.readEntry( QString::fromLatin1("Comment") );
  m_sMimeStr = config.readEntry( QString::fromLatin1("MimeType") );

  if ( !patternStr.isEmpty() )
    patternEdit->setText( patternStr );
  if ( !commentStr.isEmpty() )
    commentEdit->setText( commentStr );
  if ( !m_sMimeStr.isEmpty() )
    mimeEdit->setText( m_sMimeStr );
  cbAutoEmbed->setTristate();
  if ( config.hasKey( QString::fromLatin1("X-KDE-AutoEmbed") ) )
      cbAutoEmbed->setChecked( config.readBoolEntry( QString::fromLatin1("X-KDE-AutoEmbed") ) );
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
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("MimeType") );

  config.writeEntry( QString::fromLatin1("Patterns"),  patternEdit->text() );
  config.writeEntry( QString::fromLatin1("Comment"), commentEdit->text(), true, false, true );
  config.writeEntry( QString::fromLatin1("MimeType"), mimeEdit->text() );
  if ( cbAutoEmbed->state() == QButton::NoChange )
      config.deleteEntry( QString::fromLatin1("X-KDE-AutoEmbed"), false );
  else
      config.writeEntry( QString::fromLatin1("X-KDE-AutoEmbed"), cbAutoEmbed->isChecked() );
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
};

KDevicePropsPlugin::KDevicePropsPlugin( KPropertiesDialog *_props ) : KPropsDlgPlugin( _props )
{
  d = new KDevicePropsPluginPrivate;
  d->m_frame = properties->dialog()->addPage(i18n("De&vice"));

  QStringList devices;
  QCString fstabFile;
  indexDevice = 0;  // device on first column
  indexMountPoint = 1; // mount point on second column
  if ( QFile::exists(QString::fromLatin1("/etc/fstab")) ) // Linux, ...
  {
    fstabFile = "/etc/fstab";
  }
  else if ( QFile::exists(QString::fromLatin1("/etc/vfstab")) ) // Solaris
  {
    fstabFile = "/etc/vfstab";
    indexMountPoint++;
  }

  // insert your favorite location for fstab here
  if ( !fstabFile.isEmpty() )
  {
    QFile f( fstabFile );
    if ( f.open( IO_ReadOnly ) )
    {
      QTextStream stream( &f );
      while ( !stream.eof() )
      {
        QString line = stream.readLine();
        line = line.simplifyWhiteSpace();
        if (!line.isEmpty() && line[0] == '/') // skip comments but also
        {
          QStringList lst = QStringList::split( ' ', line );
          if ( lst.count() > 2 && lst[indexDevice] != QString::fromLatin1("/proc")
              && lst[indexMountPoint] != QString::fromLatin1("none")
              && lst[indexMountPoint] != QString::fromLatin1("-") )
          {
            devices.append( lst[indexDevice]+QString::fromLatin1(" (")
                             +lst[indexMountPoint]+QString::fromLatin1(")") );
            m_devicelist.append( line );
          }
        }
      }
      f.close();
    }
  }


  QGridLayout *layout = new QGridLayout( d->m_frame, 0, 3, KDialog::marginHint(),
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
  readonly->setText(  i18n("Read Only") );
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
  QString deviceStr = config.readEntry( QString::fromLatin1("Dev") );
  QString mountPointStr = config.readEntry( QString::fromLatin1("MountPoint") );
  bool ro = config.readBoolEntry( QString::fromLatin1("ReadOnly"), false );
  QString unmountedStr = config.readEntry( QString::fromLatin1("UnmountIcon") );

  device->setEditText( deviceStr );
  if ( !deviceStr.isEmpty() ) {
    // Set default options for this device (first matching entry)
    int index = 0;
    for ( QStringList::Iterator it = m_devicelist.begin();
          it != m_devicelist.end(); ++it, ++index ) {
      // WARNING : this works only if indexDevice == 0
      if ( (*it).left( deviceStr.length() ) == deviceStr ) {
        //kdDebug(250) << "found it " << index << endl;
        slotActivated( index );
        break;
      }
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
  QStringList lst = QStringList::split( ' ', m_devicelist[index] );
  device->setEditText( lst[indexDevice] );
  mountpoint->setText( lst[indexMountPoint] );
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
    KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient access to write to <b>%1</b>.</qt>").arg(path));
    return;
  }
  f.close();

  KSimpleConfig config( path );
  config.setDesktopGroup();
  config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("FSDevice") );

  config.writeEntry( QString::fromLatin1("Dev"), device->currentText() );
  config.writeEntry( QString::fromLatin1("MountPoint"), mountpoint->text() );

  config.writeEntry( QString::fromLatin1("UnmountIcon"), unmounted->icon() );
  kdDebug(250) << "unmounted->icon() = " << unmounted->icon() << endl;

  config.writeEntry( QString::fromLatin1("ReadOnly"), readonly->isChecked() );

  config.sync();
}

void KPropertiesDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KPropsDlgPlugin::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kpropertiesdialog.moc"
