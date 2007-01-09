/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "audiohw.h"

#include "soliddefs_p.h"
#include <solid/ifaces/audiohw.h>
#include <QStringList>
#include <config-alsa.h>
#include <alsa/asoundlib.h>
#include <kdebug.h>

namespace Solid
{
    class AudioHwPrivate
    {
        public:
#ifdef HAVE_LIBASOUND2
            AudioHwPrivate()
                : cardnum( -1 ),
                devicenum( -1 ),
                soundcardType( -1 )
            {
            }

            int cardnum;
            int devicenum;
            int soundcardType;
            QStringList driverHandles;
#endif
    };
} // namespace Solid

Solid::AudioHw::AudioHw( QObject *backendObject )
    : Capability( backendObject ),
    d( new AudioHwPrivate )
{
}

Solid::AudioHw::~AudioHw()
{
    delete d;
    d = 0;
}


Solid::AudioHw::AudioDriver Solid::AudioHw::driver()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), UnknownAudioDriver, driver() );
}

QStringList Solid::AudioHw::driverHandles()
{
#ifdef HAVE_LIBASOUND2
    if ( !d->driverHandles.isEmpty() )
    {
        // cached
        return d->driverHandles;
    }
#endif

    Ifaces::AudioHw *iface = qobject_cast<Ifaces::AudioHw*>( backendObject() );
    if ( iface )
    {
        QString handle = iface->driverHandler();
#ifdef HAVE_LIBASOUND2
        if ( iface->driver() == Alsa )
        {
            // we expect the handle to be of the form hw:X or hw:X,Y
            const int colon = handle.indexOf( ':' );
            if ( -1 == colon )
            {
                return QStringList( handle );
            }
            handle = handle.right(handle.size() - colon - 1);

            // get cardnum and devicenum
            const int comma = handle.indexOf( ',' );
            if (comma > -1)
            {
                d->devicenum = handle.right(handle.size() - 1 - comma).toInt();
                d->cardnum = handle.left(comma).toInt();
            }
            else
            {
                d->cardnum = handle.toInt();
            }

            if ( iface->deviceType() & Solid::AudioHw::AudioOutput )
            {
                // first try dmix for concurrent access then plain hw and if the hw formats don't
                // work plughw
                d->driverHandles << QLatin1String("dmix:") + handle;
            }
            if ( iface->deviceType() & Solid::AudioHw::AudioInput )
            {
                // first try dsnoop for concurrent access, then plain hw
                d->driverHandles << QLatin1String("dsnoop:") + handle;
            }
            d->driverHandles << QLatin1String("hw:") + handle
                << QLatin1String("plughw:") + handle;

            return d->driverHandles;
        }
        else
#endif
        {
            return QStringList( handle );
        }
    }
    return QStringList();
}

QString Solid::AudioHw::name()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), QString(), name() );
}

Solid::AudioHw::AudioHwTypes Solid::AudioHw::deviceType()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), UnknownAudioHwType, deviceType() );
}

Solid::AudioHw::SoundcardType Solid::AudioHw::soundcardType()
{
#ifdef HAVE_LIBASOUND2
    if ( d->soundcardType != -1 )
    {
        // cached
        return static_cast<SoundcardType>( d->soundcardType );
    }

    Ifaces::AudioHw *iface = qobject_cast<Ifaces::AudioHw*>( backendObject() );
    if ( !iface )
    {
        return InternalSoundcard;
    }

    if ( iface->driver() != Alsa )
    {
        return iface->soundcardType();
    }

    if ( d->cardnum == -1 )
    {
        driverHandles();
        if ( d->cardnum == -1 )
        {
            kWarning() << k_funcinfo << "no card number found" << endl;
            return AudioHw::InternalSoundcard;
        }
    }

    QByteArray ctlDevice( "hw:" );
    ctlDevice += QByteArray::number( d->cardnum );

    snd_ctl_card_info_t *cardInfo;
    snd_ctl_card_info_malloc(&cardInfo);

    snd_ctl_t *ctl;
    if ( 0 == snd_ctl_open( &ctl, ctlDevice.constData(), 0 /*open mode: blocking, sync*/ ) )
    {
        if ( 0 == snd_ctl_card_info( ctl, cardInfo ) )
        {
            QString cardName = QString( snd_ctl_card_info_get_name( cardInfo ) ).trimmed();
            if ( cardName.contains( "headset", Qt::CaseInsensitive ) ||
                    cardName.contains( "headphone", Qt::CaseInsensitive ) ||
                    iface->name().contains( "headset", Qt::CaseInsensitive ) ||
                    iface->name().contains( "headphone", Qt::CaseInsensitive ) )
            {
                d->soundcardType = AudioHw::Headset;
            }
            else if ( cardName.contains( "modem", Qt::CaseInsensitive ) ||
                    iface->name().contains( "modem", Qt::CaseInsensitive ) )
            {
                d->soundcardType = AudioHw::Modem;
            }
            else
            {
                //Get card driver name from a CTL card info.
                QString driver = snd_ctl_card_info_get_driver( cardInfo );
                if ( driver.contains( "usb", Qt::CaseInsensitive ) )
                {
                    d->soundcardType = AudioHw::UsbSoundcard;
                }
                else
                {
                    d->soundcardType = AudioHw::InternalSoundcard;
                }
            }
            snd_ctl_card_info_free(cardInfo);
            snd_ctl_close( ctl );
            return static_cast<SoundcardType>( d->soundcardType );
        }
        snd_ctl_close( ctl );
    }
    snd_ctl_card_info_free( cardInfo );
    kWarning() << k_funcinfo << "could not open ctl devices" << endl;
    return AudioHw::InternalSoundcard;
#else
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), InternalSoundcard, soundcardType() );
#endif
}

#include "audiohw.moc"
