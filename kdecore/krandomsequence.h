/* This file is part of the KDE libraries
   Copyright (c) 1999 Sean Harmer <sh@astro.keele.ac.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef K_RANDOM_SEQUENCE_H
#define K_RANDOM_SEQUENCE_H

class KRandomSequencePrivate;
class QGList;
/**
 * A class to create a pseudo-random sequence
 *
 * Given a seed number, this class will produce a sequence of
 * pseudo-random numbers.  This would typically be used in
 * applications like games.
 *
 * In general, you should instantiate a KRandomSequence object and
 * pass along your seed number in the constructor.  From then on,
 * simply call @ref getDouble or @ref getLong to obtain the next
 * number in the sequence.
 *
 * @author Sean Harmer <sh@astro.keele.ac.uk>
 */
class KRandomSequence
{
public: 
  /**
   * Creates a pseudo-random sequence based on the seed lngSeed.
   * 
   * @param lngSeed Seed to initialize the sequence with.
   *
   * If lngSeed is 0, the sequence is initialized with a value from
   * kapp->random().
   *
   * A Pseudo-random sequence is different for each seed but can be 
   * reproduced by starting the sequence with the same seed.
   *
   * If you need a single value which needs to be unpredictable, 
   * you need to use kapp->random() instead.
   * 
   */
  KRandomSequence( long lngSeed = 0 );

  /**
   * Standard destructor
   */
  virtual ~KRandomSequence();

  /**
   * Restart the sequence based on lngSeed
   */
  void setSeed( long lngSeed = 1 );

  /**
   * Get the next number from the pseudo-random sequence
   *
   * @return a psuedo-random double value between [0,1[
   */
  double getDouble(); 
  
  /**
   * Get the next number from the pseudo-random sequence
   *
   * @return a pseudo-random integer value between [0, max[
   * with 0 <= max < 1.000.000
   */
  unsigned long getLong(unsigned long max); 

  /**
   * Get a boolean from the pseudo-random sequence
   *
   * @return a boolean which is either true or false
   */
  bool getBool(); 

  /**
   * Put a list in random order
   *
   * @return modifies the order of @p list
   */
  void randomize(QGList *list);
	
private:
  void Draw(); // Generate the random number
  long m_lngSeed1;
  long m_lngSeed2;
  long m_lngShufflePos;

  static const int    m_nShuffleTableSize;
  long *m_ShuffleArray;

  KRandomSequencePrivate *d;
};

#endif

