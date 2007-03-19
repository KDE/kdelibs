/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include <QProcess>
#include <QTime>
#include <QLabel>
#include <QCheckBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QGridLayout>

#include <kurl.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kstandarddirs.h>

#include <klocale.h>

#ifdef Q_WS_X11
#include <kwin.h>
#endif

class KWidgetJobTracker::Private
{
public:
    Private(KWidgetJobTracker *object)
        : q(object), totalSize(0), totalFiles(0), totalDirs(0),
          processedSize(0), processedDirs(0), processedFiles(0),
          keepOpenChecked(false),
          widget(0), cancelClose(0), openFile(0), openLocation(0),
          keepOpen(0), pauseButton(0), sourceEdit(0), destEdit(0),
          progressLabel(0), destInvite(0), speedLabel(0), sizeLabel(0),
          resumeLabel(0), progressBar(0), suspended(false) { }

    KWidgetJobTracker *const q;

    qulonglong totalSize;
    qulonglong totalFiles;
    qulonglong totalDirs;
    qulonglong processedSize;
    qulonglong processedDirs;
    qulonglong processedFiles;

    bool keepOpenChecked;
    QString caption;

    QWidget     *widget;
    KPushButton *cancelClose;
    KPushButton *openFile;
    KPushButton *openLocation;
    QCheckBox   *keepOpen;
    KUrl        location;
    QTime       startTime;
    KPushButton *pauseButton;
    KLineEdit *sourceEdit;
    KLineEdit *destEdit;
    QLabel *progressLabel;
    QLabel *sourceInvite;
    QLabel *destInvite;
    QLabel *speedLabel;
    QLabel *sizeLabel;
    QLabel *resumeLabel;
    QProgressBar *progressBar;

    bool suspended;

    void init(QWidget *parent);
    void showTotals();
    void setDestVisible(bool visible);
    void checkDestination(const KUrl &dest);

    void _k_keepOpenToggled(bool);
    void _k_openFile();
    void _k_openLocation();
    void _k_pauseResumeClicked();
};

KWidgetJobTracker::KWidgetJobTracker(QWidget *parent)
    : KAbstractWidgetJobTracker(parent), d(new Private(this))
{
    d->init(parent);
}

KWidgetJobTracker::~KWidgetJobTracker()
{
    delete d->widget;
    delete d;
}

QWidget *KWidgetJobTracker::widget()
{
    return d->widget;
}

bool KWidgetJobTracker::keepOpen() const
{
    return d->keepOpenChecked;
}


void KWidgetJobTracker::infoMessage(KJob */*job*/, const QString &plain, const QString &/*rich*/)
{
    d->speedLabel->setText(plain);
    d->speedLabel->setAlignment(d->speedLabel->alignment() & ~Qt::TextWordWrap);
}

void KWidgetJobTracker::description(KJob */*job*/, const QString &title,
                                    const QPair<QString, QString> &field1,
                                    const QPair<QString, QString> &field2)
{
    if ( d->caption.isEmpty() ) {
        d->widget->setWindowTitle(title);
        d->caption = title;
    }

    d->sourceInvite->setText(field1.first);
    d->sourceEdit->setText(field1.second);

    if (field2.first.isEmpty()) {
        d->setDestVisible(false);
    } else {
        d->setDestVisible(true);
        d->checkDestination(KUrl(field2.second));
        d->destInvite->setText(field2.first);
        d->destEdit->setText(field2.second);
    }
}

void KWidgetJobTracker::totalAmount(KJob */*job*/, KJob::Unit unit, qulonglong amount)
{
    switch(unit)
    {
    case KJob::Bytes:
        // size is measured in bytes
        if (d->totalSize == amount)
            return;
        d->totalSize = amount;
        if (d->startTime.isNull())
            d->startTime.start();
        break;

    case KJob::Files:
        if (d->totalFiles == amount)
            return;
        d->totalFiles = amount;
        d->showTotals();
        break;

    case KJob::Directories:
        if (d->totalDirs == amount)
            return;
        d->totalDirs = amount;
        d->showTotals();
        break;
    }
}

