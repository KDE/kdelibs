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


#include <qpainter.h>
#include <qstring.h>

#include "kdatetbl.h"
#include "kdatetbl.moc"

char *day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

KDateTable::KDateTable(QWidget *parent, QDate date, const char *name, WFlags f) 
   : QTableView(parent, name, f)
{
   initMetaObject();
   
   setBackgroundColor(white);

   setNumRows(7);
   setNumCols(7);
   setTableFlags(Tbl_clipCellPainting);

   QRect rec = contentsRect();
   setCellWidth(rec.width()/7);
   setCellHeight(rec.height()/7);
   
   m_oldRow = m_oldCol = 0;
   m_selRow = m_selCol = 0;
   m_bSelection = FALSE;

   setFocusPolicy(StrongFocus);

   // initialize date
   m_date = date;

   // get the day of the week on the first day
   QDate dayone(m_date.year(), m_date.month(), 1);
   m_firstDayOfWeek = dayone.dayOfWeek();

   // determine number of days in previous month
   QDate prvmonth;
   getPrevMonth(m_date, prvmonth);   
   m_daysInPrevMonth = prvmonth.daysInMonth();
}

KDateTable::~KDateTable()
{
}

void KDateTable::paintCell( QPainter *p, int row, int col )
{
   int w = cellWidth();
   int h = cellHeight();
   bool bSelected = FALSE;

   if (0==row) {
      p->setPen(darkBlue);
      p->setFont(QFont("Arial", 12, QFont::Bold, TRUE));
      p->drawText(0, 0, w, h, AlignCenter, day[col]);
      p->setPen(black);
      p->moveTo(0, h-1);
      p->lineTo(w-1, h-1);
   }
   else {
      int nDay = dayNum(row, col);
      
      p->setFont(QFont("Arial", 12));
      if (m_bSelection && row==m_selRow && col==m_selCol) {  // item is selected
	 bSelected = TRUE;
	 p->setBrush(darkBlue);
	 p->setPen(red);
	 p->drawEllipse(4,h/2-(w-8)/3,w-8,2*(w-8)/3); // think of better way
      }
      
      QString day;
      day.sprintf("%i", nDay);
      p->setPen(bSelected? white : black);
      if (nDay > 0 && nDay <= m_date.daysInMonth()) {
	 p->drawText(0, 0, w, h, AlignCenter, day);
      }
      else if (nDay <= 0) {
	 int nDayPrv = m_daysInPrevMonth + nDay;
	 day.sprintf("%i", nDayPrv);
	 p->setPen(bSelected? white : gray);
	 p->drawText(0, 0, w, h, AlignCenter, day);
      }
      else {
	 int nDayNext = nDay - m_date.daysInMonth();
	 day.sprintf("%i", nDayNext);
	 p->setPen(bSelected? white : gray);
	 p->drawText(0, 0, w, h, AlignCenter, day);
      }
      
      if (bSelected && hasFocus()) {
	 if ( style() == WindowsStyle)
	   p->drawWinFocusRect(1, 1, w-2, h-2);
	 else {
	    QColorGroup g = colorGroup();
	    p->setPen( black );
	    p->setBrush( NoBrush );
	    p->drawRect( 1, 1, w-2, h-2 );
	 }
      }
//      m_bSelected = FALSE;
   }
}

void KDateTable::mousePressEvent(QMouseEvent *e)
{
   if (e->type() != Event_MouseButtonPress)
     return;
   
   int row, col;
   
   QPoint mouseCoord = e->pos();
   row = findRow(mouseCoord.y());
   col = findCol(mouseCoord.x());

   if (row > 0)
     setSelection(row, col);
   
}

void KDateTable::setSelection(int row, int col)
{
   int nDay = dayNum(row, col);
   bool bDayInMonth =  row > 0 && nDay > 0 && nDay <= m_date.daysInMonth() ? TRUE : FALSE;
   
   if (!bDayInMonth)  // do nothing
     return;
   
   if (bDayInMonth) {
      m_selRow = row;
      m_selCol = col;
   } 
   
   // if we clicked on a valid day for the current month and there is something already
   // selected, then update it
   if (m_bSelection && bDayInMonth) { // if something already selected
      updateCell(m_oldRow, m_oldCol);
   }
   
   // update new selection
   if (bDayInMonth) {
      m_bSelection = TRUE;  // validating selection
      m_date.setYMD(m_date.year(),m_date.month(),nDay);
      updateCell(row, col);
      emit dateSelected(m_date);
   }
   
   // remember last selected location
   m_oldRow = row;
   m_oldCol = col; 
}

void KDateTable::resizeEvent( QResizeEvent *)
{
   QRect rec = contentsRect();
   setCellWidth(rec.width()/7);
   setCellHeight(rec.height()/7);
}

void KDateTable::goForward()
{
   // remember old number of days in month
   m_daysInPrevMonth = m_date.daysInMonth();
   
   // get next month and set new date and first day of the week
   QDate dtnext;
   getNextMonth(m_date, dtnext);
   m_date = dtnext;
   m_firstDayOfWeek = m_date.dayOfWeek();
   
   m_bSelection = FALSE;
   
   emit monthChanged(m_date);
   update();
}

void KDateTable::goBackward()
{
   // get previous month and set new date and first day of the week
   QDate dtprev;
   getPrevMonth(m_date, dtprev);
   m_date = dtprev;
   m_firstDayOfWeek = m_date.dayOfWeek();
   
   // now get the month previous to that and find out number of days
   getPrevMonth(m_date, dtprev);
   m_daysInPrevMonth = dtprev.daysInMonth();
   
   m_bSelection = FALSE;
   
   emit monthChanged(m_date);
   update();
}

// gets a date on the first day of the previous month
void KDateTable::getPrevMonth(QDate dtnow, QDate &dtprv)
{
   int month = dtnow.month() > 1 ? dtnow.month()-1 : 12;
   int year = dtnow.month() > 1 ? dtnow.year() : dtnow.year()-1;
   dtprv = QDate(year, month, 1);
}

// gets a date on the first day of the next month
void KDateTable::getNextMonth(QDate dtnow, QDate &dtnxt)
{
   int month = dtnow.month() < 12 ? dtnow.month()+1 : 1;
   int year = dtnow.month() < 12 ? dtnow.year() : dtnow.year()+1;
   dtnxt = QDate(year, month, 1);
}



int KDateTable::dayNum(int row, int col)
{
   return 7*row - 7 + 1 + col - m_firstDayOfWeek;
}
