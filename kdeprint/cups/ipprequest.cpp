/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "ipprequest.h"
#include "cupsinfos.h"

#include <stdlib.h>
#include <cups/language.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <cups/cups.h>

void dumpRequest(ipp_t *req)
{
	kdDebug() << "------------------------------" << endl;
	kdDebug() << "state = 0x" << req->state << endl;
	kdDebug() << "current tag = 0x" << req->curtag << endl;
	kdDebug() << "request operation ID = 0x" << req->request.op.operation_id << endl;
	kdDebug() << "request ID = 0x" << req->request.op.request_id << endl;
	kdDebug() << "request version = " << req->request.op.version[0] << req->request.op.version[1] << endl;
	ipp_attribute_t	*attr = req->attrs;
	while (attr)
	{
		kdDebug() << " " << endl;
		kdDebug() << "attribute: " << attr->name << endl;
		kdDebug() << "group tag = 0x" << attr->group_tag << endl;
		kdDebug() << "value tag = 0x" << attr->value_tag << endl;
		kdDebug() << "number of values = " << attr->num_values << endl;
		if (attr->value_tag >= IPP_TAG_TEXT)
			for (int i=0;i<attr->num_values;i++)
				kdDebug() << "value[" << i << "] = " << attr->values[i].string.text << endl;
		attr = attr->next;
	}
	kdDebug() << "------------------------------" << endl;
}

//*************************************************************************************

IppRequest::IppRequest()
{
	request_ = 0;
	port_ = -1;
	host_ = QString::null;
	init();
}

IppRequest::~IppRequest()
{
	ippDelete(request_);
}

void IppRequest::init()
{
	if (request_)
	{
		ippDelete(request_);
		request_ = 0;
	}
	request_ = ippNew();
	//kdDebug() << "kdeprint: IPP request, lang=" << KGlobal::locale()->language() << endl;
	cups_lang_t*	lang = cupsLangGet(KGlobal::locale()->language().latin1());
	// default charset to UTF-8 (ugly hack)
	lang->encoding = CUPS_UTF8;
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_CHARSET, "attributes-charset", NULL, cupsLangEncoding(lang));
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE, "attributes-natural-language", NULL, lang->language);
}

void IppRequest::addString_p(int group, int type, const QString& name, const QString& value)
{
	if (!name.isEmpty())
		ippAddString(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),NULL,(value.isEmpty() ? "" : value.local8Bit().data()));
}

void IppRequest::addStringList_p(int group, int type, const QString& name, const QStringList& values)
{
	if (!name.isEmpty())
	{
		ipp_attribute_t	*attr = ippAddStrings(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),(int)(values.count()),NULL,NULL);
		int	i(0);
		for (QStringList::ConstIterator it=values.begin(); it != values.end(); ++it, i++)
			attr->values[i].string.text = strdup((*it).local8Bit());
	}
}

void IppRequest::addInteger_p(int group, int type, const QString& name, int value)
{
	if (!name.isEmpty()) ippAddInteger(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),value);
}

void IppRequest::addIntegerList_p(int group, int type, const QString& name, const QValueList<int>& values)
{
	if (!name.isEmpty())
	{
		ipp_attribute_t	*attr = ippAddIntegers(request_,(ipp_tag_t)group,(ipp_tag_t)type,name.latin1(),(int)(values.count()),NULL);
		int	i(0);
		for (QValueList<int>::ConstIterator it=values.begin(); it != values.end(); ++it, i++)
			attr->values[i].integer = *it;
	}
}

void IppRequest::addBoolean(int group, const QString& name, bool value)
{
	if (!name.isEmpty()) ippAddBoolean(request_,(ipp_tag_t)group,name.latin1(),(char)value);
}

void IppRequest::addBoolean(int group, const QString& name, const QValueList<bool>& values)
{
	if (!name.isEmpty())
	{
		ipp_attribute_t	*attr = ippAddBooleans(request_,(ipp_tag_t)group,name.latin1(),(int)(values.count()),NULL);
		int	i(0);
		for (QValueList<bool>::ConstIterator it=values.begin(); it != values.end(); ++it, i++)
			attr->values[i].boolean = (char)(*it);
	}
}

void IppRequest::setOperation(int op)
{
	request_->request.op.operation_id = (ipp_op_t)op;
	request_->request.op.request_id = 1;	// 0 is not RFC-compliant, should be at least 1
}

int IppRequest::status()
{
	return (request_ ? request_->request.status.status_code : IPP_OK);
}

bool IppRequest::integerValue_p(const QString& name, int& value, int type)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), (ipp_tag_t)type);
	if (attr)
	{
		value = attr->values[0].integer;
		return true;
	}
	else return false;
}

