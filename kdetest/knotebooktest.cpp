/*
 *   knotebooktest - a test program for the KNoteBook dialog
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

#include <qlabel.h>
#include <kapp.h>
#include <knotebook.h>

int main(int argc, char **argv)
{
  KApplication a(argc,argv,"knotebooktest");  
  KNoteBook *nb = new KNoteBook();

  // /* remove the "//" comments to compile without bells'n'whistles
  nb->setDirectionsReflectsPage(true);
  nb->setEnablePopupMenu(true);
  nb->setEnableArrowButtons(true);
  nb->setOkButton();
  nb->setCancelButton();
  nb->setDefaultButton();
  nb->setHelpButton();
  QObject::connect( nb, SIGNAL(okclicked()), &a, SLOT(quit()) );
  QObject::connect( nb, SIGNAL(cancelclicked()), &a, SLOT(quit()) );
  // */ remove the "//" comments to compile without bells'n'whistles

  for(int i = 1; i < 11; i++)
  {
    QTab *tab = new QTab;
    tab->label.setNum(i);
    tab->label += ". tab";
    tab->enabled = true;
    nb->addTab( tab );
    for(int j = 1; j <= i; j++)
    {
      QString msg;
      msg.sprintf("This is page %d\nin a section with %d pages",
                   j, i);
      QLabel *l = new QLabel(nb);
      l->setAlignment(Qt::AlignCenter);
      l->setText(msg.data());
      l->setMinimumSize(300, 200);
      KWizardPage *p = new KWizardPage;
      p->w = l;
      p->title.setNum(j);
      p->title += ". page";
      p->enabled = true;
      nb->addPage( p );
    }
  }
/*
  nb->setTabEnabled(0, false); // disable tab with id 5
  nb->setTabEnabled(9, false); // disable tab with id 5
  nb->setPageEnabled(3, 3, false); // disable page with id 1 in section with id 3
*/
  a.setMainWidget(nb);
  nb->adjustSize();
  nb->show();
  //nb->getTabBar()->move(-50, 2);
  return a.exec();
}


