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

#include "kactionclasses.h"

#include <assert.h>

#include <QClipboard>
#include <QFontDatabase>
#include <QTimer>
#include <QFile>
#include <QMimeData>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfontcombo.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <ktoolbar.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>

// BEGIN KToggleAction
class KToggleAction::KToggleActionPrivate
{
public:
  KToggleActionPrivate()
    : m_checkedGuiItem(0L)
  {}

  ~KToggleActionPrivate()
  {
    delete m_checkedGuiItem;
  }

  KGuiItem* m_checkedGuiItem;
};


KToggleAction::KToggleAction( KActionCollection * parent, const char* name, QActionGroup * exclusiveGroup )
    : KAction(parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const QString & text, KActionCollection * parent, const char* name, QActionGroup * exclusiveGroup )
    : KAction(text, parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name, QActionGroup * exclusiveGroup )
    : KAction(icon, text, parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name, QActionGroup * exclusiveGroup )
    : KAction(icon, text, parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const QString& text, const KShortcut& cut,
                              KActionCollection* parent,
                              const char* name )
    : KAction( text, cut, 0,0,parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const KShortcut& cut,
                              const QObject* receiver, const char* slot,
                              KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const char* name )
  : KAction( text, pix, cut, 0,0,parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const char* name )
 : KAction( text, pix, cut, 0,0,parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KToggleAction::~KToggleAction()
{
  delete d;
}

void KToggleAction::init()
{
  d = new KToggleActionPrivate;
  setCheckable(true);
  connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
}

void KToggleAction::setCheckedState( const KGuiItem& checkedItem )
{
  delete d->m_checkedGuiItem;
  d->m_checkedGuiItem = new KGuiItem( checkedItem );
}

void KToggleAction::slotToggled( bool checked )
{
  if (d->m_checkedGuiItem) {
    QString string = d->m_checkedGuiItem->text();
    d->m_checkedGuiItem->setText(text());
    setText(string);

    string = d->m_checkedGuiItem->toolTip();
    d->m_checkedGuiItem->setToolTip(toolTip());
    setToolTip(string);

    // TODO: icon switching?
  }
}
// END

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

KSelectAction::KSelectAction( KActionCollection * parent, const char* name )
  : KAction(parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString & text, KActionCollection * parent, const char* name )
  : KAction(text, parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : KAction(icon, text, parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name )
  : KAction(icon, text, parent, name)
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const KShortcut& cut,
                              KActionCollection* parent, const char* name )
  : KAction( text, cut, 0,0,parent, name )
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const KShortcut& cut,
                              const QObject* receiver, const char* slot,
                              KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const char* name )
  : KAction( text, pix, cut, 0,0,parent, name )
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const char* name )
  : KAction( text, pix, cut, 0,0,parent, name )
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::KSelectAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const char* name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
  , d(new KSelectActionPrivate())
{
  init();
}

KSelectAction::~KSelectAction()
{
  delete d;
}

void KSelectAction::init()
{
  setToolBarWidgetFactory(this);
  connect(selectableActionGroup(), SIGNAL(triggered(QAction*)), SLOT(actionTriggered(QAction*)));
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

  int index = selectableActionGroup()->actions().indexOf(action);

  // Keep in sync with createToolBarWidget()
  foreach (QToolButton* button, d->m_buttons)
    button->menu()->addAction(action);

  foreach (KComboBox* comboBox, d->m_comboBoxes)
    comboBox->addItem(action->icon(), action->text(), action);

  if (menu())
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

  return action;
}

