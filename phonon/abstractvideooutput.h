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

#include "base.h"
#include "phonondefs.h"
#include <QObject>

class QString;

namespace Phonon
{
namespace Experimental
{
    class Visualization;
} // namespace Experimental

    class AbstractVideoOutputPrivate;

    class PHONONCORE_EXPORT AbstractVideoOutput : public Base
    {
        friend class VideoPath;
        friend class VideoPathPrivate;
        friend class Experimental::Visualization;
        K_DECLARE_PRIVATE(AbstractVideoOutput)
        protected:
            /**
             * \internal
             * Constructor that is called from derived classes.
             *
             * \param parent Standard QObject parent.
             */
            AbstractVideoOutput(AbstractVideoOutputPrivate &d);

            /**
             * \internal
             * After construction of the Iface object this method is called
             * throughout the complete class hierarchy in order to set up the
             * properties that were already set on the public interface.
             *
             * An example implementation could look like this:
             * \code
             * ParentClass::setupIface();
             * m_iface->setPropertyA(d->propertyA);
             * m_iface->setPropertyB(d->propertyB);
             * \endcode
             */
            void setupIface();
    };
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_ABSTRACTVIDEOOUTPUTBASE_H
