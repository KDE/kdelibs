/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "outputdevicechoice2.h"
#include <phonon/backendcapabilities.h>
#include <phonon/audiooutputdevice.h>
#include <phonon/audiocapturedevice.h>
#include <phonon/videocapturedevice.h>

OutputDeviceChoice::OutputDeviceChoice( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );
	notificationCombo->setModel( &m_notificationModel );
	musicCombo->setModel( &m_musicModel );
	//videoCombo->setModel( &m_videoModel );
	communicationCombo->setModel( &m_communicationModel );
	notificationFallbackCombo->setModel( &m_notificationModel2 );
	musicFallbackCombo->setModel( &m_musicModel2 );
	//videoFallbackCombo->setModel( &m_videoModel2 );
	communicationFallbackCombo->setModel( &m_communicationModel2 );
}

void OutputDeviceChoice::load()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	m_notificationModel.setModelData( list );
	m_musicModel.setModelData( list );
	//m_videoModel.setModelData( list );
	m_communicationModel.setModelData( list );

	list.removeFirst();
	m_notificationModel2.setModelData( list );
	m_musicModel2.setModelData( list );
	//m_videoModel2.setModelData( list );
	m_communicationModel2.setModelData( list );

	notificationCombo->setCurrentIndex( 0 );
	musicCombo->setCurrentIndex( 0 );
	//videoCombo->setCurrentIndex( 0 );
	communicationCombo->setCurrentIndex( 0 );

	notificationFallbackCombo->setCurrentIndex( 0 );
	musicFallbackCombo->setCurrentIndex( 0 );
	//videoFallbackCombo->setCurrentIndex( 0 );
	communicationFallbackCombo->setCurrentIndex( 0 );
}

void OutputDeviceChoice::save()
{
}

void OutputDeviceChoice::defaults()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();

	m_notificationModel.setModelData( list );
	m_musicModel.setModelData( list );
	//m_videoModel.setModelData( list );
	m_communicationModel.setModelData( list );

	list.removeFirst();
	m_notificationModel2.setModelData( list );
	m_musicModel2.setModelData( list );
	//m_videoModel2.setModelData( list );
	m_communicationModel2.setModelData( list );

	notificationCombo->setCurrentIndex( 0 );
	musicCombo->setCurrentIndex( 0 );
	//videoCombo->setCurrentIndex( 0 );
	communicationCombo->setCurrentIndex( 0 );

	notificationFallbackCombo->setCurrentIndex( 0 );
	musicFallbackCombo->setCurrentIndex( 0 );
	//videoFallbackCombo->setCurrentIndex( 0 );
	communicationFallbackCombo->setCurrentIndex( 0 );
}

void OutputDeviceChoice::on_notificationCombo_activated()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	const int posIndex = notificationCombo->currentIndex();
	const int tupIndex = m_notificationModel.tupleIndexAtPositionIndex( posIndex );
	list.removeAll( Phonon::AudioOutputDevice::fromIndex( tupIndex ) );
	const int oldIndex = notificationFallbackCombo->currentIndex();
	m_notificationModel2.setModelData( list );
	if( oldIndex == posIndex )
		notificationFallbackCombo->setCurrentIndex( 0 );
	else if( oldIndex < posIndex )
		notificationFallbackCombo->setCurrentIndex( oldIndex );
	else
		notificationFallbackCombo->setCurrentIndex( oldIndex - 1 );
}

void OutputDeviceChoice::on_musicCombo_activated()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	const int posIndex = musicCombo->currentIndex();
	const int tupIndex = m_musicModel.tupleIndexAtPositionIndex( posIndex );
	list.removeAll( Phonon::AudioOutputDevice::fromIndex( tupIndex ) );
	const int oldIndex = musicFallbackCombo->currentIndex();
	m_musicModel2.setModelData( list );
	if( oldIndex == posIndex )
		musicFallbackCombo->setCurrentIndex( 0 );
	else if( oldIndex < posIndex )
		musicFallbackCombo->setCurrentIndex( oldIndex );
	else
		musicFallbackCombo->setCurrentIndex( oldIndex - 1 );
}

//void OutputDeviceChoice::on_videoCombo_activated()
//{
	//QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	//const int posIndex = videoCombo->currentIndex();
	//const int tupIndex = m_videoModel.tupleIndexAtPositionIndex( posIndex );
	//list.removeAll( Phonon::AudioOutputDevice::fromIndex( tupIndex ) );
	//const int oldIndex = videoFallbackCombo->currentIndex();
	//m_videoModel2.setModelData( list );
	//if( oldIndex == posIndex )
		//videoFallbackCombo->setCurrentIndex( 0 );
	//else if( oldIndex < posIndex )
		//videoFallbackCombo->setCurrentIndex( oldIndex );
	//else
		//videoFallbackCombo->setCurrentIndex( oldIndex - 1 );
//}

void OutputDeviceChoice::on_communicationCombo_activated()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	const int posIndex = communicationCombo->currentIndex();
	const int tupIndex = m_communicationModel.tupleIndexAtPositionIndex( posIndex );
	list.removeAll( Phonon::AudioOutputDevice::fromIndex( tupIndex ) );
	const int oldIndex = communicationFallbackCombo->currentIndex();
	m_communicationModel2.setModelData( list );
	if( oldIndex == posIndex )
		communicationFallbackCombo->setCurrentIndex( 0 );
	else if( oldIndex < posIndex )
		communicationFallbackCombo->setCurrentIndex( oldIndex );
	else
		communicationFallbackCombo->setCurrentIndex( oldIndex - 1 );
}

#include "outputdevicechoice2.moc"
// vim: sw=4 ts=4
