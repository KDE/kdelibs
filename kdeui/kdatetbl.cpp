/*  -*- C++ -*-
    This file is part of the KDE libraries
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
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal: 
//      dateSelected(QDate)

#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include "kdatepik.h"
#include "kdatetbl.h"
#include <qdatetime.h>
#include <qstring.h>
#include <qpen.h>
#include <qpainter.h>
#include <qdialog.h>

#include "kdatetbl.moc"

KDateValidator::KDateValidator(QWidget* parent, const char* name)
  : QValidator(parent, name)
{
}

QValidator::State
KDateValidator::validate(QString& text, int&) const
{
  QDate temp;
  // ----- everything is tested in date():
  return date(text, temp);
}

QValidator::State
KDateValidator::date(const QString& text, QDate& d) const
{
  int yearwidth, y, m, day;
  unsigned int count;
  QString temp;
  bool ok;
  // -----  
  /* Input is expected to be in the following form:
     19990816.
     If 990816 is entered, 1999 is expected. */
  for(count=0; count<text.length(); ++count)
    {
      if(!text.at(count).isDigit())
	{
	  debug("KDateValidator::validate: non-digit character entered.");
	  kapp->beep();
	  return Invalid;
	}
    }
  switch(text.length())
    {
    case 8:
      yearwidth=4;
      break;
    case 6:
      yearwidth=2;
      break;
    default:
      return Valid;
    }
  temp=text.mid(0, yearwidth);
  y=temp.toInt(&ok);
  if(!ok) 
    {
      return Valid;
    }
  if(yearwidth==2)
    {
      y+=1900;
    }
  temp=text.mid(yearwidth, 2);
  m=temp.toInt(&ok);
  if(!ok) 
    {
      return Valid;
    }
  temp=text.mid(yearwidth+2, 2);
  day=temp.toInt(&ok);
  if(!ok) 
    {
      return Valid;
    }
  // ----- now y, m and d hold the numeric values
  //       lets see if this gives a valid date
  if(d.setYMD(y, m, day))
    {
      return Acceptable;
    } else {
      debug("KDateValidator::date: invalid date %i/%i/%i.", y, m, day);
      return Valid;
    }
}

KDateTable::KDateTable(QWidget *parent, QDate date_, const char* name, WFlags f)
  : QTableView(parent, name, f),
    hasSelection(false)
{
  setFontSize(10);
  if(!date_.isValid())
    {
      debug("KDateTable ctor: WARNING: Given date is invalid, using current date.");
      date_=QDate::currentDate();
    }
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  // setTableFlags(Tbl_clipCellPainting); // enable to find drawing failures
  setBackgroundColor(lightGray);
  setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth
  Days[0]=i18n("Sun"); Days[1]=i18n("Mon"); Days[2]=i18n("Tue"); 
  Days[3]=i18n("Wed"); Days[4]=i18n("Thu"); Days[5]=i18n("Fri"); 
  Days[6]=i18n("Sat");
}

void
KDateTable::paintCell(QPainter *painter, int row, int col)
{
  QRect rect;
  QString text;
  QPen pen;
  int w=cellWidth();
  int h=cellHeight();
  int pos;
  QBrush brushBlue(blue);
  QBrush brushLightblue(lightGray);
  QFont font=KGlobal::generalFont();
  // -----
  font.setPointSize(fontsize);
  if(row==0)
    { // we are drawing the headline
      font.setBold(true);
      painter->setFont(font);
      switch(col)
	{
	case 5: // saturday
	case 6: // sunday
	  painter->setPen(lightGray);
	  painter->setBrush(brushLightblue);
	  painter->drawRect(0, 0, cellWidth(), cellHeight());
	  painter->setPen(blue);
	  painter->drawText(0, 0, w, h-1, AlignCenter, 
			    date.dayName(col+1), -1, &rect);
	  break;
	default: // normal weekday
	  painter->setPen(blue);
	  painter->setBrush(brushBlue);
	  painter->drawRect(0, 0, w, h);
	  painter->setPen(white);
	  painter->drawText(0, 0, w, h-1, AlignCenter, 
			    date.dayName(col+1), -1, &rect);
	};
      painter->setPen(black);
      painter->moveTo(0, h-1);
      painter->lineTo(w-1, h-1);
      // ----- draw the weekday:
    } else {
      painter->setFont(font);
      pos=7*(row-1)+col+1;
      if(pos<firstday || (firstday+numdays<=pos))
	{ // we are either 
	  // ° painting a day of the previous month or
	  // ° painting a day of the following month
	  if(pos<firstday)
	    { // previous month
	      text.setNum(numDaysPrevMonth+pos-firstday+1);
	    } else { // following month
	      text.setNum(pos-firstday-numdays+1);
	    }
	  painter->setPen(gray);
	} else { // paint a day of the current month
	  text.setNum(pos-firstday+1);
	  painter->setPen(black);
	}
      pen=painter->pen();
      if(firstday+date.day()-1==pos)
	{
	  if(hasSelection)
	    { // draw the currently selected date
	      painter->setPen(red);
	      painter->setBrush(darkRed);
	      pen=white;
	    } else {
	      painter->setPen(darkGray);
	      painter->setBrush(darkGray);
	      pen=white;
	    }
	} else {
	  painter->setBrush(lightGray);
	  painter->setPen(lightGray);
	}
      painter->drawRect(0, 0, w, h);
      painter->setPen(pen);
      painter->drawText(0, 0, w, h, AlignCenter, text, -1, &rect);
    }
  if(rect.width()>maxCell.width()) maxCell.setWidth(rect.width());
  if(rect.height()>maxCell.height()) maxCell.setHeight(rect.height());
}

