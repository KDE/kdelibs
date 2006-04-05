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

#ifndef PHONON_AUDIOEFFECTDESCRIPTION_H
#define PHONON_AUDIOEFFECTDESCRIPTION_H

#include "namedescriptiontuple.h"

namespace Phonon
{

class AudioEffectDescriptionPrivate;

class PHONONCORE_EXPORT AudioEffectDescription : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( AudioEffectDescription )
	public:
		AudioEffectDescription();
		AudioEffectDescription( const AudioEffectDescription& effectDescription );
		AudioEffectDescription& operator=( const AudioEffectDescription& effectDescription );
		bool operator==( const AudioEffectDescription& effectDescription ) const;
		static AudioEffectDescription fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Creates new (valid) description.
		 */
		AudioEffectDescription( int index, const QString& name, const QString& description );
};

}

#endif // PHONON_AUDIOEFFECTDESCRIPTION_H
// vim: sw=4 ts=4 noet tw=80
