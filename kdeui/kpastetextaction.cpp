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

#include "kpastetextaction.h"

#include <QClipboard>
#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstringhandler.h>

#include "kmenu.h"

KPasteTextAction::KPasteTextAction( KActionCollection * parent, const QString& name )
  : KAction( parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const QString & text, KActionCollection * parent, const QString& name )
  : KAction( text, parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const KIcon & icon, const QString & text, KActionCollection * parent, const QString& name )
  : KAction( icon, text, parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const QString & icon, const QString & text, KActionCollection * parent, const QString& name )
  : KAction( icon, text, parent, name )
{
  init();
}

KPasteTextAction::KPasteTextAction( const QString& text,
                            const QString& icon,
                            const KShortcut& cut,
                            const QObject* receiver,
                            const char* slot, KActionCollection* parent,
                            const QString& name)
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
    QDBusInterface klipper("org.kde.klipper", "/klipper", "org.kde.Klipper");
    if (klipper.isValid()) {
      QDBusReply<QStringList> reply = klipper.call("getClipboardHistoryMenu");
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
    QDBusInterface klipper("org.kde.klipper", "/klipper", "org.kde.Klipper");
    if (klipper.isValid()) {
      QDBusReply<QString> reply = klipper.call("getClipboardHistoryItem",
                                                m_popup->actions().indexOf(action));
      if (!reply.isValid())
        return;
      QString clipboardText = reply;
      reply = klipper.call("setClipboardContents", clipboardText);
      if (reply.isValid())
        kDebug(129) << "Clipboard: " << qApp->clipboard()->text(QClipboard::Clipboard) << endl;
    }
}

/* vim: et sw=2 ts=2
 */

#include "kpastetextaction.moc"
