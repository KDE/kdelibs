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
#include <qdatetime.h>
#include <qregexp.h>
#include <cups/cups.h>

void dumpRequest(ipp_t *req, bool answer = false, const QString& s = QString::null)
{
	kdDebug(500) << "==========" << endl;
	if (s.isEmpty())
		kdDebug(500) << (answer ? "Answer" : "Request") << endl;
	else
		kdDebug(500) << s << endl;
	kdDebug(500) << "==========" << endl;
	if (!req)
	{
		kdDebug(500) << "Null request" << endl;
		return;
	}
	kdDebug(500) << "State = 0x" << QString::number(req->state, 16) << endl;
	kdDebug(500) << "ID = 0x" << QString::number(req->request.status.request_id, 16) << endl;
	if (answer)
	{
		kdDebug(500) << "Status = 0x" << QString::number(req->request.status.status_code, 16) << endl;
		kdDebug(500) << "Status message = " << ippErrorString(req->request.status.status_code) << endl;
	}
	else
		kdDebug(500) << "Operation = 0x" << QString::number(req->request.op.operation_id, 16) << endl;
	kdDebug(500) << "Version = " << (int)(req->request.status.version[0]) << "." << (int)(req->request.status.version[1]) << endl;
	kdDebug(500) << endl;

	ipp_attribute_t *attr = req->attrs;
	while (attr)
	{
		QString s = QString::fromLatin1("%1 (0x%2) = ").arg(attr->name).arg(attr->value_tag, 0, 16);
		for (int i=0;i<attr->num_values;i++)
		{
			switch (attr->value_tag)
			{
				case IPP_TAG_INTEGER:
				case IPP_TAG_ENUM:
					s += ("0x"+QString::number(attr->values[i].integer, 16));
					break;
				case IPP_TAG_BOOLEAN:
					s += (attr->values[i].boolean ? "true" : "false");
					break;
				case IPP_TAG_STRING:
				case IPP_TAG_TEXT:
				case IPP_TAG_NAME:
				case IPP_TAG_KEYWORD:
				case IPP_TAG_URI:
				case IPP_TAG_MIMETYPE:
				case IPP_TAG_NAMELANG:
				case IPP_TAG_TEXTLANG:
				case IPP_TAG_CHARSET:
				case IPP_TAG_LANGUAGE:
					s += attr->values[i].string.text;
					break;
				default:
					break;
			}
			if (i != (attr->num_values-1))
				s += ", ";
		}
		kdDebug(500) << s << endl;
		attr = attr->next;
	}
}

QString errorString(int status)
{
	// FIXME: in the end, should use translation => 3.1
	QString	str;
	switch (status)
	{
		case IPP_FORBIDDEN:
			str = "You don't have access to the requested resource.";
			break;
		case IPP_NOT_AUTHORIZED:
			str = "You are not authorized to access the requested resource.";
			break;
		case IPP_NOT_POSSIBLE:
			str = "The requested operation cannot be completed.";
			break;
		case IPP_SERVICE_UNAVAILABLE:
			str = "The requested service is currently unavailable.";
			break;
		case IPP_NOT_ACCEPTING:
			str = "The target printer is not accepting print jobs.";
			break;
		default:
			str = QString::fromLocal8Bit(ippErrorString((ipp_status_t)status));
			break;
	}
	return str;
}

//*************************************************************************************

IppRequest::IppRequest()
{
	request_ = 0;
	port_ = -1;
	host_ = QString::null;
	dump_ = 0;
	init();
}

IppRequest::~IppRequest()
{
	ippDelete(request_);
}

void IppRequest::init()
{
	connect_ = true;

	if (request_)
	{
		ippDelete(request_);
		request_ = 0;
	}
	request_ = ippNew();
	kdDebug(500) << "kdeprint: IPP request, lang=" << KGlobal::locale()->language() << endl;
        QCString langstr = KGlobal::locale()->language().latin1();
	cups_lang_t*	lang = cupsLangGet(langstr.data());
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
	return (request_ ? request_->request.status.status_code : (connect_ ? -1 : -2));
}

QString IppRequest::statusMessage()
{
	QString	msg;
	switch (status())
	{
		case -2:
			msg = i18n("Connection to CUPS server failed. Check that the CUPS server is correctly installed and running.");
			break;
		case -1:
			msg = i18n("The IPP request failed for an unknown reason.");
			break;
		default:
			msg = errorString(status());
			break;
	}
	return msg;
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

	connect_ = (HTTP != NULL);

	if (HTTP == NULL)
	{
		ippDelete(request_);
		request_ = 0;
		return false;
	}

	if (dump_ > 0)
	{
		dumpRequest(request_, false, "Request to "+myHost+":"+QString::number(myPort));
	}

	request_ = cupsDoFileRequest(HTTP, request_, (res.isEmpty() ? "/" : res.latin1()), (filename.isEmpty() ? NULL : filename.latin1()));
	httpClose(HTTP);

	if (dump_ > 1)
	{
		dumpRequest(request_, true);
	}

	if (!request_ || request_->state == IPP_ERROR || (request_->request.status.status_code & 0x0F00))
		return false;


	return true;
}

