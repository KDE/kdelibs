/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
                       Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_IFACES_ENUMS_H
#define SOLID_IFACES_ENUMS_H

#include <QFlags>

namespace Solid
{
namespace Ifaces
{
namespace Enums
{

    /**
     * This struct holds the enumerations used by Solid::Device
     * and Solid::Ifaces::Device. You shouldn't use it directly.
     */
    struct Device
    {
        /**
         * This enum type defines the type of change that can occur to a Device
         * property.
         *
         * - PropertyModified : A property value has changed in the device
         * - PropertyAdded : A new property has been added to the device
         * - PropertyRemoved : A property has been removed from the device
         */
        enum PropertyChange { PropertyModified, PropertyAdded, PropertyRemoved };
    };


    /**
     * This struct holds the enumerations used by Solid::Capability
     * and Solid::Ifaces::Capability. You shouldn't use it directly.
     */
    struct Capability
    {
        /**
         * This enum type defines the type of capability that a Device can have.
         *
         * - Unknown : An undetermined capability
         * - Processor : A processor
         * - Block : A block device
         * - Storage : A storage drive
         * - Cdrom : A CD-ROM drive
         * - Volume : A volume
         * - OpticalDisc : An optical disc
         * - Camera : A digital camera
         * - PortableMediaPlayer: A portable media player
         * - NetworkIface: A network interface
         * - Display : A video display
         */
        enum Type { Unknown = 0, Processor = 1, Block = 2,
                    Storage = 4, Cdrom = 8,
                    Volume = 16, OpticalDisc = 32,
                    Camera = 64, PortableMediaPlayer = 128,
                    NetworkIface = 256, AcAdapter = 512,
                    Battery = 1024, Button = 2048,
                    Display = 4096, AudioIface = 8192 };

        /**
         * This type stores an OR combination of Type values.
         */
        Q_DECLARE_FLAGS( Types, Type );
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( Capability::Types )


    /**
     * This struct holds the enumerations used by Solid::Storage
     * and Solid::Ifaces::Storage. You shouldn't use it directly.
     */
    struct Storage
    {
        /**
         * This enum type defines the type of bus a storage device is attached to.
         *
         * - Ide : An Integrated Drive Electronics (IDE) bus, also known as ATA
         * - Usb : An Universal Serial Bus (USB)
         * - Ieee1394 : An Ieee1394 bus, also known as Firewire
         * - Scsi : A Small Computer System Interface bus
         * - Sata : A Serial Advanced Technology Attachment (SATA) bus
         * - Platform : A legacy bus that is part of the underlying platform
         */
        enum Bus { Ide, Usb, Ieee1394, Scsi, Sata, Platform };

        /**
         * This enum type defines the type of drive a storage device can be.
         *
         * - HardDisk : A hard disk
         * - CdromDrive : An optical drive
         * - Floppy : A floppy disk drive
         * - Tape : A tape drive
         * - CompactFlash : A Compact Flash card reader
         * - MemoryStick : A Memory Stick card reader
         * - SmartMedia : A Smart Media card reader
         * - SdMmc : A SecureDigital/MultiMediaCard card reader
         */
        enum DriveType { HardDisk, CdromDrive, Floppy, Tape, CompactFlash, MemoryStick, SmartMedia, SdMmc };
    };


    /**
     * This struct holds the enumerations used by Solid::Cdrom
     * and Solid::Ifaces::Cdrom. You shouldn't use it directly.
     */
    struct Cdrom
    {
        /**
         * This enum type defines the type of medium a cdrom drive supports.
         *
         * - Cdr : A Recordable Compact Disc (CD-R)
         * - Cdrw : A ReWritable Compact Disc (CD-RW)
         * - Dvd : A Digital Versatile Disc (DVD)
         * - Dvdr : A Recordable Digital Versatile Disc (DVD-R)
         * - Dvdrw : A ReWritable Digital Versatile Disc (DVD-RW)
         * - Dvdram : A Random Access Memory Digital Versatile Disc (DVD-RAM)
         * - Dvdplusr : A Recordable Digital Versatile Disc (DVD+R)
         * - Dvdplusrw : A ReWritable Digital Versatile Disc (DVD+RW)
         * - Dvdplusdl : A Dual Layer Digital Versatile Disc (DVD+R DL)
         */
        enum MediumType { Cdr, Cdrw, Dvd, Dvdr, Dvdrw, Dvdram, Dvdplusr, Dvdplusrw, Dvdplusdl };

