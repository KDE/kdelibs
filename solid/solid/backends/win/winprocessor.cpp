/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "winprocessor.h"

#include <solid/backends/shared/cpufeatures.h>

#include <QSettings>

using namespace Solid::Backends::Win;

WinProcessor::WinProcessor(WinDevice *device):
    WinInterface(device)
{
    m_number = m_device->udi().mid(m_device->udi().length()-1).toInt();
}

WinProcessor::~WinProcessor()
{
}

int WinProcessor::number() const
{
    return m_number;
}

int WinProcessor::maxSpeed() const
{
    return updateCache()[m_number].speed;
}

bool WinProcessor::canChangeFrequency() const
{
    //TODO:implement
    return false;
}

Solid::Processor::InstructionSets WinProcessor::instructionSets() const
{
    static Solid::Processor::InstructionSets set = Solid::Backends::Shared::cpuFeatures();
    return set;
}

QSet<QString> WinProcessor::getUdis()
{
    static QSet<QString> out;
    if(out.isEmpty())
    {
        foreach(const ProcessorInfo &info,updateCache())
        {
            out<<QString("/org/kde/solid/win/cpu/device#%1,cpu#%2").arg(info.id).arg(info.lgicalId);
        }
    }
    return out;
}


DWORD WinProcessor::countSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}



const QMap<int,WinProcessor::ProcessorInfo> &WinProcessor::updateCache()
{
    static QMap<int,ProcessorInfo> p;
    if(p.isEmpty())
    {
        DWORD size = 1024;
        char buff[size];
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION *info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)buff;
        GetLogicalProcessorInformation(info,&size);
        size /= sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);


        uint processorCoreCount = 0;
        uint logicalProcessorCount = 0;

        for(uint i=0;i< size;++i)
        {
            if (info[i].Relationship == RelationProcessorCore)
            {
                // A hyperthreaded core supplies more than one logical processor.
                uint old = logicalProcessorCount;
                logicalProcessorCount += countSetBits(info[i].ProcessorMask);
                for(;old<logicalProcessorCount;++old)
                {
                    QSettings settings(QString("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\").append(QString::number(old)),  QSettings::NativeFormat);
                    ProcessorInfo proc;
                    proc.id = processorCoreCount;
                    proc.lgicalId = old;
                    proc.speed = settings.value("~MHz").toInt();
                    proc.vendor = settings.value("VendorIdentifier").toString().trimmed();
                    proc.name = settings.value("ProcessorNameString").toString().trimmed();
                    proc.produuct = settings.value("Identifier").toString().trimmed();

                    p[old] = proc;
                }
                processorCoreCount++;
            }
        }
    }
    return p;

}

#include "winprocessor.moc"
