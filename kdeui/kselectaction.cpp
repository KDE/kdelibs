/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "kselectaction.h"

#include <QActionEvent>
#include <QEvent>
#include <QToolButton>
#include <QToolBar>

#include <kdebug.h>

#include "kcombobox.h"
#include "kmenu.h"

// BEGIN KSelectAction
class KSelectAction::KSelectActionPrivate
{
public:
  KSelectActionPrivate()
  {
    m_edit = false;
    m_menuAccelsEnabled = true;
    m_comboWidth = -1;
    m_maxComboViewCount = -1;

    m_toolBarMode = KSelectAction::ComboBoxMode;
    m_toolButtonPopupMode = QToolButton::DelayedPopup;

    m_actionGroup = new QActionGroup(0L);
  }

  ~KSelectActionPrivate()
  {
    delete m_actionGroup;
  }

  bool m_edit, m_menuAccelsEnabled;
  int m_comboWidth;
  int m_maxComboViewCount;

  KSelectAction::ToolBarMode m_toolBarMode;
  QToolButton::ToolButtonPopupMode m_toolButtonPopupMode;

  QActionGroup* m_actionGroup;

  QList<QToolButton*> m_buttons;
  QList<KComboBox*> m_comboBoxes;

  QString makeMenuText( const QString &_text )
  {
      if ( m_menuAccelsEnabled )
        return _text;
      QString text = _text;
      int i = 0;
      while ( i < text.length() ) {
          if ( text[ i ] == '&' ) {
              text.insert( i, '&' );
              i += 2;
          }
          else
              ++i;
      }
      return text;
  }
};

KSelectAction::KSelectAction( KActionCollection * parent, const QString& name )
  : KAction(parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString & text, KActionCollection * parent, const QString& name )
  : KAction(text, parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const KIcon & icon, const QString & text, KActionCollection * parent, const QString& name )
  : KAction(icon, text, parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString & icon, const QString & text, KActionCollection * parent, const QString& name )
  : KAction(KIcon(icon), text, parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const KShortcut& cut,
                              KActionCollection* parent, const QString& name )
  : KAction( text, parent, name )
  , d(new KSelectActionPrivate())
{ 
  setShortcut(cut);
  init();
}

KSelectAction::KSelectAction( const QString& text, const KShortcut& cut,
                              const QObject* receiver, const char* slot,
                              KActionCollection* parent, const QString& name )
  : KAction( text, parent, name )
  , d(new KSelectActionPrivate())
{
  connect(this, SIGNAL(triggered(bool)), receiver, slot);
  setShortcut(cut);
  init();
}

KSelectAction::KSelectAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const QString& name )
  : KAction( text, parent, name )
  , d(new KSelectActionPrivate())
{
  setShortcut(cut);
  QAction::setIcon(pix);
  init();
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const QString& name )
  : KAction( text, parent, name )
  , d(new KSelectActionPrivate())
{
  setShortcut(cut);
  setIcon(KIcon(pix));
  init();
}

KSelectAction::KSelectAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const QString& name )
  : KAction( text, parent, name )
  , d(new KSelectActionPrivate())
{
  setShortcut(cut);
  QAction::setIcon(pix);
  connect(this, SIGNAL(triggered(bool)), receiver, slot);
  init();
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const QString& name )
  : KAction( text, parent, name )
  , d(new KSelectActionPrivate())
{
  setShortcut(cut);
  setIcon(KIcon(pix));
  connect(this, SIGNAL(triggered(bool)), receiver, slot);
  init();
}

KSelectAction::~KSelectAction()
{
  delete d;
  delete menu();
}

void KSelectAction::init()
{
  connect(selectableActionGroup(), SIGNAL(triggered(QAction*)), SLOT(actionTriggered(QAction*)));
  setMenu(new KMenu());
}

QActionGroup * KSelectAction::selectableActionGroup( ) const
{
  return d->m_actionGroup;
}

QList<QAction*> KSelectAction::actions( ) const
{
  return selectableActionGroup()->actions();
}

QAction* KSelectAction::currentAction() const
{
  return d->m_actionGroup->checkedAction();
}

int KSelectAction::currentItem() const
{
  return selectableActionGroup()->actions().indexOf(currentAction());
}

QString KSelectAction::currentText( ) const
{
  if (QAction* a = currentAction())
    return a->text();

  return QString::null;
}

void KSelectAction::setCurrentAction(QAction* action)
{
  kDebug () << "KSelectAction::setCurrentAction(" << action << ")" << endl;
  action->setChecked(true);
}

