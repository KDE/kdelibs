/* This file is part of the KDE libraries
   Copyright (C) 2002 Hans Petter bieker <bieker@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdatetimewidget.h"

#include <QtCore/QDate>
#include <QtGui/QLayout>
#include <QTimeEdit>

#include <kdebug.h>

#include "kdatewidget.h"
#include "kdialog.h"

class KDateTimeWidget::KDateTimeWidgetPrivate
{
public:
  KDateWidget * dateWidget;
  QTimeEdit * timeWidget;
};

KDateTimeWidget::KDateTimeWidget(QWidget * parent)
  : QWidget(parent)
  , d( new KDateTimeWidgetPrivate )
{
  init();
}

KDateTimeWidget::KDateTimeWidget(const QDateTime & datetime,
                                 QWidget * parent)
  : QWidget(parent)
  , d( new KDateTimeWidgetPrivate )

{
  init();

  setDateTime(datetime);
}

KDateTimeWidget::~KDateTimeWidget()
{
  delete d;
}

void KDateTimeWidget::init()
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);

  d->dateWidget = new KDateWidget(this);
  d->timeWidget = new QTimeEdit(this);
  layout->addWidget( d->dateWidget );
  layout->addWidget( d->timeWidget );

  connect(d->dateWidget, SIGNAL(changed(QDate)),
          SLOT(slotValueChanged()));
  connect(d->timeWidget, SIGNAL(timeChanged(const QTime &)),
          SLOT(slotValueChanged()));
}

void KDateTimeWidget::setDateTime(const QDateTime & datetime)
{
  d->dateWidget->setDate(datetime.date());
  d->timeWidget->setTime(datetime.time());
}

QDateTime KDateTimeWidget::dateTime() const
{
  return QDateTime(d->dateWidget->date(), d->timeWidget->time());
}

void KDateTimeWidget::slotValueChanged()
{
  QDateTime datetime(d->dateWidget->date(),
                     d->timeWidget->time());

  kDebug() << "slotValueChanged(): " << datetime << "\n";

  emit valueChanged(datetime);
}

#include "kdatetimewidget.moc"
