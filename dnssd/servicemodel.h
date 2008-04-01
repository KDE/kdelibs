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
\class ServiceModel servicemodel.h DNSSD/ServiceModel

ServiceModel implements Qt Model interface around ServiceBrowser to allow easy integration
of service discovery into GUI.
Example of combo box showing list of HTTP servers on local network:
\code
DNSSD::ServiceModel* m=new ServiceModel(new DNSSD::ServiceBrowser("_http._tcp"));
QComboBox *c=new QComboBox();
c->setModel(m);
\endcode

After user makes the selection, application typically needs pointer to selected service
in order to get host name and port. RemoteService::Ptr can be obtained from QModelIndex
using:
\code
void onSelected(const QModelIndex& selection) {
DNSSD::RemoteService::Ptr service=selection.data(DNSSD::ServiceModel::ServicePtrRole).
    value<DNSSD::RemoteService::Ptr>();
\endcode

\since 4.1
@short Model for list of Zeroconf services
@author Jakub Stachowski
*/

class KDNSSD_EXPORT ServiceModel : public QAbstractItemModel
{
Q_OBJECT

public:
	
	enum AdditionalRoles {
	    ServicePtrRole = 0xA06519DE  ///< returns pointer to service (RemoteService::Ptr type)
	};

	/**
	Default columns for this model. If service browser is not set to resolve automatically, then the model
	 has only one column (service name).
	 */	
	enum ModelColumns {
	    ServiceName = 0,
	    Host = 1,
	    Port = 2
	};
	
	/**
	Creates model for given service browses and starts browsing for services. The model becomes parent of the
	browser so there is no need to delete it afterwards.
	 */
	ServiceModel(ServiceBrowser* browser, QObject* parent=0);
	virtual ~ServiceModel();
	
	virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex() ) const;
	virtual QModelIndex parent(const QModelIndex& index ) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole ) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;


private:
	ServiceModelPrivate* const d;
	friend class ServiceModelPrivate;

};

}

#endif
