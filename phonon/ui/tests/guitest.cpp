/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#include "guitest.h"

#include <phonon/mediaobject.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <phonon/videopath.h>
#include <phonon/backendcapabilities.h>
#include <phonon/ui/videowidget.h>

#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klineedit.h>
#include <kurlcompletion.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

#include <cstdlib>
#include <phonon/ui/seekslider.h>
#include <phonon/ui/volumeslider.h>
#include <QTime>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QComboBox>
#include <phonon/objectdescriptionmodel.h>

using namespace Phonon;

OutputWidget::OutputWidget( QWidget *parent )
	: QFrame( parent )
	, m_output( new AudioOutput( Phonon::MusicCategory, this ) )
{
	setFrameShape( QFrame::Box );
	setFrameShadow( QFrame::Sunken );

	m_output->setName( "GUI-Test" );

	QVBoxLayout *vlayout = new QVBoxLayout( this );

    QComboBox *deviceComboBox = new QComboBox(this);
    vlayout->addWidget(deviceComboBox);
    QList<AudioOutputDevice> deviceList = BackendCapabilities::availableAudioOutputDevices();
    deviceComboBox->setModel(new AudioOutputDeviceModel(deviceList, deviceComboBox));
    deviceComboBox->setCurrentIndex(deviceList.indexOf(m_output->outputDevice()));
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)), SLOT(deviceChange(int)));

	m_volslider = new VolumeSlider( this );
    m_volslider->setOrientation(Qt::Vertical);
	m_volslider->setAudioOutput( m_output );
	vlayout->addWidget( m_volslider );
}

void OutputWidget::deviceChange(int modelIndex)
{
    QList<AudioOutputDevice> deviceList = BackendCapabilities::availableAudioOutputDevices();
    if (modelIndex >= 0 && modelIndex < deviceList.size()) {
        m_output->setOutputDevice(deviceList[modelIndex]);
    }
}

PathWidget::PathWidget( QWidget *parent )
	: QFrame( parent )
	, m_path( new AudioPath( this ) )
{
	setFrameShape( QFrame::Box );
	setFrameShadow( QFrame::Raised );
}

bool PathWidget::connectOutput( OutputWidget *w )
{
	return m_path->addOutput( w->output() );
}

bool ProducerWidget::connectPath( PathWidget *w )
{
	if( m_media )
	{
		m_audioPaths.append( w->path() );
		return m_media->addAudioPath( w->path() );
	}
	if( m_audioPaths.contains( w->path() ) )
		return false;
	m_audioPaths.append( w->path() );
	return true;
}

ProducerWidget::ProducerWidget( QWidget *parent )
	: QFrame( parent )
	, m_media( 0 )
	, m_length( -1 )
{
	setFrameShape( QFrame::Box );
	setFrameShadow( QFrame::Sunken );

	QVBoxLayout *topLayout = new QVBoxLayout( this );

	KLineEdit *file = new KLineEdit( this );
	file->setCompletionObject( new KUrlCompletion( KUrlCompletion::FileCompletion ) );
	file->setText( getenv( "PHONON_TESTURL" ) );
	connect( file, SIGNAL( returnPressed( const QString & ) ), SLOT( loadFile( const QString & ) ) );
	topLayout->addWidget( file );

	m_seekslider = new SeekSlider( this );
	topLayout->addWidget( m_seekslider );

	QFrame *frame0 = new QFrame( this );
	topLayout->addWidget( frame0 );
	QHBoxLayout *hlayout = new QHBoxLayout( frame0 );
	hlayout->setMargin( 0 );

	QFrame *frame1 = new QFrame( frame0 );
	hlayout->addWidget( frame1 );
	QVBoxLayout *vlayout = new QVBoxLayout( frame1 );
	vlayout->setMargin( 0 );

	m_play = new QPushButton( frame1 );
	m_play->setText( "play" );
	vlayout->addWidget( m_play );

	m_pause = new QPushButton( frame1 );
	m_pause->setText( "pause" );
	vlayout->addWidget( m_pause );

	m_stop = new QPushButton( frame1 );
	m_stop->setText( "stop" );
	vlayout->addWidget( m_stop );

	QFrame *frame2 = new QFrame( frame0 );
	hlayout->addWidget( frame2 );
	QVBoxLayout *vlayout2 = new QVBoxLayout( frame2 );
	vlayout2->setMargin( 0 );

	m_statelabel = new QLabel( frame2 );
	vlayout2->addWidget( m_statelabel );

	m_totaltime = new QLabel( frame2 );
	vlayout2->addWidget( m_totaltime );
	
	m_currenttime = new QLabel( frame2 );
	vlayout2->addWidget( m_currenttime );
	
	m_remainingtime = new QLabel( frame2 );
	vlayout2->addWidget( m_remainingtime );

	m_metaDataLabel = new QLabel( this );
	topLayout->addWidget( m_metaDataLabel );

	loadFile( getenv( "PHONON_TESTURL" ) );
}

