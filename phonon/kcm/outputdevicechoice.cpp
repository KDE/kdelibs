/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "outputdevicechoice.h"

#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtGui/QHeaderView>
#include <QtGui/QPalette>

#include <phonon/backendcapabilities.h>
#include <phonon/objectdescription.h>
#include <phonon/phononnamespace.h>
#include "../libkaudiodevicelist/audiodeviceenumerator.h"
#include "../libkaudiodevicelist/audiodevice.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#ifndef METATYPE_QLIST_INT_DEFINED
#define METATYPE_QLIST_INT_DEFINED
// Want this exactly once, see phonondefs_p.h kcm/outputdevicechoice.cpp
Q_DECLARE_METATYPE(QList<int>)
#endif

class QSettingsGroup
{
    public:
        QSettingsGroup(QSettings *settings, const QString &name)
            : m_s(settings),
            m_group(name + QLatin1Char('/'))
        {
        }

        template<typename T>
        inline T value(const QString &key, const T &def) const
        {
            return qvariant_cast<T>(value(key, QVariant::fromValue(def)));
        }

        QVariant value(const QString &key, const QVariant &def) const
        {
            return m_s->value(m_group + key, def);
        }

        template<typename T>
        inline void setValue(const QString &key, const T &value)
        {
            m_s->setValue(m_group + key, QVariant::fromValue(value));
        }

    private:
        QSettings *const m_s;
        QString m_group;
};

class CategoryItem : public QStandardItem {
    public:
        CategoryItem(Phonon::Category cat)
            : QStandardItem(Phonon::categoryToString(cat)),
            m_cat(cat)
        {
        }

        int type() const { return 1001; }
        Phonon::Category category() const { return m_cat; }

    private:
        Phonon::Category m_cat;
};

OutputDeviceChoice::OutputDeviceChoice(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    removeButton->setIcon(KIcon("list-remove"));
    deferButton->setIcon(KIcon("arrow-down"));
    preferButton->setIcon(KIcon("arrow-up"));
    //categoryTree->setDragEnabled(true);
    //categoryTree->setAcceptDrops(true);
    //categoryTree->setDropIndicatorShown(true);
    //categoryTree->setDragDropMode(QAbstractItemView::InternalMove);
//X     deviceList->setDragEnabled(true);
//X     deviceList->setAcceptDrops(true);
//X     deviceList->setDropIndicatorShown(true);
    deviceList->setDragDropMode(QAbstractItemView::InternalMove);
    const QColor bgColor = deviceList->viewport()->palette().color(deviceList->viewport()->backgroundRole());
    const QString stylesheet = QString("QListView {"
            "background-color: %1;"
            "background-image: url(%2);"
            "background-position: bottom left;"
            "background-attachment: fixed;"
            "background-repeat: no-repeat;"
            "}")
        .arg(bgColor.name())
        .arg(KStandardDirs::locate("data", "kcm_phonon/listview-background.png"));
    kDebug() << stylesheet;
    deviceList->setStyleSheet(stylesheet);
    deviceList->setAlternatingRowColors(false);
    QStandardItem *parentItem = m_categoryModel.invisibleRootItem();
    QStandardItem *outputItem = new QStandardItem(i18n("Audio Output"));
    outputItem->setEditable(false);
    outputItem->setSelectable(false);
    parentItem->appendRow(outputItem);
    /*
    m_captureItem = new QStandardItem(i18n("Audio Capture"));
    m_captureItem->setEditable(false);
    parentItem->appendRow(m_captureItem);
    */

    parentItem = outputItem;
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        m_outputModel[i] = new Phonon::AudioOutputDeviceModel;
        QStandardItem *item = new CategoryItem(static_cast<Phonon::Category>(i));
        item->setEditable(false);
        parentItem->appendRow(item);
    }
    //parentItem = m_captureItem;

    categoryTree->setModel(&m_categoryModel);
    if (categoryTree->header()) {
        categoryTree->header()->hide();
    }
    categoryTree->expandAll();

    connect(categoryTree->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &,const QModelIndex &)),
            SLOT(updateDeviceList()));

    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        connect(m_outputModel[i], SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(changed()));
        connect(m_outputModel[i], SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(changed()));
        connect(m_outputModel[i], SIGNAL(layoutChanged()), this, SIGNAL(changed()));
        connect(m_outputModel[i], SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SIGNAL(changed()));
    }
    /*
    connect(&m_captureModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(changed()));
    connect(&m_captureModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(changed()));
    connect(&m_captureModel, SIGNAL(layoutChanged()), this, SIGNAL(changed()));
    connect(&m_captureModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SIGNAL(changed()));
    */

    connect(Phonon::BackendCapabilities::notifier(), SIGNAL(availableAudioOutputDevicesChanged()), SLOT(updateAudioOutputDevices()));

    if (!categoryTree->currentIndex().isValid()) {
        categoryTree->setCurrentIndex(m_categoryModel.index(0, 0).child(0, 0));
    }
}

