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

#ifndef DNSSDDOMAINMODEL_H
#define DNSSDDOMAINMODEL_H

#include <QtCore/QAbstractItemModel>
#include <dnssd/dnssd_export.h>

namespace DNSSD
{

class DomainModelPrivate;
class DomainBrowser;


/**
\class DomainModel domainmodel.h DNSSD/DomainModel

DomainModel implements Qt Model inteface around DomainBrowser to allow easy integration
of domain discovery into GUI.
Example of combo box showing list of browsing domains:
\code
DNSSD::DomainModel* m=new DomainModel(new DNSSD::DomainBrowser(DNSSD::DomainBrowser::Browsing));
QComboBox *c=new QComboBox();
c->setModel(m);
\endcode

\since 4.1
@short Model for list of Zeroconf domains
@author Jakub Stachowski
*/

class KDNSSD_EXPORT DomainModel : public QAbstractItemModel
{
Q_OBJECT

public:
	/**
	Creates model for given domain browses and starts browsing for domain. The model becomes parent of the
	browser so there is no need to delete it afterwards.
 	 */ 
	DomainModel(DomainBrowser* browser, QObject* parent=0);
	virtual ~DomainModel();
	
	virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex() ) const;
	virtual QModelIndex parent(const QModelIndex& index ) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole ) const;
	virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;


private:
	DomainModelPrivate* const d;
	friend class DomainModelPrivate;

};

}

#endif
