/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_NET_IFACES_ENUMS_H
#define SOLID_NET_IFACES_ENUMS_H

namespace Solid
{
namespace Net
{
namespace Ifaces
{
namespace Enums
{
    struct Device
    {
        // == NM ActivationStage
        /**
         * Device connection states describe the possible states of a
         * network connection from the user's point of view.  For
         * simplicity, states from several different layers are present -
         * this is a high level view
         */
        enum ConnectionState{ Unknown, Prepare, Configure, NeedUserKey, IPStart, IPGet, IPCommit, Activated, Failed, Cancelled };
        enum Capabilities { NetworkManager = 0x1, CarrierDetect = 0x2, WirelessScan = 0x4 };
        enum Type { IEEE802_3, IEEE802_11 };
    };

    struct Network
    {
    };

    struct WirelessNetwork
    {
        enum OperationMode { Adhoc, Managed };
        // corresponding to 802.11 capabilities defined in NetworkManager.h
        enum Capabilities { WEP = 0x1, WPA = 0x2, WPA2 = 0x4, PSK = 0x8, IEEE8021X = 0x10, WEP40 = 0x20, WEP104 = 0x40, TKIP = 0x80, CCMP = 0x100 };
    };

    struct Authentication
    {

    };
    struct AuthenticationNone
    {

    };
    struct AuthenticationWEP
    {
        enum WEPType {
            WEP_ASCII,
            WEP_HEX,
            WEP_PASSPHRASE
        };

        enum WEPMethod {
            WEP_OPEN_SYSTEM,
            WEP_SHARED_KEY
        };

    };
    struct AuthenticationWPA
    {
        enum WPAProtocol {
            WPA_AUTO, /* WPA Personal */
            WPA_TKIP, /* WPA Personal */
            WPA_CCMP_AES, /* WPA Personal */
            WPA_EAP /* WPA Enterprise */
        };

        enum WPAVersion {
            WPA1, /* WPA Personal and Enterprise */
            WPA2 /* WPA Personal and Enterprise */
        };
    };
    struct AuthenticationWPAPersonal
    {

    };
    struct AuthenticationWPAEnterprise
    {
        enum EAPMethod {
            EAP_PEAP,
            EAP_TLS,
            EAP_TTLS,
        };
    };
} // Enums
} // Ifaces
} // Net
} // Solid

#endif
