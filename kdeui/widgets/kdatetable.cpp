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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

#include "kdatetable.h"
#include "kdatetable_p.h"

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>
#include <knotification.h>
#include <kcalendarsystem.h>
#include <kshortcut.h>
#include <kstandardshortcut.h>
#include "kdatepicker.h"
#include "kmenu.h"
#include "kactioncollection.h"
#include "kaction.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>
#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QDialog>
#include <QtGui/QActionEvent>
#include <QtCore/QHash>
#include <QtGui/QApplication>
#include <assert.h>

#include <cmath>

class KDateTable::KDateTablePrivate
{
public:
   KDateTablePrivate(KDateTable *q): q(q)
   {
      popupMenuEnabled=false;
      useCustomColors=false;
   }

   ~KDateTablePrivate()
   {
   }

   void nextMonth();
   void previousMonth();
   void beginningOfMonth();
   void endOfMonth();
   void beginningOfWeek();
   void endOfWeek();
  
   KDateTable *q;

   /**
   * The font size of the displayed text.
   */
   int fontsize;
   /**
   * The currently selected date.
   */
   QDate mDate;
   /**
    * The day of the first day in the month [1..7].
    */
   int firstday;
   /**
    * The number of days in the current month.
    */
   int numdays;
   /**
    * The number of days in the previous month.
    */
   int numDaysPrevMonth;
   /**
    * Save the size of the largest used cell content.
    */
   QRectF maxCell;
  
   bool popupMenuEnabled;
   bool useCustomColors;

   struct DatePaintingMode
   {
     QColor fgColor;
     QColor bgColor;
     BackgroundMode bgMode;
   };
   QHash <QString,DatePaintingMode*> customPaintingModes;

};


class KPopupFrame::KPopupFramePrivate
{
public:
  KPopupFramePrivate(KPopupFrame *q):
    q(q),
    result(0), // rejected
    main(0) {}
  
  KPopupFrame *q;
  
  /**
   * The result. It is returned from exec() when the popup window closes.
   */
  int result;
  /**
   * The only subwidget that uses the whole dialog window.
   */
  QWidget *main;
};


KDateValidator::KDateValidator(QWidget* parent)
    : QValidator(parent)
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
      return QValidator::Intermediate;
}

void
KDateValidator::fixup( QString& ) const
{

}

KDateTable::KDateTable(const QDate& date_, QWidget* parent)
  : QWidget(parent), d(new KDateTablePrivate(this))
{
  setFontSize(10);
  setFocusPolicy(Qt::StrongFocus);
  QPalette palette;
  palette.setColor(backgroundRole(), KGlobalSettings::baseColor());
  setPalette(palette);

  if(!date_.isValid())
  {
    kDebug() << "KDateTable ctor: WARNING: Given date is invalid, using current date." << endl;
    setDate(QDate::currentDate()); // this initializes firstday, numdays, numDaysPrevMonth
  }
  else
    setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth
  initAccels();
}

KDateTable::KDateTable(QWidget *parent)
  : QWidget(parent), d(new KDateTablePrivate(this))
{
  setFontSize(10);
  setFocusPolicy(Qt::StrongFocus);
  QPalette palette;
  palette.setColor(backgroundRole(), KGlobalSettings::baseColor());
  setPalette(palette);
  setDate(QDate::currentDate()); // this initializes firstday, numdays, numDaysPrevMonth
  initAccels();
}

KDateTable::~KDateTable()
{
  delete d;
}

void KDateTable::initAccels()
{
  KActionCollection* localCollection = new KActionCollection(this);
  localCollection->setAssociatedWidget(this);

  QAction* next = localCollection->addAction(QLatin1String("next"));
  next->setShortcuts(KStandardShortcut::next());
  connect(next, SIGNAL(triggered(bool)), SLOT(nextMonth()));

  QAction* prior = localCollection->addAction(QLatin1String("prior"));
  prior->setShortcuts(KStandardShortcut::prior());
  connect(prior, SIGNAL(triggered(bool)), SLOT(previousMonth()));

  QAction* beginMonth = localCollection->addAction(QLatin1String("beginMonth"));
  beginMonth->setShortcuts(KStandardShortcut::home());
  connect(beginMonth, SIGNAL(triggered(bool)), SLOT(beginningOfMonth()));

  QAction* endMonth = localCollection->addAction(QLatin1String("endMonth"));
  endMonth->setShortcuts(KStandardShortcut::end());
  connect(endMonth, SIGNAL(triggered(bool)), SLOT(endOfMonth()));

  QAction* beginWeek = localCollection->addAction(QLatin1String("beginWeek"));
  beginWeek->setShortcuts(KStandardShortcut::beginningOfLine());
  connect(beginWeek, SIGNAL(triggered(bool)), SLOT(beginningOfWeek()));

  QAction* endWeek = localCollection->addAction("endWeek");
  endWeek->setShortcuts(KStandardShortcut::endOfLine());
  connect(endWeek, SIGNAL(triggered(bool)), SLOT(endOfWeek()));

  localCollection->readSettings();
}

