/* This file is part of the KDE libraries
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "kbetterthankdialog.h"

KBetterThanKDialog::KBetterThanKDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  connect(_allowOnce, SIGNAL(clicked()), this, SLOT(allowOnceClicked()));
  connect(_allowAlways, SIGNAL(clicked()), this, SLOT(allowAlwaysClicked()));
  connect(_deny, SIGNAL(clicked()), this, SLOT(denyClicked()));
  connect(_denyForever, SIGNAL(clicked()), this, SLOT(denyForeverClicked()));
}

void KBetterThanKDialog::init()
{
    _allowOnce->setFocus();
}

void KBetterThanKDialog::setLabel( const QString & label )
{
    _label->setText(label);
}

void KBetterThanKDialog::accept()
{
    setResult(0);
}

void KBetterThanKDialog::reject()
{
    QDialog::reject();
    setResult(2);
}

void KBetterThanKDialog::allowOnceClicked()
{
    done(0);
}

void KBetterThanKDialog::allowAlwaysClicked()
{
    done(1);
}

void KBetterThanKDialog::denyClicked()
{
    done(2);
}

void KBetterThanKDialog::denyForeverClicked()
{
    done(3);
}

#include "kbetterthankdialog.moc"
