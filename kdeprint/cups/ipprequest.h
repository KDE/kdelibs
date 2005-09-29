/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef IPPREQUEST_H
#define IPPREQUEST_H

#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qmap.h>

#include <cups/ipp.h>

class IppRequest
{
public:
	IppRequest();
	~IppRequest();

	void init();	// re-initializes the request

	// request building functions
	void addMime(int group, const QString& name, const QString& mime);
	void addKeyword(int group, const QString& name, const QString& key);
	void addKeyword(int group, const QString& name, const QStringList& keys);
	void addURI(int group, const QString& name, const QString& uri);
	void addURI(int group, const QString& name, const QStringList& uris);
	void addText(int group, const QString& name, const QString& txt);
	void addText(int group, const QString& name, const QStringList& txts);
	void addName(int group, const QString& name, const QString& nm);
	void addName(int group, const QString& name, const QStringList& nms);
	void addInteger(int group, const QString& name, int value);
	void addInteger(int group, const QString& name, const QValueList<int>& values);
	void addEnum(int group, const QString& name, int value);
	void addEnum(int group, const QString& name, const QValueList<int>& values);
	void addBoolean(int group, const QString& name, bool value);
	void addBoolean(int group, const QString& name, const QValueList<bool>& values);

	void setOperation(int op);
	void setHost(const QString& host);
	void setPort(int p);

	// request answer functions
	int status();
	QString statusMessage();
	bool integer(const QString& name, int& value);
	bool boolean(const QString& name, bool& value);
	bool enumvalue(const QString& name, int& value);
	bool name(const QString& name, QString& value);
	bool name(const QString& name, QStringList& value);
	bool text(const QString& name, QString& value);
	bool text(const QString& name, QStringList& value);
	bool uri(const QString& name, QString& value);
	bool uri(const QString& name, QStringList& value);
	bool keyword(const QString& name, QString& value);
	bool keyword(const QString& name, QStringList& value);
	bool mime(const QString& name, QString& value);
	ipp_attribute_t* first();
	ipp_attribute_t* last();
	QMap<QString,QString> toMap(int group = -1);
	void setMap(const QMap<QString,QString>& opts);

	// processing functions
	bool doRequest(const QString& res);
	bool doFileRequest(const QString& res, const QString& filename = QString::null);

	// report functions
	bool htmlReport(int group, QTextStream& output);

	// debug function
	void dump(int state);

protected:
	void addString_p(int group, int type, const QString& name, const QString& value);
	void addStringList_p(int group, int type, const QString& name, const QStringList& values);
	void addInteger_p(int group, int type, const QString& name, int value);
	void addIntegerList_p(int group, int type, const QString& name, const QValueList<int>& values);
	bool stringValue_p(const QString& name, QString& value, int type);
	bool stringListValue_p(const QString& name, QStringList& values, int type);
	bool integerValue_p(const QString& name, int& value, int type);

private:
	ipp_t	*request_;
	QString	host_;
	int 	port_;
	bool	connect_;
	int	dump_;
};

inline void IppRequest::addMime(int group, const QString& name, const QString& mime)
{ addString_p(group, IPP_TAG_MIMETYPE, name, mime); }

inline void IppRequest::addKeyword(int group, const QString& name, const QString& key)
{ addString_p(group, IPP_TAG_KEYWORD, name, key); }

inline void IppRequest::addKeyword(int group, const QString& name, const QStringList& keys)
{ addStringList_p(group, IPP_TAG_KEYWORD, name, keys); }

inline void IppRequest::addURI(int group, const QString& name, const QString& uri)
{ addString_p(group, IPP_TAG_URI, name, uri); }

inline void IppRequest::addURI(int group, const QString& name, const QStringList& uris)
{ addStringList_p(group, IPP_TAG_URI, name, uris); }

inline void IppRequest::addText(int group, const QString& name, const QString& txt)
{ addString_p(group, IPP_TAG_TEXT, name, txt); }

inline void IppRequest::addText(int group, const QString& name, const QStringList& txts)
{ addStringList_p(group, IPP_TAG_TEXT, name, txts); }

inline void IppRequest::addName(int group, const QString& name, const QString& nm)
{ addString_p(group, IPP_TAG_NAME, name, nm); }

inline void IppRequest::addName(int group, const QString& name, const QStringList& nms)
{ addStringList_p(group, IPP_TAG_NAME, name, nms); }

inline void IppRequest::addInteger(int group, const QString& name, int value)
{ addInteger_p(group, IPP_TAG_INTEGER, name, value); }

inline void IppRequest::addInteger(int group, const QString& name, const QValueList<int>& values)
{ addIntegerList_p(group, IPP_TAG_INTEGER, name, values); }

inline void IppRequest::addEnum(int group, const QString& name, int value)
{ addInteger_p(group, IPP_TAG_ENUM, name, value); }

inline void IppRequest::addEnum(int group, const QString& name, const QValueList<int>& values)
{ addIntegerList_p(group, IPP_TAG_ENUM, name, values); }

inline bool IppRequest::integer(const QString& name, int& value)
{ return integerValue_p(name, value, IPP_TAG_INTEGER); }

inline bool IppRequest::enumvalue(const QString& name, int& value)
{ return integerValue_p(name, value, IPP_TAG_ENUM); }

inline bool IppRequest::name(const QString& name, QString& value)
{ return stringValue_p(name, value, IPP_TAG_NAME); }

inline bool IppRequest::name(const QString& name, QStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_NAME); }

inline bool IppRequest::text(const QString& name, QString& value)
{ return stringValue_p(name, value, IPP_TAG_TEXT); }

inline bool IppRequest::text(const QString& name, QStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_TEXT); }

inline bool IppRequest::uri(const QString& name, QString& value)
{ return stringValue_p(name, value, IPP_TAG_URI); }

inline bool IppRequest::uri(const QString& name, QStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_URI); }

inline bool IppRequest::keyword(const QString& name, QString& value)
{ return stringValue_p(name, value, IPP_TAG_KEYWORD); }

inline bool IppRequest::keyword(const QString& name, QStringList& values)
{ return stringListValue_p(name, values, IPP_TAG_KEYWORD); }

inline bool IppRequest::mime(const QString& name, QString& value)
{ return stringValue_p(name, value, IPP_TAG_MIMETYPE); }

inline bool IppRequest::doRequest(const QString& res)
{ return doFileRequest(res); }

inline ipp_attribute_t* IppRequest::first()
{ return (request_ ? request_->attrs : NULL); }

inline ipp_attribute_t* IppRequest::last()
{ return (request_ ? request_->last : NULL); }

inline void IppRequest::setHost(const QString& host)
{ host_ = host; }

inline void IppRequest::setPort(int p)
{ port_ = p; }

inline void IppRequest::dump(int state)
{ dump_ = state; }

#endif
