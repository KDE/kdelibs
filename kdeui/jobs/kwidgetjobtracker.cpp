/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2009 Shaun Reich <shaun.reich@kdemail.net>

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

#include "kwidgetjobtracker.h"
#include "kwidgetjobtracker_p.h"

#include <QProcess>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMenu>
#include <QEvent>

#include <kurl.h>
#include <kpushbutton.h>
#include <ksqueezedtextlabel.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwindowsystem.h>
#include <kseparator.h>

void KWidgetJobTracker::Private::_k_showProgressWidget()
{
    if (progressWidgetsToBeShown.isEmpty()) {
        return;
    }

    KJob *job = progressWidgetsToBeShown.dequeue();

    // If the job has been unregistered before reaching this point, widget will
    // return 0.
    QWidget *widget = q->widget(job);

    if (widget) {
        widget->show();
    }
}

KWidgetJobTracker::KWidgetJobTracker(QWidget *parent)
    : KAbstractWidgetJobTracker(parent), d(new Private(parent, this))
{
}

KWidgetJobTracker::~KWidgetJobTracker()
{
    delete d;
}

QWidget *KWidgetJobTracker::widget(KJob *job)
{
    return d->progressWidget.value(job, 0);
}

void KWidgetJobTracker::registerJob(KJob *job)
{
    Private::ProgressWidget *vi = new Private::ProgressWidget(job, this, d->parent);
    vi->jobRegistered = true;
    vi->setAttribute(Qt::WA_DeleteOnClose);
    d->progressWidget.insert(job, vi);
    d->progressWidgetsToBeShown.enqueue(job);

    KAbstractWidgetJobTracker::registerJob(job);

    QTimer::singleShot(500, this, SLOT(_k_showProgressWidget()));
}

void KWidgetJobTracker::unregisterJob(KJob *job)
{
    KAbstractWidgetJobTracker::unregisterJob(job);

    d->progressWidgetsToBeShown.removeAll(job);
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->jobRegistered = false;
    pWidget->deref();
}

bool KWidgetJobTracker::keepOpen(KJob *job) const
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return false;
    }

    return pWidget->keepOpenCheck->isChecked();
}

void KWidgetJobTracker::infoMessage(KJob *job, const QString &plain, const QString &rich)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->infoMessage(plain, rich);
}

void KWidgetJobTracker::description(KJob *job, const QString &title,
                                    const QPair<QString, QString> &field1,
                                    const QPair<QString, QString> &field2)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->description(title, field1, field2);
}

void KWidgetJobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->totalAmount(unit, amount);
}

void KWidgetJobTracker::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->processedAmount(unit, amount);
}

void KWidgetJobTracker::percent(KJob *job, unsigned long percent)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->percent(percent);
}

void KWidgetJobTracker::speed(KJob *job, unsigned long value)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->speed(value);
}

void KWidgetJobTracker::slotClean(KJob *job)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->slotClean();
}

void KWidgetJobTracker::suspended(KJob *job)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->suspended();
}

void KWidgetJobTracker::resumed(KJob *job)
{
    KWidgetJobTracker::Private::ProgressWidget *pWidget = d->progressWidget.value(job, 0);
    if (!pWidget) {
        return;
    }

    pWidget->resumed();
}

void KWidgetJobTracker::Private::ProgressWidget::ref()
{
    ++refCount;
}

void KWidgetJobTracker::Private::ProgressWidget::deref()
{
    if (refCount) {
        --refCount;
    }

    if (!refCount) {
        if (!keepOpenCheck->isChecked()) {
            closeNow();
        } else {
            slotClean();
        }
    }
}

void KWidgetJobTracker::Private::ProgressWidget::closeNow()
{
    close();

    // It might happen the next scenario:
    // - Start a job which opens a progress widget. Keep it open. Address job is 0xdeadbeef
    // - Start a new job, which is given address 0xdeadbeef. A new window is opened.
    //   This one will take much longer to complete. The key 0xdeadbeef on the widget map now
    //   stores the new widget address.
    // - Close the first progress widget that was opened (and has already finished) while the
    //   last one is still running. We remove its reference on the map. Wrong.
    // For that reason we have to check if the map stores the widget as the current one.
    // ereslibre
    if (tracker->d->progressWidget[job] == this) {
        tracker->d->progressWidget.remove(job);
        tracker->d->progressWidgetsToBeShown.removeAll(job);
    }
}

