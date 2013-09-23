/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Jakub Stachowski <qbast@go2.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DNSSDSERVICEMODEL_H
#define DNSSDSERVICEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <dnssd/dnssd_export.h>
#include <dnssd/remoteservice.h>

namespace DNSSD
{

struct ServiceModelPrivate;
class ServiceBrowser;


/**
 * @class ServiceModel servicemodel.h DNSSD/ServiceModel
 * @short Model for list of Zeroconf services
 *
 * This class provides a Qt Model for ServiceBrowser to allow easy
 * integration of service discovery into a GUI.  For example, to
 * show the HTTP servers published on the local network, you can do:
 * @code
 * DNSSD::ServiceModel *serviceModel = new ServiceModel(
 *     new DNSSD::ServiceBrowser("_http._tcp")
 *     );
 * QComboBox *serviceCombo = new QComboBox();
 * serviceCombo->setModel(serviceModel);
 * @endcode
 *
 * After the user makes a selection, the application typically needs
 * to get a pointer to the selected service in order to get the host
 * name and port.  A RemoteService::Ptr can be obtained from
 * a QModelIndex using:
 * @code
 * void onSelected(const QModelIndex &selection) {
 *     DNSSD::RemoteService::Ptr service =
 *         selection.data(DNSSD::ServiceModel::ServicePtrRole)
 *                  .value<DNSSD::RemoteService::Ptr>();
 * }
 * @endcode
 *
 * @since 4.1
 * @author Jakub Stachowski
 */

class KDNSSD_EXPORT ServiceModel : public QAbstractItemModel
{
	Q_OBJECT

public:

	/** The additional data roles provided by this model */
	enum AdditionalRoles {
		ServicePtrRole = 0xA06519DE  ///< gets a RemoteService::Ptr for the service
	};

	/**
	 * The default columns for this model.
	 *
	 * If service browser is not set to resolve automatically,
	 * then the model will only ever have one column (the service name).
	 */
	enum ModelColumns {
		ServiceName = 0,
		Host = 1,
		Port = 2
	};

	/**
	 * Creates a model for the given service browser and starts browsing
	 * for services.
	 *
	 * The model takes ownership of the browser,
	 * so there is no need to delete it afterwards.
	 *
	 * You should @b not call ServiceBrowser::startBrowse() on @p browser
	 * before passing it to ServiceModel.
	 */
	explicit ServiceModel(ServiceBrowser* browser, QObject* parent = 0);

	virtual ~ServiceModel();

	/** @reimp */
	virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
	/** @reimp */
	virtual int rowCount(const QModelIndex& parent = QModelIndex() ) const;
	/** @reimp */
	virtual QModelIndex parent(const QModelIndex& index ) const;
	/** @reimp */
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
	/** @reimp */
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole ) const;
	/** @reimp */
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	/** @reimp */
	virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;

private:
	ServiceModelPrivate* const d;
	friend struct ServiceModelPrivate;

};

}

#endif
