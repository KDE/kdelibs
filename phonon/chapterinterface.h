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

#ifndef PHONON_CHAPTERINTERFACE_H
#define PHONON_CHAPTERINTERFACE_H

#include "phonon_export.h"
#include <QObject>

namespace Phonon
{
class AbstractMediaProducer;
class ChapterInterfacePrivate;

class PHONONCORE_EXPORT ChapterInterface : public QObject
{
    Q_OBJECT
    public:
        ChapterInterface(AbstractMediaProducer *);
        ~ChapterInterface();

        bool isValid() const;

        int availableChapters() const;
        int currentChapter() const;
        void setCurrentChapter(int chapterNumber);

    Q_SIGNALS:
        void availableChaptersChanged(int availableChapters);
        void chapterChanged(int chapterNumber);

    private:
        ChapterInterfacePrivate *const d;
};

} // namespace Phonon

#endif // PHONON_CHAPTERINTERFACE_H
