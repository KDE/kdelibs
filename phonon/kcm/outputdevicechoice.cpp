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
    QList<Phonon::AudioOutputDevice> list = Phonon::BackendCapabilities::availableAudioOutputDevices();
    for (int i = 0; i < Phonon::LastCategory; ++i) {
        m_outputModel[i]->setModelData(list);
    }

    QList<Phonon::AudioCaptureDevice> list2 = Phonon::BackendCapabilities::availableAudioCaptureDevices();
    m_captureModel.setModelData(list2);
}

void OutputDeviceChoice::save()
{
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
    }
}

void OutputDeviceChoice::on_deferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    Phonon::AudioOutputDeviceModel *deviceModel = qobject_cast<Phonon::AudioOutputDeviceModel*>(model);
    if (deviceModel) {
        deviceModel->moveDown(deviceList->currentIndex());
        updateButtonsEnabled();
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
    }
}

#include "outputdevicechoice.moc"
// vim: sw=4 ts=4
