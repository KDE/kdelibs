/* -*- C++ -*-
 * Businesscard widget for the addressbook example.
 * Declaration.
 *
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, 
 *             NANA (only for debugging)
 * $Revision$
 */
#ifndef QADDRESSBOOK_BUSINESSCARD_H
#define QADDRESSBOOK_BUSINESSCARD_H

#include <qwidget.h>
#include <qpixmap.h>
#include "addressbook.h"

class BusinessCard : public QWidget
{
  Q_OBJECT
public:
  BusinessCard(QWidget* parent=0, const char* name=0);
  virtual ~BusinessCard();
  static const int Grid;
protected:
  string filename; // of the background image
  QPixmap* background;
  AddressBook::Entry current;
  void paintEvent(QPaintEvent*);
public slots:
  void currentChanged(AddressBook::Entry);
  string getBackground() { return filename; }
  void setBackground(QPixmap*);
  void setBackground(const string&); // path
};


#endif // QADDRESSBOOK_BUSINESSCARD_H
