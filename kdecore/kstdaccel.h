/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/* Convenient methods for access of the common shortcut keys in
 * the key configuration.
 */
#ifndef KSTDACCEL_H
#define KSTDACCEL_H

#include <kkeyconf.h>

class KStdAccel: public KKeyConfig
{
public:
  KStdAccel(KConfig* pConfig);

  uint open(void) const;
  uint openNew(void) const;
  uint close(void) const;
  uint save(void) const;
  uint print(void) const;
  uint quit(void) const;
  uint cut(void) const;
  uint copy(void) const;
  uint paste(void) const;
  uint undo(void) const;
  uint find(void) const;
  uint replace(void) const;
  uint insert(void) const;
  uint home(void) const;
  uint end(void) const;
  uint prior(void) const;
  uint next(void) const;
  uint help(void) const;

protected:
  uint readKey(const char* keyName, uint defaultKey=0) const;
};

#endif /*kshortcut_h*/
