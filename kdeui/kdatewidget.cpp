/*  This file is part of the KDE libraries
    Copyright (C) 2001 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlineedit.h>

#include "knuminput.h"
#include "kglobal.h"
#include "klocale.h"
//#include "kdatepicker.h"
#include "kdialog.h"

#include "kdatewidget.h"

class KDateWidgetSpinBox : public QSpinBox
{
public:
  KDateWidgetSpinBox(int min, int max, QWidget *parent)
    : QSpinBox(min, max, 1, parent)
  {
     editor()->setAlignment(AlignRight);
  }
};

class KDateWidget::KDateWidgetPrivate
{
public:
   KDateWidgetSpinBox *m_day;
   QComboBox *m_month;
   KDateWidgetSpinBox *m_year;
   QDate m_dat;
};


KDateWidget::KDateWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  init();
  setDate(QDate());
}

KDateWidget::KDateWidget( QDate date, QWidget *parent,
			    const char *name )
  : QWidget( parent, name )
{
  init();
  setDate(date);
}

void KDateWidget::init()
{
  d = new KDateWidgetPrivate;
  KLocale *locale = KGlobal::locale();
  QHBoxLayout *layout = new QHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);
  d->m_day = new KDateWidgetSpinBox(1, 31, this);
  d->m_month = new QComboBox(false, this);
  for(int i = 1; i <= 12; i++)
    d->m_month->insertItem(locale->monthName(i));

  d->m_year = new KDateWidgetSpinBox(1970, 2038, this);

  connect(d->m_day, SIGNAL(valueChanged(int)), this, SLOT(slotDateChanged()));
  connect(d->m_month, SIGNAL(activated(int)), this, SLOT(slotDateChanged()));
  connect(d->m_year, SIGNAL(valueChanged(int)), this, SLOT(slotDateChanged()));
}

KDateWidget::~KDateWidget()
{
}

void KDateWidget::setDate( QDate date )
{
  d->m_day->blockSignals(true);
  d->m_month->blockSignals(true);
  d->m_year->blockSignals(true);

  d->m_day->setMaxValue(date.daysInMonth());
  d->m_day->setValue(date.day());
  d->m_month->setCurrentItem(date.month()-1);
  d->m_year->setValue(date.year());

  d->m_day->blockSignals(false);
  d->m_month->blockSignals(false);
  d->m_year->blockSignals(false);

  d->m_dat = date;
  emit changed(d->m_dat);
}

QDate KDateWidget::date() const
{
  return d->m_dat;
}

void KDateWidget::slotDateChanged( )
{
  QDate date;
  int y,m,day;
  y = d->m_year->value();
  y = QMIN(QMAX(y, 1970), 2038);
  m = d->m_month->currentItem()+1;
  m = QMIN(QMAX(m,1), 12);
  date.setYMD(y,m,1);
  day = d->m_day->value();
  day = QMIN(QMAX(day,1), date.daysInMonth());
  date.setYMD(y,m,day);
  setDate(date);
}

void KDateWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdatewidget.moc"