void ProducerWidget::tick( qint64 t )
{
	QTime x( 0, 0 );
	x = x.addMSecs( t );
	m_currenttime->setText( x.toString( "m:ss.zzz" ) );
	x.setHMS( 0, 0, 0, 0 );
	qint64 r = m_media->remainingTime();
	if( r >= 0 )
		x = x.addMSecs( r );
	m_remainingtime->setText( x.toString( "m:ss.zzz" ) );
}

void ProducerWidget::stateChanged( Phonon::State newstate )
{
	switch( newstate )
	{
		case Phonon::ErrorState:
			m_statelabel->setText( "Error" );
			break;
		case Phonon::LoadingState:
			m_statelabel->setText( "Loading" );
			break;
		case Phonon::StoppedState:
			m_statelabel->setText( "Stopped" );
			break;
		case Phonon::PausedState:
			m_statelabel->setText( "Paused" );
			break;
		case Phonon::BufferingState:
			m_statelabel->setText( "Buffering" );
			break;
		case Phonon::PlayingState:
			m_statelabel->setText( "Playing" );
			break;
	}
}

void ProducerWidget::length( qint64 ms )
{
	m_length = ms;
	QTime x( 0, 0 );
	x = x.addMSecs( m_length );
	m_totaltime->setText( x.toString( "m:ss.zzz" ) );
	tick( m_media->currentTime() );
}

void ProducerWidget::loadFile( const QString & file )
{
	delete m_media;
	m_media = new MediaObject( this );
	connect( m_media, SIGNAL( metaDataChanged() ), SLOT( updateMetaData() ) );
	m_seekslider->setMediaProducer( m_media );
	m_media->setUrl( KUrl( file ) );
	m_media->setTickInterval( 100 );
	m_media->setAboutToFinishTime( 2000 );
	foreach( AudioPath *path, m_audioPaths )
		m_media->addAudioPath( path );
	/*if( m_media->hasVideo() && BackendCapabilities::self()->supportsVideo() )
	{
		vpath = new VideoPath( this );
		m_media->addVideoPath( vpath );
		vout = new VideoWidget( this );
		vpath->addOutput( vout );

		vout->setMinimumSize( 160, 120 );
		vout->setFullScreen( false );
		vout->show();
	}*/
	stateChanged( m_media->state() );

	connect( m_pause, SIGNAL( clicked() ), m_media, SLOT( pause() ) );
	connect( m_play, SIGNAL( clicked() ), m_media, SLOT( play() ) );
	connect( m_stop, SIGNAL( clicked() ), m_media, SLOT( stop() ) );

	connect( m_media, SIGNAL(   tick( qint64 ) ), SLOT(   tick( qint64 ) ) );
	connect( m_media, SIGNAL( length( qint64 ) ), SLOT( length( qint64 ) ) );
	length( m_media->totalTime() );
	connect( m_media, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
			SLOT( stateChanged( Phonon::State ) ) );
	connect( m_media, SIGNAL( finished() ), SLOT( slotFinished() ) );
	connect( m_media, SIGNAL( aboutToFinish( qint32 ) ), SLOT( slotAboutToFinish( qint32 ) ) );
}

