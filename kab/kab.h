/* -*- C++ -*-
 * Qt widget for the addressbook.
 * Declaration
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef ADDRESSBOOK_WIDGET_H
#define ADDRESSBOOK_WIDGET_H

class QLineEdit;
class QFrame;
class QToolButton;
class QLabel;
class QComboBox;
class QTimer;

#include "addressbook.h"  
#include <qprinter.h>
#include <qevent.h>
#include <qwidget.h>
class BusinessCard;
class SearchResults;

class AddressWidget : public QWidget, public AddressBook
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  AddressWidget(QWidget* parent=0, const char* name=0, bool readonly=false);
  ~AddressWidget();
  /** This method returns an email address. It is rather the first 
    * address or a user selection, if select is true.
    * The method returns false if there is no entry with the given 
    * key or the entry has no email address. In the latter case, 
    * "address" is not modified.
    * If select is true, the method may also return false if the user 
    * rejected the dialog.
    */
  bool emailAddress(const string& key, string& address, 
		   bool select=true);
  /** currentChanged() will be called by AddressBook-objects if the 
    * currently selected entry has been changed.
    */
  void currentChanged();
  /** updateDB() is called at the program startup. It has to do a
    * difficult task: if the database file the user has used before 
    * has been created by an earlier version of kab, it might 
    * possibly use some fields that have changed its meaning or are 
    * no more supported. updateDB() will update the data in the 
    * database to the new format. It will use the version tag in the 
    * configuration section for this.
    */
  bool updateDB();
  /** sendEmail sends an email to the address given.
   */
  bool sendEmail(const string& address, const string& subject="");
  /** Synchronize selector combobox and addressbook contents. */
  void updateSelector();
  // ----------------------------------------------------------------------------
protected:
  // the child widgets:
  QComboBox* comboSelector;
  QTimer* timer;
  QFrame* frameSeparator1;
  QFrame* frameSeparator2;
  BusinessCard* card;
  QToolButton* buttonFirst; QToolButton* buttonPrevious;
  QToolButton* buttonNext; QToolButton* buttonLast;
  QToolButton* buttonAdd; QToolButton* buttonChange; 
  QToolButton* buttonRemove; QToolButton* buttonSearch;
  // the additional windows
  bool showSearchResults;
  SearchResults *searchResults; // zero if off
  // data elements
  string tmpFile;
  bool readonlyGUI;
  // methods
  void createConnections();
  void createTooltips();
  void enableWidgets();
  bool edit(Entry&);
  // helper methods for printing:
  bool print(QPrinter&, 
	     const list<string>&,
	     const string& headline,
	     const string& footerLeft,
	     const string& footerRight);
  // they both return the height in points:
  int printHeadline(QPainter*, QRect, const string&);
  // page number, text
  int printFooter(QPainter*, QRect, int, 
		  string, string); 
  // CONSTS
  static const unsigned int ButtonSize;
  static const unsigned int Grid;
  // ----------------------------------------------------------------------------
public slots:
  void initializeGeometry();
  void first();
  void previous();
  void next();
  void last();
  void select(int);
  void copy();
  void add(); // a new entry 
  void edit();
  void changed();
  void remove(); // current entry 
  void mail();
  void mail(const char*);
  void browse();
  void talk();
  void save();
  void print();
  void search();
  // ----- exporting methods, Mirko, May 29 98:
  void exportHTML();
  void exportPlain();
  void exportTeXTable();
  void exportTeXLabels();
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  // called if data is dropped over this widget
  void dropEvent(QDropEvent *);
  void searchResultsClose();
  void selectEntry(const char*); // the key
  void setReadonlyGUI(bool state=true);
  void checkFile();
  // ----------------------------------------------------------------------------
signals:
  void entrySelected(int current, int count);
  void sizeChanged();
  void enableMail(bool);
  void enableBrowse(bool);
  void setStatus(const QString&);
  void statusChanged(int noOfEntries);
  // ############################################################################
};

#endif // ADDRESSBOOK_WIDGET_H
