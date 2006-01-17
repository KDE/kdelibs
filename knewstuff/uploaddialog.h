/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KNEWSTUFF_UPLOADDIALOG_H
#define KNEWSTUFF_UPLOADDIALOG_H

#include <kdialogbase.h>

class QLineEdit;
class QSpinBox;
class KURLRequester;
class QTextEdit;
class QComboBox;

namespace KNS {

class Engine;
class Entry;

/**
 * @short KNewStuff file upload dialog.
 *
 * Using this dialog, data can easily be uploaded to the Hotstuff servers.
 * It should however not be used on its own, instead a KNewStuff (or
 * KNewStuffGeneric) object invokes it.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class UploadDialog : public KDialogBase
{
    Q_OBJECT
  public:
    /**
      Constructor.

      @param engine a KNewStuff engine object to be used for uploads
      @param parent the parent window
    */
    UploadDialog( Engine *engine, QWidget *parent );

    /**
      Destructor.
    */
    ~UploadDialog();

    /**
      Sets the preview filename.
      This is only meaningful if the application supports previews.

      @param previewFile the preview image file
    */
    void setPreviewFile( const QString &previewFile );

    /**
      Sets the payload filename.
      This is optional, but necessary if the application wants to support
      reusing previously filled out form data based on the filename.

      @param payloadFile the payload data file
    */
    void setPayloadFile( const QString &payloadFile );

  protected Q_SLOTS:
    void slotOk();

  private:
    Engine *mEngine;

    QLineEdit *mNameEdit;
    QLineEdit *mAuthorEdit;
    QLineEdit *mVersionEdit;
    QSpinBox *mReleaseSpin;
    KURLRequester *mPreviewUrl;
    QTextEdit *mSummaryEdit;
    QComboBox *mLanguageCombo;
    QComboBox *mLicenceCombo;

    QList<Entry*> mEntryList;
    KURL mPayloadUrl;
};

}

#endif
