#include "kfileaudiopreview.h"

#include <qcheckbox.h>
#include <khbox.h>
#include <qlayout.h>
#include <qvgroupbox.h>

#include <kglobal.h>
#include <kconfig.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmediaplayer/player.h>
#include <kmimetype.h>
#include <kparts/componentfactory.h>

#include <kplayobjectfactory.h>

#include <config-kfile.h>

class KFileAudioPreviewFactory : public KLibFactory
{
protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                           const char *, const QStringList & )
    {
        return new KFileAudioPreview( dynamic_cast<QWidget*>( parent ), name );
    }
};

K_EXPORT_COMPONENT_FACTORY( kfileaudiopreview, KFileAudioPreviewFactory )


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class KFileAudioPreview::KFileAudioPreviewPrivate
{
public:
    KFileAudioPreviewPrivate( QWidget *parent )
    {
        player = KParts::ComponentFactory::createInstanceFromQuery<KMediaPlayer::Player>( "KMediaPlayer/Player", QString(), parent );
    }

    ~KFileAudioPreviewPrivate()
    {
        delete player;
    }

    KMediaPlayer::Player *player;
};


KFileAudioPreview::KFileAudioPreview( QWidget *parent, const char *name )
    : KPreviewWidgetBase( parent, name )
{
    KGlobal::locale()->insertCatalog("kfileaudiopreview");    

    QStringList formats = KDE::PlayObjectFactory::mimeTypes();
    // ###
    QStringList::ConstIterator it = formats.begin();
    for ( ; it != formats.end(); ++it )
        m_supportedFormats.insert( *it, (void*) 1 );

    QVGroupBox *box = new QVGroupBox( i18n("Media Player"), this );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( box );

    (void) new QWidget( box ); // spacer

    d = new KFileAudioPreviewPrivate( 0L ); // not box -- being reparented anyway
    if ( d->player ) // only if there actually is a component...
    {
        setSupportedMimeTypes( formats );
        KMediaPlayer::View *view = d->player->view();
        view->setEnabled( false );

        // if we have access to the video widget, show it above the player
        // So, reparent first the video widget, then the view.
        if ( view->videoWidget() )
        {
            KHBox *frame = new KHBox( box );
            frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
            frame->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
            view->videoWidget()->reparent( frame, QPoint(0,0) );
        }

        view->reparent( box, QPoint(0,0) );
    }

    m_autoPlay = new QCheckBox( i18n("Play &automatically"), box );
    KConfigGroup config( KGlobal::config(), ConfigGroup );
    m_autoPlay->setChecked( config.readEntry( "Autoplay sounds", QVariant(true )).toBool() );
    connect( m_autoPlay, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)) );
}

KFileAudioPreview::~KFileAudioPreview()
{
    KConfigGroup config( KGlobal::config(), ConfigGroup );
    config.writeEntry( "Autoplay sounds", m_autoPlay->isChecked() );

    delete d;
}

void KFileAudioPreview::showPreview( const KURL &url )
{
    if ( !d->player || !url.isValid() )
        return;

    KMimeType::Ptr mt = KMimeType::findByURL( url );
    bool supported = m_supportedFormats.find( mt->name() );
    d->player->view()->setEnabled( supported );
    if ( !supported )
        return;

    static_cast<KParts::ReadOnlyPart*>(d->player)->openURL( url );
    if ( m_autoPlay->isChecked() )
        d->player->play();
}

void KFileAudioPreview::clearPreview()
{
    if ( d->player )
    {
        d->player->stop();
        d->player->closeURL();
    }
}

void KFileAudioPreview::toggleAuto( bool on )
{
    if ( !d->player )
        return;

    if ( on && m_currentURL.isValid() && d->player->view()->isEnabled() )
        d->player->play();
    else
        d->player->stop();
}

void KFileAudioPreview::virtual_hook( int, void* )
{}

#include "kfileaudiopreview.moc"