void KWidgetJobTracker::processedAmount(KJob */*job*/, KJob::Unit unit, qulonglong amount)
{
    QString tmp;

    switch(unit)
    {
    case KJob::Bytes:
        if (d->processedSize == amount)
            return;
        d->processedSize = amount;

        tmp = i18n( "%1 of %2 complete",
                    KGlobal::locale()->formatByteSize(amount),
                    KGlobal::locale()->formatByteSize(d->totalSize));
        d->sizeLabel->setText(tmp);
        break;

    case KJob::Directories:
        if (d->processedDirs == amount)
            return;
        d->processedDirs = amount;

        tmp = i18np("%2 / %1 folder", "%2 / %1 folders", d->totalDirs,  d->processedDirs);
        tmp += "   ";
        tmp += i18np("%2 / %1 file", "%2 / %1 files", d->totalFiles,  d->processedFiles);
        d->progressLabel->setText(tmp);
        break;

    case KJob::Files:
        if (d->processedFiles == amount)
            return;
        d->processedFiles = amount;

        if (d->totalDirs > 1) {
            tmp = i18np("%2 / %1 folder", "%2 / %1 folders", d->totalDirs,  d->processedDirs);
            tmp += "   ";
        }
        tmp += i18np("%2 / %1 file", "%2 / %1 files", d->totalFiles,  d->processedFiles);
        d->progressLabel->setText(tmp);
    }
}

void KWidgetJobTracker::percent(KJob */*job*/, unsigned long percent)
{
    QString title = d->caption+" (";

    if (d->totalSize)
        title+= i18n("%1 % of %2 ", percent ,
                     KGlobal::locale()->formatByteSize(d->totalSize));
    else if(d->totalFiles)
        title+= i18np("%2 % of 1 file", "%2 % of %1 files", d->totalFiles, percent);
    else
        title+= i18n("%1 %",  percent);

    title+=')';

    d->progressBar->setValue(percent);
    d->widget->setWindowTitle(title);
}

void KWidgetJobTracker::speed(KJob */*job*/, unsigned long value)
{
    if (value == 0) {
        d->speedLabel->setText(i18n("Stalled"));
    } else {
        int remaining = 0;

        if ((value != 0) && (d->totalSize != 0)) {
            remaining = 1000*(d->totalSize - d->processedSize)/value;
        }

        d->speedLabel->setText(i18n("%1/s ( %2 remaining )", KGlobal::locale()->formatByteSize(value),
                                    KGlobal::locale()->formatDuration(remaining)));
    }
}

void KWidgetJobTracker::slotClean()
{
    if (!d->keepOpenChecked) {
        d->widget->hide();
    } else {
        percent(0, 100);
        d->cancelClose->setGuiItem(KStandardGuiItem::close());
        d->openFile->setEnabled(true);
        processedAmount(0, KJob::Bytes, d->totalSize);
        d->keepOpen->setEnabled(false);
        if (!d->startTime.isNull()) {
            int s = d->startTime.elapsed();
            if (!s)
                s = 1;
            d->speedLabel->setText(i18n("%1/s (done)",
                                        KGlobal::locale()->formatByteSize(1000 * d->totalSize / s)));
        }
        setAutoDelete(true);
    }
}

