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

#include <qlayout.h>
#include <qframe.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qstyle.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qfont.h>
#include <qvalidator.h>
#include <qpopupmenu.h>

#include "kdatepicker.h"
#include <kglobal.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <klineedit.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <kcalendarsystem.h>

#include "kdatetbl.h"
#include "kdatepicker.moc"

// Week numbers are defined by ISO 8601
// See http://www.merlyn.demon.co.uk/weekinfo.htm for details

class KDatePicker::KDatePickerPrivate
{
public:
    KDatePickerPrivate() : closeButton(0L), selectWeek(0L), todayButton(0), navigationLayout(0) {}

    void fillWeeksCombo(const QDate &date);

    KToolBar *tb;
    QToolButton *closeButton;
    QComboBox *selectWeek;
    QToolButton *todayButton;
    QBoxLayout *navigationLayout;
};

void KDatePicker::fillWeeksCombo(const QDate &date)
{
  // every year can have a different number of weeks
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  // it could be that we had 53,1..52 and now 1..53 which is the same number but different
  // so always fill with new values

  d->selectWeek->clear();

  // We show all week numbers for all weeks between first day of year to last day of year
  // This of course can be a list like 53,1,2..52

  QDate day(date.year(), 1, 1);
  int lastMonth = calendar->monthsInYear(day);
  QDate lastDay(date.year(), lastMonth, calendar->daysInMonth(QDate(date.year(), lastMonth, 1)));

  for (; day <= lastDay; day = calendar->addDays(day, 7 /*calendar->daysOfWeek()*/) )
  {
    int year = 0;
    QString week = i18n("Week %1").arg(calendar->weekNumber(day, &year));
    if ( year != date.year() ) week += "*";  // show that this is a week from a different year
    d->selectWeek->insertItem(week);
  }
}

KDatePicker::KDatePicker(QWidget *parent, QDate dt, const char *name)
  : QFrame(parent,name)
{
  init( dt );
}

KDatePicker::KDatePicker(QWidget *parent, QDate dt, const char *name, WFlags f)
  : QFrame(parent,name, f)
{
  init( dt );
}

KDatePicker::KDatePicker( QWidget *parent, const char *name )
  : QFrame(parent,name)
{
  init( QDate::currentDate() );
}

void KDatePicker::init( const QDate &dt )
{
  d = new KDatePickerPrivate();

  d->tb = new KToolBar(this);

  yearBackward = new QToolButton(d->tb);
  monthBackward = new QToolButton(d->tb);
  selectMonth = new QToolButton(d->tb);
  selectYear = new QToolButton(d->tb);
  monthForward = new QToolButton(d->tb);
  yearForward = new QToolButton(d->tb);
  QWidget *dummy = new QWidget(d->tb);
  dummy->setName("kde toolbar widget");
  d->tb->setStretchableWidget(dummy);

  line = new KLineEdit(this);
  val = new KDateValidator(this);
  table = new KDateTable(this);
  fontsize = KGlobalSettings::generalFont().pointSize();
  if (fontsize == -1)
     fontsize = QFontInfo(KGlobalSettings::generalFont()).pointSize();

  fontsize++; // Make a little bigger

  d->selectWeek = new QComboBox(false, this);  // read only week selection
  d->todayButton = new QToolButton(this);
  d->todayButton->setIconSet(SmallIconSet("today"));

  QToolTip::add(yearForward, i18n("Next year"));
  QToolTip::add(yearBackward, i18n("Previous year"));
  QToolTip::add(monthForward, i18n("Next month"));
  QToolTip::add(monthBackward, i18n("Previous month"));
  QToolTip::add(d->selectWeek, i18n("Select a week"));
  QToolTip::add(selectMonth, i18n("Select a month"));
  QToolTip::add(selectYear, i18n("Select a year"));
  QToolTip::add(d->todayButton, i18n("Select the current day"));

  // -----
  setFontSize(fontsize);
  line->setValidator(val);
  line->installEventFilter( this );
  if (  QApplication::reverseLayout() )
  {
      yearForward->setIconSet(BarIconSet(QString::fromLatin1("2leftarrow")));
      yearBackward->setIconSet(BarIconSet(QString::fromLatin1("2rightarrow")));
      monthForward->setIconSet(BarIconSet(QString::fromLatin1("1leftarrow")));
      monthBackward->setIconSet(BarIconSet(QString::fromLatin1("1rightarrow")));
  }
  else
  {
      yearForward->setIconSet(BarIconSet(QString::fromLatin1("2rightarrow")));
      yearBackward->setIconSet(BarIconSet(QString::fromLatin1("2leftarrow")));
      monthForward->setIconSet(BarIconSet(QString::fromLatin1("1rightarrow")));
      monthBackward->setIconSet(BarIconSet(QString::fromLatin1("1leftarrow")));
  }
  connect(table, SIGNAL(dateChanged(QDate)), SLOT(dateChangedSlot(QDate)));
  connect(table, SIGNAL(tableClicked()), SLOT(tableClickedSlot()));
  connect(monthForward, SIGNAL(clicked()), SLOT(monthForwardClicked()));
  connect(monthBackward, SIGNAL(clicked()), SLOT(monthBackwardClicked()));
  connect(yearForward, SIGNAL(clicked()), SLOT(yearForwardClicked()));
  connect(yearBackward, SIGNAL(clicked()), SLOT(yearBackwardClicked()));
  connect(d->selectWeek, SIGNAL(activated(int)), SLOT(weekSelected(int)));
  connect(d->todayButton, SIGNAL(clicked()), SLOT(todayButtonClicked()));
  connect(selectMonth, SIGNAL(clicked()), SLOT(selectMonthClicked()));
  connect(selectYear, SIGNAL(clicked()), SLOT(selectYearClicked()));
  connect(line, SIGNAL(returnPressed()), SLOT(lineEnterPressed()));
  table->setFocus();

  QBoxLayout * topLayout = new QVBoxLayout(this);

  d->navigationLayout = new QHBoxLayout(topLayout);
  d->navigationLayout->addWidget(d->tb);

  topLayout->addWidget(table);

  QBoxLayout * bottomLayout = new QHBoxLayout(topLayout);
  bottomLayout->addWidget(d->todayButton);
  bottomLayout->addWidget(line);
  bottomLayout->addWidget(d->selectWeek);

  table->setDate(dt);
  dateChangedSlot(dt);  // needed because table emits changed only when newDate != oldDate
}

