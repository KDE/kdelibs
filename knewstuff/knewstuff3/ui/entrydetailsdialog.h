/*
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_ENTRYDETAILS_H
#define KNEWSTUFF3_UI_ENTRYDETAILS_H

#include <QtCore/QObject>
#include <knewstuff3/core/entryinternal.h>

#include "ui_downloadwidget.h"

class QListWidgetItem;

namespace KNS3
{
    class Engine;

class EntryDetails :public QObject
{
    Q_OBJECT

public:
    EntryDetails(Engine* engine, Ui::DownloadWidget* widget);
    ~EntryDetails();
    
public Q_SLOTS:
    void setEntry(const KNS3::EntryInternal& entry);
    
private Q_SLOTS:
    void slotEntryPreviewLoaded(const KNS3::EntryInternal& entry, KNS3::EntryInternal::PreviewType type);
    void install();
    void slotInstallActionTriggered(QAction*);
    void uninstall();

    void voteGood();
    void voteBad();
    void becomeFan();
    // more details loaded
    void entryChanged(const KNS3::EntryInternal& entry);
    // installed/updateable etc
    void entryStatusChanged(const KNS3::EntryInternal& entry);
    void updateButtons();
    
    void preview1Selected();
    void preview2Selected();
    void preview3Selected();
    
private:
    void init();
    void previewSelected(int current);
    
    Engine *m_engine;
    Ui::DownloadWidget* ui;
    EntryInternal m_entry;
    QImage m_currentPreview;
    QListWidgetItem* m_previewItem1;
    QListWidgetItem* m_previewItem2;
    QListWidgetItem* m_previewItem3;
};

}

#endif
