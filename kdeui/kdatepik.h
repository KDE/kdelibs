#ifndef _KDATEPIK_H
#define _KDATEPIK_H

/////////////////// KDatePicker widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
// This version of the class is the first release.  Please
// send comments/suggestions/bug reports to <tdgilman@best.com>
//
// Written using Qt (http://www.troll.no) for the 
// KDE project (http://www.kde.org)
//
//
// Use this class to make a date picker widget
// When a date is selected by the user, it emits a signal: dateSelected(QDate)
//
// Required header for use:
//    kdatepik.h
// 
// Required files for compilation:
//
//    kdatepik.h kdatepik.cpp kdatetbl.h kdatetbl.cpp


#include <qframe.h>
#include <qdatetm.h>

class KDateTable;
class QLabel;
class QPushButton;
class QDate;

/** 
* Use this class to make a date picker widget.
*
* When a date is selected by the user, it emits a signal: dateSelected(QDate)
*
* @short A widget for selecting dates.
* @author Tim Gilman (tdgilman@best.com)
* @version $Id$
*/
class KDatePicker : public QFrame {
   Q_OBJECT
 public:
   KDatePicker(QWidget *parent=0, QDate=QDate::currentDate(), const char *name=0);

 private:
   KDateTable *m_tbl;
   QLabel *m_header;
   QLabel *m_footer;
   QPushButton *m_back;
   QPushButton *m_forward;

 protected:
   void resizeEvent(QResizeEvent *);
   void sizeElements();

 signals:
   void dateSelected(QDate);

 private slots:

 public slots:
   void updateHeader(QDate dt);

};

#endif // _KDATEPIK_H
