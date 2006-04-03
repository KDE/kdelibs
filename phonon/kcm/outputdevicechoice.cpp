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

#include "outputdevicechoice.h"
#include <phonon/backendcapabilities.h>
#include <phonon/audiooutputdevice.h>
#include <phonon/audiocapturedevice.h>
#include <phonon/videocapturedevice.h>

OutputDeviceChoice::OutputDeviceChoice( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );
	notificationDeviceList->setModel( &m_notificationModel );
	musicDeviceList->setModel( &m_musicModel );
	videoDeviceList->setModel( &m_videoModel );
	communicationDeviceList->setModel( &m_communicationModel );
	connect( notificationDeviceList->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex& , const QModelIndex& ) ),
			SLOT( updateButtonsEnabled() ) );
	connect( musicDeviceList->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex& , const QModelIndex& ) ),
			SLOT( updateButtonsEnabled() ) );
	connect( videoDeviceList->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex& , const QModelIndex& ) ),
			SLOT( updateButtonsEnabled() ) );
	connect( communicationDeviceList->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex& , const QModelIndex& ) ),
			SLOT( updateButtonsEnabled() ) );
}

void OutputDeviceChoice::load()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	m_notificationModel.setModelData( list );
	m_musicModel.setModelData( list );
	m_videoModel.setModelData( list );
	m_communicationModel.setModelData( list );
}

void OutputDeviceChoice::save()
{
}

void OutputDeviceChoice::defaults()
{
	QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
	m_notificationModel.setModelData( list );
	m_musicModel.setModelData( list );
	m_videoModel.setModelData( list );
	m_communicationModel.setModelData( list );
}

void OutputDeviceChoice::on_notificationPreferButton_clicked()
{
	m_notificationModel.moveUp( notificationDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_notificationNoPreferButton_clicked()
{
	m_notificationModel.moveDown( notificationDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_musicPreferButton_clicked()
{
	m_musicModel.moveUp( musicDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_musicNoPreferButton_clicked()
{
	m_musicModel.moveDown( musicDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_videoPreferButton_clicked()
{
	m_videoModel.moveUp( videoDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_videoNoPreferButton_clicked()
{
	m_videoModel.moveDown( videoDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_communicationPreferButton_clicked()
{
	m_communicationModel.moveUp( communicationDeviceList->currentIndex() );
	updateButtonsEnabled();
}
void OutputDeviceChoice::on_communicationNoPreferButton_clicked()
{
	m_communicationModel.moveDown( communicationDeviceList->currentIndex() );
	updateButtonsEnabled();
}

void OutputDeviceChoice::updateButtonsEnabled()
{
	notificationPreferButton->setEnabled( notificationDeviceList->currentIndex().row() > 0 );
	notificationNoPreferButton->setEnabled( notificationDeviceList->currentIndex().row() < m_notificationModel.rowCount() - 1 );
	musicPreferButton->setEnabled( musicDeviceList->currentIndex().row() > 0 );
	musicNoPreferButton->setEnabled( musicDeviceList->currentIndex().row() < m_musicModel.rowCount() - 1 );
	videoPreferButton->setEnabled( videoDeviceList->currentIndex().row() > 0 );
	videoNoPreferButton->setEnabled( videoDeviceList->currentIndex().row() < m_videoModel.rowCount() - 1 );
	communicationPreferButton->setEnabled( communicationDeviceList->currentIndex().row() > 0 );
	communicationNoPreferButton->setEnabled( communicationDeviceList->currentIndex().row() < m_communicationModel.rowCount() - 1 );
}

#include "outputdevicechoice.moc"
// vim: sw=4 ts=4 noet
