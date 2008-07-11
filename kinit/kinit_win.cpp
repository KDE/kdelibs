/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *                 (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *                 (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *                 (c) 2006 Ralf Habacker <ralf.habacker@freenet.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.    If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <psapi.h>


#include <QtCore/QProcess>
#include <QtCore/QFileInfo>
#include <QtDBus/QtDBus>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdeversion.h>


#define KDED_EXENAME "kded4"

static KComponentData *s_instance = 0;

// print verbose messages
int verbose=0;

/// holds process list for suicide mode
QList<QProcess*> startedProcesses;

// internal launch function
int launch(const QString &cmd)
{
    QProcess *proc = new QProcess();
    proc->start(cmd);
    proc->waitForStarted();
    startedProcesses << proc;
    _PROCESS_INFORMATION* _pid = proc->pid();
    int pid = _pid ? _pid->dwProcessId : 0;
    if (verbose) {
        fprintf(stderr,"%s",proc->readAllStandardError().constData());
        fprintf(stderr,"%s",proc->readAllStandardOutput().constData());
    }
    if (pid) {
       if (verbose)
           fprintf(stderr, "kdeinit4: Launched %s, pid = %ld\n", qPrintable(cmd),(long) pid);
    }
    else {
       if (verbose)
           fprintf(stderr, "kdeinit4: could not launch %s, exiting",qPrintable(cmd));
    }
    return pid;
}

/// check dbus registration
bool checkIfRegisteredInDBus(const QString &name, int _timeout=10)
{
    int timeout = _timeout * 5;
    while(timeout) {
        if ( QDBusConnection::sessionBus().interface()->isServiceRegistered( name ) )
            break;
        Sleep(200);
        timeout--;
    }
        if (!timeout) {
            if (verbose)
                fprintf(stderr,"not registered %s in dbus after %d secs\n",qPrintable(name),_timeout);
            return false;
        }
        if (verbose)
            fprintf(stderr,"%s is registered in dbus\n",qPrintable(name));
    return true;
}

class ProcessListEntry {
    public:
       ProcessListEntry(HANDLE _handle,char *_path, int _pid ) 
       {    
           QFileInfo p(_path);
           path = p.absolutePath();
           name = p.baseName();
           handle = _handle; 
           pid = _pid; 
       }
       QString name;
       QString path;
       int pid;
       HANDLE handle;
       friend QDebug operator <<(QDebug out, const ProcessListEntry &c);
};

QDebug operator <<(QDebug out, const ProcessListEntry &c)
{
    out << "(ProcessListEntry" 
        << "name" << c.name
        << "path" << c.path
        << "pid" << c.pid
        << "handle" << c.handle
        << ")";
    return out;
}    

/**
 holds system process list
*/
class ProcessList {
    public:
       ProcessList() {initProcessList(); }
       ~ProcessList();
       ProcessListEntry *hasProcessInList(const QString &name);
       bool terminateProcess(const QString &name);
       QList<ProcessListEntry *> &list() { return processList; }
    private:
       void initProcessList();
       void getProcessNameAndID( DWORD processID );
       QList<ProcessListEntry *> processList;
};


void ProcessList::getProcessNameAndID( DWORD processID )
{
    char szProcessName[MAX_PATH];

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( SYNCHRONIZE|PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ | PROCESS_TERMINATE,
                                   false, processID );

    // Get the process name.
    int ret;

    if (NULL != hProcess )
    {
       HMODULE hMod;
       DWORD cbNeeded;

       if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
              &cbNeeded) )
       {
            ret = GetModuleFileNameExA( hProcess, hMod, szProcessName,
                                           sizeof(szProcessName)/sizeof(TCHAR) );
       }
    }
    if (ret > 0)
    {
        processList << new ProcessListEntry(hProcess,szProcessName,processID );
    }
}


/**
    read process list from system and fill in global var aProcessList
*/
void ProcessList::initProcessList()
{
    // Get the list of process identifiers.

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.

    for ( i = 0; i < cProcesses; i++ )
        if( aProcesses[i] != 0 )
            getProcessNameAndID( aProcesses[i] );
}


ProcessList::~ProcessList()
{
    ProcessListEntry *ple;
    foreach(ple,processList) {
        CloseHandle(ple->handle);
        delete ple;
    }
}

/**
 return process list entry of given name
*/
ProcessListEntry *ProcessList::hasProcessInList(const QString &name)
{
    ProcessListEntry *ple;
    foreach(ple,processList) {
        if (ple->name == name || ple->name == name + ".exe") {
            return ple;
        }
    }
    return NULL;
}

/**
 terminate process from process list
*/
bool ProcessList::terminateProcess(const QString &name)
{
    ProcessListEntry *p = hasProcessInList(name);
    if (!p)
        return false;
    return TerminateProcess(p->handle,0) ? true : false;
}