void KSelectAction::actionTriggered(QAction* action)
{
  emit triggered(action);
  emit triggered(selectableActionGroup()->actions().indexOf(action));
  emit triggered(action->text());
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
  while (d->m_actionGroup->actions().count())
    delete d->m_actionGroup->actions().first();
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

QWidget * KSelectAction::createToolBarWidget( QToolBar * parent )
{
  switch (toolBarMode()) {
    case MenuMode: {
      QToolButton* button = new QToolButton(parent);
      button->setAutoRaise(true);
      button->setFocusPolicy(Qt::NoFocus);
      button->setIconSize(parent->iconSize());
      button->setToolButtonStyle(parent->toolButtonStyle());
      QObject::connect(parent, SIGNAL(iconSizeChanged(const QSize&)),
                       button, SLOT(setIconSize(const QSize&)));
      QObject::connect(parent, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                       button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
      button->setDefaultAction(this);
      QObject::connect(button, SIGNAL(triggered(QAction*)), parent, SIGNAL(actionTriggered(QAction*)));

      button->setPopupMode(toolButtonPopupMode());

      KMenu* newMenu = new KMenu();
      button->setMenu(newMenu);
      foreach (QAction* action, actions())
        newMenu->addAction(action);
      d->m_buttons.append(button);
      return button;
    }

    case ComboBoxMode: {
      KComboBox* comboBox = new KComboBox(parent);

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

// BEGIN KRecentFilesAction
class KRecentFilesAction::KRecentFilesActionPrivate
{
public:
  KRecentFilesActionPrivate()
  {
    m_maxItems = 10;
  }

  int m_maxItems;
  QMap<QAction*, QString> m_shortNames;
  QMap<QAction*, KUrl> m_urls;
};


KRecentFilesAction::KRecentFilesAction( KActionCollection * parent, const char* name )
  : KSelectAction(parent, name)
{
  d = new KRecentFilesActionPrivate;

  init();
}

KRecentFilesAction::KRecentFilesAction( const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction(text, parent, name)
{
  d = new KRecentFilesActionPrivate;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction(icon, text, parent, name)
{
  d = new KRecentFilesActionPrivate;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction(icon, text, parent, name)
{
  d = new KRecentFilesActionPrivate;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const KShortcut& cut,
                                        KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( text, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( text, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KUrl&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIcon& pix,
                                        const KShortcut& cut,
                                        KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix,
                                        const KShortcut& cut,
                                        KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QIcon& pix,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KUrl&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( const QString& text,
                                        const QString& pix,
                                        const KShortcut& cut,
                                        const QObject* receiver,
                                        const char* slot,
                                        KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( text, pix, cut, parent, name)
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  // Want to keep the ampersands
  setText(text);

  if ( receiver )
    connect( this,     SIGNAL(urlSelected(const KUrl&)),
             receiver, slot );
}

KRecentFilesAction::KRecentFilesAction( KActionCollection* parent, const char* name,
                                        int maxItems )
  : KSelectAction( parent, name )
{
  d = new KRecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

void KRecentFilesAction::init()
{
  setMenu(new KMenu());

  connect(this, SIGNAL(triggered(QAction*)), SLOT(urlSelected(QAction*)));
}

KRecentFilesAction::~KRecentFilesAction()
{
  delete menu();
  delete d;
}

void KRecentFilesAction::urlSelected( QAction* action )
{
  emit urlSelected(d->m_urls[action]);
}

int KRecentFilesAction::maxItems() const
{
    return d->m_maxItems;
}

void KRecentFilesAction::setMaxItems( int maxItems )
{
    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all excess items
    while( selectableActionGroup()->actions().count() > maxItems )
        delete removeAction(selectableActionGroup()->actions().last());
}

void KRecentFilesAction::addUrl( const KUrl& url, const QString& name )
{
    if ( url.isLocalFile() && !KGlobal::dirs()->relativeLocation("tmp", url.path()).startsWith("/"))
       return;
    const QString tmpName = name.isEmpty() ?  url.fileName() : name;
    const QString file = url.pathOrURL();

    // remove file if already in list
    foreach (QAction* action, selectableActionGroup()->actions())
    {
      if ( action->text().endsWith( file + "]" ) )
      {
        delete removeAction(action);
        break;
      }
    }
    // remove last item if already maxitems in list
    if( d->m_maxItems && selectableActionGroup()->actions().count() == d->m_maxItems )
    {
        // remove last item
        delete removeAction(selectableActionGroup()->actions().last());
    }

    // add file to list
    const QString title = tmpName + " [" + file + "]";
    QAction* action = new QAction(title, selectableActionGroup());
    addAction(action, url, tmpName);
}

void KRecentFilesAction::addAction(QAction* action, const KUrl& url, const QString& name)
{
  KSelectAction::addAction( action );

  d->m_shortNames.insert( action, name );
  d->m_urls.insert( action, url );
}

QAction* KRecentFilesAction::removeAction(QAction* action)
{
  KSelectAction::removeAction( action );

  d->m_shortNames.remove( action );
  d->m_urls.remove( action );

  return action;
}

void KRecentFilesAction::removeUrl( const KUrl& url )
{
  for (QMap<QAction*, KUrl>::ConstIterator it = d->m_urls.constBegin(); it != d->m_urls.constEnd(); ++it)
    if (it.value() == url) {
      delete removeAction(it.key());
      return;
    }
}

void KRecentFilesAction::clear()
{
    KSelectAction::clear();
    d->m_shortNames.clear();
    d->m_urls.clear();
}

void KRecentFilesAction::loadEntries( KConfig* config, const QString &groupname)
{
    clear();

    QString     key;
    QString     value;
    QString     nameKey;
    QString     nameValue;
    QString      title;
    QString     oldGroup;
    QStringList lst;
    KUrl        url;

    oldGroup = config->group();

    if (groupname.isEmpty())
      config->setGroup("RecentFiles");
    else
      config->setGroup( groupname );

    // read file list
    for( int i = 1 ; i <= d->m_maxItems ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        value = config->readPathEntry( key );
        url = KUrl::fromPathOrURL( value );

        // Don't restore if file doesn't exist anymore
        if (url.isLocalFile() && !QFile::exists(url.path()))
          continue;

        nameKey = QString( "Name%1" ).arg( i );
        nameValue = config->readPathEntry( nameKey, url.fileName() );
        title = nameValue + " [" + value + "]";
        if (!value.isNull())
        {
          addAction(new QAction(title, selectableActionGroup()), url, nameValue);
        }
    }

    config->setGroup( oldGroup );
}

void KRecentFilesAction::saveEntries( KConfig* config, const QString &groupname )
{
    QString     key;
    QString     value;
    QString     oldGroup;
    QStringList lst = items();

    oldGroup = config->group();

    QString group = groupname;
    if (groupname.isEmpty())
      group = "RecentFiles";
    config->deleteGroup( group );
    config->setGroup( group );

    // write file list
    for ( int i = 1 ; i <= selectableActionGroup()->actions().count() ; i++ )
    {
        key = QString( "File%1" ).arg( i );
        // FIXME KAction port - why is this -1?
        value = d->m_urls[ selectableActionGroup()->actions()[ i - 1 ] ].pathOrURL();
        config->writePathEntry( key, value );
        key = QString( "Name%1" ).arg( i );
        value = d->m_shortNames[ selectableActionGroup()->actions()[ i - 1 ] ];
        config->writePathEntry( key, value );
    }

    config->setGroup( oldGroup );
}
// END

// BEGIN KFontAction
KFontAction::KFontAction( uint fontListCriteria, KActionCollection * parent, const char* name )
  : KSelectAction( parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, fontListCriteria );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( KActionCollection * parent, const char* name )
  : KSelectAction( parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction( text, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction( icon, text, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction( icon, text, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString& text,
                          const KShortcut& cut, KActionCollection* parent,
                          const char* name )
  : KSelectAction( text, cut, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString& text, const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          KActionCollection* parent, const char* name )
    : KSelectAction( text, cut, receiver, slot, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString& text, const QIcon& pix,
                          const KShortcut& cut,
                          KActionCollection* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString& text, const QString& pix,
                          const KShortcut& cut,
                          KActionCollection* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString& text, const QIcon& pix,
                          const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          KActionCollection* parent, const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( const QString& text, const QString& pix,
                          const KShortcut& cut,
                          const QObject* receiver, const char* slot,
                          KActionCollection* parent, const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( uint fontListCriteria, const QString& text,
                          const KShortcut& cut, KActionCollection* parent,
                          const char* name )
    : KSelectAction( text, cut, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, fontListCriteria );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction( uint fontListCriteria, const QString& text, const QString& pix,
                          const KShortcut& cut,
                          KActionCollection* parent, const char* name )
    : KSelectAction( text, pix, cut, parent, name )
{
    QStringList list;
    KFontChooser::getFontList( list, fontListCriteria );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::~KFontAction()
{
    //Reinstate if d-pointer needed
    //delete d;
}

QWidget* KFontAction::createToolBarWidget(QToolBar* parent)
{
    KFontCombo *cb = new KFontCombo( items(), parent );
    connect( cb, SIGNAL( activated( const QString & ) ), SLOT( slotActivated( const QString & ) ) );
    cb->setMinimumWidth( cb->sizeHint().width() );
    return cb;
}

/*
 * Maintenance note: Keep in sync with KFontCombo::setCurrentFont()
 */
void KFontAction::setFont( const QString &family )
{
    QString lowerName = family.toLower();
    if (setCurrentAction(lowerName, Qt::CaseInsensitive))
       return;

    int i = lowerName.indexOf(" [");
    if (i > -1)
    {
       lowerName = lowerName.left(i);
       i = 0;
       if (setCurrentAction(lowerName, Qt::CaseInsensitive))
          return;
    }

    lowerName += " [";
    if (setCurrentAction(lowerName, Qt::CaseInsensitive))
      return;

    kDebug(129) << "Font not found " << family.toLower() << endl;
}
// END

// BEGIN KFontSizeAction
KFontSizeAction::KFontSizeAction( KActionCollection * parent, const char* name )
  : KSelectAction( parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction( text, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction( icon, text, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : KSelectAction( icon, text, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text,
                                  const KShortcut& cut,
                                  KActionCollection* parent, const char* name )
  : KSelectAction( text, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text,
                                  const KShortcut& cut,
                                  const QObject* receiver, const char* slot,
                                  KActionCollection* parent, const char* name )
  : KSelectAction( text, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIcon& pix,
                                  const KShortcut& cut,
                                  KActionCollection* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix,
                                  const KShortcut& cut,
                                  KActionCollection* parent, const char* name )
  : KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QIcon& pix,
                                  const KShortcut& cut,
                                  const QObject* receiver,
                                  const char* slot, KActionCollection* parent,
                                  const char* name )
    : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::KFontSizeAction( const QString& text, const QString& pix,
                                  const KShortcut& cut,
                                  const QObject* receiver,
                                  const char* slot, KActionCollection* parent,
                                  const char* name )
  : KSelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KFontSizeAction::~KFontSizeAction()
{
    //Reinstate if d-pointer required
    //delete d;
}

void KFontSizeAction::init()
{
    setEditable( true );
    QFontDatabase fontDB;
    QList<int> sizes = fontDB.standardSizes();
    QStringList lst;
    for ( QList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it )
        lst.append( QString::number( *it ) );

    setItems( lst );

    connect(this, SIGNAL(triggered(bool)), SLOT(slotTriggered()));
}

void KFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() ) {
        QString test = QString::number( size );
        foreach (QAction* action, actions())
        {
          if (action->text() == test)
          {
              setCurrentAction(action);
              return;
          }
        }
    }

    if ( size < 1 ) {
        kWarning() << "KFontSizeAction: Size " << size << " is out of range" << endl;
        return;
    }

    QAction* a = action( QString::number( size ) );
    if ( !a ) {
        // Insert at the correct position in the list (to keep sorting)
        QList<int> lst;
        // Convert to list of ints
        QStringList itemsList = items();
        for (QStringList::Iterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
            lst.append( (*it).toInt() );
        // New size
        lst.append( size );
        // Sort the list
        qSort( lst );
        // Convert back to string list
        QStringList strLst;
        foreach ( int it, lst ) {
            QAction* action = new QAction(QString::number(it), selectableActionGroup());
            if (it == size)
              setCurrentAction(action);
        }

    } else {
        setCurrentAction( a );
    }
}

int KFontSizeAction::fontSize() const
{
  return currentText().toInt();
}

void KFontSizeAction::slotTriggered()
{
  QAction* justTriggered = qobject_cast<QAction*>(sender());

  if (justTriggered)
    emit fontSizeChanged(justTriggered->text().toInt());
}
// END

// BEGIN KActionMenu
class KActionMenuPrivate
{
public:
  KActionMenuPrivate()
  {
    m_popup = new KMenu();
    m_delayed = true;
    m_stickyMenu = true;
  }
  ~KActionMenuPrivate()
  {
    delete m_popup; m_popup = 0;
  }
  KMenu *m_popup;
  bool m_delayed;
  bool m_stickyMenu;
};

KActionMenu::KActionMenu( KActionCollection * parent, const char* name )
  : KAction( parent, name )
  , d(new KActionMenuPrivate)
{
  setShortcutConfigurable( false );
  setToolBarWidgetFactory(this);
}

KActionMenu::KActionMenu( const QString & text, KActionCollection * parent, const char* name )
  : KAction( text, parent, name )
  , d(new KActionMenuPrivate)
{
  setShortcutConfigurable( false );
  setToolBarWidgetFactory(this);
}

KActionMenu::KActionMenu( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : KAction( icon, text, parent, name )
  , d(new KActionMenuPrivate)
{
  setShortcutConfigurable( false );
  setToolBarWidgetFactory(this);
}

KActionMenu::~KActionMenu()
{
    delete d;
    delete menu();
}

QWidget * KActionMenu::createToolBarWidget( QToolBar * parent )
{
  QToolButton* button = new QToolButton(parent);
  button->setAutoRaise(true);
  button->setFocusPolicy(Qt::NoFocus);
  button->setIconSize(parent->iconSize());
  button->setToolButtonStyle(parent->toolButtonStyle());
  QObject::connect(parent, SIGNAL(iconSizeChanged(const QSize&)),
                   button, SLOT(setIconSize(const QSize&)));
  QObject::connect(parent, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
  button->setDefaultAction(this);
  QObject::connect(button, SIGNAL(triggered(QAction*)), parent, SIGNAL(actionTriggered(QAction*)));

  if (delayed())
    button->setPopupMode(QToolButton::DelayedPopup);
  else if (stickyMenu())
    button->setPopupMode(QToolButton::InstantPopup);
  else
    button->setPopupMode(QToolButton::MenuButtonPopup);

  return button;
}

void KActionMenu::insert( KAction* cmd, QAction* before )
{
  if (!menu())
    kMenu();

  if ( cmd )
    menu()->insertAction(before, cmd);
}

void KActionMenu::remove( KAction* cmd )
{
  if (!menu())
    kMenu();

  if ( cmd )
    menu()->removeAction(cmd);
}

void KActionMenu::addAction( QAction * action )
{
  if (!menu())
    kMenu();

  menu()->addAction(action);
}

QAction* KActionMenu::addSeparator()
{
  QAction* separator = new QAction(this);
  separator->setSeparator(true);
  addAction(separator);
  return separator;
}

QAction* KActionMenu::insertSeparator(QAction* before)
{
  QAction* separator = new QAction(this);
  separator->setSeparator(true);
  insertAction(before, separator);
  return separator;
}

void KActionMenu::insertAction( QAction * before, QAction * action )
{
  if (!menu())
    kMenu();

  menu()->insertAction(before, action);
}

void KActionMenu::removeAction( QAction * action )
{
  if (!menu())
    kMenu();

  menu()->removeAction(action);
}

bool KActionMenu::delayed() const {
    return d->m_delayed;
}

void KActionMenu::setDelayed(bool _delayed) {
    d->m_delayed = _delayed;
}

bool KActionMenu::stickyMenu() const {
    return d->m_stickyMenu;
}

void KActionMenu::setStickyMenu(bool sticky) {
    d->m_stickyMenu = sticky;
}

KMenu* KActionMenu::kMenu()
{
  // FIXME is this right - parentWidget() ?
  if (!menu())
    setMenu(new KMenu(parentWidget()));

  return qobject_cast<KMenu*>(menu());
}
// END

// BEGIN KToolBarPopupAction

KToolBarPopupAction::KToolBarPopupAction( const QString& text,
                                          const QString& icon,
                                          const KShortcut& cut,
                                          KActionCollection* parent, const char* name )
  : KAction( text, icon, cut, 0,0,parent, name )
{
  m_delayed = true;
  m_stickyMenu = true;
  setToolBarWidgetFactory(this);
  setMenu(new KMenu);
}

KToolBarPopupAction::KToolBarPopupAction( const QString& text,
                                          const QString& icon,
                                          const KShortcut& cut,
                                          const QObject* receiver,
                                          const char* slot, KActionCollection* parent,
                                          const char* name )
  : KAction( text, icon, cut, receiver, slot, parent, name )
{
  m_delayed = true;
  m_stickyMenu = true;
  setToolBarWidgetFactory(this);
  setMenu(new KMenu);
}

KToolBarPopupAction::KToolBarPopupAction( const KGuiItem& item,
                                          const KShortcut& cut,
                                          const QObject* receiver,
                                          const char* slot, KActionCollection* parent,
                                          const char* name )
  : KAction( item, cut, receiver, slot, parent, name )
{
  m_delayed = true;
  m_stickyMenu = true;
  setToolBarWidgetFactory(this);
  setMenu(new KMenu);
}

KToolBarPopupAction::~KToolBarPopupAction()
{
    delete menu();
}

KMenu* KToolBarPopupAction::popupMenu() const
{
  return qobject_cast<KMenu*>(menu());
}

QWidget * KToolBarPopupAction::createToolBarWidget( QToolBar * parent )
{
  QToolButton* button = new QToolButton(parent);
  button->setAutoRaise(true);
  button->setFocusPolicy(Qt::NoFocus);
  button->setIconSize(parent->iconSize());
  button->setToolButtonStyle(parent->toolButtonStyle());
  QObject::connect(parent, SIGNAL(iconSizeChanged(const QSize&)),
                   button, SLOT(setIconSize(const QSize&)));
  QObject::connect(parent, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
  button->setDefaultAction(this);
  QObject::connect(button, SIGNAL(triggered(QAction*)), parent, SIGNAL(actionTriggered(QAction*)));

  if (m_delayed)
    if (m_stickyMenu)
      button->setPopupMode(QToolButton::MenuButtonPopup);
    else
      button->setPopupMode(QToolButton::DelayedPopup);
  else
    button->setPopupMode(QToolButton::InstantPopup);

  return button;
}

bool KToolBarPopupAction::delayed() const {
    return m_delayed;
}

void KToolBarPopupAction::setDelayed(bool delayed) {
    m_delayed = delayed;
}

bool KToolBarPopupAction::stickyMenu() const {
    return m_stickyMenu;
}

void KToolBarPopupAction::setStickyMenu(bool sticky) {
    m_stickyMenu = sticky;
}

#if 0
int KToolBarPopupAction::plug( QWidget *widget, int index )
{
  if (!KAuthorized::authorizeKAction(name()))
    return -1;
  // This is very related to KActionMenu::plug.
  // In fact this class could be an interesting base class for KActionMenu
  if ( qobject_cast<KToolBar*>( widget ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() ) {
        bar->insertButton( iconSet().pixmap(), id_, SIGNAL( buttonClicked(int, Qt::MouseButtons, Qt::KeyboardModifiers) ), this,
                           SLOT( slotButtonClicked(int, Qt::MouseButtons, Qt::KeyboardModifiers) ),
                           isEnabled(), plainText(),
                           index );
    } else {
        KInstance * instance;
        if ( m_parentCollection )
            instance = m_parentCollection->instance();
        else
            instance = KGlobal::instance();

        bar->insertButton( icon(), id_, SIGNAL( buttonClicked(int, Qt::MouseButtons, Qt::KeyboardModifiers) ), this,
                           SLOT( slotButtonClicked(int, Qt::MouseButtons, Qt::KeyboardModifiers) ),
                           isEnabled(), plainText(),
                           index, instance );
    }

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setMenu(popupMenu(), stickyMenu());
    }

    if ( !whatsThis().isEmpty() )
        bar->getButton( id_ )->setWhatsThis(whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}
#endif

////////
// END

// BEGIN KToggleToolBarAction
KToggleToolBarAction::KToggleToolBarAction( const char* toolBarName,
         const QString& text, KActionCollection* parent, const char* name )
  : KToggleAction( text, parent, name )
  , m_toolBarName( toolBarName )
  , m_toolBar( 0L )
  , m_beingToggled( false )
{
}

KToggleToolBarAction::KToggleToolBarAction( KToolBar *toolBar, const QString &text,
                                            KActionCollection *parent, const char *name )
  : KToggleAction( text, parent, name )
  , m_toolBarName( 0 ), m_toolBar( toolBar ), m_beingToggled( false )
{
    m_toolBar->installEventFilter(this);

    m_beingToggled = true;
    if (m_toolBar->isVisible())
      setChecked(true);
    m_beingToggled = false;
}

KToggleToolBarAction::~KToggleToolBarAction()
{
}

bool KToggleToolBarAction::eventFilter( QObject * watched, QEvent * event )
{
    if (m_beingToggled)
        return false;

    m_beingToggled = true;

    if (watched == m_toolBar) {
        switch (event->type()) {
            case QEvent::Hide:
                if (isChecked())
                    setChecked(false);
                break;

            case QEvent::Show:
                if (!isChecked())
                    setChecked(true);
                break;

            default:
                break;
        }
    }

    m_beingToggled = false;

    return false;
}

KToolBar * KToggleToolBarAction::toolBar( )
{
  return m_toolBar;
}

void KToggleToolBarAction::slotToggled( bool checked )
{
  if (!m_beingToggled && m_toolBar && checked != m_toolBar->isVisible() ) {
    m_beingToggled = true;

    if (checked) {
      m_toolBar->show();
    } else {
      m_toolBar->hide();
    }

    m_beingToggled = false;

    QMainWindow* mw = m_toolBar->mainWindow();
    if ( mw && qobject_cast<KMainWindow*>( mw ) )
      static_cast<KMainWindow *>( mw )->setSettingsDirty();
  }

  KToggleAction::slotToggled(checked);
}
// END

// BEGIN KToggleFullScreenAction
KToggleFullScreenAction::KToggleFullScreenAction( KActionCollection * parent, const char* name )
  : KToggleAction( KIcon("window_fullscreen"), i18n("F&ull Screen Mode"), parent, name ),
    m_window( 0L )
{
}

KToggleFullScreenAction::KToggleFullScreenAction( const KShortcut &cut,
                             const QObject* receiver, const char* slot,
                             KActionCollection* parent, QWidget* window,
                             const char* name )
  : KToggleAction( i18n("F&ull Screen Mode"), cut, receiver, slot, parent, name ),
    m_window( 0L )
{
  setIcon(KIcon("window_fullscreen"));
  setWindow( window );
}

KToggleFullScreenAction::~KToggleFullScreenAction()
{
}

void KToggleFullScreenAction::setWindow( QWidget* w )
{
  if( m_window )
    m_window->removeEventFilter( this );

  m_window = w;

  if( m_window )
    m_window->installEventFilter( this );
}

void KToggleFullScreenAction::slotToggled( bool checked )
{
  if (checked)
  {
     setText(i18n("Exit F&ull Screen Mode"));
     setIconName("window_nofullscreen");
  }
  else
  {
     setText(i18n("F&ull Screen Mode"));
     setIconName("window_fullscreen");
  }

  KToggleAction::slotToggled(checked);
}

bool KToggleFullScreenAction::eventFilter( QObject* o, QEvent* e )
{
    if( o == m_window )
        if( e->type() == QEvent::WindowStateChange )
            {
            if( m_window->isFullScreen() != isChecked())
                activate(QAction::Trigger);
            }
    return false;
}
// END

// BEGIN KWidgetAction
KWidgetAction::KWidgetAction( QWidget* widget,
    const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot,
    KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
{
  setToolBarWidgetFactory(this);
}

KWidgetAction::~KWidgetAction()
{
  delete m_widget;
}

QWidget * KWidgetAction::createToolBarWidget( QToolBar * parent )
{
  if (!m_widget)
    return 0L;

  m_widget->setParent(parent);
  return m_widget;
}

void KWidgetAction::destroyToolBarWidget(QWidget* widget)
{
  widget->setParent(0L);
}

bool KWidgetAction::event(QEvent* event)
{
  if (event->type() == QEvent::ActionChanged) {
    if (isEnabled() != m_widget->isEnabled())
      m_widget->setEnabled(isEnabled());
  }
  
  return KAction::event(event);
}
// END

// BEGIN KPasteTextAction

KPasteTextAction::KPasteTextAction( KActionCollection * parent, const char* name )
  : KAction( parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const QString & text, KActionCollection * parent, const char* name )
  : KAction( text, parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : KAction( icon, text, parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name )
  : KAction( icon, text, parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const QString& text,
                            const QString& icon,
                            const KShortcut& cut,
                            const QObject* receiver,
                            const char* slot, KActionCollection* parent,
                            const char* name)
  : KAction( text, icon, cut, receiver, slot,parent, name )
{
  init();
}

void KPasteTextAction::init()
{
  m_popup = new KMenu;
  connect(m_popup, SIGNAL(aboutToShow()), this, SLOT(menuAboutToShow()));
  connect(m_popup, SIGNAL(triggered(QAction*)), this, SLOT(slotTriggered(QAction*)));
  m_mixedMode = true;
}

KPasteTextAction::~KPasteTextAction()
{
  delete m_popup;
}

void KPasteTextAction::setMixedMode(bool mode)
{
  m_mixedMode = mode;
}

void KPasteTextAction::menuAboutToShow()
{
    m_popup->clear();
    QStringList list;
    DCOPClient *client = KApplication::dcopClient();
    if (client->isAttached() && client->isApplicationRegistered("klipper")) {
      DCOPRef klipper("klipper","klipper");
      DCOPReply reply = klipper.call("getClipboardHistoryMenu");
      if (reply.isValid())
        list = reply;
    }
    QString clipboardText = qApp->clipboard()->text(QClipboard::Clipboard);
    if (list.isEmpty())
        list << clipboardText;
    bool found = false;
    foreach (const QString& string, list)
    {
      QString text = KStringHandler::cEmSqueeze(string.simplified(), m_popup->fontMetrics(), 20);
      text.replace("&", "&&");
      QAction* action = m_popup->addAction(text);
      if (!found && string == clipboardText)
      {
        action->setChecked(true);
        found = true;
      }
    }
}

void KPasteTextAction::slotTriggered(QAction* action)
{
    DCOPClient *client = KApplication::dcopClient();
    if (client->isAttached() && client->isApplicationRegistered("klipper")) {
      DCOPRef klipper("klipper","klipper");

     DCOPReply reply = klipper.call("getClipboardHistoryItem(int)", m_popup->actions().indexOf(action));
      if (!reply.isValid())
        return;
      QString clipboardText = reply;
      reply = klipper.call("setClipboardContents(QString)", clipboardText);
      if (reply.isValid())
        kDebug(129) << "Clipboard: " << qApp->clipboard()->text(QClipboard::Clipboard) << endl;
    }
}

KActionSeparator::KActionSeparator( KActionCollection * parent, const char* name )
  : KAction(parent, name)
{
  setSeparator(true);
}

// END

/* vim: et sw=2 ts=2
 */

#include "kactionclasses.moc"