int KDateTable::posFromDate( const QDate &dt )
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  const int firstWeekDay = KGlobal::locale()->weekStartDay();
  int pos = calendar->day( dt );
  int offset = (d->firstday - firstWeekDay + 7) % 7;
  // make sure at least one day of the previous month is visible.
  // adjust this <1 if more days should be forced visible:
  if ( offset < 1 ) offset += 7;
  return pos + offset;
}

QDate KDateTable::dateFromPos( int pos )
{
  QDate pCellDate;
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  calendar->setYMD(pCellDate, calendar->year(d->mDate), calendar->month(d->mDate), 1);

  int firstWeekDay = KGlobal::locale()->weekStartDay();
  int offset = (d->firstday - firstWeekDay + 7) % 7;
  // make sure at least one day of the previous month is visible.
  // adjust this <1 if more days should be forced visible:
  if ( offset < 1 ) offset += 7;
  pCellDate = calendar->addDays( pCellDate, pos - offset );
  return pCellDate;
}

void KDateTable::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  const QRect &rectToUpdate = e->rect();
  double cellWidth = width() / 7.0;
  double cellHeight = height() / 7.0;
  int leftCol = (int)floor(rectToUpdate.left() / cellWidth);
  int topRow = (int)floor(rectToUpdate.top() / cellHeight);
  int rightCol = (int)ceil(rectToUpdate.right() / cellWidth);
  int bottomRow = (int)ceil(rectToUpdate.bottom() / cellHeight);
  bottomRow = qMin(bottomRow, 6);
  rightCol = qMin(rightCol, 6);
  p.translate(leftCol * cellWidth, topRow * cellHeight);
  for (int i = leftCol; i <= rightCol; ++i) {
    for (int j = topRow; j <= bottomRow; ++j) {
      paintCell(&p, j, i);
      p.translate(0, cellHeight);
    }
    p.translate(cellWidth, 0);
    p.translate(0, -cellHeight * (bottomRow - topRow + 1));
  }
}

