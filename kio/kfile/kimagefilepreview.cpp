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

    bool autoMode;
    KUrl currentURL;
    QTimer *timer;
    QLabel *imageLabel;
    QCheckBox *autoPreview;
    QPushButton *previewButton;
    KIO::PreviewJob *m_job;
};

KImageFilePreview::KImageFilePreview( QWidget *parent )
    : KPreviewWidgetBase(parent), d(new KImageFilePreviewPrivate)
{
    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    d->autoMode = cg.readEntry("Automatic Preview", true);

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 0 );
    vb->setSpacing( KDialog::spacingHint() );

    d->imageLabel = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->imageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    vb->addWidget(d->imageLabel);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->setSpacing( 0 );
    vb->addLayout( hb );

    d->autoPreview = new QCheckBox(i18n("&Automatic preview"), this);
    d->autoPreview->setChecked(d->autoMode);
    hb->addWidget(d->autoPreview);
    connect(d->autoPreview, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)));

    d->previewButton = new KPushButton(KIcon("thumbnail-show"), i18n("&Preview"), this);
    hb->addWidget(d->previewButton);
    connect(d->previewButton, SIGNAL(clicked()), SLOT(showPreview()));

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), SLOT(showPreview()));

    setSupportedMimeTypes( KIO::PreviewJob::supportedMimeTypes() );
}

KImageFilePreview::~KImageFilePreview()
{
    if (d->m_job) {
        d->m_job->kill();
    }

    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    cg.writeEntry("Automatic Preview", d->autoPreview->isChecked());

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

        if (d->autoMode || force) {
            int w = d->imageLabel->contentsRect().width() - 4;
            int h = d->imageLabel->contentsRect().height() - 4;

            d->m_job = createJob(url, w, h);
            if ( force ) // explicitly requested previews shall always be generated!
                d->m_job->setIgnoreMaximumSize(true);

            connect(d->m_job, SIGNAL(result(KJob *)),
                     this, SLOT( slotResult( KJob * )));
            connect(d->m_job, SIGNAL(gotPreview(const KFileItem&,
                                                const QPixmap& )),
                     SLOT( gotPreview( const KFileItem&, const QPixmap& ) ));

            connect(d->m_job, SIGNAL(failed(const KFileItem&)),
                     this, SLOT(slotFailed(const KFileItem&)));
	}
    }
}

void KImageFilePreview::toggleAuto( bool a )
{
    d->autoMode = a;
    if (d->autoMode) {
        // Pass a copy since clearPreview() will clear currentURL
        KUrl url = d->currentURL;
        showPreview( url, true );
    }
}

void KImageFilePreview::resizeEvent( QResizeEvent * )
{
    d->timer->start(100); // forces a new preview
}

QSize KImageFilePreview::sizeHint() const
{
    return QSize( 20, 200 ); // otherwise it ends up huge???
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

void KImageFilePreview::slotFailed( const KFileItem& item )
{
    if ( item.isDir() )
        d->imageLabel->clear();
    else if (item.url() == d->currentURL) // should always be the case
        d->imageLabel->setPixmap(SmallIcon( "file-broken", K3Icon::SizeLarge,
                                          K3Icon::DisabledState ));
}

void KImageFilePreview::slotResult( KJob *job )
{
    if (job == d->m_job) {
        d->m_job = 0L;
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
