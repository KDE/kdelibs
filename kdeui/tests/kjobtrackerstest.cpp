/**
  * This file is part of the KDE libraries
  * Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
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

#include "kjobtrackerstest.h"

#include <QWidget>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <kwidgetjobtracker.h>
#include <kstatusbarjobtracker.h>
#include <kuiserverjobtracker.h>

KTestJob::KTestJob(int numberOfDirs)
    : KJob(), m_numberOfDirs(numberOfDirs), m_currentSpeed(1000), m_state(Stopped)
{
    setCapabilities(KJob::Killable|KJob::Suspendable);
}

KTestJob::~KTestJob()
{
}

void KTestJob::start()
{
    connect(&m_timer, SIGNAL(timeout()),
            this, SLOT(nextStep()));
    m_state = StatingDirs;
    m_timer.start(50);
    emit description(this, QString("Copying"), qMakePair(QString("Source"), QString("file:/src")),
                     qMakePair(QString("Destination"), QString("file:/dest")));
}

void KTestJob::nextStep()
{
    switch (m_state)
    {
    case StatingDirs:
        emit infoMessage(this, QString("Initial listing"));
        stateNextDir();
        break;
    case CreatingDirs:
        emit infoMessage(this, QString("Folder creation"));
        createNextDir();
        break;
    case CopyingFiles:
        emit infoMessage(this, QString("Actual file copying"));
        copyNextFile();
        break;
    case Stopped:
        kDebug() << "Do nothing, we stopped" << endl;
    }
}

void KTestJob::stateNextDir()
{
    if (totalAmount(KJob::Directories)==m_numberOfDirs) {
        m_state = CreatingDirs;
        return;
    }

    QString directory_name = "dir"+QString::number(totalAmount(KJob::Directories));

    kDebug() << "Stating " << directory_name << endl;
    setTotalAmount(KJob::Directories, totalAmount(KJob::Directories)+1);
    setTotalAmount(KJob::Files, totalAmount(KJob::Directories)*10);
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Files)*1000);

    emit description(this, QString("Stating"), qMakePair(QString("Stating"), "file:/src/"+directory_name));
}

void KTestJob::createNextDir()
{
    if (processedAmount(KJob::Directories)==totalAmount(KJob::Directories)) {
        m_state = CopyingFiles;
        return;
    }

    QString directory_name = "dir"+QString::number(processedAmount(KJob::Directories));

    kDebug() << "Creating " << directory_name << endl;
    setProcessedAmount(KJob::Directories, processedAmount(KJob::Directories)+1);

    emit description(this, QString("Creating Dir"), qMakePair(QString("Creating"), "file:/dest/"+directory_name));
}

void KTestJob::copyNextFile()
{
    if (processedAmount(KJob::Files)==totalAmount(KJob::Files)) {
        m_state = Stopped;
        m_timer.stop();
        emitResult();
        return;
    }

    QString file_name = "dir"+QString::number(processedAmount(KJob::Files)/10)
                        +"/file"+QString::number(processedAmount(KJob::Files)%10);

    kDebug() << "Copying " << file_name << endl;
    setProcessedAmount(KJob::Files, processedAmount(KJob::Files)+1);
    setProcessedAmount(KJob::Bytes, processedAmount(KJob::Bytes)+1000);

    emit description(this, QString("Copying"), qMakePair(QString("Source"), "file:/src/"+file_name),
                     qMakePair(QString("Destination"), "file:/dest/"+file_name));

    emitSpeed(m_currentSpeed);
}

bool KTestJob::doSuspend()
{
    m_timer.stop();
    return true;
}

bool KTestJob::doResume()
{
    m_timer.start(50);
    return true;
}

bool KTestJob::doKill()
{
    m_timer.stop();
    m_state = Stopped;
    deleteLater();
    return true;
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kjobtrackerstest", "KJobTrackerTest",
                       "Test several job trackers at once", "0.0");

    KApplication app;

    QWidget *main = new QWidget;
    main->show();

    KTestJob *testJob = new KTestJob(10 /* 100000 bytes to process */);

    KWidgetJobTracker *tracker1 = new KWidgetJobTracker();
    tracker1->registerJob(testJob);

    KStatusBarJobTracker *tracker2 = new KStatusBarJobTracker(main, true);
    tracker2->registerJob(testJob);

    KUiServerJobTracker *tracker3 = new KUiServerJobTracker(main);
    tracker3->registerJob(testJob);

    testJob->start();

    tracker1->widget()->show();
    tracker2->widget()->show();

    return app.exec();
}

#include "kjobtrackerstest.moc"