void OutputDeviceChoice::updateDeviceList()
{
    QStandardItem *currentItem = m_categoryModel.itemFromIndex(categoryTree->currentIndex());
    if (deviceList->selectionModel()) {
        disconnect(deviceList->selectionModel(),
                SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),
                this, SLOT(updateButtonsEnabled()));
    }
    if (currentItem->type() == 1001) {
        CategoryItem *catItem = static_cast<CategoryItem *>(currentItem);
        deviceList->setModel(m_outputModel[catItem->category()]);
        /*
    } else if (currentItem == m_captureItem) {
        deviceList->setModel(&m_captureModel);
        */
    } else {
        deviceList->setModel(0);
    }
    updateButtonsEnabled();
    if (deviceList->selectionModel()) {
        connect(deviceList->selectionModel(),
                SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),
                this, SLOT(updateButtonsEnabled()));
    }
}

void OutputDeviceChoice::updateAudioOutputDevices()
{
    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    QHash<int, Phonon::AudioOutputDevice> hash;
    foreach (Phonon::AudioOutputDevice dev, list) {
        hash.insert(dev.index(), dev);
    }
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        Phonon::AudioOutputDeviceModel *model = m_outputModel.value(i);
        Q_ASSERT(model);

        QHash<int, Phonon::AudioOutputDevice> hashCopy(hash);
        QList<Phonon::AudioOutputDevice> orderedList;
        if (model->rowCount() > 0) {
            QList<int> order = model->tupleIndexOrder();
            foreach (int idx, order) {
                if (hashCopy.contains(idx)) {
                    orderedList << hashCopy.take(idx);
                }
            }
            if (hashCopy.size() > 1) {
                // keep the order of the original list
                foreach (const Phonon::AudioOutputDevice &dev, list) {
                    if (hashCopy.contains(dev.index())) {
                        orderedList << hashCopy.take(dev.index());
                    }
                }
            } else if (hashCopy.size() == 1) {
                orderedList += hashCopy.values();
            }
            model->setModelData(orderedList);
        } else {
            model->setModelData(list);
        }
    }
}

void OutputDeviceChoice::load()
{
    QSettings phononConfig(QLatin1String("kde.org"), QLatin1String("libphonon"));
    QSettingsGroup outputDeviceGroup(&phononConfig, QLatin1String("AudioOutputDevice"));
    QSettingsGroup captureDeviceGroup(&phononConfig, QLatin1String("AudioCaptureDevice"));

    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    QHash<int, Phonon::AudioOutputDevice> hash;
    foreach (Phonon::AudioOutputDevice dev, list) {
        hash.insert(dev.index(), dev);
    }
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        QHash<int, Phonon::AudioOutputDevice> hashCopy(hash);
        QList<int> order = outputDeviceGroup.value(QLatin1String("Category") + QString::number(i), QList<int>());
        QList<Phonon::AudioOutputDevice> orderedList;
        foreach (int idx, order) {
            if (hashCopy.contains(idx)) {
                orderedList << hashCopy.take(idx);
            }
        }
        if (hashCopy.size() > 1) {
            // keep the order of the original list
            foreach (const Phonon::AudioOutputDevice &dev, list) {
                if (hashCopy.contains(dev.index())) {
                    orderedList << hashCopy.take(dev.index());
                }
            }
        } else if (hashCopy.size() == 1) {
            orderedList += hashCopy.values();
        }
        m_outputModel[i]->setModelData(orderedList);
    }

    /*
    QList<Phonon::AudioCaptureDevice> list2 = Phonon::BackendCapabilities::availableAudioCaptureDevices();
    QList<int> order = captureDeviceGroup.value(QLatin1String("DeviceOrder"), QList<int>());
    QList<Phonon::AudioCaptureDevice> orderedList;
    foreach (int idx, order) {
        for (int i = 0; i < list2.size(); ++i) {
            if (list2.at(i).index() == idx) {
                orderedList << list2.takeAt(i);
                break; // out of the inner for loop to get the next idx
            }
        }
    }
    foreach (Phonon::AudioCaptureDevice dev, list2) {
        orderedList << dev;
    }
    m_captureModel.setModelData(orderedList);
    */
}

