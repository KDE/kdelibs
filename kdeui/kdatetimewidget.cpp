#include <qlayout.h>

#include "kdatewidget.h"
#include "kdebug.h"
#include "kdialog.h"
#include "ktimewidget.h"

#include "kdatetimewidget.h"

class KDateTimeWidget::KDateTimeWidgetPrivate
{
public:
  KDateWidget * dateWidget;
  KTimeWidget * timeWidget;
};

KDateTimeWidget::KDateTimeWidget(QWidget * parent, const char * name)
  : QWidget(parent, name)
{
  init();
}

KDateTimeWidget::KDateTimeWidget(const QDateTime & datetime,
			     QWidget * parent, const char * name)
  : QWidget(parent, name)
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
  d = new KDateTimeWidgetPrivate;

  QHBoxLayout *layout = new QHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);

  d->dateWidget = new KDateWidget(this);
  d->timeWidget = new KTimeWidget(this);

  connect(d->dateWidget, SIGNAL(changed(QDate)),
          SLOT(slotValueChanged()));
  connect(d->timeWidget, SIGNAL(valueChanged(const QTime &)),
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

  kdDebug() << "slotValueChanged(): " << datetime << "\n";

  emit valueChanged(datetime);
}

#include "kdatetimewidget.moc"