void
KDateTable::paintCell(QPainter *painter, int row, int col)
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  QRectF rect;
  QString text;
  QPen pen;
  double w = width() / 7.0;
  double h = height() / 7.0;
  w -= 1;
  h -= 1;
  QFont font=KGlobalSettings::generalFont();
  // -----

  if(row == 0)
    { // we are drawing the headline
      font.setBold(true);
      painter->setFont(font);
      bool normalday = true;
      int firstWeekDay = KGlobal::locale()->weekStartDay();
      int daynum = ( col+firstWeekDay < 8 ) ? col+firstWeekDay :
                                              col+firstWeekDay-7;
      if ( daynum == calendar->weekDayOfPray() ||
         ( daynum == 6 && calendar->calendarName() == "gregorian" ) )
          normalday=false;

      QBrush brushInvertTitle(palette().base());
      QColor titleColor(isEnabled()?( KGlobalSettings::activeTitleColor() ):( KGlobalSettings::inactiveTitleColor() ) );
      QColor textColor(isEnabled()?( KGlobalSettings::activeTextColor() ):( KGlobalSettings::inactiveTextColor() ) );

      if (!normalday)
        {
          painter->setPen(textColor);
          painter->setBrush(textColor);
          painter->drawRect(QRectF(0, 0, w, h));
          painter->setPen(titleColor);
        } else {
          painter->setPen(titleColor);
          painter->setBrush(titleColor);
          painter->drawRect(QRectF(0, 0, w, h));
          painter->setPen(textColor);
        }
      painter->drawText(QRectF(0, 0, w, h), Qt::AlignCenter,
                        calendar->weekDayName(daynum, true), &rect);
      painter->setPen(palette().color(QPalette::Text));
      painter->drawLine(QPointF(0, h), QPointF(w, h));
      // ----- draw the weekday:
    } else {
      bool paintRect=true;
      painter->setFont(font);
      int pos=7*(row-1)+col;

      QDate pCellDate = dateFromPos( pos );
      // First day of month
      text = calendar->dayString(pCellDate, true);
      if( calendar->month(pCellDate) != calendar->month(d->mDate) )
        { // we are either
          // ° painting a day of the previous month or
          // ° painting a day of the following month
          // TODO: don't hardcode gray here! Use a color with less contrast to the background than normal text.
          painter->setPen( palette().color(QPalette::Mid) );
//          painter->setPen(gray);
        } else { // paint a day of the current month
          if ( d->useCustomColors )
          {
            KDateTablePrivate::DatePaintingMode *mode=d->customPaintingModes[pCellDate.toString()];
            if (mode)
            {
              if (mode->bgMode != NoBgMode)
              {
                QBrush oldbrush=painter->brush();
                painter->setBrush( mode->bgColor );
                switch(mode->bgMode)
                {
                  case(CircleMode) : painter->drawEllipse(QRectF(0,0,w,h));break;
                  case(RectangleMode) : painter->drawRect(QRectF(0,0,w,h));break;
                  case(NoBgMode) : // Should never be here, but just to get one
                                   // less warning when compiling
                  default: break;
                }
                painter->setBrush( oldbrush );
                paintRect=false;
              }
              painter->setPen( mode->fgColor );
            } else
              painter->setPen(palette().color(QPalette::Text));
          } else //if ( firstWeekDay < 4 ) // <- this doesn' make sense at all!
          painter->setPen(palette().color(QPalette::Text));
        }

      pen=painter->pen();
      int firstWeekDay=KGlobal::locale()->weekStartDay();
      int offset=d->firstday-firstWeekDay;
      if(offset<1)
        offset+=7;
      int day = calendar->day(d->mDate);
           if( (offset+day) == (pos+1))
        {
           // draw the currently selected date
	   if (isEnabled())
	   {
           painter->setPen(palette().color(QPalette::Highlight));
           painter->setBrush(palette().color(QPalette::Highlight));
	   }
	   else
	   {
	   painter->setPen(palette().color(QPalette::Text));
           painter->setBrush(palette().color(QPalette::Text));
	   }
           pen=palette().color(QPalette::HighlightedText);
        } else {
          painter->setBrush(palette().color(QPalette::Background));
          painter->setPen(palette().color(QPalette::Background));
//          painter->setBrush(palette().base());
//          painter->setPen(palette().base());
        }

      if ( pCellDate == QDate::currentDate() )
      {
         painter->setPen(palette().color(QPalette::Text));
      }

      if ( paintRect ) painter->drawRect(QRectF(0, 0, w, h));
      painter->setPen(pen);
      painter->drawText(QRectF(0, 0, w, h), Qt::AlignCenter, text, &rect);
    }
  if(rect.width()>d->maxCell.width()) d->maxCell.setWidth(rect.width());
  if(rect.height()>d->maxCell.height()) d->maxCell.setHeight(rect.height());
}

void KDateTable::KDateTablePrivate::nextMonth()
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  q->setDate(calendar->addMonths( mDate, 1 ));
}

void KDateTable::KDateTablePrivate::previousMonth()
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  q->setDate(calendar->addMonths( mDate, -1 ));
}

void KDateTable::KDateTablePrivate::beginningOfMonth()
{
  q->setDate(mDate.addDays(1 - mDate.day()));
}

void KDateTable::KDateTablePrivate::endOfMonth()
{
  q->setDate(mDate.addDays(mDate.daysInMonth() - mDate.day()));
}

void KDateTable::KDateTablePrivate::beginningOfWeek()
{
  q->setDate(mDate.addDays(1 - mDate.dayOfWeek()));
}

void KDateTable::KDateTablePrivate::endOfWeek()
{
  q->setDate(mDate.addDays(7 - mDate.dayOfWeek()));
}