bool KWidgetJobTracker::Private::ProgressWidget::eventFilter(QObject *watched, QEvent *event)
{
    // Handle context menu events for the source/dest labels here, so that we are ref()ed while the
    // menu is exec()ed, to avoid a crash if the job finishes meanwhile. #159621.
    if ((watched == sourceEdit || watched == destEdit) && event->type() == QEvent::ContextMenu) {
        ref();
        watched->event(event);
        deref();
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void KWidgetJobTracker::Private::ProgressWidget::infoMessage(const QString &plain, const QString &/*rich*/)
{
    speedLabel->setText(plain);
    speedLabel->setAlignment(speedLabel->alignment() & ~Qt::TextWordWrap);
}

void KWidgetJobTracker::Private::ProgressWidget::description(const QString &title,
                                                                const QPair<QString, QString> &field1,
                                                                const QPair<QString, QString> &field2)
{
    setWindowTitle(title);
    caption = title;

    sourceInvite->setText(i18nc("%1 is the label, we add a ':' to it", "%1:", field1.first));
    sourceEdit->setText(field1.second);

    if (field2.first.isEmpty()) {
        setDestVisible(false);
    } else {
        setDestVisible(true);
        checkDestination(KUrl(field2.second));
        destInvite->setText(i18nc("%1 is the label, we add a ':' to it", "%1:", field2.first));
        destEdit->setText(field2.second);
    }
}

void KWidgetJobTracker::Private::ProgressWidget::totalAmount(KJob::Unit unit, qulonglong amount)
{
    switch(unit)
    {
    case KJob::Bytes:
        totalSizeKnown = true;
        // size is measured in bytes
        if (totalSize == amount)
            return;
        totalSize = amount;
        if (startTime.isNull())
            startTime.start();
        break;

    case KJob::Files:
        if (totalFiles == amount)
            return;
        totalFiles = amount;
        showTotals();
        break;

    case KJob::Directories:
        if (totalDirs == amount)
            return;
        totalDirs = amount;
        showTotals();
        break;
    }
}

void KWidgetJobTracker::Private::ProgressWidget::processedAmount(KJob::Unit unit, qulonglong amount)
{
    QString tmp;

    switch(unit)
    {
    case KJob::Bytes:
        if (processedSize == amount)
            return;
        processedSize = amount;

        if (totalSizeKnown) {
            tmp = i18np( "%2 of %3 complete", "%2 of %3 complete",
						amount,
                        KGlobal::locale()->formatByteSize(amount),
                        KGlobal::locale()->formatByteSize(totalSize));
        } else {
            tmp = KGlobal::locale()->formatByteSize(amount);
        }
        sizeLabel->setText(tmp);
        if (!totalSizeKnown) // update jumping progressbar
            progressBar->setValue(amount);
        break;

    case KJob::Directories:
        if (processedDirs == amount)
            return;
        processedDirs = amount;

        tmp = i18np("%2 / %1 folder", "%2 / %1 folders", totalDirs,  processedDirs);
        tmp += "   ";
        tmp += i18np("%2 / %1 file", "%2 / %1 files", totalFiles,  processedFiles);
        progressLabel->setText(tmp);
        break;

    case KJob::Files:
        if (processedFiles == amount)
            return;
        processedFiles = amount;

        if (totalDirs > 1) {
            tmp = i18np("%2 / %1 folder", "%2 / %1 folders", totalDirs,  processedDirs);
            tmp += "   ";
        }
        tmp += i18np("%2 / %1 file", "%2 / %1 files", totalFiles,  processedFiles);
        progressLabel->setText(tmp);
    }
}

void KWidgetJobTracker::Private::ProgressWidget::percent(unsigned long percent)
{
    QString title = caption + " (";

    if (totalSizeKnown) {
        title += i18n("%1% of %2", percent,
                      KGlobal::locale()->formatByteSize(totalSize));
    } else if (totalFiles) {
        title += i18np("%2% of 1 file", "%2% of %1 files", totalFiles, percent);
    } else {
        title += i18n("%1%", percent);
    }

    title += ')';

    progressBar->setMaximum(100);
    progressBar->setValue(percent);
    setWindowTitle(title);
}

void KWidgetJobTracker::Private::ProgressWidget::speed(unsigned long value)
{
    if (value == 0) {
        speedLabel->setText(i18n("Stalled"));
    } else {
        const QString speedStr = KGlobal::locale()->formatByteSize(value);
        if (totalSizeKnown) {
            const int remaining = 1000*(totalSize - processedSize)/value;
            speedLabel->setText(i18np("%2/s (%3 remaining)", "%2/s (%3 remaining)", remaining, speedStr,
                                     KGlobal::locale()->prettyFormatDuration(remaining)));
        } else { // total size is not known (#24228)
            speedLabel->setText(i18nc("speed in bytes per second", "%1/s", speedStr));
        }
    }
}

void KWidgetJobTracker::Private::ProgressWidget::slotClean()
{
    percent(100);
    cancelClose->setGuiItem(KStandardGuiItem::close());
    openFile->setEnabled(true);
    if (!totalSizeKnown || totalSize < processedSize)
        totalSize = processedSize;
    processedAmount(KJob::Bytes, totalSize);
    keepOpenCheck->setEnabled(false);
    pauseButton->setEnabled(false);
    if (!startTime.isNull()) {
        int s = startTime.elapsed();
        if (!s)
            s = 1;
        speedLabel->setText(i18n("%1/s (done)",
                                    KGlobal::locale()->formatByteSize(1000 * totalSize / s)));
    }
}

void KWidgetJobTracker::Private::ProgressWidget::suspended()
{
    pauseButton->setText(i18n("&Resume"));
    suspendedProperty = true;
}

void KWidgetJobTracker::Private::ProgressWidget::resumed()
{
    pauseButton->setText(i18n("&Pause"));
    suspendedProperty = false;
}

void KWidgetJobTracker::Private::ProgressWidget::closeEvent(QCloseEvent *event)
{
    if (jobRegistered && tracker->stopOnClose(job)) {
        tracker->slotStop(job);
    }

    QWidget::closeEvent(event);
}

void KWidgetJobTracker::Private::ProgressWidget::init()
{
    // Set a useful icon for this window!
    KWindowSystem::setIcons( winId(),
                             KIconLoader::global()->loadIcon( "document-save", KIconLoader::NoGroup, 32 ),
                             KIconLoader::global()->loadIcon( "document-save", KIconLoader::NoGroup, 16 ) );

    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QGridLayout *grid = new QGridLayout();
    topLayout->addLayout(grid);
    grid->addItem(new QSpacerItem(KDialog::spacingHint(),0),0,1);
    // filenames or action name
    sourceInvite = new QLabel(i18nc("The source url of a job", "Source:"), this);
    grid->addWidget(sourceInvite, 0, 0);

    sourceEdit = new KSqueezedTextLabel(this);
    sourceEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    sourceEdit->installEventFilter(this);
    grid->addWidget(sourceEdit, 0, 2);

    destInvite = new QLabel(i18nc("The destination url of a job", "Destination:"), this);
    grid->addWidget(destInvite, 1, 0);

    destEdit = new KSqueezedTextLabel(this);
    destEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    destEdit->installEventFilter(this);
    grid->addWidget(destEdit, 1, 2);

    QHBoxLayout *progressHBox = new QHBoxLayout();
    topLayout->addLayout(progressHBox);

    progressBar = new QProgressBar(this);
    progressBar->setMaximum(0); // want a jumping progress bar if percent is not emitted
    progressHBox->addWidget(progressBar);

    suspendedProperty = false;

    // processed info
    QHBoxLayout *hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    arrowButton = new KPushButton(this);
    arrowButton->setMaximumSize(QSize(32,25));
    arrowButton->setIcon(KIcon("arrow-down"));
    arrowButton->setToolTip(i18n("Click this to expand the dialog, to show details"));
    arrowState = Qt::DownArrow;
    connect(arrowButton, SIGNAL(clicked()), this, SLOT(_k_arrowToggled()));
    hBox->addWidget(arrowButton);
    hBox->addStretch(1);

    KSeparator *separator1 = new KSeparator(Qt::Horizontal, this);
    topLayout->addWidget(separator1);

    sizeLabel = new QLabel(this);
    hBox->addWidget(sizeLabel, 0, Qt::AlignLeft);

    resumeLabel = new QLabel(this);
    hBox->addWidget(resumeLabel);

    pauseButton = new KPushButton(i18n("&Pause"), this);
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(_k_pauseResumeClicked()));
    hBox->addWidget(pauseButton);

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    speedLabel = new QLabel(this);
    hBox->addWidget(speedLabel, 1);
    speedLabel->hide();

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    progressLabel = new QLabel(this);
    progressLabel->setAlignment(Qt::AlignLeft);
    hBox->addWidget(progressLabel);
    progressLabel->hide();

    keepOpenCheck = new QCheckBox(i18n("&Keep this window open after transfer is complete"), this);
    connect(keepOpenCheck, SIGNAL(toggled(bool)), this, SLOT(_k_keepOpenToggled(bool)));
    topLayout->addWidget(keepOpenCheck);
    keepOpenCheck->hide();

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    openFile = new KPushButton(i18n("Open &File"), this);
    connect(openFile, SIGNAL(clicked()), this, SLOT(_k_openFile()));
    hBox->addWidget(openFile);
    openFile->setEnabled(false);
    openFile->hide();

    openLocation = new KPushButton(i18n("Open &Destination"), this);
    connect(openLocation, SIGNAL(clicked()), this, SLOT(_k_openLocation()));
    hBox->addWidget(openLocation);
    openLocation->hide();

    hBox->addStretch(1);

    cancelClose = new KPushButton(KStandardGuiItem::cancel(), this);
    connect(cancelClose, SIGNAL(clicked()), this, SLOT(_k_stop()));
    hBox->addWidget(cancelClose);

    resize(sizeHint());
    setMaximumHeight(sizeHint().height());

    setWindowTitle(i18n("Progress Dialog")); // show something better than kuiserver
}

void KWidgetJobTracker::Private::ProgressWidget::showTotals()
{
    // Show the totals in the progress label, if we still haven't
    // processed anything. This is useful when the stat'ing phase
    // of CopyJob takes a long time (e.g. over networks).
    if (processedFiles == 0 && processedDirs == 0)
    {
        QString tmps;
        if (totalDirs > 1)
            // that we have a singular to translate looks weired but is only logical
            tmps = i18np("%1 folder", "%1 folders", totalDirs) + "   ";
        tmps += i18np("%1 file", "%1 files", totalFiles);
        progressLabel->setText( tmps );
    }
}

void KWidgetJobTracker::Private::ProgressWidget::setDestVisible(bool visible)
{
    // We can't hide the destInvite/destEdit labels,
    // because it screws up the QGridLayout.
    if (visible)
    {
        destInvite->show();
        destEdit->show();
    }
    else
    {
        destInvite->hide();
        destEdit->hide();
        destInvite->setText( QString() );
        destEdit->setText( QString() );
    }
    setMaximumHeight(sizeHint().height());
}

void KWidgetJobTracker::Private::ProgressWidget::checkDestination(const KUrl &dest)
{
    bool ok = true;

    if (dest.isLocalFile()) {
        QString path = dest.toLocalFile( KUrl::RemoveTrailingSlash );
        const QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
        for (QStringList::ConstIterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it)
            if (path.contains(*it))
                ok = false; // it's in the tmp resource
    }

    if (ok) {
        openFile->show();
        openLocation->show();
        keepOpenCheck->show();
        setMaximumHeight(sizeHint().height());
        location=dest;
    }
}

void KWidgetJobTracker::Private::ProgressWidget::_k_keepOpenToggled(bool keepOpen)
{
    if (keepOpen) {
        KGlobal::ref();
    } else {
        KGlobal::deref();
    }
}

void KWidgetJobTracker::Private::ProgressWidget::_k_openFile()
{
    QProcess::startDetached("kde-open", QStringList() << location.prettyUrl());
}

void KWidgetJobTracker::Private::ProgressWidget::_k_openLocation()
{
    KUrl dirLocation(location);
    dirLocation.setFileName(QString());
    QProcess::startDetached("kde-open", QStringList() << dirLocation.prettyUrl());
}

void KWidgetJobTracker::Private::ProgressWidget::_k_pauseResumeClicked()
{
    if (jobRegistered && !suspendedProperty) {
        tracker->slotSuspend(job);
    } else if (jobRegistered) {
        tracker->slotResume(job);
    }
}

void KWidgetJobTracker::Private::ProgressWidget::_k_stop()
{
    if (jobRegistered) {
        tracker->slotStop(job);
    }
    closeNow();
}

void KWidgetJobTracker::Private::ProgressWidget::_k_arrowToggled()
{
    if (arrowState == Qt::DownArrow) {
        //The arrow is in the down position, dialog is collapsed, expand it and change icon.
        progressLabel->show();
        speedLabel->show();
        arrowButton->setIcon(KIcon("arrow-up"));
        arrowButton->setToolTip(i18n("Click this to collapse the dialog, to hide details"));
        arrowState = Qt::UpArrow;
    } else {
        //Collapse the dialog
        progressLabel->hide();
        speedLabel->hide();
        arrowButton->setIcon(KIcon("arrow-down"));
        arrowButton->setToolTip(i18n("Click this to expand the dialog, to show details"));
        arrowState = Qt::DownArrow;
    }
    setMaximumHeight(sizeHint().height());
}

#include "kwidgetjobtracker.moc"
#include "kwidgetjobtracker_p.moc"
