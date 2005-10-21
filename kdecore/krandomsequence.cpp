/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Sean Harmer <sh@astro.keele.ac.uk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#undef QT3_SUPPORT
#include "krandomsequence.h"
#include "krandom.h"
#include <string.h>

static const int m_nShuffleTableSize = 32;

class KRandomSequence::Private
{
public:
  long _lngSeed1;
#define m_lngSeed1 d->_lngSeed1
  long _lngSeed2;
#define m_lngSeed2 d->_lngSeed2
  long _lngShufflePos;
#define m_lngShufflePos d->_lngShufflePos
  long _ShuffleArray[m_nShuffleTableSize];
#define m_ShuffleArray d->_ShuffleArray
};

//////////////////////////////////////////////////////////////////////////////
//	Construction / Destruction
//////////////////////////////////////////////////////////////////////////////

KRandomSequence::KRandomSequence( long lngSeed1 ) : d(new Private)
{
  // Seed the generator
  setSeed( lngSeed1 );
}

KRandomSequence::~KRandomSequence()
{
   delete d;
}

KRandomSequence::KRandomSequence(const KRandomSequence &a) : d(new Private)
{
  *d = *a.d;
}

KRandomSequence &
KRandomSequence::operator=(const KRandomSequence &a)
{
  *d = *a.d;
  return *this;
}


//////////////////////////////////////////////////////////////////////////////
//	Member Functions
//////////////////////////////////////////////////////////////////////////////

void KRandomSequence::setSeed( long lngSeed1 )
{
  // Convert the positive seed number to a negative one so that the Draw()
  // function can intialise itself the first time it is called. We just have
  // to make sure that the seed used != 0 as zero perpetuates itself in a
  // sequence of random numbers.
  if ( lngSeed1 < 0 )
  {
    m_lngSeed1 = -1;
  }
  else if (lngSeed1 == 0)
  {
    m_lngSeed1 = -((KRandom::random() & ~1)+1);
  }
  else
  {
    m_lngSeed1 = -lngSeed1;
  }
}

static const long sMod1           = 2147483563;
static const long sMod2           = 2147483399;

void KRandomSequence::Draw()
{
  static const long sMM1            = sMod1 - 1;
  static const long sA1             = 40014;
  static const long sA2             = 40692;
  static const long sQ1             = 53668;
  static const long sQ2             = 52774;
  static const long sR1             = 12211;
  static const long sR2             = 3791;
  static const long sDiv            = 1 + sMM1 / m_nShuffleTableSize;

  // Long period (>2 * 10^18) random number generator of L'Ecuyer with
  // Bayes-Durham shuffle and added safeguards. Returns a uniform random
  // deviate between 0.0 and 1.0 (exclusive of the endpoint values). Call
  // with a negative number to initialize; thereafter, do not alter idum
  // between successive deviates in a sequence. RNMX should approximate
  // the largest floating point value that is less than 1.

  int j; // Index for the shuffle table
  long k;

  // Initialise
  if ( m_lngSeed1 <= 0 )
  {
    m_lngSeed2 = m_lngSeed1;

    // Load the shuffle table after 8 warm-ups
    for ( j = m_nShuffleTableSize + 7; j >= 0; j-- )
    {
      k = m_lngSeed1 / sQ1;
      m_lngSeed1 = sA1 * ( m_lngSeed1 - k*sQ1) - k*sR1;
      if ( m_lngSeed1 < 0 )
      {
        m_lngSeed1 += sMod1;
      }

      if ( j < m_nShuffleTableSize )
      {
 	m_ShuffleArray[j] = m_lngSeed1;
      }
    }

    m_lngShufflePos = m_ShuffleArray[0];
  }

  // Start here when not initializing

  // Compute m_lngSeed1 = ( lngIA1*m_lngSeed1 ) % lngIM1 without overflows
  // by Schrage's method
  k = m_lngSeed1 / sQ1;
  m_lngSeed1 = sA1 * ( m_lngSeed1 - k*sQ1 ) - k*sR1;
  if ( m_lngSeed1 < 0 )
  {
    m_lngSeed1 += sMod1;
  }

  // Compute m_lngSeed2 = ( lngIA2*m_lngSeed2 ) % lngIM2 without overflows
  // by Schrage's method
  k = m_lngSeed2 / sQ2;
  m_lngSeed2 = sA2 * ( m_lngSeed2 - k*sQ2 ) - k*sR2;
  if ( m_lngSeed2 < 0 )
  {
    m_lngSeed2 += sMod2;
  }

  j = m_lngShufflePos / sDiv;
  m_lngShufflePos = m_ShuffleArray[j] - m_lngSeed2;
  m_ShuffleArray[j] = m_lngSeed1;

  if ( m_lngShufflePos < 1 )
  {
    m_lngShufflePos += sMM1;
  }
}

void
KRandomSequence::modulate(int i)
{
  m_lngSeed2 -= i;
  if ( m_lngSeed2 < 0 )
  {
    m_lngShufflePos += sMod2;
  }
  Draw();
  m_lngSeed1 -= i;
  if ( m_lngSeed1 < 0 )
  {
    m_lngSeed1 += sMod1;
  }
  Draw();
}

double
KRandomSequence::getDouble()
{
  static const double finalAmp         = 1.0 / double( sMod1 );
  static const double epsilon          = 1.2E-7;
  static const double maxRand          = 1.0 - epsilon;
  double temp;
  Draw();
  // Return a value that is not one of the endpoints
  if ( ( temp = finalAmp * m_lngShufflePos ) > maxRand )
  {
    // We don't want to return 1.0
    return maxRand;
  }
  else
  {
    return temp;
  }
}

unsigned long
KRandomSequence::getLong(unsigned long max)
{
  Draw();

  return max ? (((unsigned long) m_lngShufflePos) % max) : 0;
}

bool
KRandomSequence::getBool()
{
  Draw();

  return (((unsigned long) m_lngShufflePos) & 1);
}