int main(int argc, char **argv, char **envp)
{
    pid_t pid;
    bool launch_dbus = true;
    bool launch_klauncher = true;
    bool launch_kded = true;
    bool suicide = false;
    bool listProcesses = false;
    bool killProcesses = false;

    /** Save arguments first... **/
    char **safe_argv = (char **) malloc( sizeof(char *) * argc);
    for(int i = 0; i < argc; i++)
    {
        safe_argv[i] = strcpy((char*)malloc(strlen(argv[i])+1), argv[i]);
        if (strcmp(safe_argv[i], "--no-dbus") == 0)
            launch_dbus = false;
        if (strcmp(safe_argv[i], "--no-klauncher") == 0)
            launch_klauncher = false;
        if (strcmp(safe_argv[i], "--no-kded") == 0)
            launch_kded = false;
        if (strcmp(safe_argv[i], "--suicide") == 0)
            suicide = true;
//        if (strcmp(safe_argv[i], "--exit") == 0)
//            keep_running = 0;
        if (strcmp(safe_argv[i], "--verbose") == 0)
            verbose = 1;
        if (strcmp(safe_argv[i], "--help") == 0)
        {
           printf("Usage: kdeinit4 [options]\n");
           printf("       --no-dbus              Do not start dcopserver\n");
           printf("       --no-klauncher         Do not start klauncher\n");
           printf("       --no-kded              Do not start kded\n");
           printf("       --suicide              Terminate when no KDE applications are left running\n");
           printf("       --terminate            Terminate all running kde processes\n");
           printf("       --verbose              print verbose messages\n");
           printf("       --list                 list system process for which the user has the right to terminate\n");
      // printf("       --exit                    Terminate when kded has run\n");
           exit(0);
        }
        if (strcmp(safe_argv[i], "--list") == 0) {
            listProcesses = true;
        }
        if (strcmp(safe_argv[i], "--terminate") == 0) {
            killProcesses = true;
        }
    }

    ProcessList processList;
    QString installPrefix = KStandardDirs::installPath("kdedir");

    if (listProcesses) {
        ProcessListEntry *ple;

        foreach(ple,processList.list()) 
        {
            if (ple->path.toLower().startsWith(installPrefix.toLower()))
                fprintf(stdout,"path: %s name: %s pid: %u\n", ple->path.toLatin1().data(), ple->name.toLatin1().data(), ple->pid);
        }
        exit(0);
    }
    else if (killProcesses) {
        ProcessListEntry *ple;

        foreach(ple,processList.list()) 
        {
            if (ple->path.toLower().startsWith(installPrefix.toLower())) 
            {
                if (verbose)
                    fprintf(stdout,"terminating path: %s name: %s pid: %u\n", ple->path.toLatin1().data(), ple->name.toLatin1().data(), ple->pid);
                processList.terminateProcess(ple->name);
            }
        }
        exit(0);
    }

    /** Make process group leader (for shutting down children later) **/

    /** Create our instance **/
    s_instance = new KComponentData("kdeinit4", QByteArray(), KComponentData::SkipMainComponentRegistration);

    if (launch_dbus && !processList.hasProcessInList("dbus-daemon"))
    {
          pid = launch("dbus-launch.exe");
          if (!pid)
              pid = launch("dbus-launch.bat");
          if (!pid)
              exit(1);
    }

    if (launch_klauncher && !processList.hasProcessInList("klauncher"))
    {
          pid = launch("klauncher");
          if (!pid || !checkIfRegisteredInDBus("org.kde.klauncher",10))
              exit(1);
    }


    if (launch_kded && !processList.hasProcessInList(KDED_EXENAME))
    {
        pid = launch(KDED_EXENAME);
        if (!pid || !checkIfRegisteredInDBus("org.kde.kded",10))
            exit(1);
    }

    for(int i = 1; i < argc; i++)
    {
        if (safe_argv[i][0] == '+')
        {
            pid = launch(safe_argv[i]+1);
        }
        else if (safe_argv[i][0] == '-')
        {
            // Ignore
        }
        else
        {
            pid = launch( safe_argv[i]);
        }
    }

    /** Free arguments **/
    for(int i = 0; i < argc; i++)
    {
          free(safe_argv[i]);
    }
    free (safe_argv);

    /** wait for termination of all (core) processes */
    if (suicide) {
        QProcess *proc;
        int can_exit=1;
        do {
           foreach(proc,startedProcesses) {
             if (proc->state() != QProcess::NotRunning)
                can_exit = 0;
           }
           if (!can_exit)
             Sleep(2000);
        } while(!can_exit);
        return 0;
    }
    return 0;
}
