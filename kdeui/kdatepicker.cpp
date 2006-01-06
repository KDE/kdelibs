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

#include <qcheckbox.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qstyle.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qfont.h>
#include <qvalidator.h>
#include <qmenu.h>
#include <QMenuItem>
#include <QStyleOptionToolButton>
#include <qapplication.h>

#include "kdatepicker.h"
#include <kglobal.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <klineedit.h>
#include <kdebug.h>
#include <knotification.h>
#include <kcalendarsystem.h>
#include <QKeyEvent>

#include "kdatetable.h"
#include "kdatepicker.moc"

// Week numbers are defined by ISO 8601
// See http://www.merlyn.demon.co.uk/weekinfo.htm for details

class KDatePicker::KDatePickerPrivate
{
public:
    KDatePickerPrivate() : closeButton(0L), selectWeek(0L), todayButton(0), navigationLayout(0) {}

    void fillWeeksCombo(const QDate &date);

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

  QDate day;
  int year = calendar->year(date);
  calendar->setYMD(day, year, 1, 1);
  int lastMonth = calendar->monthsInYear(day);
  QDate lastDay, firstDayOfLastMonth;
  calendar->setYMD(firstDayOfLastMonth, year, lastMonth, 1);
  calendar->setYMD(lastDay, year, lastMonth, calendar->daysInMonth(firstDayOfLastMonth));

  for (; day <= lastDay ; day = calendar->addDays(day, 7 /*calendar->daysOfWeek()*/) )
  {
    QString week = i18n("Week %1").arg(calendar->weekNumber(day, &year));
    if ( year != calendar->year(day) ) week += "*";  // show that this is a week from a different year
    d->selectWeek->addItem(week);

    // make sure that the week of the lastDay is always inserted: in Chinese calendar
    // system, this is not always the case
    if(day < lastDay && day.daysTo(lastDay) < 7 && calendar->weekNumber(day) != calendar->weekNumber(lastDay))
      day = lastDay.addDays(-7);
  }
}

KDatePicker::KDatePicker(QWidget* parent, const QDate& dt)
  : QFrame(parent)
{
  init( dt );
}

