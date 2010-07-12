/*
   This file is part of the KDE libraries
   Copyright (C) 2004-2008 Jarosław Staniek <staniek@kde.org>

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

#include <QtGui/QApplication>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kwindowsystem.h>

#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QLibrary>

#include <stdio.h>

/**
 * MS Windows-related actions for KApplication startup.
 *
 * - Use Qt translation which will be usable for QFileDialog
 *    and other Qt-only GUIs. The "qt_<language>.qm" file should be stored
 *    in the same place as .po files for a given language.
 *
 * - Increase the default open file limit with the stdio API from 512 to 2048
 *   (2048 is the hard limit on Windows)
 * @internal
*/
void KApplication_init_windows()
{
	//QString qt_transl_file = ::locate( "locale", KGlobal::locale()->language()
	//	+ "/LC_MESSAGES/qt_" + KGlobal::locale()->language() + ".qm" );

	QString qt_transl_file = QString("qt_") + QLocale::system().name();
	qt_transl_file.truncate(5);
	QTranslator *qt_transl = new QTranslator();
	if (qt_transl->load( qt_transl_file,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		qApp->installTranslator( qt_transl );
	else
		delete qt_transl;

    // For apps like KMail which have lots of open files, the default is too low
	// so increase it to the maximum.
#ifndef _WIN32_WCE
    _setmaxstdio(2048);
#endif

}

// <copy of kdepim/libkdepim/utils.cpp, TODO: move to a shared helper library>

#include <windows.h>
#include <winperf.h>
#include <psapi.h>
#include <signal.h>
#include <unistd.h>

#include <QtCore/QList>
#include <QtCore/QtDebug>

static PPERF_OBJECT_TYPE FirstObject( PPERF_DATA_BLOCK PerfData )
{
  return (PPERF_OBJECT_TYPE)((PBYTE)PerfData + PerfData->HeaderLength);
}

static PPERF_INSTANCE_DEFINITION FirstInstance( PPERF_OBJECT_TYPE PerfObj )
{
  return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj + PerfObj->DefinitionLength);
}

static PPERF_OBJECT_TYPE NextObject( PPERF_OBJECT_TYPE PerfObj )
{
  return (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + PerfObj->TotalByteLength);
}

static PPERF_COUNTER_DEFINITION FirstCounter( PPERF_OBJECT_TYPE PerfObj )
{
  return (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj + PerfObj->HeaderLength);
}

static PPERF_INSTANCE_DEFINITION NextInstance( PPERF_INSTANCE_DEFINITION PerfInst )
{
  PPERF_COUNTER_BLOCK PerfCntrBlk
    = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst + PerfInst->ByteLength);
  return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfCntrBlk + PerfCntrBlk->ByteLength);
}

static PPERF_COUNTER_DEFINITION NextCounter( PPERF_COUNTER_DEFINITION PerfCntr )
{
  return (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr + PerfCntr->ByteLength);
}

static PPERF_COUNTER_BLOCK CounterBlock(PPERF_INSTANCE_DEFINITION PerfInst)
{
  return (PPERF_COUNTER_BLOCK) ((LPBYTE) PerfInst + PerfInst->ByteLength);
}

#define GETPID_TOTAL 64 * 1024
#define GETPID_BYTEINCREMENT 1024
#define GETPID_PROCESS_OBJECT_INDEX 230
#define GETPID_PROC_ID_COUNTER 784

QString fromWChar(const wchar_t *string, int size = -1)
{
  return (sizeof(wchar_t) == sizeof(QChar)) ? QString::fromUtf16((ushort *)string, size)
    : QString::fromUcs4((uint *)string, size);
}