bool KSelectAction::setCurrentItem( int index )
{
  kDebug () << "KSelectAction::setCurrentIndex(" << index << ")" << endl;
  if (QAction* a = action(index)) {
    setCurrentAction(a);
    return true;
  }

  kDebug () << "\tdoing the deselect" << endl;
  if (selectableActionGroup()->checkedAction())
    selectableActionGroup()->checkedAction()->setChecked(false);

  return false;
}

QAction * KSelectAction::action( int index ) const
{
  if (index >= 0 && index < selectableActionGroup()->actions().count())
    return selectableActionGroup()->actions().at(index);

  return 0L;
}

QAction * KSelectAction::action( const QString & text, Qt::CaseSensitivity cs ) const
{
  QString compare;
  if (cs == Qt::CaseSensitive)
    compare = text;
  else
    compare = text.toLower();

  foreach (QAction* action, selectableActionGroup()->actions()) {
    if (cs == Qt::CaseSensitive) {
      if (action->text() == compare) {
        return action;
      }

    } else if (cs == Qt::CaseInsensitive) {
      if (action->text().toLower() == compare) {
        return action;
      }
    }
  }

  return 0L;
}

bool KSelectAction::setCurrentAction( const QString & text, Qt::CaseSensitivity cs)
{
  kDebug () << "KSelectAction::setCurrentAction(" << text << ",cs=" << cs << ")" << endl;
  if (QAction* a = action(text, cs)) {
    a->setChecked(true);
    return true;
  }

  kDebug () << "\tfailed" << endl;
  return false;
}

void KSelectAction::setComboWidth( int width )
{
  if ( width < 0 )
    return;

  d->m_comboWidth = width;

  foreach (KComboBox* box, d->m_comboBoxes)
    box->setMaximumWidth(d->m_comboWidth);

  emit changed();
}

void KSelectAction::setMaxComboViewCount( int n )
{
  d->m_maxComboViewCount = n;

  foreach (KComboBox* box, d->m_comboBoxes)
    if ( d->m_maxComboViewCount != -1 )
      box->setMaxVisibleItems(d->m_maxComboViewCount);
    else
      // hardcoded qt default
      box->setMaxVisibleItems(10);

  emit changed();
}

void KSelectAction::addAction(QAction* action)
{
  kDebug () << "KSelectAction::addAction(" << action << ")" << endl;

  action->setActionGroup(selectableActionGroup());
  action->setCheckable( true );

  // Keep in sync with createToolBarWidget()
  foreach (QToolButton* button, d->m_buttons)
    button->menu()->addAction(action);

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->addAction(action);

  menu()->addAction(action);
}

QAction* KSelectAction::addAction(const QString& text)
{
  QAction* newAction = new QAction(text, selectableActionGroup());

  if (!d->m_menuAccelsEnabled) {
    newAction->setText(text);
    newAction->setShortcut(QKeySequence());
  }

  addAction(newAction);
  return newAction;
}

QAction* KSelectAction::addAction(const QIcon& icon, const QString& text)
{
  QAction* newAction = new QAction(icon, text, selectableActionGroup());

  if (!d->m_menuAccelsEnabled) {
    newAction->setText(text);
    newAction->setShortcut(QKeySequence());
  }

  addAction(newAction);
  return newAction;
}

QAction* KSelectAction::removeAction(QAction* action)
{
  kDebug () << "KSelectAction::removeAction(" << action << ")" << endl;
  int index = selectableActionGroup()->actions().indexOf(action);
  kDebug () << "\tindex=" << index << endl;

  action->setActionGroup(0L);

  foreach (QToolButton* button, d->m_buttons)
    button->menu()->removeAction(action);

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->removeAction(action);

  menu()->removeAction(action);

  return action;
}

void KSelectAction::actionTriggered(QAction* action)
{
  // cache values so we don't need access to members in the action
  // after we've done an emit()
  QString text = action->text();
  int index = selectableActionGroup()->actions().indexOf(action);
  kDebug () << "KSelectAction::actionTriggered(" << action << ") text=" << text
            << " index=" << index  << " emitting triggered()" << endl;

  emit triggered(action);
  emit triggered(index);
  emit triggered(text);
}

QStringList KSelectAction::items() const
{
  QStringList ret;

  foreach (QAction* action, d->m_actionGroup->actions())
    ret << action->text();

  return ret;
}

void KSelectAction::changeItem( int index, const QString& text )
{
  if ( index < 0 || index >= actions().count() )
  {
    kWarning() << "KSelectAction::changeItem Index out of scope" << endl;
    return;
  }

  actions()[index]->setText( d->makeMenuText( text ) );
}

