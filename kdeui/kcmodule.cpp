/*

   This file is part of the KDE libraries

   Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/  


#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>


#include <klocale.h>


#include "kcmodule.h"


KCModule::KCModule(QWidget *parent, const char *name)
  : QWidget(parent, name), _btn(Help|Default|Reset|Cancel|Apply|Ok)
{
}


  
KCDialog::KCDialog(KCModule *client, QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f), _client(client)
{
  client->reparent(this,0,QPoint(0,0),true);

  _buttons = new QWidget(this);

  _sep = new QFrame(this);
  _sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  _sep->show();

  QHBoxLayout *box = new QHBoxLayout(_buttons, 2, 2);

  _help = new QPushButton(i18n("Help"), _buttons);
  box->addWidget(_help);
  _default = new QPushButton(i18n("Default"), _buttons);
  box->addWidget(_default);
  _reset = new QPushButton(i18n("Reset"), _buttons);
  box->addWidget(_reset);
  box->addStretch();
  _cancel = new QPushButton(i18n("Cancel"), _buttons);
  box->addWidget(_cancel);
  _apply = new QPushButton(i18n("Apply"), _buttons);
  box->addWidget(_apply);
  _ok = new QPushButton(i18n("Ok"), _buttons);
  box->addWidget(_ok);

  // only enable the requested buttons
  int b = _client->buttons();
  _help->setEnabled(b & KCModule::Help);
  _default->setEnabled(b & KCModule::Default);
  _reset->setEnabled(b & KCModule::Reset);
  _cancel->setEnabled(b & KCModule::Cancel);
  _apply->setEnabled(b & KCModule::Apply);
  _ok->setEnabled(b & KCModule::Ok);

  _buttons->setFixedHeight(_help->sizeHint().height()+4);
  resizeEvent(0);
  _buttons->show();

  connect(_help, SIGNAL(clicked()), this, SLOT(helpClicked()));
  connect(_default, SIGNAL(clicked()), this, SLOT(defaultClicked()));
  connect(_reset, SIGNAL(clicked()), this, SLOT(resetClicked()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(_ok, SIGNAL(clicked()), this, SLOT(okClicked()));
}


void KCDialog::helpClicked()
{
  //TODO
}


void KCDialog::defaultClicked()
{
  _client->defaults();
}


void KCDialog::resetClicked()
{
  _client->load();
}


void KCDialog::cancelClicked()
{
  reject();
}


void KCDialog::applyClicked()
{
  _client->save();
}


void KCDialog::okClicked()
{
  _client->save();
  accept();
}


void KCDialog::resizeEvent(QResizeEvent *event)
{
  QDialog::resizeEvent(event);

  _sep->setGeometry(0,height()-_buttons->height()-2,width(),2);
  _buttons->setGeometry(0,height()-_buttons->height(),width(),_buttons->height());
  _client->setGeometry(0,0,width(),height()-_buttons->height()-2);
}


