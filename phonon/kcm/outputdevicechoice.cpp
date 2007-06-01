/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "outputdevicechoice.h"

#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtGui/QHeaderView>

#include <phonon/backendcapabilities.h>
#include <phonon/objectdescription.h>
#include <phonon/phononnamespace.h>
#include "../libkaudiodevicelist/audiodeviceenumerator.h"
#include "../libkaudiodevicelist/audiodevice.h"

#include <klocale.h>

Q_DECLARE_METATYPE(QList<int>)

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
    //categoryTree->setDragEnabled(true);
    //categoryTree->setAcceptDrops(true);
    //categoryTree->setDropIndicatorShown(true);
    //categoryTree->setDragDropMode(QAbstractItemView::InternalMove);
    deviceList->setDragEnabled(true);
    deviceList->setAcceptDrops(true);
    deviceList->setDropIndicatorShown(true);
    deviceList->setWrapping(true);
    //deviceList->setSpacing(4);
    deviceList->setDragDropMode(QAbstractItemView::InternalMove);
    /*qApp->setStyleSheet("QListView { background-image: url(/home/mkretz/KDE/src/kdelibs/phonon/kcm/speaker.svg);"
           "background-position: bottom left;"
          "background-repeat: no-repeat; }"); */
    deviceList->setAlternatingRowColors(true);
    QStandardItem *parentItem = m_categoryModel.invisibleRootItem();
    QStandardItem *outputItem = new QStandardItem(i18n("Audio Output"));
    outputItem->setEditable(false);
    outputItem->setSelectable(false);
    parentItem->appendRow(outputItem);
    m_captureItem = new QStandardItem(i18n("Audio Capture"));
    m_captureItem->setEditable(false);
    parentItem->appendRow(m_captureItem);

    parentItem = outputItem;
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        m_outputModel[i] = new Phonon::AudioOutputDeviceModel;
        QStandardItem *item = new CategoryItem(static_cast<Phonon::Category>(i));
        item->setEditable(false);
        parentItem->appendRow(item);
    }
    parentItem = m_captureItem;

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
    connect(&m_captureModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(changed()));
    connect(&m_captureModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(changed()));
    connect(&m_captureModel, SIGNAL(layoutChanged()), this, SIGNAL(changed()));
    connect(&m_captureModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SIGNAL(changed()));
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
    } else if (currentItem == m_captureItem) {
        deviceList->setModel(&m_captureModel);
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
            //} else {
                //orderedList << Phonon::AudioOutputDevice();
            //}
        }
        foreach (Phonon::AudioOutputDevice dev, hashCopy) {
            orderedList << dev;
        }
        m_outputModel[i]->setModelData(orderedList);
    }

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
}

void OutputDeviceChoice::save()
{
    kDebug() << k_funcinfo << endl;
    QSettings config(QLatin1String("kde.org"), QLatin1String("libphonon"));
    {
        QSettingsGroup globalGroup(&config, QLatin1String("AudioOutputDevice"));
        for (int i = 0; i <= Phonon::LastCategory; ++i) {
            if (m_outputModel.value(i)) {
                globalGroup.setValue(QLatin1String("Category") + QString::number(i), m_outputModel.value(i)->tupleIndexOrder());
            }
        }
    }
    {
        QSettingsGroup globalGroup(&config, QLatin1String("AudioCaptureDevice"));
        globalGroup.setValue(QLatin1String("DeviceOrder"), m_captureModel.tupleIndexOrder());
    }
}

void OutputDeviceChoice::defaults()
{
    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i <= Phonon::LastCategory; ++i) {
        m_outputModel[i]->setModelData(list);
    }

    QList<Phonon::AudioCaptureDevice> list2 = Phonon::BackendCapabilities::availableAudioCaptureDevices();
    m_captureModel.setModelData(list2);
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
    }
}

void OutputDeviceChoice::updateButtonsEnabled()
{
    //kDebug() << k_funcinfo << endl;
    if (deviceList->model()) {
        //kDebug() << "model available" << endl;
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