void KSelectAction::setItems( const QStringList &lst )
{
  kDebug () << "KSelectAction::setItems(" << lst << ")" << endl;

  clear();

  foreach (const QString& string, lst) {
    if ( !string.isEmpty() ) {
      KAction* action = new KAction(string, parentCollection(), 0);

      if (!d->m_menuAccelsEnabled) {
        action->setText(string);
        action->setShortcut(QKeySequence());
      }

      action->setShortcutConfigurable(false);

      addAction(action);

    } else {
      QAction* action = new QAction(this);
      action->setSeparator(true);
      addAction(action);
    }
  }

  // Disable if empty and not editable
  setEnabled( lst.count() > 0 || d->m_edit );
}

int KSelectAction::comboWidth() const
{
  return d->m_comboWidth;
}

void KSelectAction::clear()
{
  kDebug () << "KSelectAction::clear()" << endl;

  // we need to delete the actions later since we may get a call to clear()
  // from a method called due to a triggered(...) signal
  foreach (QAction* action, d->m_actionGroup->actions())
  {
    // deleteLater() only removes us from the actions() list (among
    // other things) on the next entry into the event loop.  Until then,
    // e.g. action() and setCurrentItem() will be working on items
    // that are supposed to have been deleted.  So detach the action to
    // prevent this from happening.
    removeAction (action);

    action->deleteLater();
  }
}

void KSelectAction::removeAllActions( )
{
  while (d->m_actionGroup->actions().count())
    removeAction(d->m_actionGroup->actions().first());
}

void KSelectAction::setEditable( bool edit )
{
  d->m_edit = edit;

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->setEditable(edit);

  emit changed();
}

bool KSelectAction::isEditable() const
{
  return d->m_edit;
}

KSelectAction::ToolBarMode KSelectAction::toolBarMode() const
{
  return d->m_toolBarMode;
}

void KSelectAction::setToolBarMode( ToolBarMode mode )
{
  d->m_toolBarMode = mode;
}

QToolButton::ToolButtonPopupMode KSelectAction::toolButtonPopupMode( ) const
{
  return d->m_toolButtonPopupMode;
}

void KSelectAction::setToolButtonPopupMode( QToolButton::ToolButtonPopupMode mode )
{
  d->m_toolButtonPopupMode = mode;
}

void KSelectAction::comboBoxDeleted(QObject* object)
{
  foreach (KComboBox* comboBox, d->m_comboBoxes)
    if (object == comboBox) {
      d->m_comboBoxes.removeAll(static_cast<KComboBox*>(object));
      break;
    }
}

void KSelectAction::comboBoxCurrentIndexChanged(int index)
{
  kDebug () << "KSelectAction::comboBoxCurrentIndexChanged(" << index << ")" << endl;

  QAction *a = action(index);
  kDebug () << "\ta=" << a << endl;
  if (a) {
    setCurrentAction(a);
    emit actionTriggered(a);
  } else {
    if (selectableActionGroup()->checkedAction())
      selectableActionGroup()->checkedAction()->setChecked(false);
  }
}

void KSelectAction::setMenuAccelsEnabled( bool b )
{
  d->m_menuAccelsEnabled = b;
}

bool KSelectAction::menuAccelsEnabled() const
{
  return d->m_menuAccelsEnabled;
}

QWidget * KSelectAction::createWidget( QWidget * parent )
{
  QToolBar *toolBar = qobject_cast<QToolBar *>(parent);
  if (!toolBar)
    return 0;
  switch (toolBarMode()) {
    case MenuMode: {
      QToolButton* button = new QToolButton(toolBar);
      button->setAutoRaise(true);
      button->setFocusPolicy(Qt::NoFocus);
      button->setIconSize(toolBar->iconSize());
      button->setToolButtonStyle(toolBar->toolButtonStyle());
      QObject::connect(toolBar, SIGNAL(iconSizeChanged(const QSize&)),
                       button, SLOT(setIconSize(const QSize&)));
      QObject::connect(toolBar, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                       button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
      button->setDefaultAction(this);
      QObject::connect(button, SIGNAL(triggered(QAction*)), toolBar, SIGNAL(actionTriggered(QAction*)));

      button->setPopupMode(toolButtonPopupMode());

      button->setMenu(menu());
      d->m_buttons.append(button);
      return button;
    }

    case ComboBoxMode: {
      KComboBox* comboBox = new KComboBox(toolBar);
      comboBox->installEventFilter (this);

      if ( d->m_maxComboViewCount != -1 )
        comboBox->setMaxVisibleItems( d->m_maxComboViewCount );

      if ( d->m_comboWidth > 0 )
        comboBox->setMaximumWidth( d->m_comboWidth );

      comboBox->setEditable(isEditable());

      foreach (QAction* action, selectableActionGroup()->actions())
        comboBox->addAction(action);

      connect(comboBox, SIGNAL(destroyed(QObject*)), SLOT(comboBoxDeleted(QObject*)));
      connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(comboBoxCurrentIndexChanged(int)));
      d->m_comboBoxes.append(comboBox);

      return comboBox;
    }
  }

  return 0L;
}

