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
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qtimer.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
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
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs( config, ConfigGroup );
    autoMode = config->readBoolEntry( "Automatic Preview", true );

    QVBoxLayout *vb = new QVBoxLayout( this, KDialog::marginHint() );

    imageLabel = new QLabel( this );
    imageLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    imageLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    imageLabel->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    vb->addWidget( imageLabel, 1 );

    QHBoxLayout *hb = new QHBoxLayout( vb );

    autoPreview = new QCheckBox( i18n("&Automatic preview"), this );
    autoPreview->setChecked( autoMode );
    hb->addWidget( autoPreview );
    connect( autoPreview, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)) );

    previewButton = new QPushButton( i18n("&Preview"), this );
    hb->addWidget( previewButton );
    connect( previewButton, SIGNAL(clicked()), SLOT(showPreview()) );

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(showPreview()) );
}

KImageFilePreview::~KImageFilePreview()
{
    if ( m_job )
        m_job->kill();

    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs( config, ConfigGroup );
    config->writeEntry( "Automatic Preview", autoPreview->isChecked() );
}

// called via KPreviewWidgetBase interface
void KImageFilePreview::showPreview( const KURL& url )
{
    showPreview( url, false );
}

void KImageFilePreview::showPreview( const KURL &url, bool force )
{
    if ( url != currentURL || force )
    {
        imageLabel->setPixmap( QPixmap() );
        if ( m_job ) {
            m_job->kill();
            m_job = 0L;
        }

	currentURL = url;

	if ( !url.isMalformed() && (autoMode || force) )
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
	showPreview( currentURL, true );
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
    if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( SmallIcon( "file_broken", KIcon::SizeLarge,
                                          KIcon::DisabledState ));
}

void KImageFilePreview::slotResult( KIO::Job *job )
{
    if ( job == m_job )
        m_job = 0L;
}

#include "kimagefilepreview.moc"
