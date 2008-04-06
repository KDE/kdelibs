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

void ErrorExit(LPWSTR lpszFunction)
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
    ExitProcess(-1);
}

void PrintUsage(void)
{
    wprintf(L"A Wrapper for cmd.exe\n\n");
    wprintf(L"calls shell /S /C argument with proper quoting\n");
    wprintf(L"Usage: kcmdwrapper shell argument");
    ExitProcess(-1);
}

void printError(const char * p) {}
#define COMMAND_BUFFER_SIZE 34000

int main(int argc, char **argv)
{
    LPWSTR *Arglist;
    int nArgs;
    WCHAR Command[COMMAND_BUFFER_SIZE];
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD ExitCode;

    Arglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

    if (NULL == Arglist)
        ErrorExit(L"CommandLineToArgvW");

    if (nArgs != 3)
        PrintUsage();

    _snwprintf(Command, COMMAND_BUFFER_SIZE, L"/S /C \"%s\"", Arglist[2]);


    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );


    if (CreateProcessW(Arglist[1], Command, NULL, NULL, true, 0, NULL, NULL, &si, &pi) == 0)
        ErrorExit(L"CreateProcessW");

    CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
    CloseHandle(GetStdHandle(STD_ERROR_HANDLE));
    CloseHandle(GetStdHandle(STD_OUTPUT_HANDLE));

    if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED)
        ErrorExit(L"WaitForSingleObject");

    if (!GetExitCodeProcess(pi.hProcess, &ExitCode))
        ErrorExit(L"WaitForSingleObject");

    if (ExitCode == STILL_ACTIVE )
        OutputDebugStringW(L"cmdwrapper: STILL_ACTIVE returned");

    return ExitCode;
}