KDatePicker::~KDatePicker()
{
  delete d;
}

bool
KDatePicker::eventFilter(QObject *o, QEvent *e )
{
   if ( e->type() == QEvent::KeyPress ) {
      QKeyEvent *k = (QKeyEvent *)e;

      if ( (k->key() == Qt::Key_Prior) ||
           (k->key() == Qt::Key_Next)  ||
           (k->key() == Qt::Key_Up)    ||
           (k->key() == Qt::Key_Down) )
       {
          QApplication::sendEvent( table, e );
          table->setFocus();
          return true; // eat event
       }
   }
   return QFrame::eventFilter( o, e );
}

void
KDatePicker::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);
}

void
KDatePicker::dateChangedSlot(QDate date)
{
    kdDebug(298) << "KDatePicker::dateChangedSlot: date changed (" << date.year() << "/" << date.month() << "/" << date.day() << ")." << endl;

    const KCalendarSystem * calendar = KGlobal::locale()->calendar();

    line->setText(KGlobal::locale()->formatDate(date, true));
    selectMonth->setText(calendar->monthName(date, false));
    fillWeeksCombo(date);

    // calculate the item num in the week combo box; normalize selected day so as if 1.1. is the first day of the week
    QDate firstDay(date.year(), 1, 1);
    d->selectWeek->setCurrentItem((calendar->dayOfYear(date) + calendar->dayOfWeek(firstDay) - 2) / 7/*calendar->daysInWeek()*/);

    selectYear->setText(calendar->yearString(date, false));

    emit(dateChanged(date));
}

void
KDatePicker::tableClickedSlot()
{
  kdDebug(298) << "KDatePicker::tableClickedSlot: table clicked." << endl;
  emit(dateSelected(table->getDate()));
  emit(tableClicked());
}

const QDate&
KDatePicker::getDate() const
{
  return table->getDate();
}

const QDate &
KDatePicker::date() const
{
    return table->getDate();
}

bool
KDatePicker::setDate(const QDate& date)
{
    if(date.isValid())
    {
        table->setDate(date);  // this also emits dateChanged() which then calls our dateChangedSlot()
        return true;
    }
    else
    {
        kdDebug(298) << "KDatePicker::setDate: refusing to set invalid date." << endl;
        return false;
    }
}

void
KDatePicker::monthForwardClicked()
{
    QDate temp;
    temp = KGlobal::locale()->calendar()->addMonths( table->getDate(), 1 );

    setDate( temp );
}

void
KDatePicker::monthBackwardClicked()
{
    QDate temp;
    temp = KGlobal::locale()->calendar()->addMonths( table->getDate(), -1 );

    setDate( temp );
}

void
KDatePicker::yearForwardClicked()
{
    QDate temp;
    temp = KGlobal::locale()->calendar()->addYears( table->getDate(), 1 );

    setDate( temp );
}

void
KDatePicker::yearBackwardClicked()
{
    QDate temp;
    temp = KGlobal::locale()->calendar()->addYears( table->getDate(), -1 );

    setDate( temp );
}

void KDatePicker::selectWeekClicked() {}  // ### in 3.2 obsolete; kept for binary compatibility

void
KDatePicker::weekSelected(int week)
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  QDate date = table->getDate();
  int year = calendar->year(date);

  calendar->setYMD(date, year, 1, 1);  // first day of selected year

  // calculate the first day in the selected week (day 1 is first day of week)
  date = calendar->addDays(date, week * 7/*calendar->daysOfWeek()*/ -calendar->dayOfWeek(date) + 1);

  setDate(date);
}