#ifndef _WIN32_WCE
void KApplication_getProcessesIdForName( const QString& processName, QList<int>& pids )
{
  qDebug() << "KApplication_getProcessesIdForName" << processName;
  PPERF_OBJECT_TYPE perfObject;
  PPERF_INSTANCE_DEFINITION perfInstance;
  PPERF_COUNTER_DEFINITION perfCounter, curCounter;
  PPERF_COUNTER_BLOCK counterPtr;
  DWORD bufSize = GETPID_TOTAL;
  PPERF_DATA_BLOCK perfData = (PPERF_DATA_BLOCK) malloc( bufSize );

  char key[64];
  sprintf(key,"%d %d", GETPID_PROCESS_OBJECT_INDEX, GETPID_PROC_ID_COUNTER);
  LONG lRes;
  while( (lRes = RegQueryValueExA( HKEY_PERFORMANCE_DATA,
                               key,
                               NULL,
                               NULL,
                               (LPBYTE) perfData,
                               &bufSize )) == ERROR_MORE_DATA )
  {
    // get a buffer that is big enough
    bufSize += GETPID_BYTEINCREMENT;
    perfData = (PPERF_DATA_BLOCK) realloc( perfData, bufSize );
  }

  // Get the first object type.
  perfObject = FirstObject( perfData );

  // Process all objects.
  for( uint i = 0; i < perfData->NumObjectTypes; i++ ) {
    if (perfObject->ObjectNameTitleIndex != GETPID_PROCESS_OBJECT_INDEX) {
      perfObject = NextObject( perfObject );
      continue;
    }
    pids.clear();
    perfCounter = FirstCounter( perfObject );
    perfInstance = FirstInstance( perfObject );
    // retrieve the instances
    qDebug() << "INSTANCES: " << perfObject->NumInstances;
    for( int instance = 0; instance < perfObject->NumInstances; instance++ ) {
      curCounter = perfCounter;
      const QString foundProcessName(
        fromWChar( (wchar_t *)( (PBYTE)perfInstance + perfInstance->NameOffset ) ) );
      qDebug() << "foundProcessName: " << foundProcessName;
      if ( foundProcessName == processName ) {
        // retrieve the counters
        for( uint counter = 0; counter < perfObject->NumCounters; counter++ ) {
          if (curCounter->CounterNameTitleIndex == GETPID_PROC_ID_COUNTER) {
            counterPtr = CounterBlock(perfInstance);
            DWORD *value = (DWORD*)((LPBYTE) counterPtr + curCounter->CounterOffset);
            pids.append( int( *value ) );
            qDebug() << "found PID: " << int( *value );
            break;
          }
          curCounter = NextCounter( curCounter );
        }
      }
      perfInstance = NextInstance( perfInstance );
    }
  }
  free(perfData);
  RegCloseKey(HKEY_PERFORMANCE_DATA);
}

bool KApplication_otherProcessesExist( const QString& processName )
{
  QList<int> pids;
  KApplication_getProcessesIdForName( processName, pids );
  int myPid = getpid();
  foreach ( int pid, pids ) {
    if (myPid != pid) {
//      kDebug() << "Process ID is " << pid;
      return true;
    }
  }
  return false;
}

bool KApplication_killProcesses( const QString& processName )
{
  QList<int> pids;
  KApplication_getProcessesIdForName( processName, pids );
  if ( pids.empty() )
    return true;
  qWarning() << "Killing process \"" << processName << " (pid=" << pids[0] << ")..";
  int overallResult = 0;
  foreach( int pid, pids ) {
    int result = kill( pid, SIGTERM );
    if ( result == 0 )
      continue;
    result = kill( pid, SIGKILL );
    if ( result != 0 )
      overallResult = result;
  }
  return overallResult == 0;
}

struct EnumWindowsStruct
{
  EnumWindowsStruct() : windowId( 0 ) {}
  int pid;
  HWND windowId;
};

BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
  if ( GetWindowLong( hwnd, GWL_STYLE ) & WS_VISIBLE ) {
    DWORD pidwin;
    GetWindowThreadProcessId(hwnd, &pidwin);
    if ( pidwin == ((EnumWindowsStruct*)lParam)->pid ) {
      ((EnumWindowsStruct*)lParam)->windowId = hwnd;
      return false;
    }
  }
  return true;
}

void KApplication_activateWindowForProcess( const QString& executableName )
{
  QList<int> pids;
  KApplication_getProcessesIdForName( executableName, pids );
  int myPid = getpid();
  int foundPid = 0;
  foreach ( int pid, pids ) {
    if (myPid != pid) {
      qDebug() << "activateWindowForProcess(): PID to activate:" << pid;
      foundPid = pid;
      break;
    }
  }
  if ( foundPid == 0 )
    return;
  EnumWindowsStruct winStruct;
  winStruct.pid = foundPid;
  EnumWindows( EnumWindowsProc, (LPARAM)&winStruct );
  if ( winStruct.windowId == NULL )
    return;
  KWindowSystem::forceActiveWindow( winStruct.windowId, 0 );
}
#endif

// </copy>
