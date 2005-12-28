/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

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


#include "kactionselector.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h> // for spacingHint()
#include <kdebug.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qevent.h>
#include <QListWidget>

class KActionSelectorPrivate {
  public:
  QListWidget *availableListWidget, *selectedListWidget;
  QToolButton *btnAdd, *btnRemove, *btnUp, *btnDown;
  QLabel *lAvailable, *lSelected;
  bool moveOnDoubleClick, keyboardEnabled;
  KActionSelector::ButtonIconSize iconSize;
  QString addIcon, removeIcon, upIcon, downIcon;
  KActionSelector::InsertionPolicy availableInsertionPolicy, selectedInsertionPolicy;
  bool showUpDownButtons;
};

//BEGIN Constructor/destructor

KActionSelector::KActionSelector( QWidget *parent )
  : QWidget( parent )
{
  d = new KActionSelectorPrivate();
  d->moveOnDoubleClick = true;
  d->keyboardEnabled = true;
  d->iconSize = SmallIcon;
  d->addIcon = QApplication::isRightToLeft()? "back" : "forward";
  d->removeIcon = QApplication::isRightToLeft()? "forward" : "back";
  d->upIcon = "up";
  d->downIcon = "down";
  d->availableInsertionPolicy = Sorted;
  d->selectedInsertionPolicy = BelowCurrent;
  d->showUpDownButtons = true;

  //int isz = IconSize( KIcon::Small );

  QHBoxLayout *lo = new QHBoxLayout( this );
  lo->setSpacing( KDialog::spacingHint() );

  QVBoxLayout *loAv = new QVBoxLayout( lo );
  d->lAvailable = new QLabel( i18n("&Available:"), this );
  loAv->addWidget( d->lAvailable );
  d->availableListWidget = new QListWidget( this );
  loAv->addWidget( d->availableListWidget );
  d->lAvailable->setBuddy( d->availableListWidget );

  QVBoxLayout *loHBtns = new QVBoxLayout( lo );
  loHBtns->addStretch( 1 );
  d->btnAdd = new QToolButton( this );
  loHBtns->addWidget( d->btnAdd );
  d->btnRemove = new QToolButton( this );
  loHBtns->addWidget( d->btnRemove );
  loHBtns->addStretch( 1 );

  QVBoxLayout *loS = new QVBoxLayout( lo );
  d->lSelected = new QLabel( i18n("&Selected:"), this );
  loS->addWidget( d->lSelected );
  d->selectedListWidget = new QListWidget( this );
  loS->addWidget( d->selectedListWidget );
  d->lSelected->setBuddy( d->selectedListWidget );

  QVBoxLayout *loVBtns = new QVBoxLayout( lo );
  loVBtns->addStretch( 1 );
  d->btnUp = new QToolButton( this );
  d->btnUp->setAutoRepeat( true );
  loVBtns->addWidget( d->btnUp );
  d->btnDown = new QToolButton( this );
  d->btnDown->setAutoRepeat( true );
  loVBtns->addWidget( d->btnDown );
  loVBtns->addStretch( 1 );

  loadIcons();

  connect( d->btnAdd, SIGNAL(clicked()), this, SLOT(buttonAddClicked()) );
  connect( d->btnRemove, SIGNAL(clicked()), this, SLOT(buttonRemoveClicked()) );
  connect( d->btnUp, SIGNAL(clicked()), this, SLOT(buttonUpClicked()) );
  connect( d->btnDown, SIGNAL(clicked()), this, SLOT(buttonDownClicked()) );
  connect( d->availableListWidget, SIGNAL(doubleClicked(QListWidgetItem*)),
           this, SLOT(itemDoubleClicked(QListWidgetItem*)) );
  connect( d->selectedListWidget, SIGNAL(doubleClicked(QListWidgetItem*)),
           this, SLOT(itemDoubleClicked(QListWidgetItem*)) );
  connect( d->availableListWidget, SIGNAL(currentChanged(QListWidgetItem*)),
           this, SLOT(slotCurrentChanged(QListWidgetItem *)) );
  connect( d->selectedListWidget, SIGNAL(currentChanged(QListWidgetItem*)),
           this, SLOT(slotCurrentChanged(QListWidgetItem *)) );

  d->availableListWidget->installEventFilter( this );
  d->selectedListWidget->installEventFilter( this );
}

KActionSelector::~KActionSelector()
{
  delete d;
}

//END Constructor/destroctor

//BEGIN Public Methods

QListWidget *KActionSelector::availableListWidget() const
{
  return d->availableListWidget;
}

QListWidget *KActionSelector::selectedListWidget() const
{
  return d->selectedListWidget;
}

