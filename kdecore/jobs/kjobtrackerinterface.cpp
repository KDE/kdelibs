#include "kjobtrackerinterface.h"

#include "kjob.h"

class KJobTrackerInterface::Private
{
public:
    Private(KJobTrackerInterface *interface) : q(interface)
    {

    }

    KJobTrackerInterface *const q;
};

KJobTrackerInterface::KJobTrackerInterface(QObject *parent)
    : QObject(parent), d(new Private(this))
{

}

KJobTrackerInterface::~KJobTrackerInterface()
{

}

void KJobTrackerInterface::registerJob(KJob *job)
{
    QObject::connect(job, SIGNAL(finished(KJob*)),
                     this, SLOT(finished(KJob*)));

    QObject::connect(job, SIGNAL(suspended(KJob*)),
                     this, SLOT(suspended(KJob*)));
    QObject::connect(job, SIGNAL(resumed(KJob*)),
                     this, SLOT(resumed(KJob*)));

    QObject::connect(job, SIGNAL(description(KJob*, const QString&,
                                             const QPair<QString, QString>&,
                                             const QPair<QString, QString>&)),
                     this, SLOT(description(KJob*, const QString&,
                                            const QPair<QString, QString>&,
                                            const QPair<QString, QString>&)));
    QObject::connect(job, SIGNAL(infoMessage(KJob*, const QString&, const QString&)),
                     this, SLOT(infoMessage(KJob*, const QString&, const QString&)));
    QObject::connect(job, SIGNAL(warning(KJob*, const QString&, const QString&)),
                     this, SLOT(warning(KJob*, const QString&, const QString&)));

    QObject::connect(job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
                     this, SLOT(totalAmount(KJob*, KJob::Unit, qulonglong)));
    QObject::connect(job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
                     this, SLOT(processedAmount(KJob*, KJob::Unit, qulonglong)));
    QObject::connect(job, SIGNAL(percent(KJob*, unsigned long)),
                     this, SLOT(percent(KJob*, unsigned long)));
    QObject::connect(job, SIGNAL(speed(KJob*, unsigned long)),
                     this, SLOT(speed(KJob*, unsigned long)));
}

void KJobTrackerInterface::unregisterJob(KJob *job)
{
    QObject::disconnect(job, SIGNAL(finished(KJob*)),
                        this, SLOT(finished(KJob*)));

    QObject::disconnect(job, SIGNAL(suspended(KJob*)),
                        this, SLOT(suspended(KJob*)));
    QObject::disconnect(job, SIGNAL(resumed(KJob*)),
                        this, SLOT(resumed(KJob*)));

    QObject::disconnect(job, SIGNAL(description(KJob*, const QString&,
                                                const QPair<QString, QString>&,
                                                const QPair<QString, QString>&)),
                        this, SLOT(description(KJob*, const QString&,
                                               const QPair<QString, QString>&,
                                               const QPair<QString, QString>&)));
    QObject::disconnect(job, SIGNAL(infoMessage(KJob*, const QString&, const QString&)),
                        this, SLOT(infoMessage(KJob*, const QString&, const QString&)));
    QObject::disconnect(job, SIGNAL(warning(KJob*, const QString&, const QString&)),
                        this, SLOT(warning(KJob*, const QString&, const QString&)));

    QObject::disconnect(job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
                        this, SLOT(totalAmount(KJob*, KJob::Unit, qulonglong)));
    QObject::disconnect(job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
                        this, SLOT(processedAmount(KJob*, KJob::Unit, qulonglong)));
    QObject::disconnect(job, SIGNAL(totalSize(KJob*, qulonglong)),
                        this, SLOT(totalSize(KJob*, qulonglong)));
    QObject::disconnect(job, SIGNAL(processedSize(KJob*, qulonglong)),
                        this, SLOT(processedSize(KJob*, qulonglong)));
    QObject::disconnect(job, SIGNAL(percent(KJob*, unsigned long)),
                        this, SLOT(percent(KJob*, unsigned long)));
    QObject::disconnect(job, SIGNAL(speed(KJob*, unsigned long)),
                        this, SLOT(speed(KJob*, unsigned long)));
}

void KJobTrackerInterface::finished(KJob */*job*/)
{

}

void KJobTrackerInterface::suspended(KJob */*job*/)
{

}

void KJobTrackerInterface::resumed(KJob */*job*/)
{

}

void KJobTrackerInterface::description(KJob */*job*/, const QString &/*title*/,
                                       const QPair<QString, QString> &/*field1*/,
                                       const QPair<QString, QString> &/*field2*/)
{

}

void KJobTrackerInterface::infoMessage(KJob */*job*/, const QString &/*plain*/, const QString &/*rich*/)
{

}

void KJobTrackerInterface::warning(KJob */*job*/, const QString &/*plain*/, const QString &/*rich*/)
{

}

void KJobTrackerInterface::totalAmount(KJob */*job*/, KJob::Unit /*unit*/, qulonglong /*amount*/)
{

}

void KJobTrackerInterface::processedAmount(KJob */*job*/, KJob::Unit /*unit*/, qulonglong /*amount*/)
{

}

void KJobTrackerInterface::percent(KJob */*job*/, unsigned long /*percent*/)
{

}

void KJobTrackerInterface::speed(KJob */*job*/, unsigned long /*value*/)
{

}

#include "kjobtrackerinterface.moc"
