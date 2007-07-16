/* This file is part of the KDE project
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

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
#include "kbuildsycocaprogressdialog.h"
#include <klocale.h>
#include <kapplication.h>
#include <QtDBus/QtDBus>

class KBuildSycocaProgressDialogPrivate
{
public:
    QTimer m_timer;
    int m_timeStep;
};

void KBuildSycocaProgressDialog::rebuildKSycoca(QWidget *parent)
{
  KBuildSycocaProgressDialog dlg(parent,
                                 i18n("Updating System Configuration"),
                                 i18n("Updating system configuration."));

  QDBusInterface kbuildsycoca("org.kde.kded", "/kbuildsycoca",
                              "org.kde.kbuildsycoca");
  if (kbuildsycoca.isValid())
  {
     kbuildsycoca.callWithCallback("recreate", QVariantList(), &dlg, SLOT(slotFinished()));
     dlg.exec();
  }
}

KBuildSycocaProgressDialog::KBuildSycocaProgressDialog(QWidget *_parent,
                          const QString &_caption, const QString &text)
 : QProgressDialog(_parent)
 , d( new KBuildSycocaProgressDialogPrivate )
{
  connect(&d->m_timer, SIGNAL(timeout()), this, SLOT(slotProgress()));
  setWindowTitle(_caption);
  setModal(true);
  setLabelText(text);
  setRange(0, 20);
  d->m_timeStep = 700;
  d->m_timer.start(d->m_timeStep);
  setAutoClose(false);
}

KBuildSycocaProgressDialog::~KBuildSycocaProgressDialog()
{
    delete d;
}

void
KBuildSycocaProgressDialog::slotProgress()
{
  const int p = value();
  if (p == 18)
  {
     reset();
     setValue(1);
     d->m_timeStep = d->m_timeStep * 2;
     d->m_timer.start(d->m_timeStep);
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
  d->m_timer.stop();
  QTimer::singleShot(1000, this, SLOT(close()));
}


#include "kbuildsycocaprogressdialog.moc"