void KActionSelector::setButtonIcon( const QString &icon, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->addIcon = icon;
    d->btnAdd->setIcon( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonRemove:
    d->removeIcon = icon;
    d->btnRemove->setIcon( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonUp:
    d->upIcon = icon;
    d->btnUp->setIcon( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonDown:
    d->downIcon = icon;
    d->btnDown->setIcon( SmallIconSet( icon, d->iconSize ) );
    break;
    default:
    kdDebug(13001)<<"KActionSelector::setButtonIcon: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonIconSet( const QIcon &iconset, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setIcon( iconset );
    break;
    case ButtonRemove:
    d->btnRemove->setIcon( iconset );
    break;
    case ButtonUp:
    d->btnUp->setIcon( iconset );
    break;
    case ButtonDown:
    d->btnDown->setIcon( iconset );
    break;
    default:
    kdDebug(13001)<<"KActionSelector::setButtonIconSet: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonTooltip( const QString &tip, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setTextLabel( tip );
    break;
    case ButtonRemove:
    d->btnRemove->setTextLabel( tip );
    break;
    case ButtonUp:
    d->btnUp->setTextLabel( tip );
    break;
    case ButtonDown:
    d->btnDown->setTextLabel( tip );
    break;
    default:
    kdDebug(13001)<<"KActionSelector::setButtonToolTip: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonWhatsThis( const QString &text, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setWhatsThis(text );
    break;
    case ButtonRemove:
    d->btnRemove->setWhatsThis(text );
    break;
    case ButtonUp:
    d->btnUp->setWhatsThis(text );
    break;
    case ButtonDown:
    d->btnDown->setWhatsThis(text );
    break;
    default:
    kdDebug(13001)<<"KActionSelector::setButtonWhatsThis: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonsEnabled()
{
  d->btnAdd->setEnabled( d->availableListWidget->currentRow() > -1 );
  d->btnRemove->setEnabled( d->selectedListWidget->currentRow() > -1 );
  d->btnUp->setEnabled( d->selectedListWidget->currentRow() > 0 );
  d->btnDown->setEnabled( d->selectedListWidget->currentRow() > -1 &&
                          d->selectedListWidget->currentRow() < d->selectedListWidget->count() - 1 );
}

//END Public Methods

//BEGIN Properties

bool KActionSelector::moveOnDoubleClick() const
{
  return d->moveOnDoubleClick;
}

void KActionSelector::setMoveOnDoubleClick( bool b )
{
  d->moveOnDoubleClick = b;
}

bool KActionSelector::keyboardEnabled() const
{
  return d->keyboardEnabled;
}

void KActionSelector::setKeyboardEnabled( bool b )
{
  d->keyboardEnabled = b;
}

QString KActionSelector::availableLabel() const
{
  return d->lAvailable->text();
}

void KActionSelector::setAvailableLabel( const QString &text )
{
  d->lAvailable->setText( text );
}

QString KActionSelector::selectedLabel() const
{
  return d->lSelected->text();
}

void KActionSelector::setSelectedLabel( const QString &text )
{
  d->lSelected->setText( text );
}

KActionSelector::ButtonIconSize KActionSelector::buttonIconSize() const
{
  return d->iconSize;
}

void KActionSelector::setButtonIconSize( ButtonIconSize size )
{
  d->iconSize = size;
  // reload icons
  loadIcons();
}

KActionSelector::InsertionPolicy KActionSelector::availableInsertionPolicy() const
{
  return d->availableInsertionPolicy;
}

void KActionSelector::setAvailableInsertionPolicy( InsertionPolicy p )
{
  d->availableInsertionPolicy = p;
}

KActionSelector::InsertionPolicy KActionSelector::selectedInsertionPolicy() const
{
  return d->selectedInsertionPolicy;
}

void KActionSelector::setSelectedInsertionPolicy( InsertionPolicy p )
{
  d->selectedInsertionPolicy = p;
}

bool KActionSelector::showUpDownButtons() const
{
  return d->showUpDownButtons;
}

void KActionSelector::setShowUpDownButtons( bool show )
{
  d->showUpDownButtons = show;
  if ( show )
  {
    d->btnUp->show();
    d->btnDown->show();
  }
  else
  {
    d->btnUp->hide();
    d->btnDown->hide();
  }
}

//END Properties

//BEGIN Public Slots

void KActionSelector::polish()
{
  setButtonsEnabled();
}

//END Public Slots

//BEGIN Protected
void KActionSelector::keyPressEvent( QKeyEvent *e )
{
  if ( ! d->keyboardEnabled ) return;
  if ( (e->state() & Qt::ControlModifier) )
  {
    switch ( e->key() )
    {
      case Qt::Key_Right:
      buttonAddClicked();
      break;
      case Qt::Key_Left:
      buttonRemoveClicked();
      break;
      case Qt::Key_Up:
      buttonUpClicked();
      break;
      case Qt::Key_Down:
      buttonDownClicked();
      break;
      default:
      e->ignore();
      return;
    }
  }
}

bool KActionSelector::eventFilter( QObject *o, QEvent *e )
{
  if ( d->keyboardEnabled && e->type() == QEvent::KeyPress )
  {
    if  ( (((QKeyEvent*)e)->state() & Qt::ControlModifier) )
    {
      switch ( ((QKeyEvent*)e)->key() )
      {
        case Qt::Key_Right:
        buttonAddClicked();
        break;
        case Qt::Key_Left:
        buttonRemoveClicked();
        break;
        case Qt::Key_Up:
        buttonUpClicked();
        break;
        case Qt::Key_Down:
        buttonDownClicked();
        break;
        default:
        return QWidget::eventFilter( o, e );
        break;
      }
      return true;
    }
    else if ( QListWidget *lb = qobject_cast<QListWidget*>(o) )
    {
      switch ( ((QKeyEvent*)e)->key() )
      {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        int index = lb->currentRow();
        if ( index < 0 ) break;
        moveItem( lb->item( index ) );
        return true;
      }
    }
  }
  return QWidget::eventFilter( o, e );
}

//END Protected

//BEGIN Private Slots

void KActionSelector::buttonAddClicked()
{
  // move all selected items from available to selected listbox
  QList<QListWidgetItem *> list = d->availableListWidget->selectedItems();
  foreach (QListWidgetItem* item, list) {
    d->availableListWidget->takeItem( d->availableListWidget->row( item ) );
    d->selectedListWidget->insertItem( insertionIndex( d->selectedListWidget, d->selectedInsertionPolicy ), item );
    d->selectedListWidget->setCurrentItem( item );
    emit added( item );
  }
  if ( d->selectedInsertionPolicy == Sorted )
    d->selectedListWidget->sortItems();
  d->selectedListWidget->setFocus();
}

void KActionSelector::buttonRemoveClicked()
{
  // move all selected items from selected to available listbox
  QList<QListWidgetItem *> list = d->selectedListWidget->selectedItems();
  foreach (QListWidgetItem* item, list) {
    d->selectedListWidget->takeItem( d->selectedListWidget->row( item ) );
    d->availableListWidget->insertItem( insertionIndex( d->availableListWidget, d->availableInsertionPolicy ), item );
    d->availableListWidget->setCurrentItem( item );
    emit removed( item );
  }
  if ( d->availableInsertionPolicy == Sorted )
    d->availableListWidget->sortItems();
  d->availableListWidget->setFocus();
}

void KActionSelector::buttonUpClicked()
{
  int c = d->selectedListWidget->currentRow();
  if ( c < 1 ) return;
  QListWidgetItem *item = d->selectedListWidget->item( c );
  d->selectedListWidget->takeItem( c );
  d->selectedListWidget->insertItem( c-1, item );
  d->selectedListWidget->setCurrentItem( item );
  emit movedUp( item );
}

void KActionSelector::buttonDownClicked()
{
  int c = d->selectedListWidget->currentRow();
  if ( c < 0 || c == d->selectedListWidget->count() - 1 ) return;
  QListWidgetItem *item = d->selectedListWidget->item( c );
  d->selectedListWidget->takeItem( c );
  d->selectedListWidget->insertItem( c+1, item );
  d->selectedListWidget->setCurrentItem( item );
  emit movedDown( item );
}

void KActionSelector::itemDoubleClicked( QListWidgetItem *item )
{
  if ( d->moveOnDoubleClick )
    moveItem( item );
}

//END Private Slots

//BEGIN Private Methods

void KActionSelector::loadIcons()
{
  d->btnAdd->setIcon( SmallIconSet( d->addIcon, d->iconSize ) );
  d->btnRemove->setIcon( SmallIconSet( d->removeIcon, d->iconSize ) );
  d->btnUp->setIcon( SmallIconSet( d->upIcon, d->iconSize ) );
  d->btnDown->setIcon( SmallIconSet( d->downIcon, d->iconSize ) );
}

void KActionSelector::moveItem( QListWidgetItem *item )
{
  QListWidget *lbFrom = item->listWidget();
  QListWidget *lbTo;
  if ( lbFrom == d->availableListWidget )
    lbTo = d->selectedListWidget;
  else if ( lbFrom == d->selectedListWidget )
    lbTo = d->availableListWidget;
  else  //?! somewhat unlikely...
    return;

  InsertionPolicy p = ( lbTo == d->availableListWidget ) ?
                        d->availableInsertionPolicy : d->selectedInsertionPolicy;

  lbFrom->takeItem( lbFrom->row( item ) );
  lbTo->insertItem( insertionIndex( lbTo, p ), item );
  lbTo->setFocus();
  lbTo->setCurrentItem( item );

  if ( p == Sorted )
    lbTo->sortItems();
  if ( lbTo == d->selectedListWidget )
    emit added( item );
  else
    emit removed( item );
}

int KActionSelector::insertionIndex( QListWidget *lb, InsertionPolicy policy )
{
  int index;
  switch ( policy )
  {
    case BelowCurrent:
    index = lb->currentRow();
    if ( index > -1 ) index += 1;
    break;
    case AtTop:
    index = 0;
    break;
    default:
    index = -1;
  }
  return index;
}

//END Private Methods
#include "kactionselector.moc"