void ProducerWidget::updateMetaData()
{
	QString metaData;
	QStringList keys = m_media->metaDataKeys();
	foreach( QString key, keys )
	{
		metaData += key + QLatin1String( ": " ) + m_media->metaDataItem( key ) + "\n";
	}
	m_metaDataLabel->setText( metaData.left( metaData.length() - 1 ) );
}

void ProducerWidget::slotFinished()
{
	kDebug() << "finished signal from MediaObject with URL " << m_media->url() << endl;
}

void ProducerWidget::slotAboutToFinish( qint32 remaining )
{
	kDebug() << "aboutToFinish(" << remaining << ") signal from MediaObject with URL " << m_media->url() << endl;
}

ConnectionWidget::ConnectionWidget( QWidget *parent )
	: QFrame( parent )
{
	setFrameShape( QFrame::StyledPanel );
	setFrameShadow( QFrame::Sunken );
	setMinimumWidth( 120 );
}

void ConnectionWidget::mousePressEvent( QMouseEvent *me )
{
	m_pressPos = me->pos();
}

void ConnectionWidget::mouseReleaseEvent( QMouseEvent *me )
{
	emit madeConnection( m_pressPos, me->pos() );
	m_pressPos = QPoint();
	m_currentLine = QLine();
	update();
}

void ConnectionWidget::mouseMoveEvent( QMouseEvent *me )
{
	if( m_pressPos.isNull() )
		return;

	m_currentLine = QLine( m_pressPos, me->pos() );
	update();
}

void ConnectionWidget::addConnection( QWidget *a, QWidget *b )
{
	m_lines.append( WidgetConnection( a, b ) );
	update();
}

void ConnectionWidget::paintEvent( QPaintEvent *pe )
{
	QFrame::paintEvent( pe );
	QPainter p( this );
	foreach( WidgetConnection x, m_lines )
	{
		QLine l( 0, x.a->geometry().center().y(),
				width(), x.b->geometry().center().y() );
		p.drawLine( l );
	}
	p.setPen( Qt::darkRed );
	p.drawLine( m_currentLine );
}

MainWidget::MainWidget( QWidget *parent )
	: QWidget( parent )
{
	QHBoxLayout *topLayout = new QHBoxLayout( this );

	m_producerFrame = new QFrame( this );
	topLayout->addWidget( m_producerFrame );
	new QVBoxLayout( m_producerFrame );
	m_producerFrame->layout()->setMargin( 0 );

	m_connectionFrame1 = new ConnectionWidget( this );
	topLayout->addWidget( m_connectionFrame1 );
	connect( m_connectionFrame1, SIGNAL( madeConnection( const QPoint&, const QPoint& ) ),
			SLOT( madeConnection1( const QPoint&, const QPoint& ) ) );

	m_pathFrame = new QFrame( this );
	topLayout->addWidget( m_pathFrame );
	new QVBoxLayout( m_pathFrame );
	m_pathFrame->layout()->setMargin( 0 );

	m_connectionFrame2 = new ConnectionWidget( this );
	topLayout->addWidget( m_connectionFrame2 );
	connect( m_connectionFrame2, SIGNAL( madeConnection( const QPoint&, const QPoint& ) ),
			SLOT( madeConnection2( const QPoint&, const QPoint& ) ) );

	m_outputFrame = new QFrame( this );
	topLayout->addWidget( m_outputFrame );
	new QVBoxLayout( m_outputFrame );
	m_outputFrame->layout()->setMargin( 0 );

	QPushButton *addProducerButton = new QPushButton( "add Producer", m_producerFrame );
	m_producerFrame->layout()->addWidget( addProducerButton );
	connect( addProducerButton, SIGNAL( clicked() ), SLOT( addProducer() ) );

	QPushButton *addPathButton = new QPushButton( "add Path", m_pathFrame );
	m_pathFrame->layout()->addWidget( addPathButton );
	connect( addPathButton, SIGNAL( clicked() ), SLOT( addPath() ) );

	QPushButton *addOutputButton = new QPushButton( "add Output", m_outputFrame );
	m_outputFrame->layout()->addWidget( addOutputButton );
	connect( addOutputButton, SIGNAL( clicked() ), SLOT( addOutput() ) );

	addProducer();
	addPath();
	addOutput();
	if( m_producers.first()->connectPath( m_paths.first() ) )
		m_connectionFrame1->addConnection( m_producers.first(), m_paths.first() );
	if( m_paths.first()->connectOutput( m_outputs.first() ) )
		m_connectionFrame2->addConnection( m_paths.first(), m_outputs.first() );
}

