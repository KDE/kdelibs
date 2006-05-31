MediaPlayer::MediaPlayer( QWidget* parent )
	: QWidget( parent )
{
	QVBoxLayout* layout = new QVBoxLayout( this );

	m_vwidget = new VideoWidget( this );
	layout->addWidget( m_vwidget );
	m_vwidget->hide();

	m_vpath = new VideoPath( this );

	m_aoutput = new AudioOutput( this );
	m_apath = new AudioPath( this );

	m_media = new MediaObject( this );
	m_media->addVideoPath( m_vpath );
	m_vpath->addOutput( m_vwidget );
	m_media->addAudioPath( m_apath );
	m_apath->addOutput( m_aoutput );

	m_controls = new MediaControls( this );
	layout->addWidget( m_controls );
	m_controls->setMediaProducer( m_media );
	m_controls->setAudioOutput( m_aoutput );
}

void MediaPlayer::setUrl( const KUrl& url )
{
	m_media->setUrl( url );
	m_vwidget->setVisible( m_media->hasVideo() );
}
