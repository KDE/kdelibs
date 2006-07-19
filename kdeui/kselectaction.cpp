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
  action->setChecked(true);
}

bool KSelectAction::setCurrentItem( int index )
{
  if (QAction* a = action(index)) {
    setCurrentAction(a);
    return true;
  }

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
  if (QAction* a = action(text, cs)) {
    a->setChecked(true);
    return true;
  }

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
  action->setActionGroup(selectableActionGroup());
  action->setCheckable( true );

  // Keep in sync with createToolBarWidget()
  foreach (QToolButton* button, d->m_buttons)
    button->menu()->addAction(action);

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->addItem(action->icon(), action->text(), action);

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
  int index = selectableActionGroup()->actions().indexOf(action);

  action->setActionGroup(0L);

  foreach (QToolButton* button, d->m_buttons)
    button->menu()->removeAction(action);

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->removeItem(index);

  menu()->removeAction(action);

  return action;
}

void KSelectAction::actionTriggered(QAction* action)
{
  // cache values so we don't need access to members in the action
  // after we've done an emit()
  QString text = action->text();
  int index = selectableActionGroup()->actions().indexOf(action);

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
  // we need to delete the actions later since we may get a call to clear()
  // from a method called due to a triggered(...) signal
  foreach (QAction* action, d->m_actionGroup->actions())
    action->deleteLater();
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

      if ( d->m_maxComboViewCount != -1 )
        comboBox->setMaxVisibleItems( d->m_maxComboViewCount );

      if ( d->m_comboWidth > 0 )
        comboBox->setMaximumWidth( d->m_comboWidth );

      comboBox->setEditable(isEditable());

      foreach (QAction* action, selectableActionGroup()->actions())
        comboBox->addItem(action->icon(), action->text(), action);

      connect(comboBox, SIGNAL(destroyed(QObject*)), SLOT(comboBoxDeleted(QObject*)));
      d->m_comboBoxes.append(comboBox);

      return comboBox;
    }
  }

  return 0L;
}
// END

/* vim: et sw=2 ts=2
 */

#include "kselectaction.moc"