void OutputDeviceChoice::save()
{
    QSettings config(QLatin1String("kde.org"), QLatin1String("libphonon"));
    {
        QSettingsGroup globalGroup(&config, QLatin1String("AudioOutputDevice"));
        for (int i = 0; i <= Phonon::LastCategory; ++i) {
            if (m_outputModel.value(i)) {
                globalGroup.setValue(QLatin1String("Category") + QString::number(i), m_outputModel.value(i)->tupleIndexOrder());
            }
        }
    }
    /*
    {
        QSettingsGroup globalGroup(&config, QLatin1String("AudioCaptureDevice"));
        globalGroup.setValue(QLatin1String("DeviceOrder"), m_captureModel.tupleIndexOrder());
    }
    */
}

void OutputDeviceChoice::defaults()
{
    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        m_outputModel[i]->setModelData(list);
    }

    /*
    QList<Phonon::AudioCaptureDevice> list2 = Phonon::BackendCapabilities::availableAudioCaptureDevices();
    m_captureModel.setModelData(list2);
    */
}

void OutputDeviceChoice::on_preferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    Phonon::AudioOutputDeviceModel *deviceModel = qobject_cast<Phonon::AudioOutputDeviceModel *>(model);
    if (deviceModel) {
        deviceModel->moveUp(deviceList->currentIndex());
        updateButtonsEnabled();
        emit changed();
    }
}

void OutputDeviceChoice::on_deferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    Phonon::AudioOutputDeviceModel *deviceModel = qobject_cast<Phonon::AudioOutputDeviceModel *>(model);
    if (deviceModel) {
        deviceModel->moveDown(deviceList->currentIndex());
        updateButtonsEnabled();
        emit changed();
    }
}

void OutputDeviceChoice::on_removeButton_clicked()
{
    QModelIndex idx = deviceList->currentIndex();

    QAbstractItemModel *model = deviceList->model();
    Phonon::AudioOutputDeviceModel *playbackModel = qobject_cast<Phonon::AudioOutputDeviceModel *>(model);
    if (playbackModel && idx.isValid()) {
        Phonon::AudioOutputDevice deviceToRemove = playbackModel->modelData(idx);
        QList<Phonon::AudioDevice> deviceList = Phonon::AudioDeviceEnumerator::availablePlaybackDevices();
        foreach (Phonon::AudioDevice dev, deviceList) {
            if (dev.index() == deviceToRemove.index()) {
                // remove from persistent store
                if (dev.ceaseToExist()) {
                    // remove from all models, idx.row() is only correct for the current model
                    foreach (Phonon::AudioOutputDeviceModel *model, m_outputModel) {
                        QList<Phonon::AudioOutputDevice> data = model->modelData();
                        for (int row = 0; row < data.size(); ++row) {
                            if (data[row] == deviceToRemove) {
                                model->removeRows(row, 1);
                                break;
                            }
                        }
                    }
                    updateButtonsEnabled();
                    emit changed();
                }
            }
        }
        /*
    } else {
        Phonon::AudioCaptureDeviceModel *captureModel = qobject_cast<Phonon::AudioCaptureDeviceModel *>(model);
        if (captureModel && idx.isValid()) {
            Phonon::AudioCaptureDevice deviceToRemove = captureModel->modelData(idx);
            QList<Phonon::AudioDevice> deviceList = Phonon::AudioDeviceEnumerator::availableCaptureDevices();
            foreach (Phonon::AudioDevice dev, deviceList) {
                if (dev.index() == deviceToRemove.index()) {
                    // remove from persistent store
                    if (dev.ceaseToExist()) {
                        m_captureModel.removeRows(idx.row(), 1);
                        updateButtonsEnabled();
                        emit changed();
                    }
                }
            }
        }
        */
    }
}

void OutputDeviceChoice::updateButtonsEnabled()
{
    //kDebug() ;
    if (deviceList->model()) {
        //kDebug() << "model available";
        QModelIndex idx = deviceList->currentIndex();
        preferButton->setEnabled(idx.isValid() && idx.row() > 0);
        deferButton->setEnabled(idx.isValid() && idx.row() < deviceList->model()->rowCount() - 1);
        removeButton->setEnabled(idx.isValid() && !(idx.flags()  & Qt::ItemIsEnabled));
    } else {
        preferButton->setEnabled(false);
        deferButton->setEnabled(false);
        removeButton->setEnabled(false);
    }
}

#include "outputdevicechoice.moc"
// vim: sw=4 ts=4