// QAction::setText("Hi") and then KPopupAccelManager exec'ing, causes
// QAction::text() to return "&Hi" :(  Comboboxes don't have accels and
// display ampersands literally.
static QString DropAmpersands (const QString &text)
{
  QString ret = text;
  ret.remove ('&');
  return ret;
}

// KSelectAction::eventFilter() is called before action->setChecked()
// invokes the signal to update QActionGroup so KSelectAction::currentItem()
// returns an old value.
static int TrueCurrentItem (KSelectAction *sa)
{
  QAction *curAction = sa->currentAction ();
  kDebug () << "\tTrueCurrentItem(" << sa << ") curAction=" << curAction << endl;

  foreach (QAction *action, sa->actions ())
  {
    if (action->isChecked ())
    {
       kDebug () << "\t\taction " << action << " (text=" << action->text () << ") isChecked" << endl;
       if (action != curAction)
       {
         kDebug () << "\t\t\tmust be newly selected one" << endl;
         return sa->actions ().indexOf (action);
       }
    }
  }

  kDebug () << "\t\tcurrent action still selected? " << curAction->isChecked () << endl;
  return curAction->isChecked () ? sa->actions ().indexOf (curAction) : -1;
}


bool KSelectAction::eventFilter (QObject *watched, QEvent *event)
{
  KComboBox *comboBox = qobject_cast <KComboBox *> (watched);
  if (!comboBox)
    return false/*propagate*/;

  bool eatEvent = false;

  comboBox->blockSignals (true);

  if (event->type () == QEvent::ActionAdded)
  {
    QActionEvent * const e = static_cast <QActionEvent *> (event);
    
    const int index = e->before () ?
      comboBox->findData ((int) e->before ()) :
      comboBox->count ();
    const int newItem = ::TrueCurrentItem (this);
    kDebug () << "KSelectAction::eventFilter(ActionAdded)"
              << "    comboBox: ptr=" << comboBox
              << " currentItem=" << comboBox->currentIndex ()
              << " index=" << index
              << "    action new: e->before=" << e->before ()
              << " ptr=" << e->action ()
              << " icon=" << e->action ()->icon ()
              << " text=" << e->action ()->text ()
              << " currentItem=" << newItem
              << endl;
    comboBox->insertItem (index,
      e->action()->icon(),
      ::DropAmpersands (e->action()->text()),
      (int) e->action ());

    comboBox->setCurrentItem (newItem);

    eatEvent = true;
  }
  else if (event->type () == QEvent::ActionChanged)
  {
    QActionEvent * const e = static_cast <QActionEvent *> (event);
    
    const int index = comboBox->findData ((int) e->action ());
    const int newItem = ::TrueCurrentItem (this);
    kDebug () << "KSelectAction::eventFilter(ActionChanged)"
              << "    comboBox: ptr=" << comboBox
              << " currentItem=" << comboBox->currentIndex ()
              << " index=" << index
              << "    action new: ptr=" << e->action ()
              << " icon=" << e->action ()->icon ()
              << " text=" << e->action ()->text ()
              << " currentItem=" << newItem
              << endl;
    comboBox->setItemIcon (index, e->action ()->icon ());
    comboBox->setItemText (index, ::DropAmpersands (e->action ()->text ()));

    comboBox->setCurrentItem (newItem);

    eatEvent = true;
  }
  else if (event->type () == QEvent::ActionRemoved)
  {
    QActionEvent * const e = static_cast <QActionEvent *> (event);

    const int index = comboBox->findData ((int) e->action ());
    const int newItem = ::TrueCurrentItem (this);
    kDebug () << "KSelectAction::eventFilter(ActionRemoved)"
              << "    comboBox: ptr=" << comboBox
              << " currentItem=" << comboBox->currentIndex ()
              << " index=" << index
              << "    new: currentItem=" << newItem
              << endl;
    comboBox->removeItem (index);

    comboBox->setCurrentItem (newItem);

    eatEvent = true;
  }
  
  comboBox->blockSignals (false);

  return eatEvent;
}

// END

/* vim: et sw=2 ts=2
 */

#include "kselectaction.moc"
