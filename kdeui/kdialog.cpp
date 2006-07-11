/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *                      by Holger Freyther <freyther@kde.org>
 *            2005-2006 by Olivier Goffart (ogoffart at kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QHash>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QList>
#include <QPointer>
#include <QSignalMapper>
#include <QTimer>
#include <QVBoxLayout>
#include <QWhatsThis>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstaticdeleter.h>
#include <kstdguiitem.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kwhatsthismanager_p.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <netwm.h>
#endif

#include "kdialog.h"

struct KDialog::Private
{
  Private( KDialog *parent )
    : mParent( parent ), mDetails( false ), mSettingDetails( false ), mDetailsWidget( 0 ),
      mTopLayout( 0 ), mMainWidget( 0 ), mUrlHelp( 0 ), mActionSeparator( 0 ),
      mButtonOrientation( Qt::Horizontal ),
      mButtonBox( 0 )
  {
  }

  void setupLayout();
  void setButtonFocus( QPushButton *p, bool isDefault, bool isFocus );
  void appendButton( ButtonCode code , const KGuiItem &item );
  KPushButton *button( ButtonCode code ) const;


  KDialog* mParent;
  bool mDetails;
  bool mSettingDetails;
  QWidget *mDetailsWidget;
  QSize mIncSize;
  QSize mMinSize;
  QString mDetailsButton;

  QBoxLayout *mTopLayout;
  QWidget *mMainWidget;
  KUrlLabel *mUrlHelp;
  KSeparator *mActionSeparator;

  QString mAnchor;
  QString mHelpApp;
  QString mHelpLinkText;

  Qt::Orientation mButtonOrientation;
  ButtonCode mEscapeButton;

  QDialogButtonBox *mButtonBox;
  QHash<int, KPushButton*> mButtonList;
  QSignalMapper mButtonSignalMapper;

  static int mMarginSize;
  static int mSpacingSize;
};

int KDialog::Private::mMarginSize = 11;
int KDialog::Private::mSpacingSize = 6;

void KDialog::Private::setupLayout()
{
  delete mTopLayout;

  if ( mButtonOrientation == Qt::Horizontal )
    mTopLayout = new QVBoxLayout( mParent );
  else
    mTopLayout = new QHBoxLayout( mParent );

  mTopLayout->setMargin( KDialog::marginHint() );
  mTopLayout->setSpacing( KDialog::spacingHint() );

  if ( mUrlHelp )
    mTopLayout->addWidget( mUrlHelp, 0, Qt::AlignRight );

  if ( mMainWidget )
    mTopLayout->addWidget( mMainWidget, 10 );

  if ( mDetailsWidget )
    mTopLayout->addWidget( mDetailsWidget );

  if ( mActionSeparator )
    mTopLayout->addWidget( mActionSeparator );

  if ( mButtonBox )
    mTopLayout->addWidget( mButtonBox );
}

void KDialog::Private::setButtonFocus( QPushButton *button, bool isDefault, bool isFocus )
{
  button->setDefault( isDefault );
  if ( isFocus )
      button->setFocus();
}

void KDialog::Private::appendButton( ButtonCode key, const KGuiItem &item )
{
  QDialogButtonBox::ButtonRole role = QDialogButtonBox::InvalidRole;
  switch ( key ) {
    case Help:
    case Details:
      role = QDialogButtonBox::HelpRole;
      break;
    case Default:
      role = QDialogButtonBox::DestructiveRole;
      break;
    case Ok:
    case Apply:
    case Try:
    case Close:
    case Yes:
      role = QDialogButtonBox::AcceptRole;
      break;
    case Cancel:
    case No:
      role = QDialogButtonBox::RejectRole;
      break;
    case User1:
    case User2:
    case User3:
      role = QDialogButtonBox::ActionRole;
      break;
    case NoDefault:
    default:
      role = QDialogButtonBox::InvalidRole;
      break;
  }

  if ( role == QDialogButtonBox::InvalidRole )
    return;

  KPushButton *button = new KPushButton( item );
  mButtonBox->addButton( button, role );

  mButtonList.insert( key, button );
  mButtonSignalMapper.setMapping( button, key );

  connect( button, SIGNAL( clicked() ),
           &mButtonSignalMapper, SLOT( map() ) );
}

KPushButton *KDialog::Private::button( ButtonCode id ) const
{
  return mButtonList.contains( id ) ? mButtonList[ id ] : 0;
}


KDialog::KDialog( QWidget *parent, Qt::WFlags flags )
  : QDialog( parent, flags | Qt::MSWindowsFixedSizeDialogHint ),
    d( new Private( this ) )
{
  setModal( true );
  KWhatsThisManager::init();

  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  connect( this, SIGNAL( layoutHintChanged() ), SLOT( updateGeometry() ) );
  connect( &d->mButtonSignalMapper , SIGNAL( mapped( int ) ),
           this, SLOT( slotButtonClicked( int ) ) );
}

KDialog::~KDialog()
{
  delete d;
}

void KDialog::setButtons( ButtonCodes buttonMask )
{
  if ( d->mButtonBox ) {
    d->mButtonList.clear();

    delete d->mButtonBox;
    d->mButtonBox = 0;
  }

  if ( buttonMask & Cancel )
    buttonMask &= ~Close;

  if ( buttonMask & Apply )
    buttonMask &= ~Try;

  if ( buttonMask & Details )
    buttonMask &= ~Default;

  if ( buttonMask == None ) {
    d->setupLayout();
    return; // When we want no button box
  }

  d->mEscapeButton = (buttonMask & Cancel) ? Cancel : Close;
  d->mButtonBox = new QDialogButtonBox( this );

  if ( buttonMask & Help )
    d->appendButton( Help, KStdGuiItem::help() );
  if ( buttonMask & Default )
    d->appendButton( Default, KStdGuiItem::defaults() );
  if ( buttonMask & User3 )
    d->appendButton( User3, KGuiItem() );
  if ( buttonMask & User2 )
    d->appendButton( User2, KGuiItem() );
  if ( buttonMask & User1 )
    d->appendButton( User1, KGuiItem() );
  if ( buttonMask & Ok )
    d->appendButton( Ok, KStdGuiItem::ok() );
  if ( buttonMask & Apply )
    d->appendButton( Apply, KStdGuiItem::apply() );
  if ( buttonMask & Try )
    d->appendButton( Try, i18n( "&Try" ) );
  if ( buttonMask & Cancel )
    d->appendButton( Cancel, KStdGuiItem::cancel() );
  if ( buttonMask & Close )
    d->appendButton( Close, KStdGuiItem::close() );
  if ( buttonMask & Yes )
    d->appendButton( Yes, KStdGuiItem::yes() );
  if ( buttonMask & No )
    d->appendButton( No, KStdGuiItem::no() );
  if ( buttonMask & Details ) {
    d->appendButton( Details, QString() );
    setDetails( false );
  }

  d->setupLayout();
}


void KDialog::setButtonsOrientation( Qt::Orientation orientation )
{
  if ( d->mButtonOrientation != orientation ) {
    d->mButtonOrientation = orientation;

    if ( d->mActionSeparator )
      d->mActionSeparator->setOrientation( d->mButtonOrientation );

    if ( d->mButtonOrientation == Qt::Vertical )
      enableLinkedHelp( false ); // 2000-06-18 Espen: No support for this yet.
  }
}

void KDialog::setEscapeButton( ButtonCode id )
{
  d->mEscapeButton = id;
}

void KDialog::setDefaultButton( ButtonCode defaultButton )
{
  if ( defaultButton != NoDefault ) {
    KPushButton *button = d->button( defaultButton );
    if ( button )
      d->setButtonFocus( button, true, false );
  }
}

KDialog::ButtonCode KDialog::defaultButton() const
{
  QHashIterator<int, KPushButton*> it( d->mButtonList );
  while ( it.hasNext() ) {
    it.next();
    if ( it.value()->isDefault() )
      return (ButtonCode)it.key();
  }

  return NoDefault;
}

void KDialog::setMainWidget( QWidget *widget )
{
  d->mMainWidget = widget;
  d->setupLayout();
}

QWidget *KDialog::mainWidget()
{
  return d->mMainWidget;
}

QSize KDialog::sizeHint() const
{
 return d->mMinSize.expandedTo( minimumSizeHint() ) + d->mIncSize;
}

QSize KDialog::minimumSizeHint() const
{
  const int m = marginHint();
  const int s = spacingHint();

  const QSize zeroByS( 0, s );

  QSize s1( 0, 0 );
  QSize s2( 0, 0 );

  //
  // Url help area
  //
  if ( d->mUrlHelp )
    s2 = d->mUrlHelp->minimumSize() + zeroByS;

  s1.rwidth() = qMax( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // User widget
  //
  if ( d->mMainWidget ) {
    s2 = d->mMainWidget->sizeHint() + zeroByS;
    s2 = s2.expandedTo( d->mMainWidget->minimumSize() );
    s2 = s2.expandedTo( d->mMainWidget->minimumSizeHint() );
    if ( s2.isEmpty() )
      s2 = QSize( 100, 100+s );
  } else
    s2 = QSize( 100, 100+s );

  s1.rwidth()  = qMax( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  if ( d->mDetailsWidget && d->mDetails ) {
    s2 = d->mDetailsWidget->sizeHint() + zeroByS;
    s2 = s2.expandedTo( d->mDetailsWidget->minimumSize() );
    s2 = s2.expandedTo( d->mDetailsWidget->minimumSizeHint() );
    s1.rwidth()  = qMax( s1.rwidth(), s2.rwidth() );
    s1.rheight() += s2.rheight();
  }

  //
  // Button separator
  //
  if ( d->mActionSeparator )
    s1.rheight() += d->mActionSeparator->minimumSize().height() + s;

  //
  // The button box
  //
  if ( d->mButtonBox ) {
    s2 = d->mButtonBox->minimumSizeHint();
    if ( d->mButtonOrientation == Qt::Horizontal ) {
      s1.rwidth()   = qMax( s1.rwidth(), s2.rwidth() );
      s1.rheight() += s2.rheight();
    } else {
      s1.rwidth() += s2.rwidth();
      s1.rheight() = qMax( s1.rheight(), s2.rheight() );
    }
  }

  //
  // Outer margings
  //
  s1.rheight() += 2*m;
  s1.rwidth()  += 2*m;

  return s1;
}

//
// Grab QDialogs keypresses if non-modal.
//
void KDialog::keyPressEvent( QKeyEvent *event )
{
  if ( event->modifiers() == 0 ) {
    if ( event->key() == Qt::Key_F1 ) {
      KPushButton *button = d->button( Help );

      if ( button ) {
        button->animateClick();
        event->accept();
        return;
      }
    }

    if ( event->key() == Qt::Key_Escape ) {
      KPushButton *button = d->button( d->mEscapeButton );

      if ( button ) {
        button->animateClick();
        event->accept();
        return;
      }

    }
  } else if ( event->key() == Qt::Key_F1 && event->modifiers() == Qt::ShiftModifier ) {
    QWhatsThis::enterWhatsThisMode();
    event->accept();
    return;
  } else if ( event->modifiers() == Qt::ControlModifier &&
            ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ) ) {
    // accept the dialog when Ctrl-Return is pressed
    KPushButton *button = d->button( Ok );

    if ( button ) {
      button->animateClick();
      event->accept();
      return;
    }
  }

  QDialog::keyPressEvent( event );
}

int KDialog::marginHint()
{
  return Private::mMarginSize;
}

int KDialog::spacingHint()
{
  return Private::mSpacingSize;
}

void KDialog::setCaption( const QString &_caption )
{
  QString caption = kapp ? kapp->makeStdCaption( _caption ) : _caption;
  setPlainCaption( caption );
}

void KDialog::setPlainCaption( const QString &caption )
{
  QDialog::setWindowTitle( caption );

#ifdef Q_WS_X11
  NETWinInfo info( QX11Info::display(), winId(), QX11Info::appRootWindow(), 0 );
  info.setName( caption.toUtf8().data() );
#endif
}

void KDialog::resizeLayout( QWidget *widget, int margin, int spacing ) //static
{
  if ( widget->layout() )
    resizeLayout( widget->layout(), margin, spacing );

  if ( widget->children().count() > 0 ) {
    QList<QObject*> list = widget->children();
    foreach ( QObject *object, list ) {
      if ( object->isWidgetType() )
        resizeLayout( (QWidget*)object, margin, spacing );
    }
  }
}

void KDialog::resizeLayout( QLayout *layout, int margin, int spacing ) //static
{
  QLayoutItem *child;
  int pos = 0;

  while ( (child = layout->itemAt( pos ) ) ) {
    if ( child->layout() )
      resizeLayout( child->layout(), margin, spacing );

    ++pos;
  }

  if ( layout->layout() ) {
    layout->layout()->setMargin( margin );
    layout->layout()->setSpacing( spacing );
  }
}

static QRect screenRect( QWidget *widget, int screen )
{
  QDesktopWidget *desktop = QApplication::desktop();
  KConfig gc( "kdeglobals", false, false );
  gc.setGroup( "Windows" );
  if ( desktop->isVirtualDesktop() &&
       gc.readEntry( "XineramaEnabled", true ) &&
       gc.readEntry( "XineramaPlacementEnabled", true ) ) {

    if ( screen < 0 || screen >= desktop->numScreens() ) {
      if ( screen == -1 )
        screen = desktop->primaryScreen();
      else if ( screen == -3 )
        screen = desktop->screenNumber( QCursor::pos() );
      else
        screen = desktop->screenNumber( widget );
    }

    return desktop->availableGeometry( screen );
  } else
    return desktop->geometry();
}

void KDialog::centerOnScreen( QWidget *widget, int screen )
{
  if ( !widget )
    return;

  QRect rect = screenRect( widget, screen );

  widget->move( rect.center().x() - widget->width() / 2,
                rect.center().y() - widget->height() / 2 );
}

bool KDialog::avoidArea( QWidget *widget, const QRect& area, int screen )
{
  if ( !widget )
    return false;

  QRect fg = widget->frameGeometry();
  if ( !fg.intersects( area ) )
    return true; // nothing to do.

  QRect scr = screenRect( widget, screen );
  QRect avoid( area ); // let's add some margin
  avoid.translate( -5, -5 );
  avoid.setRight( avoid.right() + 10 );
  avoid.setBottom( avoid.bottom() + 10 );

  if ( qMax( fg.top(), avoid.top() ) <= qMin( fg.bottom(), avoid.bottom() ) ) {
    // We need to move the widget up or down
    int spaceAbove = qMax( 0, avoid.top() - scr.top() );
    int spaceBelow = qMax( 0, scr.bottom() - avoid.bottom() );
    if ( spaceAbove > spaceBelow ) // where's the biggest side?
      if ( fg.height() <= spaceAbove ) // big enough?
        fg.setY( avoid.top() - fg.height() );
      else
        return false;
    else
      if ( fg.height() <= spaceBelow ) // big enough?
        fg.setY( avoid.bottom() );
      else
        return false;
  }

  if ( qMax( fg.left(), avoid.left() ) <= qMin( fg.right(), avoid.right() ) ) {
    // We need to move the widget left or right
    int spaceLeft = qMax( 0, avoid.left() - scr.left() );
    int spaceRight = qMax( 0, scr.right() - avoid.right() );
    if ( spaceLeft > spaceRight ) // where's the biggest side?
      if ( fg.width() <= spaceLeft ) // big enough?
        fg.setX( avoid.left() - fg.width() );
      else
        return false;
    else
      if ( fg.width() <= spaceRight ) // big enough?
        fg.setX( avoid.right() );
      else
        return false;
  }

  widget->move( fg.x(), fg.y() );

  return true;
}

void KDialog::showButtonSeparator( bool state )
{
  if ( state ) {
    if ( d->mActionSeparator )
      return;

     d->mActionSeparator = new KSeparator( this );
     d->mActionSeparator->setFocusPolicy( Qt::NoFocus );
     d->mActionSeparator->setOrientation( d->mButtonOrientation );
     d->mActionSeparator->show();
  } else {
    if ( !d->mActionSeparator )
      return;

    delete d->mActionSeparator;
    d->mActionSeparator = 0;
  }

  d->setupLayout();
}

void KDialog::setInitialSize( const QSize &size )
{
  d->mMinSize = size;
  adjustSize();
}

void KDialog::incrementInitialSize( const QSize &size )
{
  d->mIncSize = size;
  adjustSize();
}


void KDialog::enableButton( ButtonCode id, bool state )
{
  KPushButton *button = d->button( id );
  if ( button )
    button->setEnabled( state );
}

bool KDialog::isButtonEnabled( ButtonCode id ) const
{
  KPushButton *button = d->button( id );
  if ( button )
    return button->isEnabled();

  return false;
}

void KDialog::enableButtonOk( bool state )
{
  enableButton( Ok, state );
}

void KDialog::enableButtonApply( bool state )
{
  enableButton( Apply, state );
}

void KDialog::enableButtonCancel( bool state )
{
  enableButton( Cancel, state );
}

void KDialog::showButton( ButtonCode id, bool state )
{
  KPushButton *button = d->button( id );
  if ( button )
    state ? button->show() : button->hide();
}

void KDialog::setButtonGuiItem( ButtonCode id, const KGuiItem &item )
{
  KPushButton *button = d->button( id );
  if ( !button )
    return;

  button->setGuiItem( item );
}

void KDialog::setButtonMenu( ButtonCode id, QMenu *menu )
{
  KPushButton *button = d->button( id );
  if ( button )
    button->setMenu( menu );
}

void KDialog::setButtonText( ButtonCode id, const QString &text )
{
  if ( !d->mSettingDetails && (id == Details) ) {
    d->mDetailsButton = text;
    setDetails( d->mDetails );
    return;
  }

  KPushButton *button = d->button( id );
  if ( button )
    button->setText( text );
}

QString KDialog::buttonText( ButtonCode id ) const
{
  KPushButton *button = d->button( id );
  if ( button )
    return button->text();
  else
    return QString();
}

void KDialog::setButtonIcon( ButtonCode id, const QIcon &icon )
{
  KPushButton *button = d->button( id );
  if ( button )
    button->setIcon( icon );
}

QIcon KDialog::buttonIcon( ButtonCode id ) const
{
  KPushButton *button = d->button( id );
  if ( button )
    return button->icon();
  else
    return QIcon();
}

void KDialog::setButtonToolTip( ButtonCode id, const QString &text )
{
  KPushButton *button = d->button( id );
  if ( button ) {
    if ( text.isEmpty() )
      button->setToolTip( QString() );
    else
      button->setToolTip( text );
  }
}

QString KDialog::buttonToolTip( ButtonCode id ) const
{
  KPushButton *button = d->button( id );
  if ( button )
    return button->toolTip();
  else
    return QString();
}

void KDialog::setButtonWhatsThis( ButtonCode id, const QString &text )
{
  KPushButton *button = d->button( id );
  if ( button ) {
    if ( text.isEmpty() )
      button->setWhatsThis( QString() );
    else
      button->setWhatsThis( text );
  }
}

QString KDialog::buttonWhatsThis( ButtonCode id ) const
{
  KPushButton *button = d->button( id );
  if ( button )
    return button->whatsThis();
  else
    return QString();
}

void KDialog::setButtonFocus( ButtonCode id )
{
  KPushButton *button = d->button( id );
  if ( button )
    d->setButtonFocus( button, false, true );
}

void KDialog::setDetailsWidget( QWidget *detailsWidget )
{
  delete d->mDetailsWidget;
  d->mDetailsWidget = detailsWidget;

  if ( d->mDetailsWidget->parentWidget() != this )
    d->mDetailsWidget->setParent( this );

  d->mDetailsWidget->hide();
  d->setupLayout();

  if ( !d->mSettingDetails )
    setDetails( d->mDetails );
}

void KDialog::setDetails( bool showDetails )
{
  if ( d->mDetailsButton.isEmpty() )
    d->mDetailsButton = i18n( "&Details" );

  d->mSettingDetails = true;
  d->mDetails = showDetails;
  if ( d->mDetails ) {
    emit aboutToShowDetails();
    setButtonText( Details, d->mDetailsButton + " <<" );
    if ( d->mDetailsWidget ) {
      if ( layout() )
        layout()->setEnabled( false );

      adjustSize();

      d->mDetailsWidget->show();

      if ( layout() ) {
        layout()->activate();
        layout()->setEnabled( true );
      }
    }
  } else {
    setButtonText( Details, d->mDetailsButton + " >>" );
    if ( d->mDetailsWidget )
      d->mDetailsWidget->hide();

    if ( layout() )
      layout()->activate();

    adjustSize();
  }

  d->mSettingDetails = false;
}

void KDialog::delayedDestruct()
{
  if ( isVisible() )
    hide();

  deleteLater();
}


void KDialog::slotButtonClicked( int button )
{
  emit buttonClicked( static_cast<KDialog::ButtonCode>(button) );

  switch( button ) {
    case Ok:
      emit okClicked();
      accept();
      break;
    case Apply:
        emit applyClicked();
      break;
    case Try:
      emit tryClicked();
      break;
    case User3:
      emit user3Clicked();
      break;
    case User2:
        emit user2Clicked();
      break;
    case User1:
        emit user1Clicked();
      break;
    case Yes:
      emit yesClicked();
      done( Yes );
      break;
    case No:
        emit noClicked();
      done( No );
      break;
    case Cancel:
      emit cancelClicked();
      reject();
      break;
    case Close:
      emit closeClicked();
      close();
      break;
    case Help:
      emit helpClicked();
      if ( !d->mAnchor.isEmpty() || !d->mHelpApp.isEmpty() )
        KToolInvocation::invokeHelp( d->mAnchor, d->mHelpApp );
      break;
    case Default:
      emit defaultClicked();
      break;
    case Details:
      setDetails( !d->mDetails );
      break;
  }
}

void KDialog::enableLinkedHelp( bool state )
{
  if ( state ) {
    if ( d->mUrlHelp )
      return;

    d->mUrlHelp = new KUrlLabel( this );
    d->mUrlHelp->setText( helpLinkText() );
    d->mUrlHelp->setFloatEnabled( true );
    d->mUrlHelp->setUnderline( true );
    d->mUrlHelp->setMinimumHeight( fontMetrics().height() + marginHint() );
    connect( d->mUrlHelp, SIGNAL( leftClickedUrl( const QString& ) ),
             SLOT( helpClickedSlot( const QString& ) ) );

    d->mUrlHelp->show();
  } else {
    if ( !d->mUrlHelp )
      return;

    delete d->mUrlHelp;
    d->mUrlHelp = 0;
  }

  d->setupLayout();
}


void KDialog::setHelp( const QString &anchor, const QString &appname )
{
  d->mAnchor  = anchor;
  d->mHelpApp = appname;
}


void KDialog::setHelpLinkText( const QString &text )
{
  d->mHelpLinkText = text;
  if ( d->mUrlHelp )
    d->mUrlHelp->setText( helpLinkText() );
}

QString KDialog::helpLinkText() const
{
  return ( d->mHelpLinkText.isNull() ? i18n( "Get help..." ) : d->mHelpLinkText );
}

void KDialog::updateGeometry()
{
  if ( d->mTopLayout ) {
    d->mTopLayout->setMargin( marginHint() );
    d->mTopLayout->setSpacing( spacingHint() );
  }
}

void KDialog::hideEvent( QHideEvent *event )
{
  emit hidden();

  if ( !event->spontaneous() )
    emit finished();
}

void KDialog::closeEvent( QCloseEvent *event )
{
  KPushButton *button = d->button( d->mEscapeButton );
  if ( button && !isHidden() )
    button->animateClick();
  else
    QDialog::closeEvent( event );
}

void KDialog::restoreDialogSize( KConfigBase *cfg )
{
  int width, height;
  int scnum = QApplication::desktop()->screenNumber( parentWidget() );
  QRect desk = QApplication::desktop()->screenGeometry( scnum );

  width = sizeHint().width();
  height = sizeHint().height();

  width = cfg->readEntry( QString::fromLatin1( "Width %1" ).arg( desk.width() ), width );
  height = cfg->readEntry( QString::fromLatin1( "Height %1" ).arg( desk.height() ), height );

  resize( width, height );
}

void KDialog::saveDialogSize( KConfigBase* config, KConfigBase::WriteConfigFlags options ) const
{
   int scnum = QApplication::desktop()->screenNumber( parentWidget() );
   QRect desk = QApplication::desktop()->screenGeometry( scnum );

   QSize sizeToSave = size();

   config->writeEntry( QString::fromLatin1("Width %1").arg( desk.width() ), sizeToSave.width(), options );
   config->writeEntry( QString::fromLatin1("Height %1").arg( desk.height() ), sizeToSave.height(), options );
}


class KDialogQueue::Private
{
  public:
    QList< QPointer<QDialog> > queue;
    bool busy;
};

static KStaticDeleter<KDialogQueue> ksdkdq;

KDialogQueue *KDialogQueue::_self = 0;

KDialogQueue* KDialogQueue::self()
{
  if ( !_self )
    _self = ksdkdq.setObject( _self, new KDialogQueue );

  return _self;
}

KDialogQueue::KDialogQueue()
  : d( new Private )
{
  d->busy = false;
}

KDialogQueue::~KDialogQueue()
{
  delete d;
  _self = 0;
}

// static
void KDialogQueue::queueDialog( QDialog *dialog )
{
  KDialogQueue *_this = self();
  _this->d->queue.append( dialog );

  QTimer::singleShot( 0, _this, SLOT( slotShowQueuedDialog() ) );
}

void KDialogQueue::slotShowQueuedDialog()
{
  if ( d->busy )
    return;

  QDialog *dialog;
  do {
    if ( d->queue.isEmpty() )
      return;
    dialog = d->queue.first();
    d->queue.pop_front();
  } while( !dialog );

  d->busy = true;
  dialog->exec();
  d->busy = false;
  delete dialog;

  if ( !d->queue.isEmpty() )
    QTimer::singleShot( 20, this, SLOT( slotShowQueuedDialog() ) );
  else
    ksdkdq.destructObject(); // Suicide.
}

#include "kdialog.moc"
