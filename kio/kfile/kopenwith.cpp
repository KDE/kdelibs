/*  This file is part of the KDE libraries

    Copyright (C) 1997 Torben Weis <weis@stud.uni-frankfurt.de>
    Copyright (C) 1999 Dirk A. Mueller <dmuell@gmx.net>
    Portions copyright (C) 1999 Preston Brown <pbrown@kde.org>

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

// $Id$

#include <qfile.h>
#include <qdir.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kdesktopfile.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimemagic.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kuserprofile.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <dcopclient.h>
#include <kmimetype.h>
#include <kservicegroup.h>
#include <klistview.h>
#include <ksycoca.h>

#include "kopenwith.h"
#include "kopenwith_p.h"

#include <kdebug.h>
#include <assert.h>
#include <stdlib.h>

template class QPtrList<QString>;

#define SORT_SPEC (QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( KListView* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, QString p, QString c )
    : QListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( QListViewItem* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, QString p, QString c )
    : QListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

void KAppTreeListItem::init(const QPixmap& pixmap, bool parse, bool dir, QString _path, QString _exec)
{
    setPixmap(0, pixmap);
    parsed = parse;
    directory = dir;
    path = _path; // relative path
    exec = _exec;
    exec.simplifyWhiteSpace();
    exec.truncate(exec.find(' '));
}


// ----------------------------------------------------------------------
// Ensure that dirs are sorted in front of files and case is ignored

QString KAppTreeListItem::key(int column, bool /*ascending*/) const
{
    if (directory)
        return QString::fromLatin1(" ") + text(column).upper();
    else
        return text(column).upper();
}

void KAppTreeListItem::activate()
{
    if ( directory )
        setOpen(!isOpen());
}

void KAppTreeListItem::setOpen( bool o )
{
    if( o && !parsed ) { // fill the children before opening
        ((KApplicationTree *) parent())->addDesktopGroup( path, this );
        parsed = true;
    }
    QListViewItem::setOpen( o );
}

bool KAppTreeListItem::isDirectory()
{
    return directory;
}

// ----------------------------------------------------------------------

KApplicationTree::KApplicationTree( QWidget *parent )
    : KListView( parent ), currentitem(0)
{
    addColumn( i18n("Known Applications") );
    setRootIsDecorated( true );

    addDesktopGroup( QString::null );

    connect( this, SIGNAL( currentChanged(QListViewItem*) ), 
            SLOT( slotItemHighlighted(QListViewItem*) ) );
    connect( this, SIGNAL( selectionChanged(QListViewItem*) ), 
            SLOT( slotSelectionChanged(QListViewItem*) ) );
}

// ----------------------------------------------------------------------

bool KApplicationTree::isDirSel()
{
    if (!currentitem) return false; // if currentitem isn't set
    return currentitem->isDirectory();
}

// ----------------------------------------------------------------------

void KApplicationTree::addDesktopGroup( QString relPath, KAppTreeListItem *item)
{
   KServiceGroup::Ptr root = KServiceGroup::group(relPath);
   KServiceGroup::List list = root->entries();

   KAppTreeListItem * newItem;
   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
   {
      QString icon;
      QString text;
      QString relPath;
      QString exec;
      bool isDir = false;
      KSycocaEntry *p = (*it);
      if (p->isType(KST_KService))
      {
         KService *service = static_cast<KService *>(p);

         if (service->noDisplay())
            continue;

         icon = service->icon();
         text = service->name();
         exec = service->exec();
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);

         if (serviceGroup->noDisplay())
            continue;

         icon = serviceGroup->icon();
         text = serviceGroup->caption();
         relPath = serviceGroup->relPath();
         isDir = true;
         if ( text[0] == '.' ) // skip ".hidden" like kicker does
           continue;
      }
      else
      {
         kdWarning(250) << "KServiceGroup: Unexpected object in list!" << endl;
         continue;
      }

      QPixmap pixmap = SmallIcon( icon );

      if (item)
         newItem = new KAppTreeListItem( item, text, pixmap, false, isDir,
                                         relPath, exec );
      else
         newItem = new KAppTreeListItem( this, text, pixmap, false, isDir,
                                         relPath, exec );
      if (isDir)
         newItem->setExpandable( true );
   }
}


// ----------------------------------------------------------------------

void KApplicationTree::slotItemHighlighted(QListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    currentitem = item;

    if( (!item->directory ) && (!item->exec.isEmpty()) )
        emit highlighted( item->text(0), item->exec );
}


// ----------------------------------------------------------------------