void KWidgetJobTracker::Private::init(QWidget *parent)
{
    widget = new QWidget(parent);

#ifdef Q_WS_X11 //FIXME(E): Remove once all the KWin::foo calls have been ported to QWS
    // Set a useful icon for this window!
    KWin::setIcons( widget->winId(),
                    KIconLoader::global()->loadIcon( "document-save", K3Icon::NoGroup, 32 ),
                    KIconLoader::global()->loadIcon( "document-save", K3Icon::NoGroup, 16 ) );
#endif

    QVBoxLayout *topLayout = new QVBoxLayout(widget);
    topLayout->setMargin(KDialog::marginHint());
    topLayout->setSpacing(KDialog::spacingHint() );
    topLayout->addStrut( 360 );   // makes dlg at least that wide

    QGridLayout *grid = new QGridLayout();
    topLayout->addLayout(grid);
    grid->addItem(new QSpacerItem(KDialog::spacingHint(),0),0,1); //addColSpacing(1, KDialog::spacingHint());
    // filenames or action name
    sourceInvite = new QLabel(i18n("Source:"), widget);
    grid->addWidget(sourceInvite, 0, 0);

    sourceEdit = new KLineEdit(widget);
    sourceEdit->setReadOnly(true);
    sourceEdit->setSqueezedTextEnabled(true);
    grid->addWidget(sourceEdit, 0, 2);

    destInvite = new QLabel(i18n("Destination:"), widget);
    grid->addWidget(destInvite, 1, 0);

    destEdit = new KLineEdit(widget);
    destEdit->setReadOnly (true);
    destEdit->setSqueezedTextEnabled(true);
    grid->addWidget(destEdit, 1, 2);

    QHBoxLayout *progressHBox = new QHBoxLayout();
    topLayout->addLayout(progressHBox);

    progressBar = new QProgressBar(widget);
    progressHBox->addWidget(progressBar);

    suspended = false;
    pauseButton = new KPushButton(i18n("Pause"), widget);
    QObject::connect(pauseButton, SIGNAL(clicked()),
                     q, SLOT(_k_pauseResumeClicked()));
    progressHBox->addWidget(pauseButton);

    // processed info
    QHBoxLayout *hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    sizeLabel = new QLabel(widget);
    hBox->addWidget(sizeLabel);

    resumeLabel = new QLabel(widget);
    hBox->addWidget(resumeLabel);

    progressLabel = new QLabel(widget);
/*    progressLabel->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                            QSizePolicy::Preferred));*/
    progressLabel->setAlignment(Qt::AlignRight);
    hBox->addWidget(progressLabel);

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    speedLabel = new QLabel(widget);
    hBox->addWidget(speedLabel, 1);

    QFrame *line = new QFrame(widget);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    topLayout->addWidget(line);

    keepOpen = new QCheckBox(i18n("&Keep this window open after transfer is complete"), widget);
    QObject::connect(keepOpen, SIGNAL(toggled(bool)),
                     q, SLOT(_k_keepOpenToggled(bool)));
    topLayout->addWidget(keepOpen);
    keepOpen->hide();

    hBox = new QHBoxLayout();
    topLayout->addLayout(hBox);

    openFile = new KPushButton(i18n("Open &File"), widget);
    QObject::connect(openFile, SIGNAL(clicked()),
                     q, SLOT(_k_openFile()));
    hBox->addWidget(openFile);
    openFile->setEnabled(false);
    openFile->hide();

    openLocation = new KPushButton(i18n("Open &Destination"), widget);
    QObject::connect(openLocation, SIGNAL(clicked()),
                     q, SLOT(_k_openLocation()));
    hBox->addWidget(openLocation);
    openLocation->hide();

    hBox->addStretch(1);

    cancelClose = new KPushButton(KStandardGuiItem::cancel(), widget);
    QObject::connect(cancelClose, SIGNAL(clicked()),
                     q, SLOT(slotStop()));
    hBox->addWidget(cancelClose);

    widget->resize(widget->sizeHint());
    widget->setMaximumHeight(widget->sizeHint().height());

    keepOpenChecked = false;
    widget->setWindowTitle(i18n("Progress Dialog")); // show something better than kuiserver
}

void KWidgetJobTracker::Private::showTotals()
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

void KWidgetJobTracker::Private::setDestVisible(bool visible)
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
}

void KWidgetJobTracker::Private::checkDestination(const KUrl &dest)
{
    bool ok = true;

    if (dest.isLocalFile()) {
        QString path = dest.path( KUrl::RemoveTrailingSlash );
        QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
        for (QStringList::Iterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it)
            if (path.contains(*it))
                ok = false; // it's in the tmp resource
    }

    if (ok) {
        openFile->show();
        openLocation->show();
        keepOpen->show();
        location=dest;
    }
}

void KWidgetJobTracker::Private::_k_keepOpenToggled(bool keepOpen)
{
    keepOpenChecked = keepOpen;
}

void KWidgetJobTracker::Private::_k_openFile()
{
    QProcess::startDetached("konqueror", QStringList() << location.prettyUrl());
}

void KWidgetJobTracker::Private::_k_openLocation()
{
    location.setFileName("");
    _k_openFile();
}

void KWidgetJobTracker::Private::_k_pauseResumeClicked()
{
    if ( !suspended ) {
        pauseButton->setText(i18n("Resume"));
        q->slotSuspend();
        suspended = true;
    } else {
        pauseButton->setText(i18n("Pause"));
        q->slotResume();
        suspended = false;
    }
}

#include "kwidgetjobtracker.moc"
