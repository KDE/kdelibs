/*
   This file is part of the KDE libraries
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qstring.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

KDEWIN32_EXPORT 
QString getWin32RegistryValue(HKEY key, const QString& subKey, const QString& item, bool *ok)
{
#define FAILURE \
	{ if (ok) \
		*ok = false; \
	return QString::null; }

	if (!subKey)
		FAILURE;
	HKEY hKey;
	TCHAR *lszValue;
	DWORD dwType=REG_SZ;
	DWORD dwSize;
	if (ERROR_SUCCESS!=RegOpenKeyEx(key, subKey.ucs2(), NULL, KEY_READ, &hKey))
		FAILURE;

	if (ERROR_SUCCESS!=RegQueryValueEx(hKey, item.ucs2(), NULL, NULL, NULL, &dwSize))
		FAILURE;

	lszValue = new TCHAR[dwSize];

	if (ERROR_SUCCESS!=RegQueryValueEx(hKey, item.ucs2(), NULL, &dwType, (LPBYTE)lszValue, &dwSize)) {
		delete [] lszValue;
		FAILURE;
	}
	RegCloseKey(hKey);

	QString res = QString::fromUcs2(lszValue);
	delete [] lszValue;
	return res;
}

KDEWIN32_EXPORT
bool showWin32FilePropertyDialog(const QString& fileName)
{
	QString path_ = QDir::convertSeparators(QFileInfo(fileName).absFilePath());

	SHELLEXECUTEINFO execInfo;
	memset(&execInfo,0,sizeof(execInfo));
	execInfo.cbSize = sizeof(execInfo);
	execInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	const QString verb("properties");
	execInfo.lpVerb = (TCHAR*)verb.ucs2();
	execInfo.lpFile = (TCHAR*)path_.ucs2();
	return ShellExecuteEx(&execInfo);
}

KDEWIN32_EXPORT
QCString getWin32LocaleName()
{
	bool ok;
	QString localeNumber = getWin32RegistryValue(HKEY_CURRENT_USER, "Control Panel\\International", 
		"Locale", &ok);
	if (!ok)
		return QCString();
	QString localeName = getWin32RegistryValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybCodes", 
		localeNumber, &ok);
	if (!ok)
		return QCString();
	return localeName.latin1();
}

KDEWIN32_EXPORT
QString convertKFileDialogFilterToQFileDialogFilter(const QString& filter)
{
	QString kde_filters = filter;
	int pos;
	// Strip the escape characters from
	// escaped '/' characters.

	QString copy (kde_filters);
	for (pos = 0; (pos = copy.find("\\/", pos)) != -1; ++pos)
		copy.remove(pos, 1);

	//<js>
	//we need to convert KDE filter format to Qt format
	//Qt format: "some text (*.first *.second)" or "All (*)"
	//KDE format: "*.first *.second" or "*"
	QStringList filters = QStringList::split("\n",kde_filters);
	QString current;
	QString converted; //finally - converted filter
	for (QStringList::ConstIterator it = filters.constBegin(); it!=filters.constEnd();++it) {
		current = *it;
		QString new_f;//filter part
		QString new_name;//filter name part
		int p = (*it).find('|');
		if (p!=-1) {
			new_f = current.left(p);
			new_name = current.mid(p+1);
		}
		else {
			new_f = current;
			new_name = current; //nothing better
		}
		//remove (.....) from name
		p=new_name.find('(');
		int p2 = new_name.findRev(')');
		QString new_name1, new_name2;
		if (p!=-1)
			new_name1 = new_name.left(p);
		if (p2!=-1)
			new_name2 = new_name.mid(p2+1);
		if (!new_name1.isEmpty() || !new_name2.isEmpty())
			new_name = new_name1.stripWhiteSpace() + " " + new_name2.stripWhiteSpace();
		new_name.replace('(',"");
		new_name.replace(')',"");
		new_name = new_name.stripWhiteSpace();

		// make filters unique: remove uppercase extensions (case doesn't matter on win32, BTW)
		QStringList allfiltersUnique;
		QStringList origList( QStringList::split(" ", new_f) );
		for (QStringList::ConstIterator it = origList.constBegin();
			it!=origList.constEnd(); ++it)
		{
			if ((*it) == (*it).lower())
				allfiltersUnique += *it;
		}

		if (!converted.isEmpty())
			converted += ";;";

		converted += (new_name + " (" + allfiltersUnique.join(" ") + ")");
	}
	return converted;
}
