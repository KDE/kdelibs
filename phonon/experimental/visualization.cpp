/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "visualization.h"
#include "visualization_p.h"
#include "../objectdescription.h"
#include "../audiopath.h"
#include "../audiopath_p.h"
#include "../abstractvideooutput.h"
#include "../abstractvideooutput_p.h"

#define PHONON_CLASSNAME Visualization

namespace Phonon
{
namespace Experimental
{
PHONON_OBJECT_IMPL

Visualization::~Visualization()
{
    K_D(Visualization);
    if(d->audioPath)
        d->audioPath->k_ptr->removeDestructionHandler(d);
    if(d->videoOutput)
        d->videoOutput->k_ptr->removeDestructionHandler(d);
    delete k_ptr;
}

AudioPath *Visualization::audioPath() const
{
    K_D(const Visualization);
    return d->audioPath;
}

void Visualization::setAudioPath(AudioPath *audioPath)
{
    K_D(Visualization);
    d->audioPath = audioPath;
    d->audioPath->k_ptr->addDestructionHandler(d);
    if (k_ptr->backendObject())
        BACKEND_CALL1("setAudioPath", QObject *, audioPath->k_ptr->backendObject());
}

AbstractVideoOutput *Visualization::videoOutput() const
{
    K_D(const Visualization);
    return d->videoOutput;
}

void Visualization::setVideoOutput(AbstractVideoOutput *videoOutput)
{
    K_D(Visualization);
    d->videoOutput = videoOutput;
    d->videoOutput->k_ptr->addDestructionHandler(d);
    if (k_ptr->backendObject())
        BACKEND_CALL1("setVideoOutput", QObject *, videoOutput->k_ptr->backendObject());
}

VisualizationDescription Visualization::visualization() const
{
    K_D(const Visualization);
    int index;
    if (d->m_backendObject)
        BACKEND_GET(int, index, "visualization");
    else
        index = d->visualizationIndex;
    return VisualizationDescription::fromIndex(index);
}

void Visualization::setVisualization(const VisualizationDescription &newVisualization)
{
    K_D(Visualization);
    if (k_ptr->backendObject())
        BACKEND_CALL1("setVisualization", int, newVisualization.index());
    else
        d->visualizationIndex = newVisualization.index();
}

/*
bool Visualization::hasParameterWidget() const
{
    K_D(const Visualization);
    if (d->m_backendObject)
    {
        bool ret;
        BACKEND_GET(bool, ret, "hasParameterWidget");
        return ret;
    }
    return false;
}

QWidget *Visualization::createParameterWidget(QWidget *parent)
{
    K_D(Visualization);
    if (k_ptr->backendObject())
    {
        QWidget *ret;
        BACKEND_GET1(QWidget *, ret, "createParameterWidget", QWidget *, parent);
        return ret;
    }
    return 0;
}
*/

void VisualizationPrivate::phononObjectDestroyed(BasePrivate *bp)
{
    // this method is called from Phonon::BasePrivate::~BasePrivate(), meaning the AudioEffect
    // dtor has already been called and the private class is down to BasePrivate
    Q_ASSERT(bp);
    if (audioPath->k_ptr == bp)
    {
        pBACKEND_CALL1("setAudioPath", QObject *, static_cast<QObject *>(0));
        audioPath = 0;
    }
    else if (videoOutput->k_ptr == bp)
    {
        pBACKEND_CALL1("setVideoOutput", QObject *, static_cast<QObject *>(0));
        videoOutput = 0;
    }
}

bool VisualizationPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject)
        pBACKEND_GET(int, visualizationIndex, "visualization");
    return true;
}

void VisualizationPrivate::setupBackendObject()
{
    Q_ASSERT(m_backendObject);

    pBACKEND_CALL1("setVisualization", int, visualizationIndex);
    if (audioPath)
        pBACKEND_CALL1("setAudioPath", QObject *, audioPath->k_ptr->backendObject());
    if (videoOutput)
        pBACKEND_CALL1("setVideoOutput", QObject *, videoOutput->k_ptr->backendObject());
}

} // namespace Experimental
} // namespace Phonon

#include "visualization.moc"
#undef PHONON_CLASSNAME
// vim: sw=4 ts=4
