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
#ifndef Phonon_AUDIOEFFECT_H
#define Phonon_AUDIOEFFECT_H

#include "base.h"
#include "effect.h"
#include "phonondefs.h"
#include <QObject>
#include "objectdescription.h"

class QString;
class QStringList;
template<class T> class QList;

namespace Phonon
{
	class EffectParameter;
	class AudioEffectPrivate;

	/**
	 * \short Audio effects that can be inserted into an AudioPath.
	 *
	 * \warning This class is not finished.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT AudioEffect : public QObject, public Effect, public Base
	{
		friend class AudioPath;
		friend class AudioPathPrivate;
		Q_OBJECT
		K_DECLARE_PRIVATE( AudioEffect )

		public:
			/**
			 * Standard QObject constructor.
			 *
			 * \param parent QObject parent
			 */
			AudioEffect( const AudioEffectDescription& type, QObject* parent = 0 );

			AudioEffectDescription type() const;
			virtual QList<EffectParameter> parameterList() const;

		protected:
			/**
			 * \internal
			 *
			 * Constructs new audio effect with private data \p dd and a
			 * \p parent.
			 */
			AudioEffect( AudioEffectPrivate& dd, QObject* parent, const AudioEffectDescription& type = AudioEffectDescription() );

			/**
			 * \internal
			 * After construction of the Iface object this method is called
			 * throughout the complete class hierarchy in order to set up the
			 * properties that were already set on the public interface.
			 *
			 * An example implementation could look like this:
			 * \code
			 * ParentClass::setupIface();
			 * m_iface->setPropertyA( d->propertyA );
			 * m_iface->setPropertyB( d->propertyB );
			 * \endcode
			 */
			void setupIface();

		protected:
			virtual QVariant value( int parameterId ) const;
			virtual void setValue( int parameterId, QVariant newValue );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIOEFFECT_H
