/* This file is part of the KDE libraries
   Copyright (c) 2001 Malte Starostik <malte.starostik@t-online.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#ifndef kpac_discovery_h
#define kpac_discovery_h

#include <qobject.h>

#include <kurl.h>

class KProcess;

class KPACDiscovery : public QObject
{
    Q_OBJECT
public:
    KPACDiscovery();
    bool tryDiscovery();
    const KURL &curl() const { return m_curl; }

private slots:
    void slotDHCPData(KProcess *, char *, int);
    void slotDone();

private:
    void loop();

private:
    bool m_working;
    QCString m_hostname;
    enum
    {
        DHCP,
        DNSAlias
    } m_stage;
    KURL m_curl;
    QCString m_data;
};

// vim: ts=4 sw=4 et

#endif
