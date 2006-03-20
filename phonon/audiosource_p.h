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

#ifndef PHONON_AUDIOCAPTURESOURCE_P_H
#define PHONON_AUDIOCAPTURESOURCE_P_H

#include "namedescriptiontuple_p.h"

namespace Phonon
{
	class AudioSourcePrivate : public NameDescriptionTuplePrivate
	{
		Q_DECLARE_PUBLIC( AudioSource )
		protected:
			AudioSourcePrivate()
				: NameDescriptionTuplePrivate()
				, videoIndex( -1 )
			{
			}

			AudioSourcePrivate( const AudioSourcePrivate* cpy )
				: NameDescriptionTuplePrivate( cpy )
				, videoIndex( cpy->videoIndex )
			{
			}
			
			AudioSourcePrivate& operator=( const AudioSourcePrivate& rhs )
			{
				videoIndex = rhs.videoIndex;
				NameDescriptionTuplePrivate::operator=( rhs );
				return *this;
			}

			bool operator==( const AudioSourcePrivate& rhs ) const
			{
				return NameDescriptionTuplePrivate::operator==( rhs ) && videoIndex == rhs.videoIndex;
			}

			int videoIndex;
	};
} // namespace Phonon

#endif // PHONON_AUDIOCAPTURESOURCE_P_H
// vim: sw=4 ts=4 noet tw=80
