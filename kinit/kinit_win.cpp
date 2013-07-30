/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *                 (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *                 (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *                 (c) 2006-2011 Ralf Habacker <ralf.habacker@freenet.de>
 *                 (c) 2009 Patrick Spendrin <ps_ml@gmx.de>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#ifndef _WIN32_WCE
#include <Sddl.h>
#endif
#include <tlhelp32.h>
#include <psapi.h>


#include <QtCore/QProcess>
#include <QtCore/QFileInfo>
// Under wince interface is defined, so undef it otherwise it breaks it
#undef interface
#include <QtDBus/QtDBus>

#include <kstandarddirs.h> // TODO REMOVE
#include <kcoreaddons_version.h>

//#define ENABLE_SUICIDE
//#define ENABLE_EXIT

#define KDED_EXENAME "kded5"

// print verbose messages
int verbose=0;

/// holds process list for suicide mode
QList<QProcess*> startedProcesses;

/* --------------------------------------------------------------------
  sid helper - will be migrated later to a class named Sid, which could
  be used as base class for platform independent K_UID and K_GID types
  - would this be possible before KDE 5 ?
  --------------------------------------------------------------------- */

/**
 copy sid
 @param from sif to copy from
 @return copied sid, need to be free'd with free
 @note null sid's are handled too
*/
PSID copySid(PSID from)
{
    if (!from)
        return 0;
    int sidLength = GetLengthSid(from);
    PSID to = (PSID) malloc(sidLength);
    CopySid(sidLength, to, from);
    return to;
}

/**
 copy sid
 @param from sif to copy from
 @return copied sid, need to be free'd with free
 @note null sid's are handled too
*/
void freeSid(PSID sid)
{
    if (sid)
        free(sid);
}

/**
 copy sid
 @param from sif to copy from
 @return copied sid, need to be free'd with free
 @note null sid's are handled too
*/
QString toString(PSID sid)
{
    LPWSTR s;
    if (!ConvertSidToStringSid(sid, &s))
        return QString();

    QString result = QString::fromUtf16(reinterpret_cast<ushort*>(s));
    LocalFree(s);
    return result;
}

/* --------------------------------------------------------------------
  process helper
  --------------------------------------------------------------------- */

/**
 return process handle
 @param pid process id
 @return process handle
 */
static HANDLE getProcessHandle(int processID)
{
    return OpenProcess( SYNCHRONIZE|PROCESS_QUERY_INFORMATION |
                        PROCESS_VM_READ | PROCESS_TERMINATE,
                        false, processID );
}

/**
 return absolute path of process
 @param pid process id
 @return process name
 */
static QString getProcessName(DWORD pid)
{
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pid );
    if( hModuleSnap == INVALID_HANDLE_VALUE )
        return QString();

    me32.dwSize = sizeof( MODULEENTRY32 );

    if( !Module32First( hModuleSnap, &me32 ) ) {
        CloseHandle( hModuleSnap );           // clean the snapshot object
        return QString();
    }
    QString name = QString::fromWCharArray(me32.szExePath);
    CloseHandle( hModuleSnap );
    return name;
}

/**
 return sid of specific process
 @param hProcess handle to process
 @return sid pointer to PSID structure, must be freed with LocalAlloc
*/
static PSID getProcessOwner(HANDLE hProcess)
{
#ifndef _WIN32_WCE
    HANDLE hToken = NULL;
    PSID sid;

    OpenProcessToken(hProcess, TOKEN_READ, &hToken);
    if(hToken)
    {
        DWORD size;
        PTOKEN_USER userStruct;

        // check how much space is needed
        GetTokenInformation(hToken, TokenUser, NULL, 0, &size);
        if( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            userStruct = reinterpret_cast<PTOKEN_USER>( new BYTE[size] );
            GetTokenInformation(hToken, TokenUser, userStruct, size, &size);

            sid = copySid(userStruct->User.Sid);
            CloseHandle(hToken);
            delete [] userStruct;
            return sid;
        }
    }
#endif
    return 0;
}

/**
 return sid of current process owner
*/
static PSID getCurrentProcessOwner()
{
    return getProcessOwner(GetCurrentProcess());
}

/**
 holds single process
 */
class ProcessListEntry {
    public:
       ProcessListEntry( HANDLE _handle, QString _path, int _pid, PSID _owner=0 )
       {
           QFileInfo p(_path);
           path = p.absolutePath();
           name = p.baseName();
           handle = _handle;
           pid = _pid;
           owner = copySid(_owner);
       }

       ~ProcessListEntry()
       {
           freeSid(owner);
           CloseHandle(handle);
       }

       QString name;
       QString path;
       int pid;
       HANDLE handle;
       PSID owner;
       friend QDebug operator <<(QDebug out, const ProcessListEntry &c);
};

QDebug operator <<(QDebug out, const ProcessListEntry &c)
{
    out << "(ProcessListEntry"
        << "name" << c.name
        << "path" << c.path
        << "pid" << c.pid
        << "handle" << c.handle
        << "sid" << toString(c.owner)
        << ")";
    return out;
}

/**
 holds system process list snapshot

 Could be used as a public platform independent class or namespace in kdecore
 for dealing with system processes, named perhaps KSystemProcessSnapshot or similar.
 If implemented at Qt level it will be named QSystemProcessSnapshot or similar
*/
class ProcessList {
public:
    /**
    collect process
    @param userSid  sid of user for which processes should be collected or 0 for all processes
    */
    ProcessList(PSID userSid=0);

    ~ProcessList();

    /**
    find process in list
    @param name process name (with or without extension)
    @return instance of process entry
    */
    ProcessListEntry *find(const QString &name);

    /**
    killprocess from list
    @param name process name (with or without extension)
    @return ...
    */
    bool terminateProcess(const QString &name);

    /**
    return all processes
    @return list with processes
    */
    QList<ProcessListEntry *> &list() { return m_processes; }

private:
    void init();
    QList<ProcessListEntry *> m_processes;
    PSID m_userId;
};

ProcessList::ProcessList(PSID userSid)
{
    m_userId = userSid;
    init();
}

ProcessList::~ProcessList()
{
    foreach(const ProcessListEntry *ple,m_processes)
        delete ple;
}

void ProcessList::init()
{
    HANDLE h;
    PROCESSENTRY32 pe32;

    h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First( h, &pe32 ))
        return;

    do
    {
        HANDLE hProcess = getProcessHandle(pe32.th32ProcessID);
        if (!hProcess)
            continue;
        QString name = getProcessName(pe32.th32ProcessID);
#ifndef _WIN32_WCE
        PSID sid = getProcessOwner(hProcess);
        if (!sid || m_userId && !EqualSid(m_userId,sid))
        {
            freeSid(sid);
            continue;
        }
#else
        PSID sid = 0;
#endif
        m_processes << new ProcessListEntry( hProcess, name, pe32.th32ProcessID, sid);
    } while(Process32Next( h, &pe32 ));
#ifndef _WIN32_WCE
    CloseHandle(h);
#else
    CloseToolhelp32Snapshot(h);
#endif
}

