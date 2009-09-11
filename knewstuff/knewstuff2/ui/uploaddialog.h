/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2009 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KNEWSTUFF2_UI_UPLOADDIALOG_H
#define KNEWSTUFF2_UI_UPLOADDIALOG_H

#include <kdialog.h>
#include <kurl.h>
#include <knewstuff2/knewstuff_export.h>

#include "ui_UploadDialog.h"

class KComboBox;
class KLineEdit;
class KUrlRequester;
class KTextEdit;

namespace KNS
{

class Entry;

/**
 * @short KNewStuff file upload dialog.
 *
 * Using this dialog, data can easily be uploaded to the Hotstuff servers.
 * It should however not be used on its own, as an engine will invoke it
 * for the upload workflow.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT UploadDialog : public KDialog, public Ui::UploadDialog
{
    Q_OBJECT
public:
    /**
      Constructor.

      @param parent the parent window
    */
    UploadDialog(QWidget *parent);

    /**
      Destructor.
    */
    ~UploadDialog();

    /**
      Sets the preview filename.
      This is only meaningful if the application supports previews.

      @param previewFile the preview image file
    */
    void setPreviewFile(const KUrl& previewFile);

    /**
      Sets the payload filename.
      This is optional, but necessary if the application wants to support
      reusing previously filled out form data based on the filename.

      @param payloadFile the payload data file
    */
    void setPayloadFile(const KUrl& payloadFile);

    Entry *entry() const;

protected Q_SLOTS:
    void slotOk();

private:
    KUrl mPayloadUrl;

    Entry *m_entry;
    QMap<QString, QString> m_languages;
};

}

#endif
