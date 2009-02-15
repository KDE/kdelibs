/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Bernhard Loos <nhuh.put@web.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <windows.h>
#include <shellapi.h>
#include <wchar.h>
#include <stdio.h>

void errorExit(LPWSTR lpszFunction)
{
    WCHAR szBuf[100];
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR) &lpMsgBuf,
        0, NULL);

    _snwprintf(szBuf, 100,
        L"kcmdwrapper: %s failed with error %d: %s",
        lpszFunction, dw, lpMsgBuf);

    OutputDebugStringW(szBuf);
    LocalFree(lpMsgBuf);
    ExitProcess(~0U);
}

void printUsage(void)
{
    wprintf(L"A Wrapper for cmd.exe\n\n");
    wprintf(L"calls shell /S /C argument with proper quoting\n");
    wprintf(L"Usage: kcmdwrapper shell argument");
    ExitProcess(~0U);
}

void printError(const char * p) {}
#define COMMAND_BUFFER_SIZE 34000

int main(int argc, char **argv)
{
    LPWSTR *argList;
    int nArgs;
    WCHAR command[COMMAND_BUFFER_SIZE];
    OSVERSIONINFO vi;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD exitCode;

    argList = CommandLineToArgvW(GetCommandLineW(), &nArgs);

    if (NULL == argList)
        errorExit(L"CommandLineToArgvW");

    if (nArgs != 3)
        printUsage();

    // Instead of checking the OS version we might check the for presence of
    // the reg keys that enable delayed variable expansion by default - that
    // would also cover the pathological case of running cmd from win2k under
    // winNT. OTOH, who cares?
    GetVersionEx(&vi);
    _snwprintf(command, COMMAND_BUFFER_SIZE,
        (vi.dwMajorVersion >= 5 /* Win2k */) ?
            L"/V:OFF /S /C \"%s\"" : L"/S /C \"%s\"",
        argList[2]);


    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );


    if (CreateProcessW(argList[1], command, NULL, NULL, true, 0, NULL, NULL, &si, &pi) == 0)
        errorExit(L"CreateProcessW");

    CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
    CloseHandle(GetStdHandle(STD_ERROR_HANDLE));
    CloseHandle(GetStdHandle(STD_OUTPUT_HANDLE));

    if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED)
        errorExit(L"WaitForSingleObject");

    if (!GetExitCodeProcess(pi.hProcess, &exitCode))
        errorExit(L"WaitForSingleObject");

    if (exitCode == STILL_ACTIVE )
        OutputDebugStringW(L"cmdwrapper: STILL_ACTIVE returned");

    return exitCode;
}

