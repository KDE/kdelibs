/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_ABSTRACTVIDEOOUTPUTBASE_H
#define Phonon_ABSTRACTVIDEOOUTPUTBASE_H

#include "phonondefs.h"
#include "phonon_export.h"
#include <QtCore/QObject>

class QString;

namespace Phonon
{
namespace Experimental
{
    class Visualization;
    class VisualizationPrivate;
} // namespace Experimental

    class AbstractVideoOutputPrivate;

    /** \class AbstractVideoOutput abstractvideooutput.h Phonon/AbstractVideoOutput
     * \brief Common base class for all video outputs.
     *
     * \see VideoWidget
     */
    class PHONON_EXPORT AbstractVideoOutput
    {
        friend class VideoPath;
        friend class VideoPathPrivate;
        friend class Experimental::Visualization;
        friend class Experimental::VisualizationPrivate;
        K_DECLARE_PRIVATE(AbstractVideoOutput)
        protected:
            /**
             * \internal
             * Constructor that is called from derived classes.
             *
             * \param parent Standard QObject parent.
             */
            AbstractVideoOutput(AbstractVideoOutputPrivate &d);

            ~AbstractVideoOutput();

            AbstractVideoOutputPrivate *const k_ptr;
    };
} //namespace Phonon

#endif // Phonon_ABSTRACTVIDEOOUTPUTBASE_H
