/*
 * This file is part of the KDE project.
 * Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include "kfilemetapreview.h"

#include <qlayout.h>

#include <kio/previewjob.h>
#include <klibloader.h>
#include <kimagefilepreview.h>
#include <kmimetype.h>

bool KFileMetaPreview::s_tryAudioPreview = true;

KFileMetaPreview::KFileMetaPreview( QWidget *parent )
    : KPreviewWidgetBase( parent ),
      haveAudioPreview( false )
{
    QHBoxLayout *layout = new QHBoxLayout( this, 0, 0 );
    m_stack = new QStackedWidget( this );
    layout->addWidget( m_stack );

    // ###
//     m_previewProviders.setAutoDelete( true );
    initPreviewProviders();
}

KFileMetaPreview::~KFileMetaPreview()
{
}

void KFileMetaPreview::initPreviewProviders()
{
    qDeleteAll(m_previewProviders);
	m_previewProviders.clear();
    // hardcoded so far

    // image previews
    KImageFilePreview *imagePreview = new KImageFilePreview( m_stack );
    (void) m_stack->addWidget( imagePreview );
    m_stack->setCurrentWidget ( imagePreview );
    resize( imagePreview->sizeHint() );

    QStringList mimeTypes = imagePreview->supportedMimeTypes();
    QStringList::ConstIterator it = mimeTypes.begin();
    for ( ; it != mimeTypes.end(); ++it )
    {
//         qDebug(".... %s", (*it).latin1());
        m_previewProviders.insert( *it, imagePreview );
    }
}

KPreviewWidgetBase * KFileMetaPreview::previewProviderFor( const QString& mimeType )
{
//     qDebug("### looking for: %s", mimeType.latin1());
    // often the first highlighted item, where we can be sure, there is no plugin
    // (this "folders reflect icons" is a konq-specific thing, right?)
    if ( mimeType == "inode/directory" )
        return 0L;

    KPreviewWidgetBase *provider = m_previewProviders.find( mimeType ).value();
    if ( provider )
        return provider;

//qDebug("#### didn't find anything for: %s", mimeType.latin1());

    if ( s_tryAudioPreview &&
         !mimeType.startsWith("text/") && !mimeType.startsWith("image/") )
    {
        if ( !haveAudioPreview )
        {
            KPreviewWidgetBase *audioPreview = createAudioPreview( m_stack );
            if ( audioPreview )
            {
                haveAudioPreview = true;
                (void) m_stack->addWidget( audioPreview );
                QStringList mimeTypes = audioPreview->supportedMimeTypes();
                QStringList::ConstIterator it = mimeTypes.begin();
                for ( ; it != mimeTypes.end(); ++it )
                    m_previewProviders.insert( *it, audioPreview );
            }
        }
    }

    // with the new mimetypes from the audio-preview, try again
    provider = m_previewProviders.find( mimeType ).value();
    if ( provider )
        return provider;

    // ### mimetype may be image/* for example, try that
    int index = mimeType.indexOf( '/' );
    if ( index > 0 )
    {
        provider = m_previewProviders.find( mimeType.left( index + 1 ) + "*" ).value();
        if ( provider )
            return provider;
    }

    KMimeType::Ptr mimeInfo = KMimeType::mimeType( mimeType );
    if ( mimeInfo )
    {
        // check mime type inheritance
        QString parentMimeType = mimeInfo->parentMimeType();
        while ( !parentMimeType.isEmpty() )
        {
            provider = m_previewProviders.find( parentMimeType ).value();
            if ( provider )
                return provider;

            KMimeType::Ptr parentMimeInfo = KMimeType::mimeType( parentMimeType );
            if ( !parentMimeInfo ) break;

            parentMimeType = parentMimeInfo->parentMimeType();
        }

        // check X-KDE-Text property
        QVariant textProperty = mimeInfo->property( "X-KDE-text" );
        if ( textProperty.isValid() && textProperty.type() == QVariant::Bool )
        {
            if ( textProperty.toBool() )
            {
                provider = m_previewProviders.find( "text/plain" ).value();
                if ( provider )
                    return provider;

                provider = m_previewProviders.find( "text/*" ).value();
                if ( provider )
                    return provider;
            }
        }
    }

    return 0L;
}

void KFileMetaPreview::showPreview(const KURL &url)
{
    KMimeType::Ptr mt = KMimeType::findByURL( url );
    KPreviewWidgetBase *provider = previewProviderFor( mt->name() );
    if ( provider )
    {
        if ( provider != m_stack->currentWidget() ) // stop the previous preview
            clearPreview();

        m_stack->setEnabled( true );
        m_stack->setCurrentWidget( provider );
        provider->showPreview( url );
    }
    else
    {
        clearPreview();
        m_stack->setEnabled( false );
    }
}

void KFileMetaPreview::clearPreview()
{
    if ( m_stack->currentWidget() )
        static_cast<KPreviewWidgetBase*>( m_stack->currentWidget() )->clearPreview();
}

void KFileMetaPreview::addPreviewProvider( const QString& mimeType,
                                           KPreviewWidgetBase *provider )
{
    m_previewProviders.insert( mimeType, provider );
}


void KFileMetaPreview::clearPreviewProviders()
{
	QHash<QString, KPreviewWidgetBase*>::const_iterator i = m_previewProviders.constBegin();
	while (i != m_previewProviders.constEnd())
	{
		m_stack->removeWidget(i.value());
		++i;
	}
	qDeleteAll(m_previewProviders);
    m_previewProviders.clear();
}

// static
KPreviewWidgetBase * KFileMetaPreview::createAudioPreview( QWidget *parent )
{
    KLibFactory *factory = KLibLoader::self()->factory( "kfileaudiopreview" );
    if ( !factory )
    {
        s_tryAudioPreview = false;
        return 0L;
    }

    return dynamic_cast<KPreviewWidgetBase*>( factory->create( parent, "kfileaudiopreview" ));
}

void KFileMetaPreview::virtual_hook( int, void* ) {}

#include "kfilemetapreview.moc"
