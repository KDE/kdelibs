/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
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
//           (C) 1998-2001 Mirko Boehm
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
#include <kglobalsettings.h>
#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>
#include <knotifyclient.h>
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
  QDate tmp = KGlobal::locale()->readDate(text);
  if (!tmp.isNull()) 
    {
      d = tmp;
      return Acceptable;
    } else
      return Valid;
}

void 
KDateValidator::fixup( QString& ) const
{

}

KDateTable::KDateTable(QWidget *parent, QDate date_, const char* name, WFlags f)
  : QTableView(parent, name, f),
    hasSelection(false)
{
  setFontSize(10);
  if(!date_.isValid())
    {
      kdDebug() << "KDateTable ctor: WARNING: Given date is invalid, using current date." << endl;
      date_=QDate::currentDate();
    }
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  // setTableFlags(Tbl_clipCellPainting); // enable to find drawing failures
  setBackgroundColor(lightGray);
  setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth
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
  QFont font=KGlobalSettings::generalFont();
  // -----
  font.setPointSize(fontsize);
  if(row==0)
    { // we are drawing the headline
      font.setBold(true);
      painter->setFont(font);
      bool normalday = true;
      QString daystr;
      if (KGlobal::locale()->weekStartsMonday()) 
        {
          daystr = KGlobal::locale()->weekDayName(col+1, true);
          if (col == 5 || col == 6)
              normalday = false;
        } else {
          daystr = KGlobal::locale()->weekDayName(col==0? 7 : col, true);
          if (col == 0 || col == 6)
              normalday = false;
        }
      if (!normalday) 
        {
          painter->setPen(lightGray);
          painter->setBrush(brushLightblue);
          painter->drawRect(0, 0, w, h);
          painter->setPen(blue);
        } else {
          painter->setPen(blue);
          painter->setBrush(brushBlue);
          painter->drawRect(0, 0, w, h);
          painter->setPen(white);
        }
      painter->drawText(0, 0, w, h-1, AlignCenter,
                        daystr, -1, &rect);
      painter->setPen(black);
      painter->moveTo(0, h-1);
      painter->lineTo(w-1, h-1);
      // ----- draw the weekday:
    } else {
      painter->setFont(font);
      pos=7*(row-1)+col;
      if (KGlobal::locale()->weekStartsMonday())
          pos++;
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
KDateTable::resizeEvent(QResizeEvent * e)
{
  QTableView::resizeEvent(e);

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
      rect=metrics.boundingRect(KGlobal::locale()->weekDayName(count+1, true));
      maxCell.setWidth(QMAX(maxCell.width(), rect.width()));
      maxCell.setHeight(QMAX(maxCell.height(), rect.height()));
    }
  // ----- compare with a real wide number and add some space:
  rect=metrics.boundingRect(QString::fromLatin1("88"));
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
      KNotifyClient::beep();
      return;
    }

  int dayoff = KGlobal::locale()->weekStartsMonday() ? 1 : 0;
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
  if(pos+dayoff<=firstday)
    { // this day is in the previous month
      KNotifyClient::beep();
      return;
    }
  if(firstday+numdays<pos+dayoff)
    { // this date is in the next month
      KNotifyClient::beep();
      return;
    }
  if(hasSelection)
    { // clear the old selected cell
      hasSelection=false;
      temp=firstday+date.day()-dayoff;
      updateCell(temp/7, temp%7, false);
    }
  hasSelection=true;
  updateCell(row, col, false);
  // assert(QDate(date.year(), date.month(), pos-firstday+dayoff).isValid());
  setDate(QDate(date.year(), date.month(), pos-firstday+dayoff));
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
      kdDebug() << "KDateTable::setDate: refusing to set invalid date." << endl;
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
      kdDebug() << "KDateTable::sizeHint: obscure failure - " << endl;
      return QSize(-1, -1);
    }
}

