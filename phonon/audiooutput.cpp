/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#include "audiooutput.h"
#include "audiooutput_p.h"
#include "factory.h"
#include "objectdescription.h"
#include "audiooutputadaptor.h"
#include "globalconfig.h"
#include "audiooutputinterface.h"

#include <cmath>
#include <knotification.h>
#include <klocale.h>
#include <QApplication>

#define PHONON_CLASSNAME AudioOutput
#define PHONON_INTERFACENAME AudioOutputInterface

namespace Phonon
{
AudioOutput::AudioOutput( Phonon::Category category, QObject* parent )
	: AbstractAudioOutput( *new AudioOutputPrivate, parent )
{
	K_D( AudioOutput );
	d->category = category;

	// select hardware device according to the category
	d->outputDeviceIndex = GlobalConfig().audioOutputDeviceFor( d->category );

	d->createIface();
	new AudioOutputAdaptor( this );
	for( int i = 0; !QDBusConnection::sessionBus().registerObject( "/AudioOutputs/" + QString::number( i ), this ); ++i );
}

void AudioOutputPrivate::createIface()
{
	if( backendObject )
		return;
	K_Q( AudioOutput );
    backendObject = Factory::createAudioOutput(q);
	if( backendObject )
		q->setupIface();
}

QString AudioOutput::name() const
{
	K_D( const AudioOutput );
	return d->name;
}

void AudioOutput::setName( const QString& newName )
{
	K_D( AudioOutput );
	d->name = newName;
}

PHONON_INTERFACE_GETTER( float, volume, d->volume )
PHONON_INTERFACE_SETTER( setVolume, volume, float )

static const double log10over20 = 0.1151292546497022842; // ln(10) / 20

double AudioOutput::volumeDecibel() const
{
	return -log( volume() ) / log10over20;
}

void AudioOutput::setVolumeDecibel( double newVolumeDecibel )
{
	setVolume( exp( -newVolumeDecibel * log10over20 ) );
}

Category AudioOutput::category() const
{
	K_D( const AudioOutput );
	return d->category;
}

AudioOutputDevice AudioOutput::outputDevice() const
{
	K_D( const AudioOutput );
	int index;
	if( d->backendObject )
        index = INTERFACE_CALL(outputDevice, ());
	else
		index = d->outputDeviceIndex;
	return AudioOutputDevice::fromIndex( index );
}

bool AudioOutput::setOutputDevice(const AudioOutputDevice &newAudioOutputDevice)
{
    K_D(AudioOutput);
    if (!newAudioOutputDevice.isValid()) {
        d->outputDeviceOverridden = false;
        d->outputDeviceIndex = GlobalConfig().audioOutputDeviceFor(d->category);
    } else {
        d->outputDeviceOverridden = true;
        d->outputDeviceIndex = newAudioOutputDevice.index();
    }
    if (iface()) {
        return INTERFACE_CALL(setOutputDevice, (d->outputDeviceIndex));
    }
    return true;
}

bool AudioOutputPrivate::aboutToDeleteIface()
{
    if (backendObject) {
        volume = pINTERFACE_CALL(volume, ());
    }
    return AbstractAudioOutputPrivate::aboutToDeleteIface();
}

void AudioOutput::setupIface()
{
	K_D( AudioOutput );
	Q_ASSERT( d->backendObject );
	AbstractAudioOutput::setupIface();

	connect( d->backendObject, SIGNAL( volumeChanged( float ) ), SIGNAL( volumeChanged( float ) ) );
    connect(d->backendObject, SIGNAL(audioDeviceFailed()), SLOT(_k_audioDeviceFailed()));

	// set up attributes
    INTERFACE_CALL(setVolume, (d->volume));

    // if the output device is not available and the device was not explicitely set
    if (!INTERFACE_CALL(setOutputDevice, (d->outputDeviceIndex)) && !d->outputDeviceOverridden) {
        // fall back in the preference list of output devices
        QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(d->category);
        if (d->outputDeviceIndex == deviceList.takeFirst()) { // removing the first device so that
            // if it's the same device as the one we tried we only try all the following
            foreach (int devIndex, deviceList) {
                if (INTERFACE_CALL(setOutputDevice, (devIndex))) {
                    d->handleFallback(devIndex);
                    break; // found one that works
                }
            }
        }
    }
}

void AudioOutputPrivate::_k_revertFallback()
{
    if (deviceBeforeFallback == -1) {
        return;
    }
    outputDeviceIndex = deviceBeforeFallback;
    pINTERFACE_CALL(setOutputDevice, (outputDeviceIndex));
    Q_Q(AudioOutput);
    emit q->outputDeviceChanged(AudioOutputDevice::fromIndex(outputDeviceIndex));
}

void AudioOutputPrivate::_k_audioDeviceFailed()
{
    kDebug(600) << k_funcinfo << endl;
    // outputDeviceIndex identifies a failing device
    // fall back in the preference list of output devices
    QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category);
    foreach (int devIndex, deviceList) {
        // if it's the same device as the one that failed, ignore it
        if (outputDeviceIndex != devIndex) {
            if (pINTERFACE_CALL(setOutputDevice, (devIndex))) {
                handleFallback(devIndex);
                break; // found one that works
            }
        }
    }
}

void AudioOutputPrivate::handleFallback(int newIndex)
{
    Q_Q(AudioOutput);
    deviceBeforeFallback = outputDeviceIndex;
    outputDeviceIndex = newIndex;
    emit q->outputDeviceChanged(AudioOutputDevice::fromIndex(outputDeviceIndex));
    if (QApplication::type() != QApplication::Tty) {
        KNotification *notification = new KNotification("AudioDeviceFallback");
        notification->setComponentData(Factory::componentData());
        AudioOutputDevice device1 = AudioOutputDevice::fromIndex(deviceBeforeFallback);
        AudioOutputDevice device2 = AudioOutputDevice::fromIndex(outputDeviceIndex);
        notification->setText(i18n("The audio playback device '<i>%1</i>' does not work. "
                    "Falling back to '<i>%2</i>'.", device1.name(), device2.name()));
        //notification->setPixmap(...);
        notification->setActions(QStringList(i18n("Revert back to device '%1'", device1.name())));
        notification->addContext(QLatin1String("Application"), KGlobal::mainComponent().componentName());
        QObject::connect(notification, SIGNAL(activated(unsigned int)), q, SLOT(_k_revertFallback()));
        notification->sendEvent();
    }
}

} //namespace Phonon

#include "audiooutput.moc"

// vim: sw=4 ts=4 tw=100 et