        /**
         * This type stores an OR combination of MediumType values.
         */
        Q_DECLARE_FLAGS( MediumTypes, MediumType );
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( Cdrom::MediumTypes )


    /**
     * This struct holds the enumerations used by Solid::Volume
     * and Solid::Ifaces::Volume. You shouldn't use it directly.
     */
    struct Volume
    {
        /**
         * This enum type defines the how a volume is used.
         *
         * - FileSystem : A mountable filesystem volume
         * - PartitionTable : A volume containing a partition table
         * - Raid : A volume member of a raid set (not mountable)
         * - Other : A not mountable volume (like a swap partition)
         * - Unused : An unused or free volume
         */
        enum UsageType { FileSystem, PartitionTable, Raid, Other, Unused };
    };


    /**
     * This struct holds the enumerations used by Solid::OpticalDisc
     * and Solid::Ifaces::OpticalDisc. You shouldn't use it directly.
     */
    struct OpticalDisc
    {
        /**
         * This enum type defines the type of content available in an optical disc.
         *
         * - Audio : A disc containing audio
         * - Data : A disc containing data
         * - VideoCd : A Video Compact Disc (VCD)
         * - SuperVideoCd : A Super Video Compact Disc (SVCD)
         * - VideoDvd : A Video Digital Versatile Disc (DVD-Video)
         */
        enum ContentType { Audio, Data, VideoCd, SuperVideoCd, VideoDvd };

        /**
         * This type stores an OR combination of ContentType values.
         */
        Q_DECLARE_FLAGS( ContentTypes, ContentType )

        /**
         * This enum type defines the type of optical disc it can be.
         *
         * - CdRom : A Compact Disc Read-Only Memory (CD-ROM)
         * - CdRecordable : A Compact Disc Recordable (CD-R)
         * - CdRewritable : A Compact Disc ReWritable (CD-RW)
         * - DvdRom : A Digital Versatile Disc Read-Only Memory (DVD-ROM)
         * - DvdRam : A Digital Versatile Disc Random Access Memory (DVD-RAM)
         * - DvdRecordable : A Digital Versatile Disc Recordable (DVD-R)
         * - DvdRewritable : A Digital Versatile Disc ReWritable (DVD-RW)
         * - DvdPlusRecordable : A Digital Versatile Disc Recordable (DVD+R)
         * - DvdPlusRewritable : A Digital Versatile Disc ReWritable (DVD+RW)
         * - UnknownDiscType : An undetermined disc type
         */
        enum DiscType { CdRom, CdRecordable, CdRewritable, DvdRom, DvdRam,
                        DvdRecordable, DvdRewritable,
                        DvdPlusRecordable, DvdPlusRewritable, UnknownDiscType };
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( OpticalDisc::ContentTypes )


    /**
     * This struct holds the enumerations used by Solid::Camera
     * and Solid::Ifaces::Camera. You shouldn't use it directly.
     */
    struct Camera
    {
        /**
         * This enum type defines the access method that can be used for a camera.
         *
         * - MassStorage : A mass storage camera
         * - Ptp : A camera supporting Picture Transfer Protocol (PTP)
         * - Proprietary : A camera using a proprietary protocol
         */
        enum AccessType { MassStorage, Ptp, Proprietary };
    };

    /**
     * This struct holds the enumerations used by Solid::PortableMediaPlayer
     * and Solid::Ifaces::PortableMediaPlayer. You shouldn't use it directly.
     */
    struct PortableMediaPlayer
    {
        /**
         * This enum type defines the access method that can be used for a portable media player
         *
         * - MassStorage : A mass storage portable media player
         * - Proprietary : A portable media player using a proprietary protocol
         */
         enum AccessType { MassStorage, Proprietary };
    };

