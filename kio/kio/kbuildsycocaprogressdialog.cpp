#include "kbuildsycocaprogressdialog.h"
#include <klocale.h>
#include <kapplication.h>
#include <dbus/qdbus.h>

void KBuildSycocaProgressDialog::rebuildKSycoca(QWidget *parent)
{
  KBuildSycocaProgressDialog dlg(parent,
                                 i18n("Updating System Configuration"),
                                 i18n("Updating system configuration."));

  QByteArray data;
  QDBusInterfacePtr kbuildsycoca("org.kde.kded", "/modules/kbuildsycoca",
                                 "org.kde.kbuildsycoca");
  if (kbuildsycoca->isValid())
  {
     kbuildsycoca->callWithArgs("recreate", &dlg, SLOT(slotFinished()));
     dlg.exec();
  }
}

KBuildSycocaProgressDialog::KBuildSycocaProgressDialog(QWidget *_parent,
                          const QString &_caption, const QString &text)
 : QProgressDialog(_parent)
{
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotProgress()));
  setWindowTitle(_caption);
  setModal(true);
  setLabelText(text);
  setRange(0, 20);
  m_timeStep = 700;
  m_timer.start(m_timeStep);
  setAutoClose(false);
}

void
KBuildSycocaProgressDialog::slotProgress()
{
  const int p = value();
  if (p == 18)
  {
     reset();
     setValue(1);
     m_timeStep = m_timeStep * 2;
     m_timer.start(m_timeStep);
  }
  else
  {
     setValue(p+1);
  }
}

void
KBuildSycocaProgressDialog::slotFinished()
{
  setValue(20);
  m_timer.stop();
  QTimer::singleShot(1000, this, SLOT(close()));
}


#include "kbuildsycocaprogressdialog.moc"