ProcessListEntry *ProcessList::find(const QString &name)
{
    ProcessListEntry *ple;
    foreach(ple,m_processes) {
        if (ple->pid < 0) {
            qDebug() << "negative pid!";
            continue;
        }

        if (ple->name != name && ple->name != name + ".exe") {
            continue;
        }

        if (!ple->path.isEmpty() && !ple->path.toLower().startsWith(KStandardDirs::installPath("kdedir").toLower())) {
            // process is outside of installation directory
            qDebug() << "path of the process" << name << "seems to be outside of the installPath:" << ple->path << KStandardDirs::installPath("kdedir");
            continue;
        }
        return ple;
    }
    return NULL;
}

bool ProcessList::terminateProcess(const QString &name)
{
    qDebug() << "going to terminate process" << name;
    ProcessListEntry *p = find(name);
    if (!p) {
        qDebug() << "could not find ProcessListEntry for process name" << name;
        return false;
    }

    bool ret = TerminateProcess(p->handle,0);
    if (ret) {
        int i = m_processes.indexOf(p);
        if(i != -1) m_processes.removeAt(i);
        delete p;
        return true;
    } else {
        return false;
    }
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
           fprintf(stderr, "kdeinit5: Launched %s, pid = %ld\n", qPrintable(cmd),(long) pid);
    }
    else {
       if (verbose)
           fprintf(stderr, "kdeinit5: could not launch %s, exiting\n",qPrintable(cmd));
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
    QString installPrefix = KStandardDirs::installPath("kdedir");

    foreach(const ProcessListEntry *ple, processList.list())
    {
        if (!ple->path.isEmpty() && ple->path.toLower().startsWith(installPrefix.toLower()))
            fprintf(stderr,"path: %s name: %s pid: %u\n", ple->path.toLatin1().data(), ple->name.toLatin1().data(), ple->pid);
    }
}

void terminateAllRunningKDEProcesses(ProcessList &processList)
{
    QString installPrefix = KStandardDirs::installPath("kdedir");

    foreach(const ProcessListEntry *ple, processList.list())
    {
        if (!ple->path.isEmpty() && ple->path.toLower().startsWith(installPrefix.toLower()))
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
        if (service.startsWith(QLatin1String("org.freedesktop.DBus")) || service.startsWith(QLatin1Char(':')))
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
        if (service.startsWith(QLatin1String("org.freedesktop.DBus")) || service.startsWith(QLatin1Char(':')))
            continue;
        QDBusInterface *iface = new QDBusInterface(service,
                               QLatin1String("/MainApplication"),
                               QLatin1String("org.kde.KApplication"),
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
            printf("KDE: %s\n", KCOREADDONS_VERSION_STRING);
            exit(0);
        }
        if (strcmp(safe_argv[i], "--help") == 0)
        {
           printf("Usage: kdeinit5 [options]\n");
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

    PSID currentSid = getCurrentProcessOwner();
    if (verbose)
        fprintf(stderr,"current user sid: %s\n",qPrintable(toString(currentSid)));
    ProcessList processList(currentSid);
    freeSid(currentSid);

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

#ifdef _DEBUG
    // first try to launch dbus-daemond in debug mode
    if (launch_dbus && processList.find("dbus-daemond"))
          launch_dbus = false;
    if (launch_dbus)
    {
          pid = launch("dbus-launchd.exe");
          if (!pid)
              pid = launch("dbus-launchd.bat");
          launch_dbus = (pid == 0);
    }
#endif
    if (launch_dbus && !processList.find("dbus-daemon"))
    {
          if (!pid)
              pid = launch("dbus-launch.exe");
          if (!pid)
              pid = launch("dbus-launch.bat");
          if (!pid)
              exit(1);
    }

    if (launch_klauncher && !processList.find("klauncher"))
    {
          pid = launch("klauncher");
          if (!pid || !checkIfRegisteredInDBus("org.kde.klauncher5",10))
              exit(1);
    }


    if (launch_kded && !processList.find(KDED_EXENAME))
    {
        pid = launch(KDED_EXENAME);
        if (!pid || !checkIfRegisteredInDBus("org.kde.kded5",10))
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