void MainWidget::addProducer()
{
	ProducerWidget* x = new ProducerWidget( m_producerFrame );
	m_producerFrame->layout()->addWidget( x );
	m_producers << x;
	m_connectionFrame1->update();
}

void MainWidget::addPath()
{
	PathWidget* x = new PathWidget( m_pathFrame );
	m_pathFrame->layout()->addWidget( x );
	m_paths << x;
	m_connectionFrame1->update();
	m_connectionFrame2->update();
}

void MainWidget::addOutput()
{
	OutputWidget* x = new OutputWidget( m_outputFrame );
	m_outputFrame->layout()->addWidget( x );
	m_outputs << x;
	m_connectionFrame2->update();
}

void MainWidget::madeConnection1( const QPoint &a, const QPoint &b )
{
	QPoint left;
	QPoint right;
	if( a.x() < m_connectionFrame1->width() / 3 )
		left = a;
	else if( a.x() > m_connectionFrame1->width() * 2 / 3 )
		right = a;
	if( b.x() < m_connectionFrame1->width() / 3 )
		left = b;
	else if( b.x() > m_connectionFrame1->width() * 2 / 3 )
		right = b;
	if( left.isNull() || right.isNull() )
		return;

	kDebug() << k_funcinfo << left << right << endl;

	ProducerWidget *producer = 0;
	foreach( ProducerWidget *x, m_producers )
	{
		QRect rect = x->geometry();
		kDebug() << rect << endl;
		left.setX( rect.center().x() );
		if( rect.contains( left ) )
			producer = x;
	}
	if( !producer )
		return;

	PathWidget *path = 0;
	foreach( PathWidget *x, m_paths )
	{
		QRect rect = x->geometry();
		kDebug() << rect << endl;
		right.setX( rect.center().x() );
		if( rect.contains( right ) )
			path = x;
	}
	if( !path )
		return;

	if( producer->connectPath( path ) )
		m_connectionFrame1->addConnection( producer, path );
}

void MainWidget::madeConnection2( const QPoint &a, const QPoint &b )
{
	QPoint left;
	QPoint right;
	if( a.x() < m_connectionFrame2->width() / 3 )
		left = a;
	else if( a.x() > m_connectionFrame2->width() * 2 / 3 )
		right = a;
	if( b.x() < m_connectionFrame2->width() / 3 )
		left = b;
	else if( b.x() > m_connectionFrame2->width() * 2 / 3 )
		right = b;
	if( left.isNull() || right.isNull() )
		return;

	kDebug() << k_funcinfo << left << right << endl;

	PathWidget *path = 0;
	foreach( PathWidget *x, m_paths )
	{
		QRect rect = x->geometry();
		kDebug() << rect << endl;
		left.setX( rect.center().x() );
		if( rect.contains( left ) )
			path = x;
	}
	if( !path )
		return;

	OutputWidget *output = 0;
	foreach( OutputWidget *x, m_outputs )
	{
		QRect rect = x->geometry();
		kDebug() << rect << endl;
		right.setX( rect.center().x() );
		if( rect.contains( right ) )
			output = x;
	}
	if( !output )
		return;

	if( path->connectOutput( output ) )
		m_connectionFrame2->addConnection( path, output );
}

int main( int argc, char **argv )
{
	KAboutData about( "phonontest", "KDE Multimedia Test",
			"0.2", "Testprogram",
			KAboutData::License_LGPL, 0 );
	about.addAuthor( "Matthias Kretz", 0, "kretz@kde.org" );
	KCmdLineArgs::init( argc, argv, &about );
	KApplication app;
	MainWidget w;
	w.show();
	return app.exec();
}

#include "guitest.moc"

// vim: sw=4 ts=4
