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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __kio_rename_dialog__
#define __kio_rename_dialog__

#include <kurl.h>
#include <QtGui/QDialog>
#include <QtCore/QCharRef>
#include <sys/types.h>

#include <kio/global.h>

namespace KIO {

// KDE4: get rid of M_OVERWRITE_ITSELF, trigger it internally if src==dest
enum RenameDialog_Mode { M_OVERWRITE = 1, M_OVERWRITE_ITSELF = 2, M_SKIP = 4, M_SINGLE = 8, M_MULTI = 16, M_RESUME = 32, M_NORENAME = 64 };

/**
 * The result of open_RenameDialog().
 */
enum RenameDialog_Result { R_RESUME = 6, R_RESUME_ALL = 7, R_OVERWRITE = 4, R_OVERWRITE_ALL = 5, R_SKIP = 2, R_AUTO_SKIP = 3, R_RENAME = 1, R_CANCEL = 0 };


/**
 * A dialog for the options to rename two files.
 * @short A dialog for renaming files.
 */
class KIO_EXPORT RenameDialog : public QDialog
{
  Q_OBJECT
public:
  /**
   * Construct a "rename" dialog.
   * @param parent parent widget (often 0)
   * @param caption the caption for the dialog box
   * @param src the url to the file/dir we're trying to copy, as it's part of the text message
   * @param dest the path to destination file/dir, i.e. the one that already exists
   * @param mode parameters for the dialog (which buttons to show...),
   * @param sizeSrc size of source file
   * @param sizeDest size of destination file
   * @param ctimeSrc creation time of source file
   * @param ctimeDest creation time of destination file
   * @param mtimeSrc modification time of source file
   * @param mtimeDest modification time of destination file
   * @see RenameDialog_Mode
   */
  RenameDialog( QWidget *parent, const QString & caption,
             const KUrl & src, const KUrl & dest,
             RenameDialog_Mode mode,
             KIO::filesize_t sizeSrc = (KIO::filesize_t) -1,
             KIO::filesize_t sizeDest = (KIO::filesize_t) -1,
             time_t ctimeSrc = (time_t) -1,
             time_t ctimeDest = (time_t) -1,
             time_t mtimeSrc = (time_t) -1,
             time_t mtimeDest = (time_t) -1 );
  ~RenameDialog();

  /**
   * @return the new destination
   * valid only if RENAME was chosen
   */
  KUrl newDestUrl();

  /**
   * Given a directory path and a filename (which usually exists already),
   * this function returns a suggested name for a file that doesn't exist
   * in that directory. The existence is only checked for local urls though.
   * The suggested file name is of the form foo_1 foo_2 etc.
   */
  static QString suggestName(const KUrl& baseURL, const QString& oldName);

public Q_SLOTS:
  void cancelPressed();
  void renamePressed();
  void skipPressed();
  void autoSkipPressed();
  void overwritePressed();
  void overwriteAllPressed();
  void resumePressed();
  void resumeAllPressed();
  void suggestNewNamePressed();

protected Q_SLOTS:
  void enableRenameButton(const QString &);
private:
 class RenameDialogPrivate;
 RenameDialogPrivate* const d;
 void pluginHandling( );
};

  /**
   * \relates KIO::RenameDialog
   * Construct a modal, parent-less "rename" dialog, and return
   * a result code, as well as the new dest. Much easier to use than the
   * class RenameDialog directly.

   * @param caption the caption for the dialog box
   * @param src the URL of the file/dir we're trying to copy, as it's part of the text message
   * @param dest the URL of the destination file/dir, i.e. the one that already exists
   * @param mode parameters for the dialog (which buttons to show...),
   *             see RenameDialog_Mode
   * @param newDestPath the new destination path, valid if R_RENAME was returned.
   * @param sizeSrc size of source file
   * @param sizeDest size of destination file
   * @param ctimeSrc creation time of source file
   * @param ctimeDest creation time of destination file
   * @param mtimeSrc modification time of source file
   * @param mtimeDest modification time of destination file
   * @return the result
   */
KIO_EXPORT RenameDialog_Result open_RenameDialog( const QString & caption,
                                 const KUrl& src, const KUrl & dest,
                                 RenameDialog_Mode mode, QString& newDestPath,
                                 KIO::filesize_t sizeSrc = (KIO::filesize_t) -1,
                                 KIO::filesize_t sizeDest = (KIO::filesize_t) -1,
                                 time_t ctimeSrc = (time_t) -1,
                                 time_t ctimeDest = (time_t) -1,
                                 time_t mtimeSrc = (time_t) -1,
                                 time_t mtimeDest = (time_t) -1
                                 );

}
#endif
