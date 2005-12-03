/* This file is part of the KDE libraries
    Copyright (C) 1997 Mario Weilguni (mweilguni@sime.com)

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
#include "kbuttonbox.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

int main(int argc, char **argv) {
   KAboutData about("kbuttonboxtest", "kbuttonboxtest", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;

  // example 1
  {
    QDialog *w = new QDialog(0, "A common dialog", true);
    w->setCaption("Example 1");
    QVBoxLayout *tl = new QVBoxLayout(w, 5);
    QLabel *l = new QLabel("A very common dialog\n\n"\
			   "OK and Cancel are left aligned, Help\n"\
			   "is right aligned. Try resizing\n"\
			   "the window!\n"
			   "Press OK or Cancel when done"
			   , w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KButtonBox *bbox = new KButtonBox(w);
    QPushButton *b = bbox->addButton("OK");
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    w->connect(bbox->addButton("Cancel"), SIGNAL(clicked()),
	       w, SLOT(accept()));
    
    bbox->addStretch(1);
    bbox->addButton("Help");
    bbox->layout();
    tl->addWidget(bbox,0);
    tl->activate();
    w->exec();
    delete w;
  }

  // example 2
  {    
    QDialog *w = new QDialog(0, "Right aligned buttons", true);
    w->setCaption("Example 2");
    QVBoxLayout *tl = new QVBoxLayout(w, 5);
    QLabel *l = new QLabel("Another common dialog\n\n"\
			   "OK and Cancel are right aligned\n"\
			   "Try resizing the window!", w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KButtonBox *bbox = new KButtonBox(w);
    bbox->addStretch(1);
    QPushButton *b = bbox->addButton("OK");
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    w->connect(bbox->addButton("Cancel"), SIGNAL(clicked()),
	       w, SLOT(accept()));
    bbox->layout();
    tl->addWidget(bbox,0);
    tl->activate();  
    w->exec();
    delete w;
  }  

  // example 3
  {    
    QDialog *w = new QDialog(0, "Middle aligned buttons", true);
    w->setCaption("Example 3");
    QVBoxLayout *tl = new QVBoxLayout(w, 5);
    QLabel *l = new QLabel("Another common dialog\n\n"\
			   "OK and Cancel are middle aligned\n"\
			   "Try resizing the window!", w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KButtonBox *bbox = new KButtonBox(w);
    bbox->addStretch(1);
    QPushButton *b = bbox->addButton("OK");
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    w->connect(bbox->addButton("Cancel"), SIGNAL(clicked()),
	       w, SLOT(accept()));
    bbox->addStretch(1);
    bbox->layout();
    tl->addWidget(bbox,0);
    tl->activate();  
    w->exec();
    delete w;
  }  

  // example 4
  {    
    QDialog *w = new QDialog(0, "Bad example", true);
    w->setCaption("Example 4");
    QVBoxLayout *tl = new QVBoxLayout(w, 5);
    QLabel *l = new QLabel("A very bad example\n\n"\
			   "Sometimes it's a bad idea to take\n"
			   "the maximum width for all buttons\n"
			   "since it will look ugly -- "
			   "anyway, it works", w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KButtonBox *bbox = new KButtonBox(w);
    bbox->addStretch(1);
    QPushButton *b = bbox->addButton("OK");
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    w->connect(bbox->addButton("Cancel"), SIGNAL(clicked()),
	       w, SLOT(accept()));

    bbox->addButton("Push me and I give you a banana");
    bbox->addStretch(1);
    bbox->layout();
    tl->addWidget(bbox,0);
    tl->activate();  
    w->exec();
    delete w;
  }

  // example 5
  {    
    QDialog *w = new QDialog(0, "Bad example", true);
    w->setCaption("Example 5");
    QVBoxLayout *tl = new QVBoxLayout(w, 5);
    QLabel *l = new QLabel("A better solution for the bad example\n\n"\
			   "Use \"addButton(\"Push me and I give"
			   "you a banana\", true);\" to\n"
			   "prevent this strange effect as seen\n"
			   "the former example!"
			   ,w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KButtonBox *bbox = new KButtonBox(w);
    bbox->addStretch(1);
    QPushButton *b = bbox->addButton("OK");
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    w->connect(bbox->addButton("Cancel"), SIGNAL(clicked()),
	       w, SLOT(accept()));

    bbox->addButton("Push me and I give you a banana", true);
    bbox->addStretch(1);
    bbox->layout();
    tl->addWidget(bbox,0);
    tl->activate();  
    w->exec();
    delete w;
  }

  // example 6
  {    
    QDialog *w = new QDialog(0, "Vertical", true);
    w->setCaption("Example 6 (last)");
    QHBoxLayout *tl = new QHBoxLayout(w, 5);
    QLabel *l = new QLabel("Did I mention that it's possible\n"
			   "to make vertically aligned buttons\n"
			   "too?"
			   ,w);
    l->setAlignment(Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap);
    l->setMinimumSize(l->sizeHint());
    tl->addWidget(l,1);
    KButtonBox *bbox = new KButtonBox(w, Qt::Vertical);
    QPushButton *b = bbox->addButton("OK");
    b->setDefault(true);
    w->connect(b, SIGNAL(clicked()),
	       w, SLOT(accept()));
    w->connect(bbox->addButton("Cancel"), SIGNAL(clicked()),
	       w, SLOT(accept()));

    bbox->addStretch(1);
    bbox->layout();
    tl->addWidget(bbox,0);
    tl->activate();  
    w->exec();
    delete w;
  }

  return 0;
}
