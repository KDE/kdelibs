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
#include <phonon/backendcapabilities.h>
#include <phonon/objectdescription.h>
#include <phonon/phononnamespace.h>
#include <klocale.h>
#include <QHeaderView>
#include <ksimpleconfig.h>
#include <phonon/alsadeviceenumerator.h>
#include <phonon/alsadevice.h>
#include <QList>

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
    removeButton->setIcon(KIcon("remove"));
    QStandardItem *parentItem = m_categoryModel.invisibleRootItem();
    QStandardItem *outputItem = new QStandardItem(i18n("Audio Output"));
    outputItem->setSelectable(false);
    parentItem->appendRow(outputItem);
    QStandardItem *captureItem = new QStandardItem(i18n("Audio Capture"));
    parentItem->appendRow(captureItem);

    parentItem = outputItem;
    for (int i = 0; i < Phonon::LastCategory; ++i) {
        m_outputModel[i] = new Phonon::AudioOutputDeviceModel;
        QStandardItem *item = new CategoryItem(static_cast<Phonon::Category>(i));
        parentItem->appendRow(item);
    }
    parentItem = captureItem;

    categoryTree->setModel(&m_categoryModel);
    if (categoryTree->header()) {
        categoryTree->header()->hide();
    }
    categoryTree->expandAll();

    connect(categoryTree->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),
            SLOT(updateDeviceList()));
}

void OutputDeviceChoice::updateDeviceList()
{
    kDebug() << k_funcinfo << endl;
    QStandardItem *currentItem = m_categoryModel.itemFromIndex(categoryTree->currentIndex());
    if (currentItem->type() == 1001) {
        if (deviceList->selectionModel()) {
            disconnect(deviceList->selectionModel(),
                    SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),
                    this, SLOT(updateButtonsEnabled()));
        }
        CategoryItem *catItem = static_cast<CategoryItem*>(currentItem);
        deviceList->setModel(m_outputModel[catItem->category()]);
        connect(deviceList->selectionModel(),
                SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),
                this, SLOT(updateButtonsEnabled()));
    } else {
        if (deviceList->selectionModel()) {
            disconnect(deviceList->selectionModel(),
                    SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),
                    this, SLOT(updateButtonsEnabled()));
        }
        deviceList->setModel(&m_captureModel);
        connect(deviceList->selectionModel(),
                SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),
                this, SLOT(updateButtonsEnabled()));
    }
}

void OutputDeviceChoice::load()
{
    KSimpleConfig phononrc("phononrc", true);
    KConfigGroup outputDeviceGroup(&phononrc, QLatin1String("AudioOutputDevice"));
    KConfigGroup captureDeviceGroup(&phononrc, QLatin1String("AudioCaptureDevice"));

    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    QHash<int, Phonon::AudioOutputDevice> hash;
    foreach (Phonon::AudioOutputDevice dev, list) {
        hash.insert(dev.index(), dev);
    }
    for (int i = 0; i < Phonon::LastCategory; ++i) {
        QHash<int, Phonon::AudioOutputDevice> hashCopy(hash);
        QList<int> order = outputDeviceGroup.readEntry<QList<int> >(QLatin1String("Category") + QString::number(i), QList<int>());
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
    QList<int> order = captureDeviceGroup.readEntry<QList<int> >(QLatin1String("DeviceOrder"), QList<int>());
    QList<Phonon::AudioCaptureDevice> orderedList;
    foreach (int idx, order) {
        for (int i = 0; i < list2.size(); ++i) {
            if (list2.at(i).index() == idx) {
                orderedList << list2.takeAt(i);
                break; // out of the inner for loop to get the next idx
            }
        }
    }
    m_captureModel.setModelData(orderedList);
}

void OutputDeviceChoice::save()
{
    kDebug() << k_funcinfo << endl;
    KSimpleConfig config("phononrc");
    {
        KConfigGroup globalGroup(&config, QLatin1String("AudioOutputDevice"));
        for (int i = 0; i < Phonon::LastCategory; ++i) {
            if (m_outputModel.value(i)) {
                globalGroup.writeEntry(QLatin1String("Category") + QString::number(i), m_outputModel.value(i)->tupleIndexOrder());
            }
        }
    }
    {
        KConfigGroup globalGroup(&config, QLatin1String("AudioCaptureDevice"));
        globalGroup.writeEntry(QLatin1String("DeviceOrder"), m_captureModel.tupleIndexOrder());
    }
}

void OutputDeviceChoice::defaults()
{
    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i < Phonon::LastCategory; ++i) {
        m_outputModel[i]->setModelData(list);
    }

    QList<Phonon::AudioCaptureDevice> list2 = Phonon::BackendCapabilities::availableAudioCaptureDevices();
    m_captureModel.setModelData(list2);
}

void OutputDeviceChoice::on_preferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    Phonon::AudioOutputDeviceModel *deviceModel = qobject_cast<Phonon::AudioOutputDeviceModel*>(model);
    if (deviceModel) {
        deviceModel->moveUp(deviceList->currentIndex());
        updateButtonsEnabled();
        emit changed();
    }
}

void OutputDeviceChoice::on_deferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    Phonon::AudioOutputDeviceModel *deviceModel = qobject_cast<Phonon::AudioOutputDeviceModel*>(model);
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
    Phonon::AudioOutputDeviceModel *playbackModel = qobject_cast<Phonon::AudioOutputDeviceModel*>(model);
    if (playbackModel && idx.isValid()) {
        Phonon::AudioOutputDevice deviceToRemove = playbackModel->modelData(idx);
        QList<Phonon::AlsaDevice> deviceList = Phonon::AlsaDeviceEnumerator::availablePlaybackDevices();
        foreach (Phonon::AlsaDevice dev, deviceList) {
            if (dev.index() == deviceToRemove.index()) {
                // remove from persistent store
                if (dev.ceaseToExist()) {
                    // remove from all models
                    foreach (Phonon::AudioOutputDeviceModel *model, m_outputModel) {
                        QList<Phonon::AudioOutputDevice> data = model->modelData();
                        data.removeAll(deviceToRemove);
                        model->setModelData(data);
                    }
                    updateButtonsEnabled();
                    emit changed();
                }
            }
        }
    } else {
        Phonon::AudioCaptureDeviceModel *captureModel = qobject_cast<Phonon::AudioCaptureDeviceModel*>(model);
        if (captureModel && idx.isValid()) {
            Phonon::AudioCaptureDevice deviceToRemove = captureModel->modelData(idx);
            QList<Phonon::AlsaDevice> deviceList = Phonon::AlsaDeviceEnumerator::availableCaptureDevices();
            foreach (Phonon::AlsaDevice dev, deviceList) {
                if (dev.index() == deviceToRemove.index()) {
                    // remove from persistent store
                    if (dev.ceaseToExist()) {
                        QList<Phonon::AudioCaptureDevice> data = m_captureModel.modelData();
                        data.removeAll(deviceToRemove);
                        m_captureModel.setModelData(data);
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
    kDebug() << k_funcinfo << endl;
    if (deviceList->model()) {
        kDebug() << "model available" << endl;
        QModelIndex idx = deviceList->currentIndex();
        preferButton->setEnabled(idx.isValid() && idx.row() > 0);
        deferButton->setEnabled(idx.isValid() && idx.row() < deviceList->model()->rowCount() - 1);
        removeButton->setEnabled(idx.isValid() && !(idx.flags() & Qt::ItemIsEnabled));
    } else {
        preferButton->setEnabled(false);
        deferButton->setEnabled(false);
        removeButton->setEnabled(false);
    }
}

#include "outputdevicechoice.moc"
// vim: sw=4 ts=4