void KApplicationTree::slotSelectionChanged(QListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    currentitem = item;

    if( ( !item->directory ) && (!item->exec.isEmpty() ) )
        emit selected( item->text(0), item->exec );
}

// ----------------------------------------------------------------------

void KApplicationTree::resizeEvent( QResizeEvent * e)
{
    setColumnWidth(0, width()-QApplication::style().pixelMetric(QStyle::PM_ScrollBarExtent)
                         -2*QApplication::style().pixelMetric(QStyle::PM_DefaultFrameWidth));
    KListView::resizeEvent(e);
}


/***************************************************************
 *
 * KOpenWithDlg
 *
 ***************************************************************/

KOpenWithDlg::KOpenWithDlg( const KURL::List& _urls, QWidget* parent )
             :QDialog( parent, 0L, true )
{
    setCaption( i18n( "Open With" ) );
    QString text;
    if( _urls.count() == 1 )
    {
        text = i18n("<qt>Select the program that should be used to open <b>%1</b>. "
                     "If the program is not listed, enter the name or click "
                     "the browse button.</qt>").arg( _urls.first().fileName() );
    }
    else
        // Should never happen ??
        text = i18n( "Choose the name of the program with which to open the selected files." );
    setServiceType( _urls );
    init( text, QString() );
}

KOpenWithDlg::KOpenWithDlg( const KURL::List& _urls, const QString&_text,
                            const QString& _value, QWidget *parent)
             :QDialog( parent, 0L, true )
{
  QString caption = KStringHandler::csqueeze( _urls.first().prettyURL() );
  if (_urls.count() > 1)
      caption += QString::fromLatin1("...");
  setCaption(caption);
  setServiceType( _urls );
  init( _text, _value );
}

KOpenWithDlg::KOpenWithDlg( const QString &serviceType, const QString& value,
                            QWidget *parent)
             :QDialog( parent, 0L, true )
{
  setCaption(i18n("Choose Application for %1").arg(serviceType));
  QString text = i18n("<qt>Select the program for the file type: <b>%1</b>. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>").arg(serviceType);
  qServiceType = serviceType;
  init( text, value );
  if (remember)
  {
      remember->setChecked( true );
      remember->hide();
  }
}

KOpenWithDlg::KOpenWithDlg( QWidget *parent)
             :QDialog( parent, 0L, true )
{
  setCaption(i18n("Choose Application"));
  QString text = i18n("<qt>Select a program. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>");
  qServiceType = QString::null;
  init( text, QString::null );
}

void KOpenWithDlg::setServiceType( const KURL::List& _urls )
{
  if ( _urls.count() == 1 )
  {
    qServiceType = KMimeType::findByURL( _urls.first())->name();
    if (qServiceType == QString::fromLatin1("application/octet-stream"))
      qServiceType = QString::null;
  }
  else
      qServiceType = QString::null;
}