    /**
     * This struct holds the enumerations used by Solid::Battery
     * and Solid::Ifaces::Battery. You shouldn't use it directly.
     */
    struct Battery
    {
        /**
         * This enum type defines the type of the device holding the battery
         *
         * - PdaBattery : A battery in a Personal Digital Assistant
         * - UpsBattery : A battery in an Uninterruptible Power Supply
         * - PrimaryBattery : A primary battery for the system (for example laptop battery)
         * - MouseBattery : A battery in a mouse
         * - KeyboardBattery : A battery in a keyboard
         * - KeyboardMouseBattery : A battery in a combined keyboard and mouse
         * - CameraBattery : A battery in a camera
         * - UnknownBattery : A battery in an unknown device
         */
        enum BatteryType { UnknownBattery, PdaBattery, UpsBattery,
                           PrimaryBattery, MouseBattery, KeyboardBattery,
                           KeyboardMouseBattery, CameraBattery };

        /**
         * This enum type defines the kind of charge level a battery can expose
         *
         * - MaxLevel : The maximum charge level the battery got designed for
         * - LastFullLevel : The last charge level the battery got when full
         * - CurrentLevel : The current charge level
         * - WarningLevel : The battery is in 'warning' state below this level
         * - LowLevel : The battery is in 'low' state below this level
         */
        enum LevelType { MaxLevel, LastFullLevel, CurrentLevel,
                         WarningLevel, LowLevel };

        /**
         * This enum type defines charge state of a battery
         *
         * - NoCharge : Battery charge is stable, not charging or discharging
         * - Charging : Battery is charging
         * - Discharging : Battery is discharging
         */
        enum ChargeState { NoCharge, Charging, Discharging };
    };

    /**
     * This struct holds the enumeration used by Solid::Button
     * and Solid::Ifaces::Button. You shouldn't use it directly.
     */
    struct Button
    {
        /**
         * This enum type defines the type of button.
         *
         * - LidButton : The switch on a laptop that senses whether the lid is open or closed.
         * - PowerButton : The main power button on the computer.
         * - SleepButton : The sleep button on a computer capable of putting the computer into a suspend state.
         * - UnknowButtonType : The type of the button is unknow.
         */
         enum ButtonType{ LidButton, PowerButton, SleepButton, UnknowButtonType };
    };

    /**
     * This struct holds the enumerations used by Solid::Display
     * and Solid::Ifaces::Display. You shouldn't use it directly.
     */
    struct Display
    {
        /**
         * This enum type defines the type of display a device can be.
         *
         * - Lcd: A LCD (Liquid Crystal Display) panel
         * - Crt: A CRT (Cathode Ray Tube) tube
         * - TvOut: A TV Out port
         * - UnknowDisplayType: An undetermined display type
         */
        enum DisplayType{ Lcd, Crt, TvOut, UnknowDisplayType };
    };

    /**
     * This struct holds the enumerations used by Solid::AudioIface
     * and Solid::Ifaces::AudioIface. You shouldn't use it directly.
     */
    struct AudioIface
    {
        /**
         * This enum type defines the type of driver required to
         * interact with the device.
         *
         * - Alsa: An Advanced Linux Sound Architecture (ALSA) driver device
         * - OpenSoundSystem: An Open Sound System (OSS) driver device
         * - UnknownAudioDriver: An unknown driver device
         */
        enum AudioDriver{ Alsa, OpenSoundSystem, UnknownAudioDriver };

        /**
         * This enum type defines the type of audio interface this
         * device expose.
         *
         * - AudioControl: A control/mixer interface
         * - AudioInput: An audio source
         * - AudioOutput: An audio sink
         * - UnknownAudioIfaceType: An unknown audio interface
         */
        enum AudioIfaceType{ UnknownAudioIfaceType, AudioControl, AudioInput, AudioOutput };

        /**
         * This type stores an OR combination of AudioIfaceType values.
         */
        Q_DECLARE_FLAGS( AudioIfaceTypes, AudioIfaceType )
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( AudioIface::AudioIfaceTypes )


