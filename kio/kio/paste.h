/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef __kio_paste_h__
#define __kio_paste_h__

#include <qstring.h>
#include <kurl.h>
#include <qmemarray.h>

namespace KIO {
  class Job;

  /**
   * Pastes the content of the clipboard (e.g. URLs) to the
   * given destination URL.
   * @param _dest_url the URL to receive the data
   * @param move true to move the data, false to copy
   * @return the job that handles the operation
   * @see pasteData()
   */
  Job *pasteClipboard( const KURL& _dest_url, bool move = false );

  /**
   * Pastes the given @p _data to the
   * given destination URL.
   * @param _dest_url the URL to receive the data
   * @param _data the data to copy
   * @see pasteClipboard()
   */
  void pasteData( const KURL& _dest_url, const QByteArray& _data );

  /**
   * Checks whether the clipboard is empty.
   * @return true if empty
   */
  bool isClipboardEmpty();
};

#endif