void KOpenWithDlg::init( const QString& _text, const QString& _value )
{
  bool bReadOnly = kapp && !kapp->authorize("shell_access");
  m_terminaldirty = false;
  m_pTree = 0L;
  m_pService = 0L;

  QBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
          KDialog::spacingHint() );
  label = new QLabel( _text, this );
  topLayout->addWidget(label);

  QHBoxLayout* hbox = new QHBoxLayout(topLayout);

  QToolButton *clearButton = new QToolButton( this );
  clearButton->setIconSet( BarIcon( "locationbar_erase" ) );
  clearButton->setFixedSize( clearButton->sizeHint() );
  connect( clearButton, SIGNAL( pressed() ), SLOT( slotClear() ) );
  QToolTip::add( clearButton, i18n( "Clear input field" ) );

  hbox->addWidget( clearButton );

  if (!bReadOnly)
  {
    // init the history combo and insert it into the URL-Requester
    KHistoryCombo *combo = new KHistoryCombo();
    combo->setDuplicatesEnabled( false );
    KConfig *kc = KGlobal::config();
    KConfigGroupSaver ks( kc, QString::fromLatin1("Open-with settings") );
    int max = kc->readNumEntry( QString::fromLatin1("Maximum history"), 15 );
    combo->setMaxCount( max );
    QStringList list = kc->readListEntry( QString::fromLatin1("History") );
    combo->setHistoryItems( list, true );
    edit = new KURLRequester( combo, this );
  }
  else
  { 
    clearButton->hide();
    edit = new KURLRequester( this );
    edit->lineEdit()->setReadOnly(true);
    edit->button()->hide();
  }

  edit->setURL( _value );

  hbox->addWidget(edit);

  if ( edit->comboBox() ) {
    KURLCompletion *comp = new KURLCompletion( KURLCompletion::ExeCompletion );
    edit->comboBox()->setCompletionObject( comp );
  }

  connect ( edit, SIGNAL(returnPressed()), SLOT(slotOK()) );
  connect ( edit, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged()) );

  m_pTree = new KApplicationTree( this );
  topLayout->addWidget(m_pTree);

  connect( m_pTree, SIGNAL( selected( const QString&, const QString& ) ),
           SLOT( slotSelected( const QString&, const QString& ) ) );
  connect( m_pTree, SIGNAL( highlighted( const QString&, const QString& ) ),
           SLOT( slotHighlighted( const QString&, const QString& ) ) );
  connect( m_pTree, SIGNAL( doubleClicked(QListViewItem*) ),
           SLOT( slotDbClick() ) );

  terminal = new QCheckBox( i18n("Run in &terminal"), this );
  if (bReadOnly)
     terminal->hide();
  connect(terminal, SIGNAL(toggled(bool)), SLOT(slotTerminalToggled(bool)));

  topLayout->addWidget(terminal);

  if (!qServiceType.isNull())
  {
    remember = new QCheckBox(i18n("&Remember application association for this type of file"), this);
    //    remember->setChecked(true);
    topLayout->addWidget(remember);
  }
  else
    remember = 0L;

  // Use KButtonBox for the aligning pushbuttons nicely
  KButtonBox* b = new KButtonBox( this );
  b->addStretch( 2 );

  ok = b->addButton(  i18n ( "&OK" ) );
  ok->setDefault( true );
  connect(  ok, SIGNAL( clicked() ), SLOT( slotOK() ) );

  cancel = b->addButton(  i18n( "&Cancel" ) );
  connect(  cancel, SIGNAL( clicked() ), SLOT( reject() ) );

  b->layout();
  topLayout->addWidget( b );

  //edit->setText( _value );
  // This is what caused "can't click on items before clicking on Name header".
  // Probably due to the resizeEvent handler using width().
  //resize( minimumWidth(), sizeHint().height() );
  edit->setFocus();
}


// ----------------------------------------------------------------------

