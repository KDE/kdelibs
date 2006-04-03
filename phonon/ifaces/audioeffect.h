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
#ifndef Phonon_IFACES_AUDIOEFFECT_H
#define Phonon_IFACES_AUDIOEFFECT_H

#include "base.h"
#include "../effectparameter.h"

class QString;
class QStringList;
template<class T> class QList;

namespace Phonon
{
namespace Ifaces
{
	class AudioEffectPrivate;

	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class AudioEffect : virtual public Base
	{
		Q_DECLARE_PRIVATE( AudioEffect )
		public:
			QList<Phonon::EffectParameter> parameterList() const;
			virtual QString type() const = 0;
			virtual float value( int parameterId ) const = 0;
			virtual void setType( const QString& type ) = 0;
			virtual void setValue( int parameterId, float newValue ) = 0;

		protected:
			AudioEffect();
			AudioEffect( AudioEffectPrivate& dd );
			void addParameter( int parameterId,
					EffectParameter::Hints hints, float min, float max,
					float defaultValue, const QString& name,
					const QString& description = QString() );
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_IFACES_AUDIOEFFECT_H
