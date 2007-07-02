/* This file is part of the KDE libraries
    Copyright (C) 1997 Mario Weilguni (mweilguni@sime.com)
    Copyright (C) 2006 Olivier Goffart
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
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include "kdialogbuttonbox.h"
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QBoxLayout>

int main(int argc, char **argv) {
   KAboutData about("kdialogbuttonboxtest", 0, ki18n("kbuttonboxtest"), "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;

  // example 1
  {
    QDialog *w = new QDialog;
    w->setObjectName( "A common dialog" );
    w->setModal(true);
    w->setWindowTitle("Example 1");
    QVBoxLayout *tl = new QVBoxLayout(w);
    tl->setMargin(5);
    QLabel *l = new QLabel("A very common dialog\n\n"\
			   "OK and Cancel are left aligned, Help\n"\
			   "is right aligned. Try resizing\n"\
			   "the window!\n"
			   "Press OK or Cancel when done"
			   , w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KDialogButtonBox *bbox = new KDialogButtonBox(w);
    QPushButton *b = bbox->addButton(QLatin1String("OK"), QDialogButtonBox::AcceptRole);
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
   bbox->addButton(QLatin1String("Cancel"),QDialogButtonBox::RejectRole,  w, SLOT(accept()));

   bbox->addButton(QLatin1String("Help"), QDialogButtonBox::HelpRole);
    

    tl->addWidget(bbox,0);
    tl->activate();
    w->exec();
    delete w;
  }


  // example 2
  {    
    QDialog *w = new QDialog(0);
    w->setObjectName("Vertical");
    w->setModal(true);
    w->setWindowTitle("Example 2 ");
    QHBoxLayout *tl = new QHBoxLayout(w);
    tl->setMargin(5);
    QLabel *l = new QLabel("Did I mention that it's possible\n"
			   "to make vertically aligned buttons\n"
			   "too?"
			   ,w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KDialogButtonBox *bbox = new KDialogButtonBox(w, Qt::Vertical);

    QPushButton *b = bbox->addButton(QLatin1String("OK"), QDialogButtonBox::AcceptRole);
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    bbox->addButton(QLatin1String("Cancel"),QDialogButtonBox::RejectRole,  w, SLOT(accept()));

    bbox->addButton(QLatin1String("Help"), QDialogButtonBox::HelpRole);


    tl->addWidget(bbox,0);
    tl->activate();  
    w->exec();
    delete w;
  }

  return 0;
}
