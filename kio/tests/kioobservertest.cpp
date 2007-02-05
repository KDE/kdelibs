/**
  * This file is part of the KDE libraries
  * Copyright (C) 2007 Rafael Fernández López <ereslibre@gmail.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "kioobservertest.h"
#include <kio/observer.h>
#include <QTimer>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

KJobDelegateTest::KJobDelegateTest()
    : KJobUiDelegate()
{
}

KJobDelegateTest::~KJobDelegateTest()
{
}

void KJobDelegateTest::connectJob(KJob *job)
{
    job->setProgressId(Observer::self()->newJob(job, Observer::JobShown));

    connect(job, SIGNAL(percent(KJob*,unsigned long)),
            Observer::self(), SLOT(slotPercent(KJob*,unsigned long)));

    connect(job, SIGNAL(infoMessage(KJob*,const QString&,const QString&)),
            Observer::self(), SLOT(slotInfoMessage(KJob*,const QString&)));

    connect(job, SIGNAL(totalSize(KJob*,qulonglong)),
            Observer::self(), SLOT(slotTotalSize(KJob*,qulonglong)));

    connect(job, SIGNAL(processedSize(KJob*,qulonglong)),
            Observer::self(), SLOT(slotProcessedSize(KJob*,qulonglong)));

    connect(job, SIGNAL(finished(KJob*,int)),
            this, SLOT(slotFinished(KJob*)));

    connect(job, SIGNAL(warning(KJob*,const QString&) ),
            this, SLOT(slotWarning(KJob*,const QString&)));
}

void KJobDelegateTest::slotFinished(KJob *job)
{
    Observer::self()->jobFinished(job->progressId());
}

KJobTest::KJobTest(int numberOfSeconds)
    : KJob(), timer(new QTimer(this)), clockTimer(new QTimer(this)), seconds(numberOfSeconds)
{
    setCapabilities(KJob::NoCapabilities);
    setUiDelegate(new KJobDelegateTest());
}

KJobTest::~KJobTest()
{
}

void KJobTest::start()
{
    connect(timer, SIGNAL(timeout()), this,
            SLOT(timerTimeout()));

    connect(clockTimer, SIGNAL(timeout()), this,
            SLOT(updateMessage()));

    timer->setSingleShot(true);
    timer->start(seconds * 1000);

    updateMessage();

    clockTimer->start(1000);
}

void KJobTest::timerTimeout()
{
    clockTimer->stop();

    emit finished(this, progressId());
}

void KJobTest::updateMessage()
{
    emit infoMessage(this, i18n("Testing kuiserver (%1 seconds remaining)", seconds), i18n("Testing kuiserver (%1 seconds remaining)", seconds));

    seconds--;
}

#include "kioobservertest.moc"

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kjobtest", "KJobTest", "A KJob tester", "0.01");

    KApplication app;

    KJobTest *myJob = new KJobTest(10 /* 10 seconds before it gets removed */);
    myJob->start();

    return app.exec();
}