void
KDateTable::resizeEvent(QResizeEvent *)
{
  setCellWidth(width()/7);
  setCellHeight(height()/7);
}

void
KDateTable::setFontSize(int size)
{
  int count;
  QFontMetrics metrics(fontMetrics());
  QRect rect;
  // ----- store rectangles:
  fontsize=size;
  // ----- find largest day name:
  maxCell.setWidth(0);
  maxCell.setHeight(0);
  for(count=0; count<7; ++count)
    {
      rect=metrics.boundingRect(Days[count]);
      maxCell.setWidth(QMAX(maxCell.width(), rect.width()));
      maxCell.setHeight(QMAX(maxCell.height(), rect.height()));
    }
  // ----- compare with a real wide number and add some space:
  rect=metrics.boundingRect("88");
  maxCell.setWidth(QMAX(maxCell.width()+2, rect.width()));
  maxCell.setHeight(QMAX(maxCell.height()+4, rect.height()));
}

void
KDateTable::mousePressEvent(QMouseEvent *e)
{
  if(e->type()!=QEvent::MouseButtonPress)
    { // the KDatePicker only reacts on mouse press events:
      return;
    }
  if(!isEnabled())
    {
      kapp->beep();
      return;
    }
  // -----
  int row, col, pos, temp;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=findRow(mouseCoord.y());
  col=findCol(mouseCoord.x());
  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      return;
    }
  pos=7*(row-1)+col+1;
  if(pos<firstday)
    { // this day is in the previous month
      kapp->beep();
      return;
    }
  if(firstday+numdays<=pos)
    { // this date is in the next month
      kapp->beep();
      return;
    }
  if(hasSelection)
    { // clear the old selected cell
      hasSelection=false;
      temp=firstday+date.day()-1;
      updateCell(temp/7, temp%7, false);
    }
  hasSelection=true;
  updateCell(row, col, false);
  // assert(QDate(date.year(), date.month(), pos-firstday+1).isValid());
  setDate(QDate(date.year(), date.month(), pos-firstday+1));
  emit(tableClicked());
}

bool
KDateTable::setDate(const QDate& date_)
{
  bool changed=false;
  QDate temp;
  // -----
  if(!date_.isValid())
    {
      debug("KDateTable::setDate: refusing to set invalid date.");
      return false;
    }
  if(date!=date_)
    {
      date=date_;
      changed=true;
    }
  temp.setYMD(date.year(), date.month(), 1);
  firstday=temp.dayOfWeek();
  if(firstday==1) firstday=8;
  numdays=date.daysInMonth();
  if(date.month()==1)
    { // set to december of previous year
      temp.setYMD(date.year()-1, 12, 1);
    } else { // set to previous month
      temp.setYMD(date.year(), date.month()-1, 1);
    }
  numDaysPrevMonth=temp.daysInMonth();
  if(changed)
    {
      repaint(false);
    }
  emit(dateChanged(date));
  return true;
}

const QDate&
KDateTable::getDate()
{
  return date;
}

QSize
KDateTable::sizeHint() const 
{
  if(maxCell.height()>0 && maxCell.width()>0)
    {
      return QSize(maxCell.width()*numCols()+2*frameWidth(), 
		   (maxCell.height()+2)*numRows()+2*frameWidth());
    } else {
      debug("KDateTable::sizeHint: obscure failure - "
	    "constructor not called for this object?");
      return QSize(-1, -1);
    }
}

