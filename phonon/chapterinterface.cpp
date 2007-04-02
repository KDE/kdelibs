/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "chapterinterface.h"
#include "abstractmediaproducer.h"
#include "addoninterface.h"
#include <QList>
#include <QVariant>
#include "frontendinterface_p.h"

namespace Phonon
{

class ChapterInterfacePrivate : public FrontendInterfacePrivate
{
    public:
        ChapterInterfacePrivate(AbstractMediaProducer *mp) : FrontendInterfacePrivate(mp) {}

        virtual void backendObjectChanged(QObject *);
        ChapterInterface *q;
};

ChapterInterface::ChapterInterface(AbstractMediaProducer *mp)
    : QObject(mp),
    d(new ChapterInterfacePrivate(mp))
{
    d->q = this;
}

void ChapterInterfacePrivate::backendObjectChanged(QObject *backendObject)
{
    QObject::connect(backendObject, SIGNAL(chapterChanged(int)), q, SIGNAL(chapterChanged(int)));
    QObject::connect(backendObject, SIGNAL(availableChaptersChanged(int)), q, SIGNAL(availableChaptersChanged(int)));
}

ChapterInterface::~ChapterInterface()
{
    delete d;
}

bool ChapterInterface::isValid() const
{
    if (!d || !d->media) {
        return false;
    }
    return d->media->hasInterface<ChapterInterface>();
}

#define IFACE(retDefault) \
    AddonInterface *iface = d->iface(); \
    do { \
        if (!iface) { \
            return retDefault; \
        } \
    } while (false)

int ChapterInterface::availableChapters() const
{
    IFACE(0);
    return iface->interfaceCall(AddonInterface::ChapterInterface,
            AddonInterface::availableChapters).toInt();
}

int ChapterInterface::currentChapter() const
{
    IFACE(0);
    return iface->interfaceCall(AddonInterface::ChapterInterface,
            AddonInterface::chapter).toInt();
}

void ChapterInterface::setCurrentChapter(int trackNumber)
{
    IFACE();
    iface->interfaceCall(AddonInterface::ChapterInterface,
            AddonInterface::setChapter, QList<QVariant>() << QVariant(trackNumber));
}

} // namespace Phonon
#include "chapterinterface.moc"
// vim: sw=4 sts=4 et tw=100
