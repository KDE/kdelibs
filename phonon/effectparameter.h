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
#include <QVariant>

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
	friend class VideoEffect;
	friend class VideoEffectPrivate;
	public:
		/**
		 * \internal
		 *
		 * Creates an invalid effect parameter.
		 *
		 * \see isValid()
		 */
		EffectParameter();

		/**
		 * \internal
		 *
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
		 * Returns whether the parameter should be considered a boolean toggle.
		 *
		 * \return \c true: all values are booleans
		 * \return \c false: all values are doubles or integers depending on
		 * isIntegerControl()
		 */
		bool isToggleControl() const;

		/**
		 * Returns whether the parameter should be
		 * displayed in a logarithmic scale. This is particularly useful for
		 * frequencies and gains.
		 */
		bool isLogarithmicControl() const;

		/**
		 * Returns whether the parameter should be displayed as integers using
		 * a stepped control.
		 *
		 * \return \c true: all values are integers
		 * \return \c false: all values are doubles or booleans depending on
		 * isToggleControl()
		 */
		bool isIntegerControl() const;

		/**
		 * The minimum value to be used for the control to edit the parameter.
		 *
		 * If the returned QVariant is invalid the value is not bounded from
		 * below.
		 */
		QVariant minimumValue() const;

		/**
		 * The maximum value to be used for the control to edit the parameter.
		 *
		 * If the returned QVariant is invalid the value is not bounded from
		 * above.
		 */
		QVariant maximumValue() const;

		/**
		 * The default value.
		 */
		QVariant defaultValue() const;

		/**
		 * The current value of the parameter.
		 */
		QVariant value() const;

		/**
		 * Sets the new value for the parameter.
		 *
		 * \param newValue Depending on the isIntegerControl() hint the QVariant
		 * is expected either as an integer or a double.
		 */
		void setValue( QVariant newValue );

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
		EffectParameter( int parameterId, Hints hints, QVariant min, QVariant max,
				QVariant defaultValue, const QString& name, const QString& description = QString() );

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
		QVariant min;
		QVariant max;
		QVariant defaultValue;
		QString name;
		QString description;
		EffectParameter::Hints hints;
};

} // namespace Phonon

Q_DECLARE_OPERATORS_FOR_FLAGS( Phonon::EffectParameter::Hints )

#endif // PHONON_EFFECTPARAMETER_H
// vim: sw=4 ts=4 noet tw=80
