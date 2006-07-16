/* This file is part of the KDE libraries
    Copyright (C) 1997 Mario Weilguni (mweilguni@sime.com)
    Copyright (C) 2006 Olivier Goffart <ogoffart@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdialogbuttonbox.h"

#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <QList>


KDialogButtonBox::KDialogButtonBox(QWidget *parent, Qt::Orientation _orientation)
  :  QDialogButtonBox(_orientation, parent) , d(0)
{
}

KDialogButtonBox::~KDialogButtonBox() {}

QPushButton *KDialogButtonBox::addButton(const QString & text, ButtonRole role, QObject *receiver,  const char *slot)
{
  QPushButton * pb = addButton(text,role);

  if (pb && receiver && slot)
    QObject::connect(pb, SIGNAL(clicked()), receiver, slot);

  return pb;
}

KPushButton *KDialogButtonBox::addButton(  const KGuiItem& guiitem, ButtonRole role, QObject *receiver,  const char *slot)
{
  KPushButton *pb=new KPushButton(guiitem, this);
  QDialogButtonBox::addButton(static_cast<QAbstractButton*>(pb),role);

  if (receiver && slot)
    QObject::connect(pb, SIGNAL(clicked()), receiver, slot);

  return pb;
}

#include "kdialogbuttonbox.moc"
