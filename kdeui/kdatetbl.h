#ifndef _KDATETBL_H 
#define _KDATETBL_H

/////////////////// KDateTable widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal: dateSelected(QDate)


#include <qtablevw.h>
#include <qdatetm.h>

class KDateTable: public QTableView {
   Q_OBJECT
 public:
   KDateTable(QWidget *parent=0, QDate date=QDate::currentDate(), const char *name=0, WFlags f=0);
   ~KDateTable();

 public slots:
   void goForward();
   void goBackward();

 signals:
   void monthChanged(QDate);
   void dateSelected(QDate);

 protected:
   void paintCell( QPainter *p, int row, int col );
   void resizeEvent( QResizeEvent * );
   void mousePressEvent(QMouseEvent *e);

 private:
   QDate m_date;
   int m_firstDayOfWeek;
   int m_daysInPrevMonth;
   int m_oldRow;
   int m_oldCol;
   bool m_bSelection;
   int m_selRow;
   int m_selCol;

   void setSelection(int row, int col);
   void getPrevMonth(QDate dtnow, QDate &dtprv);
   void getNextMonth(QDate dtnow, QDate &dtnxt);
   int dayNum(int row, int col);

 private:  // Disabled copy constructor and operator=
   KDateTable(const KDateTable & ) {}
   KDateTable &operator=(const KDateTable &) { return *this; }
};

#endif _KDATETBL_H
