
#include <qstring.h>
#include <qdir.h>
#include <qfileinfo.h>

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
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER, subKey.ucs2(), NULL, KEY_READ, &hKey))
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