bool IppRequest::htmlReport(int group, QTextStream& output)
{
	if (!request_) return false;
	// start table
	output << "<table border=\"1\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
	output << "<tr><th bgcolor=\"dark blue\"><font color=\"white\">" << i18n("Attribute") << "</font></th>" << endl;
	output << "<th bgcolor=\"dark blue\"><font color=\"white\">" << i18n("Values") << "</font></th></tr>" << endl;
	// go to the first attribute of the specified group
	ipp_attribute_t	*attr = request_->attrs;
	while (attr && attr->group_tag != group)
		attr = attr->next;
	// print each attribute
	ipp_uchar_t	*d;
	QCString	dateStr;
	QDateTime	dt;
	bool	bg(false);
	while (attr && attr->group_tag == group)
	{
		output << "  <tr bgcolor=\"" << (bg ? "#ffffd9" : "#ffffff") << "\">\n    <td><b>" << attr->name << "</b></td>\n    <td>" << endl;
		bg = !bg;
		for (int i=0; i<attr->num_values; i++)
		{
			switch (attr->value_tag)
			{
				case IPP_TAG_INTEGER:
					if (attr->name && strstr(attr->name, "time"))
					{
						dt.setTime_t((unsigned int)(attr->values[i].integer));
						output << dt.toString();
					}
					else
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
					output << "[ " << (attr->values[i].range.lower > 0 ? attr->values[i].range.lower : 1)
					       << ", " << (attr->values[i].range.upper > 0 ? attr->values[i].range.upper : 65535) << " ]";
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

QMap<QString,QString> IppRequest::toMap(int group)
{
	QMap<QString,QString>	opts;
	if (request_)
	{
		ipp_attribute_t	*attr = first();
		while (attr)
		{
			if (group != -1 && attr->group_tag != group)
			{
				attr = attr->next;
				continue;
			}
			QString	value;
			for (int i=0; i<attr->num_values; i++)
			{
				switch (attr->value_tag)
				{
					case IPP_TAG_INTEGER:
					case IPP_TAG_ENUM:
						value.append(QString::number(attr->values[i].integer)).append(",");
						break;
					case IPP_TAG_BOOLEAN:
						value.append((attr->values[i].boolean ? "true" : "false")).append(",");
						break;
					case IPP_TAG_RANGE:
						if (attr->values[i].range.lower > 0)
							value.append(QString::number(attr->values[i].range.lower));
						if (attr->values[i].range.lower != attr->values[i].range.upper)
						{
							value.append("-");
							if (attr->values[i].range.upper > 0)
								value.append(QString::number(attr->values[i].range.upper));
						}
						value.append(",");
						break;
					case IPP_TAG_STRING:
					case IPP_TAG_TEXT:
					case IPP_TAG_NAME:
					case IPP_TAG_KEYWORD:
					case IPP_TAG_URI:
					case IPP_TAG_MIMETYPE:
					case IPP_TAG_NAMELANG:
					case IPP_TAG_TEXTLANG:
					case IPP_TAG_CHARSET:
					case IPP_TAG_LANGUAGE:
						value.append(QString::fromLocal8Bit(attr->values[i].string.text)).append(",");
						break;
					default:
						break;
				}
			}
			if (!value.isEmpty())
				value.truncate(value.length()-1);
			opts[QString::fromLocal8Bit(attr->name)] = value;
			attr = attr->next;
		}
	}
	return opts;
}

void IppRequest::setMap(const QMap<QString,QString>& opts)
{
	if (!request_)
		return;

	QRegExp	re("^\"|\"$");
	cups_option_t	*options = NULL;
	int	n = 0;
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.key().startsWith("kde-") || it.key().startsWith("app-"))
			continue;
		QString	value = it.data().stripWhiteSpace(), lovalue;
		value.replace(re, "");
		lovalue = value.lower();

		// handles specific cases: boolean, empty strings, or option that has that boolean
		// keyword as value (to prevent them from conversion to real boolean)
		if (value == "true" || value == "false")
			addBoolean(IPP_TAG_JOB, it.key(), (value == "true"));
		else if (value.isEmpty() || lovalue == "off" || lovalue == "on"
		         || lovalue == "yes" || lovalue == "no"
			 || lovalue == "true" || lovalue == "false")
			addName(IPP_TAG_JOB, it.key(), value);
		else
			n = cupsAddOption(it.key().local8Bit(), value.local8Bit(), n, &options);
	}
	if (n > 0)
		cupsEncodeOptions(request_, n, options);
	cupsFreeOptions(n, options);

	// find an remove that annoying "document-format" attribute
	ipp_attribute_t	*attr = request_->attrs;
	while (attr)
	{
		if (attr->next && strcmp(attr->next->name, "document-format") == 0)
		{
			ipp_attribute_t	*attr2 = attr->next;
			attr->next = attr2->next;
			_ipp_free_attr(attr2);
			break;
		}
		attr = attr->next;
	}
}
