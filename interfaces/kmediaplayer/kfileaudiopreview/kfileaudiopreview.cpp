/* This file is part of the KDE libraries
   Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfileaudiopreview.h"

#include <QCheckBox>
#include <QLayout>
#include <QGroupBox>

#include <khbox.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kparts/componentfactory.h>

#include <config-kfile.h>

#include <phonon/mediaobject.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <phonon/videopath.h>
#include <phonon/backendcapabilities.h>
#include <phonon/ui/videowidget.h>
#include <phonon/ui/mediacontrols.h>

class KFileAudioPreviewFactory : public KLibFactory
{
protected:
    virtual QObject *createObject( QObject *parent, const char *,
                           const char *, const QStringList & )
    {
        return new KFileAudioPreview( dynamic_cast<QWidget*>( parent ) );
    }
};

K_EXPORT_COMPONENT_FACTORY( kfileaudiopreview, KFileAudioPreviewFactory )


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

using namespace Phonon;
using namespace Phonon::Ui;

class KFileAudioPreview::Private
{
public:
    Private()
        : player( 0 )
        , audioPath( 0 )
        , audioOutput( 0 )
        , videoPath( 0 )
        , videoWidget( 0 )
    {
    }

    MediaObject* player;
    AudioPath* audioPath;
    AudioOutput* audioOutput;
    VideoPath* videoPath;
    VideoWidget* videoWidget;
    MediaControls* controls;
};


KFileAudioPreview::KFileAudioPreview( QWidget *parent )
    : KPreviewWidgetBase( parent )
    , d( new Private )
{
    KGlobal::locale()->insertCatalog("kfileaudiopreview");    

    QGroupBox *box = new QGroupBox( i18n("Media Player"), this );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( box );

    (void) new QWidget( box ); // spacer

    QStringList formats;
    KMimeType::List mimeTypes = BackendCapabilities::self()->knownMimeTypes();
    foreach( KMimeType::Ptr p, mimeTypes )
        formats.append( p->name() );
    setSupportedMimeTypes( formats );

    d->audioOutput = new AudioOutput( this );
    d->audioPath = new AudioPath( this );
    d->audioPath->addOutput( d->audioOutput );

    KHBox *frame = new KHBox( box );
    frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    frame->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    d->videoWidget = new VideoWidget( frame );
    d->videoPath = new VideoPath( this );
    d->videoPath->addOutput( d->videoWidget->videoOutput() );

    d->controls = new MediaControls( box );
    d->controls->setEnabled( false );
    d->controls->setAudioOutput( d->audioOutput );

    m_autoPlay = new QCheckBox( i18n("Play &automatically"), box );
    KConfigGroup config( KGlobal::config(), ConfigGroup );
    m_autoPlay->setChecked( config.readEntry( "Autoplay sounds", true ) );
    connect( m_autoPlay, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)) );
}

KFileAudioPreview::~KFileAudioPreview()
{
    KConfigGroup config( KGlobal::config(), ConfigGroup );
    config.writeEntry( "Autoplay sounds", m_autoPlay->isChecked() );

    delete d;
}

void KFileAudioPreview::stateChanged( Phonon::State newstate, Phonon::State oldstate )
{
    if( oldstate == Phonon::LoadingState && newstate != Phonon::ErrorState )
        d->controls->setEnabled( true );
    disconnect( d->player, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
            this, SLOT( stateChanged( Phonon::State, Phonon::State ) ) );
}

void KFileAudioPreview::showPreview( const KURL &url )
{
    delete d->player;
    d->player = new MediaObject( this );
    d->player->setUrl( url );
    if( d->player->state() == Phonon::ErrorState )
    {
        delete d->player;
        d->player = 0;
        return;
    }

    d->controls->setMediaProducer( d->player );
    if( d->player->state() == Phonon::StoppedState )
        d->controls->setEnabled( true );
    else
        connect( d->player, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
                SLOT( stateChanged( Phonon::State, Phonon::State ) ) );

    if( m_autoPlay->isChecked() )
        d->player->play();
}

void KFileAudioPreview::clearPreview()
{
    if( d->player )
    {
        d->player->stop();
        delete d->player;
        d->player = 0;
        d->controls->setEnabled( false );
    }
}

void KFileAudioPreview::toggleAuto( bool on )
{
    if( !d->player )
        return;

    if( on && d->controls->isEnabled() )
        d->player->play();
    else
        d->player->stop();
}

#include "kfileaudiopreview.moc"
