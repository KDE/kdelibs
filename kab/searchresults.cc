/* -*- C++ -*-
 * This file implements the dialog for starting a search for 
 * different parameters.
 *
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include <kapp.h>
#include <qtooltip.h>
#include "searchresults.h"
#include "debug.h"

const int SearchResults::Grid=3;

SearchResults::SearchResults
(QWidget* parent,
 const char* name)
  : QWidget(parent, name)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "SearchResults constructor: creating window.\n");
  listbox=new QListBox(this);
  frameHorline=new QFrame(this);
  frameHorline->setFrameStyle(QFrame::HLine 
			      | QFrame::Raised);
  buttonClose=new QPushButton(i18n("Close"), this);
  connect(buttonClose, SIGNAL(clicked()),
	  SLOT(closeClicked()));
  connect(listbox, SIGNAL(highlighted(int)),
	  SLOT(selected(int)));
  QToolTip::add(this, i18n("search results"));
  // ########################################################  
}

void SearchResults::resizeEvent(QResizeEvent*)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "SearchResults::resizeEvent: "
     "setting position of subwidgets.\n");
  int cy=0;
  int listboxheight=5*(listbox->itemHeight()+1);
  frameHorline->setGeometry(0, cy, 
			    width(), Grid);
  cy+=2*Grid;	   
  listbox->setGeometry(0, cy, width(), listboxheight);
  cy+=listboxheight+Grid;
  buttonClose->setGeometry(width()-
			   buttonClose->sizeHint().width(),
			   cy, 
			   buttonClose->sizeHint().width(),
			   buttonClose->sizeHint().height());
  cy+=buttonClose->sizeHint().height();
  // ########################################################
}

QSize SearchResults::sizeHint()
{
  ID(bool GUARD=false);
  // ########################################################
  int x=listbox->maxItemWidth();
  int y=5*(listbox->itemHeight()+1)
    +Grid+buttonClose->sizeHint().height()
    +2*Grid;
  LG(GUARD, "SearchResults::sizeHint: %ix%i pixels.\n",
     x, y);
  return QSize(x, y);
  // ########################################################
}

void SearchResults::closeClicked()
{
  // ########################################################
  emit(closed());
  // ########################################################
}

void SearchResults::selected(int index)
{
  // ########################################################
  // this implementation is slow I suppose
  REQUIRE(index>=0);
  CHECK((unsigned)index<listbox->count());
  list<string>::iterator pos=results.begin();
  advance(pos, index);
  emit(entrySelected((*pos).c_str()));
  // ########################################################
}

void SearchResults::select(int index)
{
  // ########################################################
  listbox->setCurrentItem(index);
  // ########################################################
}

void SearchResults::add
(const string& key, 
 const string& name)
{
  // ########################################################
  listbox->insertItem(name.c_str());
  results.push_back(key);
  // ########################################################
}

void SearchResults::clear()
{
  // ########################################################
  listbox->clear();
  results.erase(results.begin(), results.end());
  CHECK(results.empty() && listbox->count()==0);
  // ########################################################
}

#include "searchresults.moc"
