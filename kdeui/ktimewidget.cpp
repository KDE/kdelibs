#include <q3datetimeedit.h>
#include <qlayout.h>

#include "kdebug.h"
#include "kdialog.h"

#include "ktimewidget.h"

class KTimeWidget::KTimeWidgetPrivate
{
public:
  Q3TimeEdit * timeWidget;
};

KTimeWidget::KTimeWidget(QWidget * parent)
  : QWidget(parent)
{
  init();
}

KTimeWidget::KTimeWidget(const QTime & time,
                         QWidget * parent)
  : QWidget(parent)
{
  init();

  setTime(time);
}

KTimeWidget::~KTimeWidget()
{
  delete d;
}

void KTimeWidget::init()
{
  d = new KTimeWidgetPrivate;

  QHBoxLayout *layout = new QHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);

  d->timeWidget = new Q3TimeEdit(this);

  connect(d->timeWidget, SIGNAL(valueChanged(const QTime &)),
          SIGNAL(valueChanged(const QTime &)));
}

void KTimeWidget::setTime(const QTime & time)
{
  d->timeWidget->setTime(time);
}

QTime KTimeWidget::time() const
{
  return d->timeWidget->time();
}

#include "ktimewidget.moc"
