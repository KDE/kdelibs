
#include <QString>
#include <QByteArray>
#include <QDir>
#include <QFileInfo>

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

#if defined(__MINGW32__)
# define WIN32_CAST_CHAR (WCHAR*)
#else
# define WIN32_CAST_CHAR (LPCSTR)
#endif

KDEWIN32_EXPORT 
QString getWin32RegistryValue(HKEY key, const QString& subKey, const QString& item, bool *ok)
{
#define FAILURE \
	{ if (ok) \
		*ok = false; \
	return QString::null; }

	if (subKey.isEmpty())
		FAILURE;
	HKEY hKey;
	TCHAR *lszValue;
	DWORD dwType=REG_SZ;
	DWORD dwSize;

	if (ERROR_SUCCESS!=RegOpenKeyEx(key, WIN32_CAST_CHAR subKey.utf16(), NULL, KEY_READ, &hKey))
		FAILURE;

	if (ERROR_SUCCESS!=RegQueryValueEx(hKey, WIN32_CAST_CHAR item.utf16(), NULL, NULL, NULL, &dwSize))
		FAILURE;

	lszValue = new TCHAR[dwSize];

	if (ERROR_SUCCESS!=RegQueryValueEx(hKey, WIN32_CAST_CHAR item.utf16(), NULL, &dwType, (LPBYTE)lszValue, &dwSize)) {
		delete [] lszValue;
		FAILURE;
	}
	RegCloseKey(hKey);

	QString res = QString::fromUtf16( (const ushort*)lszValue );
	delete [] lszValue;
	return res;
}

KDEWIN32_EXPORT
bool showWin32FilePropertyDialog(const QString& fileName)
{
	QString path_ = QDir::convertSeparators(QFileInfo(fileName).absoluteFilePath());

	SHELLEXECUTEINFO execInfo;
	memset(&execInfo,0,sizeof(execInfo));
	execInfo.cbSize = sizeof(execInfo);
	execInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	const QString verb("properties");
	execInfo.lpVerb = (TCHAR*)verb.utf16();
	execInfo.lpFile = (TCHAR*)path_.utf16();
	return ShellExecuteEx(&execInfo);
}

KDEWIN32_EXPORT
QByteArray getWin32LocaleName()
{
	bool ok;
	QString localeNumber = getWin32RegistryValue(HKEY_CURRENT_USER, "Control Panel\\International", 
		"Locale", &ok);
	if (!ok)
		return QByteArray();
	QString localeName = getWin32RegistryValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybCodes", 
		localeNumber, &ok);
	if (!ok)
		return QByteArray();
	return localeName.toLatin1();
}
