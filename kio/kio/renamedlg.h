/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                  2001 Holger Freyther <freyther@kde.org>

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

#ifndef __kio_rename_dlg__
#define __kio_rename_dlg__ "$Id$"

#include <kurl.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qstring.h>
#include <sys/types.h>

#include <kio/global.h>

namespace KIO {

enum RenameDlg_Mode { M_OVERWRITE = 1, M_OVERWRITE_ITSELF = 2, M_SKIP = 4, M_SINGLE = 8, M_MULTI = 16, M_RESUME = 32, M_NORENAME = 64 };

enum RenameDlg_Result { R_RESUME = 6, R_RESUME_ALL = 7, R_OVERWRITE = 4, R_OVERWRITE_ALL = 5, R_SKIP = 2, R_AUTO_SKIP = 3, R_RENAME = 1, R_CANCEL = 0 };


/**
 * A dialog for the options to rename two files
 */
class RenameDlg : public QDialog
{
  Q_OBJECT
public:
  /**
   * Construct a "rename" dialog
   * @param parent parent widget (often 0)
   * @param caption the caption for the dialog box
   * @param src the url to the file/dir we're trying to copy, as it's part of the text message
   * @param dest the path to destination file/dir, i.e. the one that already exists
   * @param mode parameters for the dialog (which buttons to show...),
   * @see RenameDlg_Mode
   *
   * The following parameters bring optionnal information about @p src and @p dest
   * @param sizeSrc size of source file
   * @param sizeDest size of destination file
   * @param ctimeSrc creation time of source file
   * @param ctimeDest creation time of destination file
   * @param mtimeSrc modification time of source file
   * @param mtimeDest modification time of destination file
   *
   * @param modal set to true for a modal dialog
   */
  RenameDlg( QWidget *parent, const QString & caption,
             const QString & src, const QString & dest, RenameDlg_Mode mode,
             KIO::filesize_t sizeSrc = (KIO::filesize_t) -1,
             KIO::filesize_t sizeDest = (KIO::filesize_t) -1,
             time_t ctimeSrc = (time_t) -1,
             time_t ctimeDest = (time_t) -1,
             time_t mtimeSrc = (time_t) -1,
             time_t mtimeDest = (time_t) -1,
             bool modal = FALSE );
  ~RenameDlg();

  /**
   * @return the new destination
   * valid only if RENAME was chosen
   */
  KURL newDestURL();



public slots:
  void b0Pressed();
  void b1Pressed();
  void b2Pressed();
  void b3Pressed();
  void b4Pressed();
  void b5Pressed();
  void b6Pressed();
  void b7Pressed();
  void b8Pressed();

protected slots:
  void enableRenameButton(const QString &);
private:
 class RenameDlgPrivate;
 RenameDlgPrivate *d;
 void pluginHandling( );
 QString mime( const QString &string );
};

  /**
   * \addtogroup renamedlg "RenameDlg related Functions"
   *  @{
   * \relate KIO::RenameDlg
   * Construct a modal, parent-less "rename" dialog, and return
   * a result code, as well as the new dest. Much easier to use than the
   * class @ref RenameDlg directly.

   * @param caption the caption for the dialog box
   * @param src the file/dir we're trying to copy, as it's part of the text message
   * @param dest the destination file/dir, i.e. the one that already exists
   * @param mode parameters for the dialog (which buttons to show...),
   * @see RenameDlg_Mode
   * @param newDest the new destination path, valid if R_RENAME was returned.
   *
   * The following parameters bring optionnal information about @p src and @p dest
   * @param sizeSrc size of source file
   * @param sizeDest size of destination file
   * @param ctimeSrc creation time of source file
   * @param ctimeDest creation time of destination file
   * @param mtimeSrc modification time of source file
   * @param mtimeDest modification time of destination file
   *
   */
RenameDlg_Result open_RenameDlg( const QString & caption,
                                 const QString& src, const QString & dest,
                                 RenameDlg_Mode mode, QString& newDestPath,
                                 KIO::filesize_t sizeSrc = (KIO::filesize_t) -1,
                                 KIO::filesize_t sizeDest = (KIO::filesize_t) -1,
                                 time_t ctimeSrc = (time_t) -1,
                                 time_t ctimeDest = (time_t) -1,
                                 time_t mtimeSrc = (time_t) -1,
                                 time_t mtimeDest = (time_t) -1
                                 );

/*! @} */

}
#endif