bool IppRequest::stringValue_p(const QString& name, QString& value, int type)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), (ipp_tag_t)type);
	if (attr)
	{
		value = QString::fromLocal8Bit(attr->values[0].string.text);
		return true;
	}
	else return false;
}

bool IppRequest::stringListValue_p(const QString& name, QStringList& values, int type)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), (ipp_tag_t)type);
	values.clear();
	if (attr)
	{
		for (int i=0;i<attr->num_values;i++)
			values.append(QString::fromLocal8Bit(attr->values[i].string.text));
		return true;
	}
	else return false;
}

bool IppRequest::boolean(const QString& name, bool& value)
{
	if (!request_ || name.isEmpty()) return false;
	ipp_attribute_t	*attr = ippFindAttribute(request_, name.latin1(), IPP_TAG_BOOLEAN);
	if (attr)
	{
		value = (bool)attr->values[0].boolean;
		return true;
	}
	else return false;
}

bool IppRequest::doFileRequest(const QString& res, const QString& filename)
{
	QString	myHost = host_;
	int 	myPort = port_;
	if (myHost.isEmpty()) myHost = CupsInfos::self()->host();
	if (myPort <= 0) myPort = CupsInfos::self()->port();
	http_t	*HTTP = httpConnect(myHost.latin1(),myPort);

	if (HTTP == NULL)
		return false;
#if 0
kdDebug() << "---------------------\nRequest\n---------------------" << endl;
dumpRequest(request_);
#endif
	request_ = cupsDoFileRequest(HTTP, request_, (res.isEmpty() ? "/" : res.latin1()), (filename.isEmpty() ? NULL : filename.latin1()));
	httpClose(HTTP);
	if (!request_ || request_->state == IPP_ERROR || (request_->request.status.status_code & 0x0F00))
		return false;
#if 0
kdDebug() << "---------------------\nAnswer\n---------------------" << endl;
dumpRequest(request_);
#endif
	return true;
}

bool IppRequest::htmlReport(int group, QTextStream& output)
{
	if (!request_) return false;
	// start table
	output << "<table border=\"1\">" << endl;
	output << "<tr><th bgcolor=\"dark blue\"><font color=\"white\">" << i18n("Attribute") << "</font></th>" << endl;
	output << "<th bgcolor=\"dark blue\"><font color=\"white\">" << i18n("Values") << "</font></th></tr>" << endl;
	// go to the first attribute of the specified group
	ipp_attribute_t	*attr = request_->attrs;
	while (attr && attr->group_tag != group)
		attr = attr->next;
	// print each attribute
	ipp_uchar_t	*d;
	QCString	dateStr;
	while (attr && attr->group_tag == group)
	{
		output << "  <tr>\n    <td><b>" << attr->name << "</b></td>\n    <td>" << endl;
		for (int i=0; i<attr->num_values; i++)
		{
			switch (attr->value_tag)
			{
				case IPP_TAG_INTEGER:
					output << attr->values[i].integer;
					break;
				case IPP_TAG_ENUM:
					output << "0x" << hex << attr->values[i].integer << dec;
					break;
				case IPP_TAG_BOOLEAN:
					output << (attr->values[i].boolean ? i18n("True") : i18n("False"));
					break;
				case IPP_TAG_STRING:
				case IPP_TAG_TEXTLANG:
				case IPP_TAG_NAMELANG:
				case IPP_TAG_TEXT:
				case IPP_TAG_NAME:
				case IPP_TAG_KEYWORD:
				case IPP_TAG_URI:
				case IPP_TAG_CHARSET:
				case IPP_TAG_LANGUAGE:
				case IPP_TAG_MIMETYPE:
					output << attr->values[i].string.text;
					break;
				case IPP_TAG_RESOLUTION:
					output << "( " << attr->values[i].resolution.xres
					       << ", " << attr->values[i].resolution.yres << " )";
					break;
				case IPP_TAG_RANGE:
					output << "[ " << attr->values[i].range.lower
					       << ", " << attr->values[i].range.upper << " ]";
					break;
				case IPP_TAG_DATE:
					d = attr->values[i].date;
					dateStr.sprintf("%.4d-%.2d-%.2d, %.2d:%.2d:%.2d %c%.2d%.2d",
							d[0]*256+d[1], d[2], d[3],
							d[4], d[5], d[6],
							d[8], d[9], d[10]);
					output << dateStr;
					break;
				default:
					continue;
			}
			if (i < attr->num_values-1)
				output << "<br>";
		}
		output << "</td>\n  </tr>" << endl;
		attr = attr->next;
	}
	// end table
	output << "</table>" << endl;

	return true;
}