void
KDateTable::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
    case Qt::Key_Up:
            setDate(d->mDate.addDays(-7));
        break;
    case Qt::Key_Down:
            setDate(d->mDate.addDays(7));
        break;
    case Qt::Key_Left:
            setDate(d->mDate.addDays(-1));
        break;
    case Qt::Key_Right:
            setDate(d->mDate.addDays(1));
        break;
    case Qt::Key_Minus:
        setDate(d->mDate.addDays(-1));
	break;
    case Qt::Key_Plus:
        setDate(d->mDate.addDays(1));
	break;
    case Qt::Key_N:
        setDate(QDate::currentDate());
	break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit tableClicked();
        break;
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Shift:
      // Don't beep for modifiers
      break;
    default:
      if (!e->modifiers()) { // hm
    KNotification::beep();
}
    }
}

void
KDateTable::setFontSize(int size)
{
  int count;
  QFontMetricsF metrics(fontMetrics());
  QRectF rect;
  // ----- store rectangles:
  d->fontsize=size;
  // ----- find largest day name:
  d->maxCell.setWidth(0);
  d->maxCell.setHeight(0);
  for(count=0; count<7; ++count)
    {
      rect=metrics.boundingRect(KGlobal::locale()->calendar()
                                ->weekDayName(count+1, true));
      d->maxCell.setWidth(qMax(d->maxCell.width(), rect.width()));
      d->maxCell.setHeight(qMax(d->maxCell.height(), rect.height()));
    }
  // ----- compare with a real wide number and add some space:
  rect=metrics.boundingRect(QLatin1String("88"));
  d->maxCell.setWidth(qMax(d->maxCell.width()+2, rect.width()));
  d->maxCell.setHeight(qMax(d->maxCell.height()+4, rect.height()));
}

void
KDateTable::wheelEvent ( QWheelEvent * e )
{
    setDate(d->mDate.addMonths( -(int)(e->delta()/120)) );
    e->accept();
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
      KNotification::beep();
      return;
    }

  // -----
  int row, col, pos, temp;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=mouseCoord.y() / (height() / 7);
  col=mouseCoord.x() / (width() / 7);
  if(row<1 || col<0)
    { // the user clicked on the frame of the table
      return;
    }

  // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
  // the row with the days of the week in the calculation.

  // old selected date:
  temp = posFromDate( d->mDate );
  // new position and date
  pos = (7 * (row - 1)) + col;
  QDate clickedDate = dateFromPos( pos );

  // set the new date. If it is in the previous or next month, the month will
  // automatically be changed, no need to do that manually...
  setDate( clickedDate );

  // This could be optimized to only call update over the regions
  // of old and new cell, but 99% of times there is also a call to
  // setDate that already calls update() so no need to optimize that
  // much here
  update();

  emit tableClicked();

  if (  e->button() == Qt::RightButton && d->popupMenuEnabled )
  {
        KMenu *menu = new KMenu();
        menu->addTitle( KGlobal::locale()->formatDate(clickedDate) );
        emit aboutToShowContextMenu( menu, clickedDate );
        menu->popup(e->globalPos());
  }
}

bool
KDateTable::setDate(const QDate& date_)
{
  bool changed=false;
  QDate temp;
  // -----
  if(!date_.isValid())
    {
      kDebug() << "KDateTable::setDate: refusing to set invalid date." << endl;
      return false;
    }
  if(d->mDate!=date_)
    {
      emit(dateChanged(d->mDate, date_));
      d->mDate=date_;
      emit(dateChanged(d->mDate));
      changed=true;
    }
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  calendar->setYMD(temp, calendar->year(d->mDate), calendar->month(d->mDate), 1);
  //temp.setYMD(d->mDate.year(), d->mDate.month(), 1);
  //kDebug() << "firstDayInWeek: " << temp.toString() << endl;
  d->firstday=temp.dayOfWeek();
  d->numdays=calendar->daysInMonth(d->mDate);

  temp = calendar->addMonths(temp, -1);
  d->numDaysPrevMonth=calendar->daysInMonth(temp);
  if(changed)
    {
      update();
    }
  return true;
}

const QDate&
KDateTable::date() const
{
  return d->mDate;
}

