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
#ifndef ADDRESSBOOK_BUSINESSCARD_H
#define ADDRESSBOOK_BUSINESSCARD_H

#include <qwidget.h>
#include <qpixmap.h>
#include "addressbook.h"

class KURLLabel;

class BusinessCard : public QWidget
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  BusinessCard(QWidget* parent=0, const char* name=0);
  virtual ~BusinessCard();
  static const int Grid;
  // ----------------------------------------------------------------------------
protected:
  /** If tile is true, the card is displayed with a tile background,
    * if it is false, the background color is used. The background
    * color defaults to something.
    */
  bool tile;
  string filename; // of the background image
  QColor bgColor;
  QPixmap* background;
  AddressBook::Entry current;
  KURLLabel *urlEmail, *urlHome;
  void paintEvent(QPaintEvent*);
  // ----------------------------------------------------------------------------
public slots:
  void useTile(bool);
  void currentChanged(AddressBook::Entry);
  string getBackground() { return filename; }
  void setBackground(QPixmap*);
  void setBackground(const string&); // path
  void mailURLClicked(const char*);
  void homeURLClicked(const char*);
  // ----------------------------------------------------------------------------
signals:
  void mailURLActivated();
  void homeURLActivated();
  // ############################################################################
};


#endif // ADDRESSBOOK_BUSINESSCARD_H
