/*
 * This file is part of the KDE project
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtimer.h>

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

#include "kimagefilepreview.h"
#include "config-kfile.h"

/**** KImageFilePreview ****/

KImageFilePreview::KImageFilePreview( QWidget *parent )
    : KPreviewWidgetBase( parent ),
      m_job( 0L )
{
    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    autoMode = cg.readBoolEntry( "Automatic Preview", true );

    QGridLayout *vb = new QGridLayout( this, 2, 2, 0, KDialog::spacingHint() );
    vb->addItem( new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 0 );

    imageLabel = new QLabel( this );
    imageLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    imageLabel->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    vb->addWidget( imageLabel, 0, 1 );

    vb->addItem( new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 2 );

    QHBoxLayout *hb = new QHBoxLayout( 0 );
    vb->addLayout( hb, 1, 1 );

    autoPreview = new QCheckBox( i18n("&Automatic preview"), this );
    autoPreview->setChecked( autoMode );
    hb->addWidget( autoPreview );
    connect( autoPreview, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)) );

    previewButton = new KPushButton( SmallIconSet("thumbnail"), i18n("&Preview"), this );
    hb->addWidget( previewButton );
    connect( previewButton, SIGNAL(clicked()), SLOT(showPreview()) );

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(showPreview()) );

    setSupportedMimeTypes( KIO::PreviewJob::supportedMimeTypes() );
}

KImageFilePreview::~KImageFilePreview()
{
    if ( m_job )
        m_job->kill();

    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    cg.writeEntry( "Automatic Preview", autoPreview->isChecked() );
}

void KImageFilePreview::showPreview()
{
    // Pass a copy since clearPreview() will clear currentURL
    KURL url = currentURL;
    showPreview( url, true );
}

// called via KPreviewWidgetBase interface
void KImageFilePreview::showPreview( const KURL& url )
{
    showPreview( url, false );
}

void KImageFilePreview::showPreview( const KURL &url, bool force )
{
    if ( !url.isValid() ) {
        clearPreview();
        return;
    }

    if ( url != currentURL || force )
    {
        clearPreview();
	currentURL = url;

	if ( autoMode || force )
	{
            int w = imageLabel->contentsRect().width() - 4;
            int h = imageLabel->contentsRect().height() - 4;

            m_job =  createJob( url, w, h );
            connect( m_job, SIGNAL( result( KIO::Job * )),
                     this, SLOT( slotResult( KIO::Job * )));
            connect( m_job, SIGNAL( gotPreview( const KFileItem*,
                                                const QPixmap& )),
                     SLOT( gotPreview( const KFileItem*, const QPixmap& ) ));

            connect( m_job, SIGNAL( failed( const KFileItem* )),
                     this, SLOT( slotFailed( const KFileItem* ) ));
	}
    }
}

void KImageFilePreview::toggleAuto( bool a )
{
    autoMode = a;
    if ( autoMode )
    {
        // Pass a copy since clearPreview() will clear currentURL
        KURL url = currentURL;
        showPreview( url, true );
    }
}

void KImageFilePreview::resizeEvent( QResizeEvent * )
{
    timer->start( 100, true ); // forces a new preview
}

QSize KImageFilePreview::sizeHint() const
{
    return QSize( 20, 200 ); // otherwise it ends up huge???
}

KIO::PreviewJob * KImageFilePreview::createJob( const KURL& url, int w, int h )
{
    KURL::List urls;
    urls.append( url );
    return KIO::filePreview( urls, w, h, 0, 0, true, false );
}

void KImageFilePreview::gotPreview( const KFileItem* item, const QPixmap& pm )
{
    if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( pm );
}

void KImageFilePreview::slotFailed( const KFileItem* item )
{
    if ( item->isDir() )
        imageLabel->clear();
    else if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( SmallIcon( "file_broken", KIcon::SizeLarge,
                                          KIcon::DisabledState ));
}

void KImageFilePreview::slotResult( KIO::Job *job )
{
    if ( job == m_job )
        m_job = 0L;
}

void KImageFilePreview::clearPreview()
{
    if ( m_job ) {
        m_job->kill();
        m_job = 0L;
    }

    imageLabel->clear();
    currentURL = KURL();
}

void KImageFilePreview::virtual_hook( int id, void* data )
{ KPreviewWidgetBase::virtual_hook( id, data ); }

#include "kimagefilepreview.moc"