void KDatePicker::init( const QDate &dt )
{
  d = new KDatePickerPrivate();

  QBoxLayout * topLayout = new QVBoxLayout(this);

  d->navigationLayout = new QHBoxLayout(topLayout);
  d->navigationLayout->addStretch();
  yearBackward = new QToolButton(this);
  yearBackward->setAutoRaise(true);
  d->navigationLayout->addWidget(yearBackward);
  monthBackward = new QToolButton(this);
  monthBackward ->setAutoRaise(true);
  d->navigationLayout->addWidget(monthBackward);
  d->navigationLayout->addSpacing(KDialog::spacingHint());

  selectMonth = new QToolButton(this);
  selectMonth ->setAutoRaise(true);
  d->navigationLayout->addWidget(selectMonth);
  selectYear = new QToolButton(this);
  selectYear->setToggleButton(true);
  selectYear->setAutoRaise(true);
  d->navigationLayout->addWidget(selectYear);
  d->navigationLayout->addSpacing(KDialog::spacingHint());

  monthForward = new QToolButton(this);
  monthForward ->setAutoRaise(true);
  d->navigationLayout->addWidget(monthForward);
  yearForward = new QToolButton(this);
  yearForward ->setAutoRaise(true);
  d->navigationLayout->addWidget(yearForward);
  d->navigationLayout->addStretch();

  line = new KLineEdit(this);
  val = new KDateValidator(this);
  table = new KDateTable(this);
  fontsize = KGlobalSettings::generalFont().pointSize();
  if (fontsize == -1)
     fontsize = QFontInfo(KGlobalSettings::generalFont()).pointSize();

  fontsize++; // Make a little bigger

  d->selectWeek = new QComboBox(false, this);  // read only week selection
  d->todayButton = new QToolButton(this);
  d->todayButton->setIcon(SmallIconSet("today"));

  yearForward->setToolTip(i18n("Next year"));
  yearBackward->setToolTip(i18n("Previous year"));
  monthForward->setToolTip(i18n("Next month"));
  monthBackward->setToolTip(i18n("Previous month"));
  d->selectWeek->setToolTip(i18n("Select a week"));
  selectMonth->setToolTip(i18n("Select a month"));
  selectYear->setToolTip(i18n("Select a year"));
  d->todayButton->setToolTip(i18n("Select the current day"));

  // -----
  setFontSize(fontsize);
  line->setValidator(val);
  line->installEventFilter( this );
  if (  QApplication::isRightToLeft() )
  {
      yearForward->setIcon(BarIconSet(QLatin1String("2leftarrow")));
      yearBackward->setIcon(BarIconSet(QLatin1String("2rightarrow")));
      monthForward->setIcon(BarIconSet(QLatin1String("1leftarrow")));
      monthBackward->setIcon(BarIconSet(QLatin1String("1rightarrow")));
  }
  else
  {
      yearForward->setIcon(BarIconSet(QLatin1String("2rightarrow")));
      yearBackward->setIcon(BarIconSet(QLatin1String("2leftarrow")));
      monthForward->setIcon(BarIconSet(QLatin1String("1rightarrow")));
      monthBackward->setIcon(BarIconSet(QLatin1String("1leftarrow")));
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
  connect(selectYear, SIGNAL(toggled(bool)), SLOT(selectYearClicked()));
  connect(line, SIGNAL(returnPressed()), SLOT(lineEnterPressed()));
  table->setFocus();


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

      if ( (k->key() == Qt::Key_PageUp) ||
           (k->key() == Qt::Key_PageDown)  ||
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
    QDate firstDay;
    calendar->setYMD(firstDay, calendar->year(date), 1, 1);
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
  const int months = calendar->monthsInYear(date);

  QMenu popup(selectMonth);

  for (int i = 1; i <= months; i++)
    popup.addAction(calendar->monthName(i, calendar->year(date)))->setData(i);

  //QMenuItem *item = popup.findItem (calendar->month(date) - 1);
  QAction *item=popup.actions()[calendar->month(date)-1];
  if (item) // if this happens the above should already given an assertion
    popup.setActiveAction(item);
 
  if ( (item = popup.exec(selectMonth->mapToGlobal(QPoint(0, 0)), item)) == 0 ) return;  // canceled

  int day = calendar->day(date);
  // ----- construct a valid date in this month:
  //date.setYMD(date.year(), month, 1);
  //date.setYMD(date.year(), month, qMin(day, date.daysInMonth()));
  calendar->setYMD(date, calendar->year(date), item->data().toInt(),
                   qMin(day, calendar->daysInMonth(date)));
  // ----- set this month
  setDate(date);
}

void
KDatePicker::selectYearClicked()
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  if (!selectYear->isChecked ())
  {
    return;
  }

  int year;
  KPopupFrame* popup = new KPopupFrame(this);
  KDateInternalYearSelector* picker = new KDateInternalYearSelector(popup);
  // -----
  picker->resize(picker->sizeHint());
  picker->setYear( table->getDate().year() );
  picker->selectAll();
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
      //date.setYMD(year, date.month(), qMin(day, date.daysInMonth()));
      calendar->setYMD(date, year, calendar->month(date),
                       qMin(day, calendar->daysInMonth(date)));
      // ----- set this month
      setDate(date);
    } else {
      KNotification::beep();
    }
  selectYear->setOn( false );
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
      KNotification::beep();
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
      maxMonthRect.setWidth(qMax(r.width(), maxMonthRect.width()));
      maxMonthRect.setHeight(qMax(r.height(),  maxMonthRect.height()));
    }

  QStyleOptionToolButton opt;
  
  // stolen from KToolBarButton
  opt.init(this);
  opt.font      = selectMonth->font();
  opt.icon      = selectMonth->icon();
  opt.text      = selectMonth->text();
  opt.features  = selectMonth->menu() ? QStyleOptionToolButton::Menu : QStyleOptionToolButton::None; //### FIXME: delay?
  opt.subControls       = QStyle::SC_All;
  opt.activeSubControls = 0; //### FIXME: !!
  
  QSize metricBound = style()->sizeFromContents(QStyle::CT_ToolButton,
                                               &opt, 
                                               maxMonthRect, selectMonth);
  selectMonth->setMinimumSize(metricBound);

  table->setFontSize(s);
}

void
KDatePicker::setCloseButton( bool enable )
{
    if ( enable == (d->closeButton != 0L) )
        return;

    if ( enable ) {
        d->closeButton = new QToolButton( this );
        d->closeButton->setAutoRaise(true);
        d->navigationLayout->addSpacing(KDialog::spacingHint());
        d->navigationLayout->addWidget(d->closeButton);
        d->closeButton->setToolTip(i18n("Close"));
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
    return (d->closeButton);
}

void KDatePicker::virtual_hook( int /*id*/, void* /*data*/ )
{ /*BASE::virtual_hook( id, data );*/ }