KDateInternalMonthPicker::KDateInternalMonthPicker
(int fontsize, QWidget* parent, const char* name)
  : QTableView(parent, name),
    result(0) // invalid
{
  int temp;
  QRect rect;
  QFont font;
  // -----
  font=KGlobal::generalFont();
  font.setPointSize(fontsize);    
  setFont(font);
  setNumRows(4); 
  setNumCols(3);
  // enable to find drawing failures:
  // setTableFlags(Tbl_clipCellPainting); 
  setBackgroundColor(lightGray); // for consistency with the datepicker
  // ----- find the preferred size
  //       (this is slow, possibly, but unfortunatly it is needed here):
  QFontMetrics metrics(font);
  for(temp=1; temp<13; ++temp)
    {
      rect=metrics.boundingRect(*(KDatePicker::Month[temp-1]));
      if(max.width()<rect.width()) max.setWidth(rect.width());
      if(max.height()<rect.height()) max.setHeight(rect.height());
    }
  
}

QSize
KDateInternalMonthPicker::sizeHint() const
{
  return QSize((max.width()+6)*numCols()+2*frameWidth(),
	       (max.height()+6)*numRows()+2*frameWidth());
}

int 
KDateInternalMonthPicker::getResult()
{
  return result;
}

void 
KDateInternalMonthPicker::setupPainter(QPainter *p)
{
  p->setPen(black);
}

void 
KDateInternalMonthPicker::resizeEvent(QResizeEvent*)
{
  setCellWidth(width()/3);
  setCellHeight(height()/4);
}

void 
KDateInternalMonthPicker::paintCell(QPainter* painter, int row, int col)
{
  int index;
  QString text;
  // ----- find the number of the cell:
  index=3*row+col+1;
  text=*(KDatePicker::Month[index-1]);
  painter->drawText(0, 0, cellWidth(), cellHeight(), AlignCenter, text);
}

void 
KDateInternalMonthPicker::mousePressEvent(QMouseEvent *e)
{
  if(e->type()!=QEvent::MouseButtonPress)
    { // we only react on mouse press events:
      return;
    }
  if(!isEnabled())
    {
      kapp->beep();
      return;
    }
  // -----
  int row, col, pos;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=findRow(mouseCoord.y());
  col=findCol(mouseCoord.x());
  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      emit(closeMe(0));
    }
  pos=3*row+col+1;
  result=pos;
  emit(closeMe(1));
}

KDateInternalYearSelector::KDateInternalYearSelector
(int fontsize, QWidget* parent, const char* name)
  : QLineEdit(parent, name),
    val(new QIntValidator(this)),
    result(0)
{
  QFont font;
  // -----
  font=KGlobal::generalFont();
  font.setPointSize(fontsize);    
  setFont(font);
  // we have to respect the limits of QDate here, I fear:
  val->setRange(0, 8000); 
  setValidator(val);
  connect(this, SIGNAL(returnPressed()), SLOT(yearEnteredSlot()));
}

void 
KDateInternalYearSelector::yearEnteredSlot()
{
  bool ok;
  int year;
  QDate date;
  // ----- check if this is a valid year: 
  year=text().toInt(&ok);
  if(!ok)
    {
      kapp->beep();
      return;
    }
  date.setYMD(year, 1, 1);
  if(!date.isValid())
    {
      kapp->beep();
      return;
    }
  result=year;
  emit(closeMe(1));
}  

int 
KDateInternalYearSelector::getYear()
{
  return result;
}

void 
KDateInternalYearSelector::setYear(int year)
{
  QString temp;
  // -----
  temp.setNum(year);
  setText(temp);
}

QPopupFrame::QPopupFrame(QWidget* parent, const char*  name)
  : QFrame(parent, name, WType_Popup),
    result(0), // rejected
    main(0)
{
  setFrameStyle(QFrame::Box|QFrame::Raised);
  setMidLineWidth(2);
}

void 
QPopupFrame::keyPressEvent(QKeyEvent* e)
{
  if(e->key()==Key_Escape)
    {
      result=0; // rejected
      kapp->exit_loop();
    }
}

void 
QPopupFrame::close(int r)
{
  result=r;
  kapp->exit_loop();
}

void 
QPopupFrame::setMainWidget(QWidget* m)
{
  main=m;
  if(main!=0)
    {
      resize(main->width()+2*frameWidth(), main->height()+2*frameWidth());
    }
}

void 
QPopupFrame::resizeEvent(QResizeEvent*)
{
  if(main!=0)
    {
      main->setGeometry
	(frameWidth(), frameWidth(), 
	 width()-2*frameWidth(), height()-2*frameWidth());
    }
}

int 
QPopupFrame::exec(QPoint p)
{
  return exec(p.x(), p.y());
}

int 
QPopupFrame::exec(int x, int y)
{
  move(x, y);
  show();
  repaint();
  qApp->enter_loop();
  hide();
  return result;
}

