/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>

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

#ifndef KABSTRACTWIDGETJOBTRACKER_P
#define KABSTRACTWIDGETJOBTRACKER_P

class KAbstractWidgetJobTracker::Private
{
public:
    Private(KAbstractWidgetJobTracker *parent)
        : q(parent)
    {
    }
    
    virtual ~Private()
    {
    }

    KAbstractWidgetJobTracker *const q;

    //### KDE 5: make this methods virtual on KAbstractWidgetJobTracker and get rid out of this
    //           workaround. (ereslibre)
    virtual void setStopOnClose(KJob *job, bool stopOnClose)
    {
        Q_UNUSED(job);
        Q_UNUSED(stopOnClose);
    }

    virtual bool stopOnClose(KJob *job) const
    {
        Q_UNUSED(job);
        return true;
    }

    virtual void setAutoDelete(KJob *job, bool autoDelete)
    {
        Q_UNUSED(job);
        Q_UNUSED(autoDelete);
    }

    virtual bool autoDelete(KJob *job) const
    {
        Q_UNUSED(job);
        return true;
    }
};

#endif // KABSTRACTWIDGETJOBTRACKER_P