KOpenWithDlg::~KOpenWithDlg()
{
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotClear()
{
    edit->setURL(QString::null);
    edit->setFocus();
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotSelected( const QString& /*_name*/, const QString& _exec )
{
    kdDebug(250)<<"KOpenWithDlg::slotSelected"<<endl;
    KService::Ptr pService = m_pService;
    edit->setURL( _exec ); // calls slotTextChanged :(
    m_pService = pService;
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotHighlighted( const QString& _name, const QString& )
{
    kdDebug(250)<<"KOpenWithDlg::slotHighlighted"<<endl;
    qName = _name;
    m_pService = KService::serviceByName( qName );
    if (!m_terminaldirty)
    {
        // ### indicate that default value was restored
        terminal->setChecked(m_pService->terminal());
        m_terminaldirty = false; // slotTerminalToggled changed it
    }
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotTextChanged()
{
    kdDebug(250)<<"KOpenWithDlg::slotTextChanged"<<endl;
    // Forget about the service
    m_pService = 0L;
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotTerminalToggled(bool)
{
    // ### indicate that default value was overridden
    m_terminaldirty = true;
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotDbClick()
{
   if (m_pTree->isDirSel() ) return; // check if a directory is selected
   slotOK();
}

void KOpenWithDlg::slotOK()
{
  if (!m_pService) {
    // no service was found, maybe they typed the name into the text field
    m_pService = KService::serviceByDesktopName(edit->url());
    if (m_pService)
    {
      KService::Ptr pService = m_pService;
      edit->setURL(m_pService->exec()); // calls slotTextChanged :(
      m_pService = pService;
    }
  }

  if (terminal->isChecked()) {
    KSimpleConfig conf(QString::fromLatin1("konquerorrc"), true);
    conf.setGroup(QString::fromLatin1("Misc Defaults"));
    m_command = conf.readEntry(QString::fromLatin1("Terminal"), QString::fromLatin1("konsole"));

    m_command += QString::fromLatin1(" -e ");
    m_command += edit->url();
    kdDebug(250) << "Setting m_command to " << m_command << endl;
  }
  if ( m_pService && terminal->isChecked() != m_pService->terminal() )
      m_pService = 0L; // It's not exactly this service we're running

  if (m_pService && !remember) {
    accept();
    return;
  }

  if (remember)
    if (!remember->isChecked()) {
      accept();
      return;
    }
  // if we got here, we can't seem to find a service for what they
  // wanted.  The other possibility is that they have asked for the
  // association to be remembered.  Create/update service.
  QString keepExec(edit->url());
  QString serviceName;
  QString pathName;
  if (!m_pService) {
    if (keepExec.contains('/'))
    {
      serviceName = keepExec.mid(keepExec.findRev('/') + 1);
      if (serviceName.isEmpty())
      {
        // Hmm, add a KMessageBox::error here after 2.0
        return;
      }
      pathName = serviceName;
    }
    else
    {
      // Creating a new one. Let's hide it to avoid cluttering the K menu.
      pathName = ".hidden/";
      pathName += keepExec;
      serviceName = keepExec;
    }
  } else
    serviceName = pathName = m_pService->desktopEntryPath();

  if (pathName.right(8) != QString::fromLatin1(".desktop"))
    pathName += QString::fromLatin1(".desktop");
  QString path(locateLocal("apps", pathName));

  int maxPreference = 1;
  if (!qServiceType.isEmpty())
  {
    KServiceTypeProfile::OfferList offerList = KServiceTypeProfile::offers( qServiceType );
    if (!offerList.isEmpty())
      maxPreference = offerList.first().preference();
  }

  KDesktopFile desktop(path);
  desktop.writeEntry(QString::fromLatin1("Type"), QString::fromLatin1("Application"));
  desktop.writeEntry(QString::fromLatin1("Name"), m_pService ? m_pService->name() : serviceName);
  desktop.writeEntry(QString::fromLatin1("Exec"), keepExec);
  desktop.writeEntry(QString::fromLatin1("InitialPreference"), maxPreference + 1);
  if (remember)
    if (remember->isChecked()) {
      QStringList mimeList;
      KDesktopFile oldDesktop(locate("apps", pathName), true);
      mimeList = oldDesktop.readListEntry(QString::fromLatin1("MimeType"), ';');
      if (!qServiceType.isEmpty() && !mimeList.contains(qServiceType))
        mimeList.append(qServiceType);
      desktop.writeEntry(QString::fromLatin1("MimeType"), mimeList, ';');
      if (terminal->isChecked())
        desktop.writeEntry(QString::fromLatin1("Terminal"), true);
      else
        desktop.writeEntry(QString::fromLatin1("Terminal"), false);

      if ( !qServiceType.isEmpty() )
      {
        // Also make sure the "auto embed" setting for this mimetype is off
        KDesktopFile mimeDesktop( locateLocal( "mime", qServiceType + ".desktop" ) );
        mimeDesktop.writeEntry( QString::fromLatin1( "X-KDE-AutoEmbed" ), false );
        mimeDesktop.sync();
      }
    }


  // write it all out to the file
  desktop.sync();

  QApplication::setOverrideCursor( waitCursor );

  // rebuild the database
  QStringList args;
  args.append("--incremental");
  KApplication::kdeinitExecWait( "kbuildsycoca", args );

  // get the new service pointer
  kdDebug(250) << pathName << endl;
  // We need to read in the new database. It seems the databaseChanged()
  // signal hasn't been processed in this process yet, since we haven't been
  // to the event loop yet.
  QStringList lst;
  lst << QString::fromLatin1("apps");
  KSycoca::self()->notifyDatabaseChanged( lst );

  m_pService = KService::serviceByDesktopPath( pathName );
  QApplication::restoreOverrideCursor();

  Q_ASSERT( m_pService );

  accept();
}

QString KOpenWithDlg::text() const
{
    if (!m_command.isEmpty())
        return m_command;
    else
        return edit->url();
}

void KOpenWithDlg::accept()
{
    KHistoryCombo *combo = static_cast<KHistoryCombo*>( edit->comboBox() );
    if ( combo ) {
        combo->addToHistory( edit->url() );

        KConfig *kc = KGlobal::config();
        KConfigGroupSaver ks( kc, QString::fromLatin1("Open-with settings") );
        kc->writeEntry( QString::fromLatin1("History"), combo->historyItems() );
        // don't store the completion-list, as it contains all of KURLCompletion's
        // executables
        kc->sync();
    }

    QDialog::accept();
}


///////////////

#ifndef KDE_NO_COMPAT
bool KFileOpenWithHandler::displayOpenWithDialog( const KURL::List& urls )
{
    KOpenWithDlg l( urls, i18n("Open with:"), QString::null, 0L );
    if ( l.exec() )
    {
      KService::Ptr service = l.service();
      if ( !!service )
        return KRun::run( *service, urls );

      kdDebug(250) << "No service set, running " << l.text() << endl;
      return KRun::run( l.text(), urls );
    }
    return false;
}
#endif

#include "kopenwith.moc"
#include "kopenwith_p.moc"

