/* -*- C++ -*-
 * This class displays the search results inside the main view.
 * 
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include "stl_headers.h"
#include <qwidget.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <qframe.h>
#include <qsize.h>

class SearchResults : public QWidget
{
  Q_OBJECT
public:
  SearchResults(QWidget* parent=0, 
		const char* name=0);
  void add(const string& key, const string& name);
  void clear(); // deletes all entries
  QSize sizeHint();
  void select(int);
  int size() { return results.size(); }
protected:
  list<string> results;
  QListBox* listbox;
  QPushButton* buttonClose;
  QFrame* frameHorline;

  void resizeEvent(QResizeEvent*);
  static const int Grid;
signals:
  void closed();
  void entrySelected(const char*);
public slots:
  void closeClicked();
  void selected(int);
};

#endif // SEARCHRESULTS_H
