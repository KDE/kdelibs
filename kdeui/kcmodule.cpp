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
#include <kapp.h>


#include "kcmodule.h"


KCModule::KCModule(QWidget *parent, const char *name)
  : QWidget(parent, name), _btn(Help|Default|Reset|Cancel|Apply|Ok)
{
}

void setVisible(QPushButton *btn, bool vis)
{
  if (vis)
    btn->show();
  else
    btn->hide();
}

KCDialog::KCDialog(KCModule *client, const QString &docpath, QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f), _client(client), _docpath(docpath)
{
  client->reparent(this,0,QPoint(0,0),true);

  _sep = new QFrame(this);
  _sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  _sep->show();

  _help = new QPushButton(i18n("Help"), this);
  _default = new QPushButton(i18n("Default"), this);
  _reset = new QPushButton(i18n("Reset"), this);
  _cancel = new QPushButton(i18n("Cancel"), this);
  _apply = new QPushButton(i18n("Apply"), this);
  _ok = new QPushButton(i18n("Ok"), this);

  // only enable the requested buttons
  int b = _client->buttons();
  setVisible(_help, b & KCModule::Help);
  setVisible(_default, b & KCModule::Default);
  setVisible(_reset, b & KCModule::Reset);
  setVisible(_cancel, b & KCModule::Cancel);
  setVisible(_apply, b & KCModule::Apply);
  setVisible(_ok, b & KCModule::Ok);

  // disable initial buttons
  _reset->setEnabled(false);
  _apply->setEnabled(false);

  connect(_help, SIGNAL(clicked()), this, SLOT(helpClicked()));
  connect(_default, SIGNAL(clicked()), this, SLOT(defaultClicked()));
  connect(_reset, SIGNAL(clicked()), this, SLOT(resetClicked()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(_ok, SIGNAL(clicked()), this, SLOT(okClicked()));

  QGridLayout *top = new QGridLayout(this, 4, 6, 5);
  top->addMultiCellWidget(client, 0, 0, 0, 6);
  top->addMultiCellWidget(_sep, 1, 1, 0, 6);
  top->addWidget(_help, 2, 0);
  top->addWidget(_default, 2, 1);
  top->addWidget(_reset, 2, 2);
  top->addWidget(_apply, 2, 4);
  top->addWidget(_ok, 2, 5);
  top->addWidget(_cancel, 2, 6);

  top->setRowStretch(0, 1);
  top->setColStretch(3, 1);

  top->activate();
}


void KCDialog::helpClicked()
{
  if(_docpath != QString::null)
    kapp->invokeHTMLHelp(_docpath, "");
}


void KCDialog::defaultClicked()
{
  _client->defaults();
  clientChanged(true);
}


void KCDialog::resetClicked()
{
  _client->load();
  clientChanged(false);
}


void KCDialog::cancelClicked()
{
  reject();
}


void KCDialog::applyClicked()
{
  _client->save();
  clientChanged(false);
}


void KCDialog::okClicked()
{
  _client->save();
  accept();
}

void KCDialog::clientChanged(bool state)
{
  // enable/disable buttons
  _reset->setEnabled(state);
  _apply->setEnabled(state);
  _ok->setEnabled(state);
}