// what are those repaintContents() good for? (pfeiffer)
void KDateTable::focusInEvent( QFocusEvent *e )
{
//    repaintContents(false);
    QWidget::focusInEvent( e );
}

void KDateTable::focusOutEvent( QFocusEvent *e )
{
//    repaintContents(false);
    QWidget::focusOutEvent( e );
}

QSize
KDateTable::sizeHint() const
{
  if(d->maxCell.height()>0 && d->maxCell.width()>0)
    {
      return QSize(qRound(d->maxCell.width()*7),
             (qRound(d->maxCell.height()+2)*7));
    } else {
      kDebug() << "KDateTable::sizeHint: obscure failure - " << endl;
      return QSize(-1, -1);
    }
}

void KDateTable::setPopupMenuEnabled( bool enable )
{
   d->popupMenuEnabled=enable;
}

bool KDateTable::popupMenuEnabled() const
{
   return d->popupMenuEnabled;
}

void KDateTable::setCustomDatePainting(const QDate &date, const QColor &fgColor, BackgroundMode bgMode, const QColor &bgColor)
{
    if (!fgColor.isValid())
    {
        unsetCustomDatePainting( date );
        return;
    }

    KDateTablePrivate::DatePaintingMode *mode=new KDateTablePrivate::DatePaintingMode;
    mode->bgMode=bgMode;
    mode->fgColor=fgColor;
    mode->bgColor=bgColor;

    d->customPaintingModes.insert( date.toString(), mode );
    d->useCustomColors=true;
    update();
}

void KDateTable::unsetCustomDatePainting( const QDate &date )
{
    d->customPaintingModes.remove( date.toString() );
}



KDateInternalYearSelector::KDateInternalYearSelector
(QWidget* parent)
  : QLineEdit(parent),
    val(new QIntValidator(this)),
    result(0)
{
  QFont font;
  // -----
  font=KGlobalSettings::generalFont();
  setFont(font);
  setFrame(false);
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
      KNotification::beep();
      return;
    }
  //date.setYMD(year, 1, 1);
  KGlobal::locale()->calendar()->setYMD(date, year, 1, 1);
  if(!date.isValid())
    {
      KNotification::beep();
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

KPopupFrame::KPopupFrame(QWidget* parent)
  : QFrame(parent, Qt::Popup), d(new KPopupFramePrivate(this))
{
  setFrameStyle(QFrame::Box|QFrame::Raised);
  setMidLineWidth(2);
}

KPopupFrame::~KPopupFrame()
{
  delete d;
}

void
KPopupFrame::keyPressEvent(QKeyEvent* e)
{
  if(e->key()==Qt::Key_Escape)
    {
      d->result=0; // rejected
      emit leaveModality();
      //qApp->exit_loop();
    }
}

void
KPopupFrame::close(int r)
{
  d->result=r;
  emit leaveModality();
  //qApp->exit_loop();
}

void
KPopupFrame::setMainWidget(QWidget* m)
{
  d->main=m;
  if(d->main)
    {
      resize(d->main->width()+2*frameWidth(), d->main->height()+2*frameWidth());
    }
}

void
KPopupFrame::resizeEvent(QResizeEvent*)
{
  if(d->main)
    {
      d->main->setGeometry(frameWidth(), frameWidth(),
          width()-2*frameWidth(), height()-2*frameWidth());
    }
}

void
KPopupFrame::popup(const QPoint &pos)
{
  // Make sure the whole popup is visible.
  QRect d = KGlobalSettings::desktopGeometry(pos);

  int x = pos.x();
  int y = pos.y();
  int w = width();
  int h = height();
  if (x+w > d.x()+d.width())
    x = d.width() - w;
  if (y+h > d.y()+d.height())
    y = d.height() - h;
  if (x < d.x())
    x = 0;
  if (y < d.y())
    y = 0;

  // Pop the thingy up.
  move(x, y);
  show();
}

int
KPopupFrame::exec(const QPoint &pos)
{
  popup(pos);
  repaint();
  QEventLoop eventLoop;
  connect(this, SIGNAL(leaveModality()),
          &eventLoop, SLOT(quit()));
  eventLoop.exec();

  hide();
  return d->result;
}

int
KPopupFrame::exec(int x, int y)
{
  return exec(QPoint(x, y));
}

#include "kdatetable.moc"
#include "kdatetable_p.moc"
