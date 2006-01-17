/* This file is part of the KDE libraries
   Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KFILEAUDIOPREVIEW_H
#define KFILEAUDIOPREVIEW_H

#include <q3dict.h>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class QCheckBox;
class QPushButton;
class QLabel;

class KFileDialog;
class KFileItem;

/**
 * Audio "preview" widget for the file dialog.
 */
class KFileAudioPreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:
    KFileAudioPreview(QWidget *parent = 0, const char *name = 0 );
    ~KFileAudioPreview();

public Q_SLOTS:
    virtual void showPreview(const KUrl &url);
    virtual void clearPreview();

private Q_SLOTS:
    void toggleAuto(bool);

private:
    Q3Dict<void> m_supportedFormats;
    KUrl m_currentURL;
    QCheckBox *m_autoPlay;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileAudioPreviewPrivate;
    KFileAudioPreviewPrivate *d;
};

#endif // KFILEAUDIOPREVIEW_H
