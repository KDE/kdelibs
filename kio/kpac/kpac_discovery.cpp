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

#include <unistd.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kprocess.h>

#include "kpac_discovery.moc"

KPACDiscovery::KPACDiscovery()
    : QObject(),
      m_stage(DHCP)
{
    char hostname[256];
    if (gethostname(hostname, 255) == 0)
        m_hostname = hostname;
}

bool KPACDiscovery::tryDiscovery()
{
    m_curl = KURL();
    switch (m_stage)
    {
        case DHCP:
        {
            m_stage = DNSAlias;
            KProcess proc;
            proc << "kpac_dhcp_helper";
            connect(&proc, SIGNAL(receivedStdout(KProcess *, char *, int)),
                SLOT(slotDHCPData(KProcess *, char *, int)));
            connect(&proc, SIGNAL(processExited(KProcess *)), SLOT(slotDone()));
            m_data = 0;
            kdDebug(7025) << "KPACDiscovery::tryDiscovery(): trying DHCP" << endl;
            if ((m_working = proc.start(KProcess::NotifyOnExit, KProcess::Stdout)))
            {
                loop();
                if (proc.normalExit() && proc.exitStatus() == 0)
                {
                    m_curl = m_data.data();
                    m_data = 0;
                    kdDebug(7025) << "KPACDiscovery::tryDiscovery(): found " << m_curl.prettyURL() << " via DHCP" << endl;
                    return true;
                }
            }
        } // Fallthrough
        case DNSAlias:
        {
            kdDebug(7025) << "KPACDiscovery::tryDiscovery(): trying DNS Alias" << endl;
            if (m_hostname.isEmpty())
                return false;
            else if (m_data.isEmpty())
                m_data = m_hostname;
            int pos = m_data.find('.');
            if (pos == -1)
                return false;
            m_data.remove(0, pos + 1); // strip till first dot inclusive
            if (m_data.find('.') == -1) // only TLD left, bail out
                return false;
            m_curl.setProtocol("http");
            m_curl.setHost("wpad." + m_data + ".");
            m_curl.setPath("/wpad.dat");
            kdDebug(7025) << "KPACDiscovery::tryDiscovery(): returning " << m_curl.prettyURL() << endl;
            return true;
        }    
        default:
            return false;
    }
}

void KPACDiscovery::slotDHCPData(KProcess *, char *buffer, int len)
{
    m_data += QCString(buffer, len);
}

void KPACDiscovery::slotDone()
{
    m_working = false;
}

void KPACDiscovery::loop()
{
    while (m_working)
        kapp->processOneEvent();
}

// vim: ts=4 sw=4 et