void
KDatePicker::selectMonthClicked()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  QDate date = table->getDate();
  int i, month, months = calendar->monthsInYear(date);

  QPopupMenu popup(selectMonth);

  for (i = 1; i <= months; i++)
    popup.insertItem(calendar->monthName(i, calendar->year(date)), i);

  popup.setActiveItem(calendar->month(date) - 1);

  if ( (month = popup.exec(selectMonth->mapToGlobal(QPoint(0, 0)), calendar->month(date) - 1)) == -1 ) return;  // canceled

  int day = calendar->day(date);
  // ----- construct a valid date in this month:
  //date.setYMD(date.year(), month, 1);
  //date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));
  calendar->setYMD(date, calendar->year(date), month,
                   QMIN(day, calendar->daysInMonth(date)));
  // ----- set this month
  setDate(date);
}

void
KDatePicker::selectYearClicked()
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  int year;
  KPopupFrame* popup = new KPopupFrame(this);
  KDateInternalYearSelector* picker = new KDateInternalYearSelector(popup);
  // -----
  picker->resize(picker->sizeHint());
  popup->setMainWidget(picker);
  connect(picker, SIGNAL(closeMe(int)), popup, SLOT(close(int)));
  picker->setFocus();
  if(popup->exec(selectYear->mapToGlobal(QPoint(0, selectMonth->height()))))
    {
      QDate date;
      int day;
      // -----
      year=picker->getYear();
      date=table->getDate();
      day=calendar->day(date);
      // ----- construct a valid date in this month:
      //date.setYMD(year, date.month(), 1);
      //date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));
      calendar->setYMD(date, year, calendar->month(date),
                       QMIN(day, calendar->daysInMonth(date)));
      // ----- set this month
      setDate(date);
    } else {
      KNotifyClient::beep();
    }
  delete popup;
}

void
KDatePicker::setEnabled(bool enable)
{
  QWidget *widgets[]= {
    yearForward, yearBackward, monthForward, monthBackward,
    selectMonth, selectYear,
    line, table, d->selectWeek, d->todayButton };
  const int Size=sizeof(widgets)/sizeof(widgets[0]);
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      widgets[count]->setEnabled(enable);
    }
}

void
KDatePicker::lineEnterPressed()
{
  QDate temp;
  // -----
  if(val->date(line->text(), temp)==QValidator::Acceptable)
    {
        kdDebug(298) << "KDatePicker::lineEnterPressed: valid date entered." << endl;
        emit(dateEntered(temp));
        setDate(temp);
    } else {
      KNotifyClient::beep();
      kdDebug(298) << "KDatePicker::lineEnterPressed: invalid date entered." << endl;
    }
}

void
KDatePicker::todayButtonClicked()
{
  setDate(QDate::currentDate());
}

QSize
KDatePicker::sizeHint() const
{
  return QWidget::sizeHint();
}

void
KDatePicker::setFontSize(int s)
{
  QWidget *buttons[]= {
    // yearBackward,
    // monthBackward,
    selectMonth,
    selectYear,
    // monthForward,
    // yearForward
  };
  const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
  int count;
  QFont font;
  QRect r;
  // -----
  fontsize=s;
  for(count=0; count<NoOfButtons; ++count)
    {
      font=buttons[count]->font();
      font.setPointSize(s);
      buttons[count]->setFont(font);
    }
  QFontMetrics metrics(selectMonth->fontMetrics());

  for (int i = 1; ; ++i)
    {
      QString str = KGlobal::locale()->calendar()->monthName(i,
         KGlobal::locale()->calendar()->year(table->getDate()), false);
      if (str.isNull()) break;
      r=metrics.boundingRect(str);
      maxMonthRect.setWidth(QMAX(r.width(), maxMonthRect.width()));
      maxMonthRect.setHeight(QMAX(r.height(),  maxMonthRect.height()));
    }

  QSize metricBound = style().sizeFromContents(QStyle::CT_ToolButton,
                                               selectMonth,
                                               maxMonthRect);
  selectMonth->setMinimumSize(metricBound);

  table->setFontSize(s);
}

void
KDatePicker::setCloseButton( bool enable )
{
    if ( enable == (d->closeButton != 0L) )
        return;

    if ( enable ) {
        d->closeButton = new QToolButton( d->tb );
        QToolTip::add(d->closeButton, i18n("Close"));
        d->closeButton->setPixmap( SmallIcon("remove") );
        connect( d->closeButton, SIGNAL( clicked() ),
                 topLevelWidget(), SLOT( close() ) );
    }
    else {
        delete d->closeButton;
        d->closeButton = 0L;
    }

    updateGeometry();
}

bool KDatePicker::hasCloseButton() const
{
    return (d->closeButton != 0L);
}

void KDatePicker::virtual_hook( int /*id*/, void* /*data*/ )
{ /*BASE::virtual_hook( id, data );*/ }

