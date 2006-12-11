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
#include "objectdescription.h"
#include "audiopath.h"
#include "abstractvideooutput.h"

#define PHONON_CLASSNAME Visualization

namespace Phonon
{
PHONON_OBJECT_IMPL

Visualization::~Visualization()
{
	K_D( Visualization );
    if(d->audioPath)
        d->removeDestructionHandler(d->audioPath, d);
    if(d->videoOutput)
        d->removeDestructionHandler(d->videoOutput, d);
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
    d->addDestructionHandler(d->audioPath, d);
	if( iface() )
		BACKEND_CALL1( "setAudioPath", QObject*, audioPath->iface() );
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
    d->addDestructionHandler(videoOutput, d);
	if( iface() )
		BACKEND_CALL1( "setVideoOutput", QObject*, videoOutput->iface() );
}

VisualizationDescription Visualization::visualization() const
{
	K_D( const Visualization );
	int index;
	if( d->backendObject )
		BACKEND_GET( int, index, "visualization" );
	else
		index = d->visualizationIndex;
	return VisualizationDescription::fromIndex( index );
}

void Visualization::setVisualization( const VisualizationDescription& newVisualization )
{
	K_D( Visualization );
	if( iface() )
		BACKEND_CALL1( "setVisualization", int, newVisualization.index() );
	else
		d->visualizationIndex = newVisualization.index();
}

/*
bool Visualization::hasParameterWidget() const
{
	K_D( const Visualization );
	if( d->backendObject )
	{
		bool ret;
		BACKEND_GET( bool, ret, "hasParameterWidget" );
		return ret;
	}
	return false;
}

QWidget* Visualization::createParameterWidget( QWidget* parent )
{
	K_D( Visualization );
	if( iface() )
	{
		QWidget* ret;
		BACKEND_GET1( QWidget*, ret, "createParameterWidget", QWidget*, parent );
		return ret;
	}
	return 0;
}
*/

void VisualizationPrivate::phononObjectDestroyed( Base* o )
{
	// this method is called from Phonon::Base::~Base(), meaning the AudioEffect
	// dtor has already been called, also virtual functions don't work anymore
	// (therefore qobject_cast can only downcast from Base)
	Q_ASSERT( o );
	AudioPath* path = static_cast<AudioPath*>( o );
	AbstractVideoOutput* output = static_cast<AbstractVideoOutput*>( o );
	if( audioPath == path )
	{
		pBACKEND_CALL1( "setAudioPath", QObject*, static_cast<QObject*>( 0 ) );
		audioPath = 0;
	}
	else if( videoOutput == output )
	{
		pBACKEND_CALL1( "setVideoOutput", QObject*, static_cast<QObject*>( 0 ) );
		videoOutput = 0;
	}
}

bool VisualizationPrivate::aboutToDeleteIface()
{
	if( backendObject )
		pBACKEND_GET( int, visualizationIndex, "visualization" );
	return true;
}

void Visualization::setupIface()
{
	K_D( Visualization );
	Q_ASSERT( d->backendObject );

	BACKEND_CALL1( "setVisualization", int, d->visualizationIndex );
	if( d->audioPath )
		BACKEND_CALL1( "setAudioPath", QObject*, d->audioPath->iface() );
	if( d->videoOutput )
		BACKEND_CALL1( "setVideoOutput", QObject*, d->videoOutput->iface() );
}

} // namespace Phonon

#include "visualization.moc"
// vim: sw=4 ts=4
