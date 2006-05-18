/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "visualization.h"
#include "visualization_p.h"
#include "visualizationeffect.h"
#include "audiopath.h"
#include "abstractvideooutput.h"

namespace Phonon
{
PHONON_OBJECT_IMPL( Visualization )

Visualization::~Visualization()
{
	K_D( Visualization );
	if( d->audioPath )
		d->audioPath->removeDestructionHandler( this );
	if( d->videoOutput )
		d->videoOutput->removeDestructionHandler( this );
}

AudioPath* Visualization::audioPath() const
{
	K_D( const Visualization );
	return d->audioPath;
}

void Visualization::setAudioPath( AudioPath* audioPath )
{
	K_D( Visualization );
	d->audioPath = audioPath;
	if( iface() )
		d->iface()->setAudioPath( audioPath->iface() );
}

AbstractVideoOutput* Visualization::videoOutput() const
{
	K_D( const Visualization );
	return d->videoOutput;
}

void Visualization::setVideoOutput( AbstractVideoOutput* videoOutput )
{
	K_D( Visualization );
	d->videoOutput = videoOutput;
	if( iface() )
		d->iface()->setVideoOutput( videoOutput->iface() );
}

VisualizationEffect Visualization::visualization() const
{
	K_D( const Visualization );
	return VisualizationEffect::fromIndex( d->iface() ? d->iface()->visualization() : d->visualizationIndex );
}

void Visualization::setVisualization( const VisualizationEffect& newVisualization )
{
	K_D( Visualization );
	if( iface() )
		d->iface()->setVisualization( newVisualization.index() );
	else
		d->visualizationIndex = newVisualization.index();
}

bool Visualization::hasParameterWidget() const
{
	K_D( const Visualization );
	if( d->iface() )
		return d->iface()->hasParameterWidget();
	return false;
}

QWidget* Visualization::createParameterWidget( QWidget* parent )
{
	K_D( Visualization );
	if( iface() )
		return d->iface()->createParameterWidget( parent );
	return 0;
}

void Visualization::phononObjectDestroyed( Base* o )
{
	// this method is called from Phonon::Base::~Base(), meaning the AudioEffect
	// dtor has already been called, also virtual functions don't work anymore
	// (therefore qobject_cast can only downcast from Base)
	K_D( Visualization );
	Q_ASSERT( o );
	AudioPath* path = static_cast<AudioPath*>( o );
	AbstractVideoOutput* output = static_cast<AbstractVideoOutput*>( o );
	if( d->audioPath == path )
		d->audioPath = 0;
	else if( d->videoOutput == output )
		d->videoOutput = 0;
}

bool VisualizationPrivate::aboutToDeleteIface()
{
	if( iface() )
		visualizationIndex = iface()->visualization();
	return true;
}

void Visualization::setupIface()
{
	K_D( Visualization );
	Q_ASSERT( d->iface() );

	d->iface()->setVisualization( d->visualizationIndex );
	if( d->audioPath )
		d->iface()->setAudioPath( d->audioPath->iface() );
	if( d->videoOutput )
		d->iface()->setVideoOutput( d->videoOutput->iface() );
}

} // namespace Phonon

#include "visualization.moc"
// vim: sw=4 ts=4 noet