KDateInternalMonthPicker::KDateInternalMonthPicker
(int fontsize, QWidget* parent, const char* name)
  : QTableView(parent, name),
    result(0) // invalid
{
  QRect rect;
  QFont font;
  // -----
  activeCol = -1;
  activeRow = -1;
  font=KGlobalSettings::generalFont();
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
  for(int i=1; i <= 12; ++i)
    {
      rect=metrics.boundingRect(KGlobal::locale()->monthName(i, false));
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
  text=KGlobal::locale()->monthName(index, false);
  painter->drawText(0, 0, cellWidth(), cellHeight(), AlignCenter, text);
  if ( activeCol == col && activeRow == row )
      painter->drawRect( 0, 0, cellWidth(), cellHeight() );
}

void
KDateInternalMonthPicker::mousePressEvent(QMouseEvent *e)
{
  if(!isEnabled() || e->button() != LeftButton)
    {
      KNotifyClient::beep();
      return;
    }
  // -----
  int row, col;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=findRow(mouseCoord.y());
  col=findCol(mouseCoord.x());

  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      activeCol = -1;
      activeRow = -1;
    } else {
      activeCol = col;
      activeRow = row;
      updateCell( row, col, false );
  }
}

void 
KDateInternalMonthPicker::mouseMoveEvent(QMouseEvent *e)
{
  if (e->state() & LeftButton) 
    {
      int row, col;
      QPoint mouseCoord;
      // -----
      mouseCoord = e->pos();
      row=findRow(mouseCoord.y());
      col=findCol(mouseCoord.x());
      int tmpRow = -1, tmpCol = -1;
      if(row<0 || col<0) 
        { // the user clicked on the frame of the table
          if ( activeCol > -1 ) 
            {
              tmpRow = activeRow;
              tmpCol = activeCol;
            }
          activeCol = -1;
          activeRow = -1;
        } else {
          bool differentCell = (activeRow != row || activeCol != col);
          if ( activeCol > -1 && differentCell) 
            {
              tmpRow = activeRow;
              tmpCol = activeCol;
            }
          if ( differentCell) 
            {
              activeRow = row;
              activeCol = col;
              updateCell( row, col, false ); // mark the new active cell
            }
        }
      if ( tmpRow > -1 ) // repaint the former active cell
          updateCell( tmpRow, tmpCol, true );
    }
}

void
KDateInternalMonthPicker::mouseReleaseEvent(QMouseEvent *e)
{
  if(!isEnabled())
    {
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
  font=KGlobalSettings::generalFont();
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
      KNotifyClient::beep();
      return;
    }
  date.setYMD(year, 1, 1);
  if(!date.isValid())
    {
      KNotifyClient::beep();
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

KPopupFrame::KPopupFrame(QWidget* parent, const char*  name)
  : QFrame(parent, name, WType_Popup),
    result(0), // rejected
    main(0)
{
  setFrameStyle(QFrame::Box|QFrame::Raised);
  setMidLineWidth(2);
}

void
KPopupFrame::keyPressEvent(QKeyEvent* e)
{
  if(e->key()==Key_Escape)
    {
      result=0; // rejected
      kapp->exit_loop();
    }
}

void
KPopupFrame::close(int r)
{
  result=r;
  kapp->exit_loop();
}

void
KPopupFrame::setMainWidget(QWidget* m)
{
  main=m;
  if(main!=0)
    {
      resize(main->width()+2*frameWidth(), main->height()+2*frameWidth());
    }
}

void
KPopupFrame::resizeEvent(QResizeEvent*)
{
  if(main!=0)
    {
      main->setGeometry(frameWidth(), frameWidth(),
          width()-2*frameWidth(), height()-2*frameWidth());
    }
}

void 
KPopupFrame::popup(const QPoint &pos)
{
  // Make sure the whole popup is visible.
  QWidget* desktop = QApplication::desktop();
  int sw = desktop->width();
  int sh = desktop->height();
  int x = pos.x();
  int y = pos.y();
  int w = width();
  int h = height();
  if (x+w > sw)
    x = sw - w;
  if (y+h > sh)
    y = sh - h;
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;

  // Pop the thingy up.
  move(x, y);
  show();
}

int
KPopupFrame::exec(QPoint pos)
{
  popup(pos);
  repaint();
  qApp->enter_loop();
  hide();
  return result;
}

int
KPopupFrame::exec(int x, int y)
{
  return exec(QPoint(x, y));
}