    /**
     * This struct holds the enumerations used by Solid::PowerManager
     * and Solid::Ifaces::PowerManager. You shouldn't use it directly.
     */
    struct PowerManager
    {
        /**
         * This enum type defines the different states of the system battery.
         *
         * - NoBatteryState: No battery available
         * - Normal: The battery is at its normal charge level
         * - Warning: The battery is at its warning charge level
         * - Low: The battery is at its low charge level
         * - Critical: The battery is at its critical charge level
         */
        enum BatteryState{ NoBatteryState, Normal, Warning, Low, Critical };

        /**
         * This enum type defines the different states of the AC adapter.
         *
         * - UnknownAcAdapterState: The AC adapter has an unknown state
         * - Plugged: The AC adapter is plugged
         * - Unplugged: The AC adapter is unplugged
         */
        enum AcAdapterState{ UnknownAcAdapterState, Plugged, Unplugged };

        /**
         * This enum type defines the types of system button events.
         *
         * - UnknownButtonType: An unknown button
         * - PowerButton: A power button pressed event, generally used to turn on or off the system
         * - SleepButton: A sleep button pressed event, generally used to make the system asleep
         * - LidOpen: A laptop lid open event
         * - LidClose: A laptop lid close event
         */
        enum ButtonType{ UnknownButtonType, PowerButton, SleepButton, LidOpen, LidClose };

        /**
         * This enum type defines the different suspend methods.
         *
         * - UnknownSuspendMethod: The name says it all
         * - Standby: Processes are stopped, some hardware is deactivated (ACPI S1)
         * - ToRam: Most devices are deactivated, only RAM is powered (ACPI S3)
         * - ToDisk: State of the machine is saved to disk, and it's powered down (ACPI S4)
         */
        enum SuspendMethod{ UnknownSuspendMethod = 0, Standby = 1, ToRam = 2, ToDisk = 4};

        /**
         * This type stores an OR combination of SuspendMethod values.
         */
        Q_DECLARE_FLAGS( SuspendMethods, SuspendMethod )

        /**
         * This enum type defines the different CPU frequency policies.
         *
         * - UnknownCpuFreqPolicy: The name says it all
         * - Dynamic: Frequency is changed depending on the processor load
         * - Powersave: Frequency is always set to the lowest available
         * - Performance: Frequency is always set to the highest available
         */
        enum CpuFreqPolicy{ UnknownCpuFreqPolicy = 0, Dynamic = 1, Powersave = 2, Performance = 4 };

        /**
         * This type stores an OR combination of CpuFreqPolicy values.
         */
        Q_DECLARE_FLAGS( CpuFreqPolicies, CpuFreqPolicy )
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( PowerManager::SuspendMethods )
    Q_DECLARE_OPERATORS_FOR_FLAGS( PowerManager::CpuFreqPolicies )


    struct NetworkDevice
    {
        // == NM ActivationStage
        /**
         * Device connection states describe the possible states of a
         * network connection from the user's point of view.  For
         * simplicity, states from several different layers are present -
         * this is a high level view
         */
        enum ConnectionState{ Unknown, Prepare, Configure, NeedUserKey,
                              IPStart, IPGet, IPCommit, Activated, Failed,
                              Cancelled };
        enum Capability { NetworkManager = 0x1, CarrierDetect = 0x2,
                          WirelessScan = 0x4 };
        enum Type { IEEE802_3, IEEE802_11 };

        Q_DECLARE_FLAGS( Capabilities, Capability )
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( NetworkDevice::Capabilities )

    struct Network
    {
    };

    struct WirelessNetwork
    {
        enum OperationMode { Adhoc, Managed };
        // corresponding to 802.11 capabilities defined in NetworkManager.h
        enum Capability { WEP = 0x1, WPA = 0x2, WPA2 = 0x4, PSK = 0x8,
                          IEEE8021X = 0x10, WEP40 = 0x20, WEP104 = 0x40,
                          TKIP = 0x80, CCMP = 0x100 };
        Q_DECLARE_FLAGS( Capabilities, Capability )
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( WirelessNetwork::Capabilities )

}
}
}

#endif
