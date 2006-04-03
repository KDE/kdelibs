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
#ifndef Phonon_IFACES_EFFECT_H
#define Phonon_IFACES_EFFECT_H

#include "base.h"
#include "../effectparameter.h"

class QString;
class QStringList;
template<class T> class QList;

namespace Phonon
{
namespace Ifaces
{
	class EffectPrivate;

	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class Effect : public Base
	{
		Q_DECLARE_PRIVATE( Effect )
		public:
			/**
			 * \internal
			 * This method is used from the frontend classes to get all
			 * parameters. You don't need it in a backend implementation.
			 */
			QList<Phonon::EffectParameter> parameterList() const;

			virtual float value( int parameterId ) const = 0;
			virtual void setValue( int parameterId, float newValue ) = 0;

		protected:
			Effect();
			
			/**
			 * \internal
			 * passes private object
			 */
			Effect( EffectPrivate& dd );

			/**
			 * Adds a parameter that the user may set to the effect. Call this
			 * method once for every parameter.
			 *
			 * \param parameterId This is a number to uniquely identify the
			 * parameter. The id is used for value() and setValue().
			 *
			 * \param name The name/label for this parameter.
			 *
			 * \param hints Sets the hints for the type of parameter.
			 *
			 * \param defaultValue The value that should be used as a default.
			 *
			 * \param min The minium value allowed for this parameter. You only
			 * need to set this if the BoundedBelowHint is set.
			 *
			 * \param max The maxium value allowed for this parameter. You only
			 * need to set this if the BoundedAboveHint is set.
			 *
			 * \param description A descriptive text for the parameter
			 * (explaining what it controls) to be used as a tooltip or
			 * WhatsThis help.
			 */
			void addParameter( int parameterId,
					const QString& name,
					EffectParameter::Hints hints,
					float defaultValue,
					float min = 0.0f, float max = 0.0f,
					const QString& description = QString() );
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_IFACES_EFFECT_H
