/*
 * Copyright (C)  2001, 2006 Holger Freyther <freyther@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KIO_RENAMEDLGPLUGIN_H
#define KIO_RENAMEDLGPLUGIN_H

#include <kio/renamedlg.h>
#include <qdialog.h>
#include <sys/types.h>
#include <qstring.h>
#include <qstringlist.h>


namespace KIO {

/**
 * @short Base class for RenameDlg plugins.
 *
 * Creating your own RenameDlg Plugin allows you to
 * have a different representation of data depending
 * on the to be copied files.
 */
class KIO_EXPORT RenameDlgPlugin : public QWidget {
    Q_OBJECT
public:

    /**
     * File Representation consisting of the KUrl, MimeType and filesize and
     * the times of the last changes.
     */
    class FileItem {
    public:
        FileItem( const KUrl& url, const QString& mimeSrc, KIO::filesize_t, time_t ctime, time_t mtime);

        KUrl url() const;
        QString mimeType() const;
        KIO::filesize_t fileSize() const;
        time_t cTime() const;
        time_t mTime() const;

    private:
        KUrl m_url;
        QString m_mimeType;
        KIO::filesize_t m_fileSize;
        time_t m_ctime;
        time_t m_mtime;
    };

    /**
     * The Rename Dialog will be embedded into a QDialog.
     */
    RenameDlgPlugin(QDialog *dialog);

    /**
     * This function will be invoked by the KIO::RenameDlg to check if you to handle
     * the src and destination file.
     *
     * @param mode    The actual mode of the Rename Dialog
     * @param srcFile The FileItem representation of the source file
     * @param dstFile The FileItem representation of the destination file
     *
     * @return Return TRUE if you want to handle/display a resolution
     * @see handle
     */
    virtual bool wantToHandle( RenameDlg_Mode mode, const FileItem& srcFile, const FileItem& dstFile) const = 0;

    /**
     * Present the data of @param srcFile and @param dstFile the way you want to.
     * You will be embedded into the parentWidget().
     *
     * @param mode    The actual mode of the Rename Dialog
     * @param srcFile The FileItem of the source
     * @param dstFile The FileItem of the destination
     */
    virtual void handle( KIO::RenameDlg_Mode mode, const FileItem& srcFile, const FileItem& dstFile ) = 0;
};

}

#endif

