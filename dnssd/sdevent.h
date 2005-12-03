/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Jakub Stachowski <qbast@go2.pl>
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

#ifndef DNSSDSDEVENT_H
#define DNSSDSDEVENT_H

#include <qevent.h>
#include <qstring.h>
#include <qmap.h>

namespace DNSSD
{

enum Operation { SD_ERROR = 101,SD_ADDREMOVE, SD_PUBLISH, SD_RESOLVE};

class ErrorEvent : public QCustomEvent
{
public:
	ErrorEvent() : QCustomEvent(QEvent::User+SD_ERROR) 
	{}
};
class AddRemoveEvent : public QCustomEvent
{
public:
	enum Operation { Add, Remove };
	AddRemoveEvent(Operation op,const QString& name,const QString& type,
		const QString& domain, bool last) : QCustomEvent(QEvent::User+SD_ADDREMOVE),
	m_op(op), m_name(name), m_type(type), m_domain(domain), m_last(last) 
	{}

	const Operation m_op;
	const QString m_name;
	const QString m_type;
	const QString m_domain;
	const bool m_last;
};

class PublishEvent : public QCustomEvent
{
public:
	PublishEvent(const QString& name) : QCustomEvent(QEvent::User+SD_PUBLISH), m_name(name)
	{}

	const QString m_name;
};

class ResolveEvent : public QCustomEvent
{
public:
	ResolveEvent(const QString& hostname, unsigned short port,
		     const QMap<QString,QString>& txtdata) 
		: QCustomEvent(QEvent::User+SD_RESOLVE), m_hostname(hostname),
		  m_port(port), m_txtdata(txtdata)
	{}

	const QString m_hostname;
	const unsigned short m_port;
	const QMap<QString,QString> m_txtdata;
};


}

#endif
