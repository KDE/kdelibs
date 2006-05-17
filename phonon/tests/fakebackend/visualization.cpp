#include "visualization.h"
#include "audiopath.h"
#include "abstractvideooutput.h"
#include <phonon/ifaces/abstractvideooutput.h>

namespace Phonon
{
namespace Fake
{

Visualization::Visualization( QObject* parent )
	: QObject( parent )
{
}

int Visualization::visualization() const
{
	return m_visualization;
}

void Visualization::setVisualization( int newVisualization )
{
	m_visualization = newVisualization;
}

void Visualization::setAudioPath( Ifaces::AudioPath* audioPath )
{
	Q_ASSERT( audioPath );
	AudioPath* ap = qobject_cast<AudioPath*>( audioPath->qobject() );
	Q_ASSERT( ap );
	m_audioPath = ap;
}

void Visualization::setVideoOutput( Ifaces::AbstractVideoOutput* videoOutputIface )
{
	Q_ASSERT( videoOutputIface );
	AbstractVideoOutput* vo = reinterpret_cast<Phonon::Fake::AbstractVideoOutput*>( videoOutputIface->internal1() );
	Q_ASSERT( vo );
	m_videoOutput = vo;
}

}} //namespace Phonon::Fake

#include "visualization.moc"
// vim: sw=4 ts=4 noet
