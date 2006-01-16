/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

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

#include <qtimer.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qdatetime.h>
#include <qcheckbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kstringhandler.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <klineedit.h>

#ifdef Q_WS_X11
#include <kwin.h>
#endif

#include "jobclasses.h"
#include "defaultprogress.h"

namespace KIO {

class DefaultProgress::DefaultProgressPrivate
{
public:
  bool keepOpenChecked;
  bool noCaptionYet;
  KPushButton *cancelClose;
  KPushButton *openFile;
  KPushButton *openLocation;
  QCheckBox   *keepOpen;
  KURL        location;
  QTime       startTime;
};

DefaultProgress::DefaultProgress( bool showNow )
  : ProgressBase( 0 ),
  m_iTotalSize(0), m_iTotalFiles(0), m_iTotalDirs(0),
  m_iProcessedSize(0), m_iProcessedDirs(0), m_iProcessedFiles(0),d(new DefaultProgressPrivate)
{
  init();

  if ( showNow ) {
    show();
  }
}

DefaultProgress::DefaultProgress( QWidget* parent, const char* /*name*/ )
  : ProgressBase( parent ),
  m_iTotalSize(0), m_iTotalFiles(0), m_iTotalDirs(0),
  m_iProcessedSize(0), m_iProcessedDirs(0), m_iProcessedFiles(0),d(new DefaultProgressPrivate)
{
    init();
}

bool DefaultProgress::keepOpen() const
{
    return d->keepOpenChecked;
}

void DefaultProgress::init()
{

#ifdef Q_WS_X11 //FIXME(E): Remove once all the KWin::foo calls have been ported to QWS
  // Set a useful icon for this window!
  KWin::setIcons( winId(),
          KGlobal::iconLoader()->loadIcon( "filesave", KIcon::NoGroup, 32 ),
          KGlobal::iconLoader()->loadIcon( "filesave", KIcon::NoGroup, 16 ) );
#endif

  QVBoxLayout *topLayout = new QVBoxLayout( this);
  topLayout->setMargin(KDialog::marginHint());
  topLayout->setSpacing(KDialog::spacingHint() );
  topLayout->addStrut( 360 );   // makes dlg at least that wide

  QGridLayout *grid = new QGridLayout( 2, 3 );
  topLayout->addLayout(grid);
  grid->addItem(new QSpacerItem(KDialog::spacingHint(),0),0,1); //addColSpacing(1, KDialog::spacingHint());
  // filenames or action name
  grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

  sourceEdit = new KLineEdit(this);
  sourceEdit->setReadOnly(true);
  sourceEdit->setEnableSqueezedText(true);
  grid->addWidget(sourceEdit, 0, 2);

  destInvite = new QLabel(i18n("Destination:"), this);
  grid->addWidget(destInvite, 1, 0);

  destEdit = new KLineEdit(this);
  destEdit->setReadOnly (true);
  destEdit->setEnableSqueezedText(true);
  grid->addWidget(destEdit, 1, 2);

  m_pProgressBar = new QProgressBar(this);
  topLayout->addWidget( m_pProgressBar );

  // processed info
  QHBoxLayout *hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  sizeLabel = new QLabel(this);
  hBox->addWidget(sizeLabel);

  resumeLabel = new QLabel(this);
  hBox->addWidget(resumeLabel);

  progressLabel = new QLabel( this );
/*  progressLabel->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                             QSizePolicy::Preferred ) );*/
  progressLabel->setAlignment( Qt::AlignRight );
  hBox->addWidget( progressLabel );

  hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  speedLabel = new QLabel(this);
  hBox->addWidget(speedLabel, 1);

  QFrame *line = new QFrame( this );
  line->setFrameShape( QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );
  topLayout->addWidget( line );

  d->keepOpen = new QCheckBox( i18n("&Keep this window open after transfer is complete"), this);
  connect( d->keepOpen, SIGNAL( toggled(bool) ), SLOT( slotKeepOpenToggled(bool) ) );
  topLayout->addWidget(d->keepOpen);
  d->keepOpen->hide();

  hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  d->openFile = new KPushButton( i18n("Open &File"), this );
  connect( d->openFile, SIGNAL( clicked() ), SLOT( slotOpenFile() ) );
  hBox->addWidget( d->openFile );
  d->openFile->setEnabled(false);
  d->openFile->hide();

  d->openLocation = new KPushButton( i18n("Open &Destination"), this );
  connect( d->openLocation, SIGNAL( clicked() ), SLOT( slotOpenLocation() ) );
  hBox->addWidget( d->openLocation );
  d->openLocation->hide();

  hBox->addStretch(1);

  d->cancelClose = new KPushButton( KStdGuiItem::cancel(), this );
  connect( d->cancelClose, SIGNAL( clicked() ), SLOT( slotStop() ) );
  hBox->addWidget( d->cancelClose );

  resize( sizeHint() );
  setMaximumHeight(sizeHint().height());

  d->keepOpenChecked = false;
  d->noCaptionYet = true;
  setWindowTitle(i18n("Progress Dialog")); // show something better than kio_uiserver
}

DefaultProgress::~DefaultProgress()
{
  delete d;
}

void DefaultProgress::slotTotalSize( KIO::Job*, KIO::filesize_t size )
{
  // size is measured in bytes
  if ( m_iTotalSize == size )
      return;
  m_iTotalSize = size;
  if (d->startTime.isNull())
    d->startTime.start();
}


void DefaultProgress::slotTotalFiles( KIO::Job*, unsigned long files )
{
  if ( m_iTotalFiles == files )
      return;
  m_iTotalFiles = files;
  showTotals();
}


void DefaultProgress::slotTotalDirs( KIO::Job*, unsigned long dirs )
{
  if ( m_iTotalDirs == dirs )
      return;
  m_iTotalDirs = dirs;
  showTotals();
}

void DefaultProgress::showTotals()
{
  // Show the totals in the progress label, if we still haven't
  // processed anything. This is useful when the stat'ing phase
  // of CopyJob takes a long time (e.g. over networks).
  if ( m_iProcessedFiles == 0 && m_iProcessedDirs == 0 )
  {
    QString tmps;
    if ( m_iTotalDirs > 1 )
      // that we have a singular to translate looks weired but is only logical
      tmps = i18n("%n folder", "%n folders", m_iTotalDirs) + "   ";
    tmps += i18n("%n file", "%n files", m_iTotalFiles);
    progressLabel->setText( tmps );
  }
}

//static
QString DefaultProgress::makePercentString( unsigned long percent,
                                            KIO::filesize_t totalSize,
                                            unsigned long totalFiles )
{
  if ( totalSize )
      return i18n( "%1 % of %2 " ).arg( percent ).arg( KIO::convertSize( totalSize ) );
  else if ( totalFiles )
      return i18n( "%1 % of 1 file", "%1 % of %n files", totalFiles ).arg( percent );
  else
      return i18n( "%1 %" ).arg( percent );
}

void DefaultProgress::slotPercent( KIO::Job*, unsigned long percent )
{
  QString caption = makePercentString( percent, m_iTotalSize, m_iTotalFiles );
  m_pProgressBar->setValue( percent );
  switch(mode) {
  case Copy:
    caption.append(i18n(" (Copying)"));
    break;
  case Move:
    caption.append(i18n(" (Moving)"));
    break;
  case Delete:
    caption.append(i18n(" (Deleting)"));
    break;
  case Create:
    caption.append(i18n(" (Creating)"));
    break;
  case Done:
    caption.append(i18n(" (Done)"));
    break;
  }

  setWindowTitle( caption );
  d->noCaptionYet = false;
}


void DefaultProgress::slotInfoMessage( KIO::Job*, const QString & msg )
{
  speedLabel->setText( msg );
  speedLabel->setAlignment( speedLabel->alignment() & ~Qt::TextWordWrap );
}


void DefaultProgress::slotProcessedSize( KIO::Job*, KIO::filesize_t bytes ) {
  if ( m_iProcessedSize == bytes )
    return;
  m_iProcessedSize = bytes;

  QString tmp = i18n( "%1 of %2 complete")
                .arg( KIO::convertSize(bytes) )
                .arg( KIO::convertSize(m_iTotalSize));
  sizeLabel->setText( tmp );
}


void DefaultProgress::slotProcessedDirs( KIO::Job*, unsigned long dirs )
{
  if ( m_iProcessedDirs == dirs )
    return;
  m_iProcessedDirs = dirs;

  QString tmps;
  tmps = i18n("%1 / %n folder", "%1 / %n folders", m_iTotalDirs).arg( m_iProcessedDirs );
  tmps += "   ";
  tmps += i18n("%1 / %n file", "%1 / %n files", m_iTotalFiles).arg( m_iProcessedFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotProcessedFiles( KIO::Job*, unsigned long files )
{
  if ( m_iProcessedFiles == files )
    return;
  m_iProcessedFiles = files;

  QString tmps;
  if ( m_iTotalDirs > 1 ) {
    tmps = i18n("%1 / %n folder", "%1 / %n folders", m_iTotalDirs).arg( m_iProcessedDirs );
    tmps += "   ";
  }
  tmps += i18n("%1 / %n file", "%1 / %n files", m_iTotalFiles).arg( m_iProcessedFiles );
  progressLabel->setText( tmps );
}


void DefaultProgress::slotSpeed( KIO::Job*, unsigned long speed )
{
  if ( speed == 0 ) {
    speedLabel->setText( i18n( "Stalled") );
  } else {
    speedLabel->setText( i18n( "%1/s ( %2 remaining )").arg( KIO::convertSize( speed ))
        .arg( KIO::convertSeconds( KIO::calculateRemainingSeconds( m_iTotalSize, m_iProcessedSize, speed ))) );
  }
}


void DefaultProgress::slotCopying( KIO::Job*, const KURL& src, const KURL& dest )
{
  if ( d->noCaptionYet ) {
    setWindowTitle(i18n("Copy File(s) Progress"));
    d->noCaptionYet = false;
  }
  mode = Copy;
  sourceEdit->setText(src.prettyURL());
  setDestVisible( true );
  checkDestination( dest );
  destEdit->setText(dest.prettyURL());
}


void DefaultProgress::slotMoving( KIO::Job*, const KURL& src, const KURL& dest )
{
  if ( d->noCaptionYet ) {
    setWindowTitle(i18n("Move File(s) Progress"));
    d->noCaptionYet = false;
  }
  mode = Move;
  sourceEdit->setText(src.prettyURL());
  setDestVisible( true );
  checkDestination( dest );
  destEdit->setText(dest.prettyURL());
}


void DefaultProgress::slotCreatingDir( KIO::Job*, const KURL& dir )
{
  if ( d->noCaptionYet ) {
    setWindowTitle(i18n("Creating Folder"));
    d->noCaptionYet = false;
  }
  mode = Create;
  sourceEdit->setText(dir.prettyURL());
  setDestVisible( false );
}


void DefaultProgress::slotDeleting( KIO::Job*, const KURL& url )
{
  if ( d->noCaptionYet ) {
    setWindowTitle(i18n("Delete File(s) Progress"));
    d->noCaptionYet = false;
  }
  mode = Delete;
  sourceEdit->setText(url.prettyURL());
  setDestVisible( false );
}

void DefaultProgress::slotTransferring( KIO::Job*, const KURL& url )
{
  if ( d->noCaptionYet ) {
    setWindowTitle(i18n("Loading Progress"));
    d->noCaptionYet = false;
  }
  sourceEdit->setText(url.prettyURL());
  setDestVisible( false );
}

void DefaultProgress::slotStating( KIO::Job*, const KURL& url )
{
  setWindowTitle(i18n("Examining File Progress"));
  sourceEdit->setText(url.prettyURL());
  setDestVisible( false );
}

void DefaultProgress::slotMounting( KIO::Job*, const QString & dev, const QString & point )
{
  setWindowTitle(i18n("Mounting %1").arg(dev));
  sourceEdit->setText(point);
  setDestVisible( false );
}

void DefaultProgress::slotUnmounting( KIO::Job*, const QString & point )
{
  setWindowTitle(i18n("Unmounting"));
  sourceEdit->setText(point);
  setDestVisible( false );
}

void DefaultProgress::slotCanResume( KIO::Job*, KIO::filesize_t offset )
{
  if ( offset ) {
    resumeLabel->setText( i18n("Resuming from %1").arg(KIO::number(offset)) );
  } else {
    resumeLabel->setText( i18n("Not resumable") );
  }
}

void DefaultProgress::setDestVisible( bool visible )
{
  // We can't hide the destInvite/destEdit labels,
  // because it screws up the QGridLayout.
  if (visible)
  {
      destInvite->show();
      destEdit->show();

    destInvite->setText( i18n("Destination:") );
  }
  else
  {
      destInvite->hide();
      destEdit->hide();
    destInvite->setText( QString() );
    destEdit->setText( QString() );
  }
}

void DefaultProgress::slotClean() {
  if (d->keepOpenChecked) {
    mode = Done;
    slotPercent(0, 100);
    d->cancelClose->setGuiItem( KStdGuiItem::close() );
    d->openFile->setEnabled(true);
    slotProcessedSize(0, m_iTotalSize);
    d->keepOpen->setEnabled(false);
    if (!d->startTime.isNull()) {
      int s = d->startTime.elapsed();
      if (!s)
        s = 1;
      speedLabel->setText(i18n("%1/s (done)").arg(KIO::convertSize(1000 * m_iTotalSize / s)));
    }
    setOnlyClean(false);
  }
  else
    hide();
}

void DefaultProgress::slotKeepOpenToggled(bool keepopen)
{
  d->keepOpenChecked=keepopen;
}

void DefaultProgress::checkDestination(const KURL& dest) {
  bool ok = true;
  if ( dest.isLocalFile() ) {
      QString path = dest.path( -1 );
      QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
      for ( QStringList::Iterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it )
          if ( path.contains( *it ) )
              ok = false; // it's in the tmp resource
  }

  if ( ok ) {
    d->openFile->show();
    d->openLocation->show();
    d->keepOpen->show();
    d->location=dest;
  }
}

void DefaultProgress::slotOpenFile()
{
  KProcess proc;
  proc << "konqueror" << d->location.prettyURL();
  proc.start(KProcess::DontCare);
}

void DefaultProgress::slotOpenLocation()
{
  KProcess proc;
  d->location.setFileName("");
  proc << "konqueror" << d->location.prettyURL();
  proc.start(KProcess::DontCare);
}

void DefaultProgress::virtual_hook( int id, void* data )
{ ProgressBase::virtual_hook( id, data ); }

} /* namespace */

#include "defaultprogress.moc"
