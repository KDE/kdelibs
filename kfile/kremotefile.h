// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
      
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

#ifndef KREMOTEFILE_H
#define KREMOTEFILE_H

#include <qstring.h>
#include <kurl.h>
/**
 * Obtain and keep track of the local copy of a remote file, removes
 * the file when it is deleted.
 *
 * @author rich@kde.org
 * @version $Id$
 */
class KRemoteFile {
public:
  /**
   * Fetch a remote file.
   */
  KRemoteFile(const char *url);

  /**
   * Clean up the remote file object and delete the temporary file.
   */
  virtual ~KRemoteFile();

  /**
   * Get the name of the local copy.
   */
  const char *tempName();

  /**
   * Get the url it came from.
   */
  const char *url();

private:
  QString myTempName;
  KURL myLocation;
};


#endif // KREMOTEFILE_H
