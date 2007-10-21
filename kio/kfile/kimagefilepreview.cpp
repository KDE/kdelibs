/*
 * This file is part of the KDE project
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include "kimagefilepreview.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtCore/QTimer>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kconfiggroup.h>

#include <config-kfile.h>

/**** KImageFilePreview ****/

class KImageFilePreview::KImageFilePreviewPrivate
{
public:
    KImageFilePreviewPrivate()
        : m_job(0)
    {
    }

    void _k_slotResult( KJob* );
    void _k_slotFailed( const KFileItem& );

    KUrl currentURL;
    QLabel *imageLabel;
    KIO::PreviewJob *m_job;
};

KImageFilePreview::KImageFilePreview( QWidget *parent )
    : KPreviewWidgetBase(parent), d(new KImageFilePreviewPrivate)
{
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 0 );
    vb->setSpacing( KDialog::spacingHint() );

    d->imageLabel = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->imageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    vb->addWidget(d->imageLabel);

    setSupportedMimeTypes( KIO::PreviewJob::supportedMimeTypes() );
    setMinimumWidth( 50 );
}

KImageFilePreview::~KImageFilePreview()
{
    if (d->m_job) {
        d->m_job->kill();
    }

    delete d;
}

void KImageFilePreview::showPreview()
{
    // Pass a copy since clearPreview() will clear currentURL
    KUrl url = d->currentURL;
    showPreview( url, true );
}

// called via KPreviewWidgetBase interface
void KImageFilePreview::showPreview( const KUrl& url )
{
    showPreview( url, false );
}

void KImageFilePreview::showPreview( const KUrl &url, bool force )
{
    if ( !url.isValid() ) {
        clearPreview();
        return;
    }

    if (url != d->currentURL || force) {
        clearPreview();
        d->currentURL = url;

        int w = d->imageLabel->contentsRect().width() - 4;
        int h = d->imageLabel->contentsRect().height() - 4;

        d->m_job = createJob(url, w, h);
        if ( force ) // explicitly requested previews shall always be generated!
            d->m_job->setIgnoreMaximumSize(true);

        connect(d->m_job, SIGNAL(result(KJob *)),
                 this, SLOT( _k_slotResult( KJob * )));
        connect(d->m_job, SIGNAL(gotPreview(const KFileItem&,
                                            const QPixmap& )),
                 SLOT( gotPreview( const KFileItem&, const QPixmap& ) ));

        connect(d->m_job, SIGNAL(failed(const KFileItem&)),
                 this, SLOT(_k_slotFailed(const KFileItem&)));
    }
}

void KImageFilePreview::resizeEvent( QResizeEvent * )
{
    QMetaObject::invokeMethod(this, "showPreview", Qt::QueuedConnection);
}

QSize KImageFilePreview::sizeHint() const
{
    return QSize( 100, 200 );
}

KIO::PreviewJob * KImageFilePreview::createJob( const KUrl& url, int w, int h )
{
    KUrl::List urls;
    urls.append( url );
    return KIO::filePreview( urls, w, h, 0, 0, true, false );
}

void KImageFilePreview::gotPreview( const KFileItem& item, const QPixmap& pm )
{
    if (item.url() == d->currentURL) // should always be the case
        d->imageLabel->setPixmap(pm);
}

void KImageFilePreview::KImageFilePreviewPrivate::_k_slotFailed( const KFileItem& item )
{
    if ( item.isDir() )
        imageLabel->clear();
    else if (item.url() == currentURL) // should always be the case
        imageLabel->setPixmap(SmallIcon( "image-missing", KIconLoader::SizeLarge,
                                         KIconLoader::DisabledState ));
}

void KImageFilePreview::KImageFilePreviewPrivate::_k_slotResult( KJob *job )
{
    if (job == m_job) {
        m_job = 0L;
    }
}

void KImageFilePreview::clearPreview()
{
    if (d->m_job) {
        d->m_job->kill();
        d->m_job = 0L;
    }

    d->imageLabel->clear();
    d->currentURL = KUrl();
}

#include "kimagefilepreview.moc"
