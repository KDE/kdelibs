#include "crossfade.h"
#include <phonon/mediaobject.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <phonon/fadereffect.h>
#include <kurl.h>
#include <QTimer>

static const long CROSSFADE_TIME = 3000; // 3 seconds

Crossfader::Crossfader( QObject* parent )
	: QObject( parent )
{
	m1 = new MediaObject( this ); m2 = new MediaObject( this );
	a1 = new AudioPath( this );   a2 = new AudioPath( this );
	f1 = new FaderEffect( this ); f2 = new FaderEffect( this );
	output = new AudioOutput( this );

	m1->addAudioPath( a1 );  m2->addAudioPath( a2 );
	a1->insertEffect( f1 );  a2->insertEffect( f2 );
	a1->addOutput( output ); a2->addOutput( output );
	m1->setAboutToFinishTime( CROSSFADE_TIME ); m2->setAboutToFinishTime( CROSSFADE_TIME );
	connect( m1, SIGNAL( aboutToFinish( long ) ), SLOT( crossfade( long ) ) );
	connect( m2, SIGNAL( aboutToFinish( long ) ), SLOT( crossfade( long ) ) );
	connect( m1, SIGNAL( finished() ), SLOT( setupNext() ) );
	connect( m2, SIGNAL( finished() ), SLOT( setupNext() ) );
}

void Crossfader::crossfade( long fadeTime )
{
	f1->fadeOut( fadeTime );
	f2->fadeIn( fadeTime );
	m2->play();
}

void Crossfader::setupNext()
{
	qSwap( m1, m2 ); qSwap( a1, a2 ); qSwap( f1, f2 );
	KUrl nextUrl;
	emit needNextUrl( nextUrl );
	m2->setUrl( nextUrl );
}

void Crossfader::start( const KUrl& firstUrl )
{
	m1->setUrl( firstUrl );
	m1->play();
}

void Crossfader::stop()
{
	f1->fadeOut( CROSSFADE_TIME );
	QTimer::singleShot( CROSSFADE_TIME, m1, SLOT( stop() ) );
}
