/*
 *   kwizardtest - a test program for the KWizard dialog
 *   Copyright (C) 1998  Thomas Tanghus (tanghus@kde.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <kwizard.h>

int main(int argc, char **argv)
{
  KApplication a(argc,argv,"kwizardtest");
  KWizard *wiz = new KWizard(0, "kwizardtest", false);
  // /* remove the "//" comments to compile without bells'n'whistles
  wiz->setCancelButton();
  wiz->setOkButton();
  wiz->setDefaultButton();
  wiz->setHelpButton();
  wiz->setEnablePopupMenu(true);
  wiz->setEnableArrowButtons(true);
  wiz->setDirectionsReflectsPage(true);
  QObject::connect( wiz, SIGNAL(okclicked()), &a, SLOT(quit()) );
  QObject::connect( wiz, SIGNAL(cancelclicked()), &a, SLOT(quit()) );

  // */ remove the "//" comments to compile without bells'n'whistles
  for(int i = 1; i < 11; i++)
  {
    QString msg;
    msg.sprintf("This is page %d out of 10", i);
    QLabel *l = new QLabel(wiz);
    l->setAlignment(Qt::AlignCenter);
    l->setText(msg.data());
    l->setMinimumSize(300, 200);
    QString title;
    title.setNum(i);
    title += ". page";
    KWizardPage *p = new KWizardPage;
    p->w = l;
    p->title = title.copy();
    p->enabled = true;
    wiz->addPage(p);
  }
  a.setMainWidget(wiz);
  wiz->adjustSize();
  wiz->show();
  return a.exec();
}



