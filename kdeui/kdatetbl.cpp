/* This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998, 1999 Mirko Sucker (mirko@kde.org)
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/////////////////// KDateTable widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998, 1999 Mirko Sucker
// Original header from Tim:
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal: 
//      dateSelected(QDate)


#include <qpainter.h>
#include <qstring.h>
#include <kapp.h>

#include "kdatetbl.h"
#include "kdatetbl.h"

KDateTable::KDateTable(QWidget *parent, QDate date, 
		       const char *name, WFlags f) 
  : QTableView(parent, name, f),
    fontsize(10)
{
  initMetaObject();
  // Mirko, March 17 1998: translate day names
  Days[0]=i18n("Sun"); Days[1]=i18n("Mon"); Days[2]=i18n("Tue"); 
  Days[3]=i18n("Wed"); Days[4]=i18n("Thu"); Days[5]=i18n("Fri"); 
  Days[6]=i18n("Sat");
  // ^^^^^^^^^^^^^^^^^^^^^^^^^
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
  const int w = cellWidth();
  const int h = cellHeight();
  bool bSelected = FALSE;

  if (row==0) 
    { // paint headline
      p->setPen(darkBlue);
      p->setFont(QFont("Arial", fontsize, QFont::Bold, false));
      p->drawText(0, 0, w, h, AlignCenter, Days[col]);
      p->setPen(black);
      p->moveTo(0, h-1);
      p->lineTo(w-1, h-1);
    } else { // paint day cell
      int nDay = dayNum(row, col);
      p->setFont(QFont("Arial", fontsize));
      /* Will be implemented the next time: the preset date
       * will be drawn with a gray background to give feedback 
       * to the user. --Mirko
       {
       int x, y; 
       // calculate row & column of current date:
       // this is missing
       // test if we are displaying the right year and month:
       // this is missing
       // draw the current date with a gray background:
       if(row==y && col==x)
       {
       p->setPen(lightGray);
       p->setBrush(lightGray);
       p->drawRect(0, 0, w, h);
       }
       }
      */
      if (m_bSelection && row==m_selRow && col==m_selCol) 
	{  // item is selected
	  bSelected = TRUE;
	  p->setBrush(darkBlue);
	  p->setPen(red);
	  p->drawEllipse(4,h/2-(w-8)/3,w-8,2*(w-8)/3); // think of better way
	}
      QString day;
      day.sprintf("%i", nDay);
      p->setPen(bSelected? white : black);
      if (nDay > 0 && nDay <= m_date.daysInMonth()) 
	{
	  p->drawText(0, 0, w, h, AlignCenter, day);
	} else if (nDay <= 0) 
	  {
	    int nDayPrv = m_daysInPrevMonth + nDay;
	    day.sprintf("%i", nDayPrv);
	    p->setPen(bSelected? white : gray);
	    p->drawText(0, 0, w, h, AlignCenter, day);
	  } else {
	    int nDayNext = nDay - m_date.daysInMonth();
	    day.sprintf("%i", nDayNext);
	    p->setPen(bSelected? white : gray);
	    p->drawText(0, 0, w, h, AlignCenter, day);
	  }
      if (bSelected && hasFocus()) 
	{
	  if ( style() == WindowsStyle)
	    {
	      p->drawWinFocusRect(1, 1, w-2, h-2);
	    } else {
	      QColorGroup g = colorGroup();
	      p->setPen( black );
	      p->setBrush( NoBrush );
	      p->drawRect(0, 0, w, h
			  /*1, 1, w-2, h-2 */);
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
  bool bDayInMonth =  
    (row > 0 && nDay > 0 && nDay <= m_date.daysInMonth()) 
    ? TRUE : FALSE;
  // -----
  if(bDayInMonth)
    {
      m_selRow = row;
      m_selCol = col;
      // if we clicked on a valid day for the current 
      // month and there is something already
      // selected, then update it
      if (m_bSelection) 
	{ // if something already selected
	  updateCell(m_oldRow, m_oldCol);
	}
      // update new selection
      m_bSelection = TRUE;  // validating selection
      m_date.setYMD(m_date.year(),m_date.month(),nDay);
      updateCell(row, col);
      emit dateSelected(m_date);
      // remember last selected location
      m_oldRow = row;
      m_oldCol = col; 
    } // else .. do nothing
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

/* Mirko: Implementation of sizeHint and setDate methods.
 * March 17 1998
 */

QSize KDateTable::sizeHint() const 
{
  const int Spacing=2; // pixels
  int x=0;
  int y, count, temp;
  for(count=0; count<7; count++)
    {
      temp=fontMetrics().width(Days[count]);
      if(temp>x) x=temp;
    }
  x=7*x+8*Spacing;
  y=7*fontMetrics().height()+7*Spacing;
  debug("KDateTable::sizeHint: recommending %ix%i "
	"pixels.\n", x, y);
  // the widget has 7 rows and 7 columns
  return QSize(x, y);
}

void KDateTable::setDate(QDate date)
{
  if(date.isValid())
    {
      m_date=date; 
      emit dateSelected(m_date);
      repaint(false);
    } else {
      debug("KDateTable::setDate: "
	    "date is invalid, not set.\n");
    }
}

// end of new methods from March 17 1998
/**********************************************************/

// highstick: added May 13 1998
void KDateTable::goDown()
{
  // remember old number of days in month
  m_daysInPrevMonth = m_date.daysInMonth();

	// get next month and set new date and first day of the week
  QDate dtnext;
  getNextYear(m_date, dtnext);
  m_date = dtnext;
  m_firstDayOfWeek = m_date.dayOfWeek();

  m_bSelection = FALSE;

  emit(yearChanged(m_date));
  update();
}

// highstick: added May 13 1998
void KDateTable::goUp()
{
  // get previous month and set new date and first day of the week
  QDate dtprev;
  getPrevYear(m_date, dtprev);
  m_date = dtprev;
  m_firstDayOfWeek = m_date.dayOfWeek();

  // now get the month previous to that and find out number of days
  getPrevYear(m_date, dtprev);
  m_daysInPrevMonth = dtprev.daysInMonth();

  m_bSelection = FALSE;

  emit(yearChanged(m_date));
  update();
}

// highstick: added May 13 1998
// gets a date on the first day of the previous month
void KDateTable::getPrevYear(QDate dtnow, QDate &dtprv)
{
  dtprv = QDate(dtnow.year()-1, dtnow.month(), 1);
}

// highstick: added May 13 1998
// gets a date on the first day of the next month
void KDateTable::getNextYear(QDate dtnow, QDate &dtnxt)
{
  dtnxt = QDate(dtnow.year()+1, dtnow.month(), 1);
}
// end of new methods from May 13 1998 by highstick

/* Mirko: Aug 21 1998 */
void KDateTable::setFontSize(int size)
{
  if(size>0)
    {
      fontsize=size;
    }
}


/**********************************************************/


#include "kdatetbl.moc"

