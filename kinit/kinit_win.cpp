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

//#define ENABLE_SUICIDE 
//#define ENABLE_EXIT

#define KDED_EXENAME "kded4"

static KComponentData *s_instance = 0;

// print verbose messages
int verbose=0;

/// holds process list for suicide mode
QList<QProcess*> startedProcesses;

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

void listAllRunningKDEProcesses(ProcessList &processList)
{
    ProcessListEntry *ple;
    QString installPrefix = KStandardDirs::installPath("kdedir");

    foreach(ple,processList.list()) 
    {
        if (ple->path.toLower().startsWith(installPrefix.toLower()))
            fprintf(stderr,"path: %s name: %s pid: %u\n", ple->path.toLatin1().data(), ple->name.toLatin1().data(), ple->pid);
    }
}

void terminateAllRunningKDEProcesses(ProcessList &processList)
{
    ProcessListEntry *ple;
    QString installPrefix = KStandardDirs::installPath("kdedir");

    foreach(ple,processList.list()) 
    {
        if (ple->path.toLower().startsWith(installPrefix.toLower())) 
        {
            if (verbose)
                fprintf(stderr,"terminating path: %s name: %s pid: %u\n", ple->path.toLatin1().data(), ple->name.toLatin1().data(), ple->pid);
            processList.terminateProcess(ple->name);
        }
    }
}

void listAllNamedAppsInDBus()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusConnectionInterface *bus = connection.interface();
    const QStringList services = bus->registeredServiceNames();
    foreach(const QString &service, services) {
        if (service.startsWith("org.freedesktop.DBus") || service.startsWith(':'))
            continue;
        fprintf(stderr, "%s \n", service.toLatin1().data());
    }
}

void quitApplicationsOverDBus()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusConnectionInterface *bus = connection.interface();
    const QStringList services = bus->registeredServiceNames();
    foreach(const QString &service, services) {
        if (service.startsWith("org.freedesktop.DBus") || service.startsWith(':'))
            continue;
        QDBusInterface *iface = new QDBusInterface(service,
                               QLatin1String("/MainApplication"),
//  see http://lists.kde.org/?l=kde-core-devel&m=121641642911291&w=2
#if QT_VERSION < 0x040402
                               QLatin1String(""),
#else                             
                               QLatin1String("org.kde.KApplication"),
#endif
                               connection);
        if (!iface->isValid()) {
            if (verbose)
                fprintf(stderr, "invalid interface of service %s\n", service.toLatin1().data());
            continue;
        }
        iface->call("quit");
        if (iface->lastError().isValid()) {
            if (verbose)
                fprintf(stderr,"killing %s with result\n", iface->lastError().message().toLatin1().data());
        }
        delete iface;
    }
}

int main(int argc, char **argv, char **envp)
{
    pid_t pid = 0;
    bool launch_dbus = true;
    bool launch_klauncher = true;
    bool launch_kded = true;
    bool suicide = false;
    bool listProcesses = false;
    bool killProcesses = false;
    bool listAppsInDBus = false;
    bool quitAppsOverDBus = false;
    bool shutdown = false;

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
#ifdef ENABLE_EXIT
        if (strcmp(safe_argv[i], "--exit") == 0)
            keep_running = 0;
#endif            
        if (strcmp(safe_argv[i], "--verbose") == 0)
            verbose = 1;
        if (strcmp(safe_argv[i], "--version") == 0) 
        { 
            printf("Qt: %s\n",qVersion()); 
            printf("KDE: %s\n", KDE_VERSION_STRING); 
            exit(0);
        } 
        if (strcmp(safe_argv[i], "--help") == 0)
        {
           printf("Usage: kdeinit4 [options]\n");
#ifdef ENABLE_EXIT
           printf("   --exit                     Terminate when kded has run\n");
#endif
           printf("   --help                     this help page\n");
           printf("   --list                     list kde processes\n");
           printf("   --list-dbus-apps           list all applications registered in dbus\n");
           printf("   --quit-over-dbus           quit all application registered in dbus\n");
           printf("   --no-dbus                  do not start dbus-daemon\n");
           printf("   --no-klauncher             do not start klauncher\n");
           printf("   --no-kded                  do not start kded\n");
           printf("   --shutdown                 safe shutdown of all running kde processes\n");
           printf("                              first over dbus, then using hard kill\n");
#ifdef ENABLE_SUICIDE
           printf("    --suicide                 terminate when no KDE applications are left running\n");
#endif
           printf("   --terminate                hard kill of *all* running kde processes\n");
           printf("   --verbose                  print verbose messages\n");
	   printf("   --version                  Show version information\n");
           exit(0);
        }
        if (strcmp(safe_argv[i], "--list") == 0)
            listProcesses = true;
        if (strcmp(safe_argv[i], "--shutdown") == 0)
            shutdown = true;
        if (strcmp(safe_argv[i], "--terminate") == 0 || strcmp(safe_argv[i], "--kill") == 0)
            killProcesses = true;
        if (strcmp(safe_argv[i], "--list-dbus-apps") == 0)
            listAppsInDBus = true;
        if (strcmp(safe_argv[i], "--quit-over-dbus") == 0)
            quitAppsOverDBus = true;
    }

    ProcessList processList;

    if (listProcesses) {
        listAllRunningKDEProcesses(processList);
        return 0;
    }
    else if (killProcesses) {
        terminateAllRunningKDEProcesses(processList);
        return 0;
    }
    else if (listAppsInDBus) {
        listAllNamedAppsInDBus();
        return 0;
    }
    else if (quitAppsOverDBus) {
        quitApplicationsOverDBus();
        return 0;
    }
    else if (shutdown) {
        quitApplicationsOverDBus();
        Sleep(2000);
        terminateAllRunningKDEProcesses(processList);
    }
    
    /** Create our instance **/
    s_instance = new KComponentData("kdeinit4", QByteArray(), KComponentData::SkipMainComponentRegistration);

#ifdef _DEBUG
    // first try to launch dbus-daemond in debug mode
    if (launch_dbus && processList.hasProcessInList("dbus-daemond"))
          launch_dbus = false;
    if (launch_dbus)
    {
          pid = launch("dbus-launchd.exe");
          if (!pid)
              pid = launch("dbus-launchd.bat");
          launch_dbus = (pid == 0);
    }
#endif
    if (launch_dbus && !processList.hasProcessInList("dbus-daemon"))
    {
          if (!pid)
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
#ifdef ENABLE_SUICIDE
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
#endif    
    return 0;
}
