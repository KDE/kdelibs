/* This file is part of the KDE libraries
    Copyright (C)  2001 Holger Freyther <freyther@kde.org>

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

#ifndef renamedlgplugin_h
#define renamedlgplugin_h

#include <kio/renamedlg.h>
#include <qdialog.h>
#include <sys/types.h>
#include <qstring.h>
#include <qstringlist.h>

/** This is the base class for all plugins. 
 *  
 *
 */
class RenameDlgPlugin : public QWidget
{
public:
  /**This is the c'tor
   * 
   */
  RenameDlgPlugin(QDialog *dialog, const char *name, const QStringList &/*list*/ = QStringList() ): QWidget(dialog, name ) {};

  /** This function will be called by RenameDlg. The params are infos about the files
   *  @return If the plugin want's to display it return true if not return false
   */
  virtual bool initialize(KIO::RenameDlg_Mode /*mod*/ ,  const QString &/*_src*/, const QString &/*_dest*/,
		  const QString &/*mimeSrc*/,
		  const QString &/*mimeDest*/,
		  KIO::filesize_t /*sizeSrc*/,
		  KIO::filesize_t /*sizeDest*/,
		  time_t /*ctimeSrc*/,
		  time_t /*ctimeDest*/,
		  time_t /*mtimeSrc*/,
		  time_t /*mtimeDest*/ ) {return false;};

};

#endif

