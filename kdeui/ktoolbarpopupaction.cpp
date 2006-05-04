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

#include "ktoolbarpopupaction.h"

#include <QToolBar>
#include <QToolButton>

#include <kdebug.h>
#include <klocale.h>

#include "kmenu.h"

KToolBarPopupAction::KToolBarPopupAction( const KIcon& icon,
                                          const QString& text,
                                          KActionCollection* parent, const QString& name )
  : KAction( icon, text, parent, name )
{
  m_delayed = true;
  m_stickyMenu = true;
  setToolBarWidgetFactory(this);
  setMenu(new KMenu);
}

KToolBarPopupAction::KToolBarPopupAction( const QString& text,
                                          const QString& icon,
                                          const KShortcut& cut,
                                          KActionCollection* parent, const QString& name )
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
                                          const QString& name )
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
                                          const QString& name )
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

/* vim: et sw=2 ts=2
 */

#include "ktoolbarpopupaction.moc"
