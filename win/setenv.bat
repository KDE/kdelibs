::
:: windows environment settings 
:: 
:: currently only mingw supported
::

:: mingw
if exist c:\Mingw set MINGW=C:\Mingw
if exist F:\Mingw set MINGW=F:\Mingw
if not %MINGW%=="" set QMAKESPEC=win32-g++

if not %MINGW%=="" (
	set PATH=%MINGW%\bin;%PATH%
)

:: python settings
if exist C:\Python24 set PYTHON=C:\PYTHON24
if exist F:\Python24 set PYTHON=F:\PYTHON24
if not %PYTHON%=="" (
	set PATH=%PYTHON%;%PATH%
)

:: kdewin32 library 
if exist C:\Programme\kdewin32 set PATH=C:\Programme\kdewin32\bin;%PATH%

:: gnuwin32 packages
if exist C:\Programme\gnuwin32 set PATH=C:\Programme\gnuwin32\bin;%PATH%

:: Qt
if exist C:\Qt\4.1.0 set QTDIR=C:\QT\4.1.0
if exist C:\Qt\4.1.1 set QTDIR=C:\QT\4.1.1
if exist F:\Qt\4.1.0 set QTDIR=F:\QT\4.1.0
if exist F:\Qt\4.1.1 set QTDIR=F:\QT\4.1.1
if not %QTDIR%=="" (
	set PATH=%QTDIR%\bin;%PATH%
)

:: my editor 
if exist C:\Programme\Ultraedt set PATH=c:\Programme\Ultraedt;%PATH%

:: runtime environment 
::set KDEDIR=c:\Programme\KDE4
::set KDEHOME=%KDEDIR%\home
::set PATH=%KDEDIR%\bin;%KDEDIR%\lib;%PATH%


