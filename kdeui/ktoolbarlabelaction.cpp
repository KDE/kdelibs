/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "ktoolbarlabelaction.h"

#include <qlabel.h>
#include <qapplication.h>

class KToolBarLabelAction::KToolBarLabelActionPrivate
{
public:
  KToolBarLabelActionPrivate()
    : m_label(0)
  {
  }
  QLabel* m_label;
};


KToolBarLabelAction::KToolBarLabelAction(const QString &text,
					 const KShortcut &cut,
					 const QObject *receiver, 
					 const char *slot,
					 KActionCollection *parent,
					 const char *name)
  : KWidgetAction(new QLabel(text, 0, "kde toolbar widget"), text, cut,
		  receiver, slot, parent, name), 
    d(new KToolBarLabelActionPrivate)
{
  init();
}

KToolBarLabelAction::KToolBarLabelAction(QWidget* buddy, 
					 const QString &text,
					 const KShortcut &cut,
					 const QObject *receiver, 
					 const char *slot,
 					 KActionCollection *parent, 
					 const char *name)
  : KWidgetAction(new QLabel(buddy, text, 0, "kde toolbar widget"), text, 
		  cut, receiver, slot, parent, name),
    d(new KToolBarLabelActionPrivate)
{
  init();
}

KToolBarLabelAction::KToolBarLabelAction(QLabel* label, 
					 const KShortcut &cut, 
					 const QObject *receiver,
					 const char *slot,
					 KActionCollection* parent, 
					 const char *name)
  : KWidgetAction(label, label->text(), cut, receiver, slot, parent, name),
    d(new KToolBarLabelActionPrivate)
{
  Q_ASSERT(QLatin1String("kde toolbar widget") == label->name());
  init();
}

KToolBarLabelAction::~KToolBarLabelAction()
{
  delete d;
  d = 0;
}

void KToolBarLabelAction::init()
{
  d->m_label = static_cast<QLabel*>(widget());
  /* these lines were copied from Konqueror's KonqDraggableLabel class in
     konq_misc.cc */
  d->m_label->setBackgroundMode(Qt::PaletteButton);
  d->m_label->setAlignment((QApplication::reverseLayout()
			 ? Qt::AlignRight : Qt::AlignLeft) |
 			Qt::AlignVCenter | Qt::TextShowMnemonic );
  d->m_label->adjustSize();
}

void KToolBarLabelAction::setText(const QString& text)
{
  KWidgetAction::setText(text);
  d->m_label->setText(text);
}

void KToolBarLabelAction::setBuddy(QWidget* buddy)
{
  d->m_label->setBuddy(buddy);
}

QWidget* KToolBarLabelAction::buddy() const
{
  return d->m_label->buddy();
}

QLabel* KToolBarLabelAction::label() const
{
  return d->m_label;
}

void KToolBarLabelAction::virtual_hook(int, void*)
{

}
