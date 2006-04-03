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

#ifndef PHONON_EFFECTPARAMETER_H
#define PHONON_EFFECTPARAMETER_H

#include <QSharedDataPointer>
#include <QString>
#include <kdelibs_export.h>

namespace Phonon
{

class Effect;
class EffectParameterPrivate;
class AudioEffect;

namespace Ifaces
{
	class Effect;
}

/**
 * \brief This class describes one parameter of an effect.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AudioEffect
 * \see VideoEffect
 */
class PHONONCORE_EXPORT EffectParameter
{
	friend class EffectParameterPrivate;
	friend class Phonon::Ifaces::Effect;
	friend class AudioEffect;
	friend class AudioEffectPrivate;
	public:
		/**
		 * Creates an invalid effect parameter.
		 *
		 * \see isValid()
		 */
		EffectParameter();

		/**
		 * A parameter that has no pointer to an effect is invalid.
		 */
		bool isValid() const;

		/**
		 * The name of the parameter. Can be used as the label.
		 *
		 * \return A label for the parameter.
		 */
		const QString& name() const;

		/**
		 * The parameter may come with a description (LADSPA doesn't have a
		 * field for this, so don't expect many effects to provide a
		 * description).
		 *
		 * The description can be used for a tooltip or WhatsThis help.
		 *
		 * \return A text describing the parameter.
		 */
		const QString& description() const;

		/**
		 * Returns whether the parameter should be
		 * considered a boolean toggle. Data less than or equal to zero
		 * should be considered "off" or \c false and data above zero should be
		 * considered "on" or \c true.
		 */
		bool isToggleControl() const;

		/**
		 * Returns whether the parameter should be
		 * displayed in a logarithmic scale. This is particularly useful for
		 * frequencies and gains.
		 */
		bool isLogarithmicControl() const;

		/**
		 * Returns whether the parameter should be
		 * displayed as integers using a stepped control.
		 *
		 * The bounds should be slightly wider than the actual
		 * integer range required to avoid floating point rounding
		 * errors. For instance, the integer set {0,1,2,3} might be described as
		 * [ -0.1, 3.1 ]. That means the bounds have to be rounded and not
		 * simply casted to an integer.
		 */
		bool isIntegerControl() const;

		/**
		 * Returns whether the minimumValue() method should be considered
		 * meaningful.
		 */
		bool isBoundedBelow() const;

		/**
		 * Returns whether the maximumValue() method should be considered
		 * meaningful.
		 */
		bool isBoundedAbove() const;

		/**
		 * The minimum value to be used for the control to edit the parameter.
		 */
		float minimumValue() const;

		/**
		 * The maximum value to be used for the control to edit the parameter.
		 */
		float maximumValue() const;

		/**
		 * The default value.
		 */
		float defaultValue() const;

		float value() const;
		void setValue( float ) const;

		/**
		 * Equality operator
		 */
		bool operator==( const EffectParameter& rhs ) const;

		/**
		 * Inequality operator
		 */
		inline bool operator!=( const EffectParameter& rhs ) const { return ! operator==( rhs ); }

		/**
		 * \internal
		 * compares the ids of the parameters
		 */
		bool operator<( const EffectParameter& rhs ) const;

		/**
		 * \internal
		 * compares the ids of the parameters
		 */
		bool operator>( const EffectParameter& rhs ) const;

		/* default cctor, operator=, and ~EffectParameter
		 * are sufficient */

		/**
		 * Only for backend developers:
		 *
		 * Flags to set the return values of isToggleControl(),
		 * isLogarithmicControl(), isIntegerControl(), isBoundedBelow() and
		 * isBoundedAbove(). The values of the flags correspond to the values
		 * used for LADSPA effects.
		 */
		enum Hint {
			/**
			 * If this hint is set it means that
			 * the minimumValue() returns a value to be used as lower bound for
			 * the values.
			 *
			 * \see isBoundedBelow()
			 */
			BoundedBelowHint = 0x01,
			/**
			 * If this hint is set it means that
			 * the maximumValue() returns a value to be used as upper bound for
			 * the values.
			 *
			 * \see isBoundedAbove()
			 */
			BoundedAboveHint = 0x02,
			/**
			 * If this hint is set it means that
			 * the the control has only two states: zero and non-zero.
			 *
			 * \see isToggleControl()
			 */
			ToggledHint      = 0x04,

			/* LADSPA's SAMPLE_RATE hint needs to be translated by the backend
			 * to normal bounds, as the backend knows the sample rate - and the
			 * frontend doesn't */

			/**
			 * \see isLogarithmicControl()
			 */
			LogarithmicHint  = 0x10,
			/**
			 * \see isIntegerControl
			 */
			IntegerHint      = 0x20
		};
		Q_DECLARE_FLAGS( Hints, Hint )
	protected:

		/**
		 * \internal
		 * Constructs a new effect parameter and
		 * sets all the internal data.
		 */
		EffectParameter( int parameterId, Hints hints, float min, float max,
				float defaultValue, const QString& name, const QString& description = QString() );

		/**
		 * \internal
		 * Sets the pointer to the Effect object for value() and setValue().
		 */
		void setEffect( Effect* effect );

		/**
		 * \internal
		 *
		 * Returns the parameter's id.
		 */
		int id() const;

	private:
		/**
		 * The data is implicitly shared.
		 */
		QSharedDataPointer<EffectParameterPrivate> d;
};

#include <QSharedData>

class EffectParameterPrivate : public QSharedData
{
	public:
		Effect* effect;
		int parameterId;
		float min;
		float max;
		float defaultValue;
		QString name;
		QString description;
		EffectParameter::Hints hints;
};

} // namespace Phonon

Q_DECLARE_OPERATORS_FOR_FLAGS( Phonon::EffectParameter::Hints )

#endif // PHONON_EFFECTPARAMETER_H
// vim: sw=4 ts=4 noet tw=80
