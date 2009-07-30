/* This file is part of the KDE libraries
   Copyright (C) 2009 Dario Freddi <drf at kde.org>
   Copyright (C) 2003 Tarkvara Design Inc.  (from KVIrc source code)
   Copyright (c) 2008 Roman Jarosz          <kedgedev at centrum.cz>
   Copyright (c) 2008 the Kopete developers <kopete-devel at kde.org>

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

#include "macpoller.h"

// Why does Apple have to make this so complicated?
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
{
    OSStatus  err;
    FSRef   frameworksFolderRef;
    CFURLRef baseURL;
    CFURLRef bundleURL;

    if (bundlePtr == nil)
        return(-1);

    *bundlePtr = nil;

    baseURL = nil;
    bundleURL = nil;

    err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
    if (err == noErr) {
        baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
        if (baseURL == nil)
            err = coreFoundationUnknownErr;
    }

    if (err == noErr) {
        bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
        if (bundleURL == nil)
            err = coreFoundationUnknownErr;
    }

    if (err == noErr) {
        *bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
        if (*bundlePtr == nil)
            err = coreFoundationUnknownErr;
    }

    if (err == noErr) {
        if (!CFBundleLoadExecutable(*bundlePtr))
            err = coreFoundationUnknownErr;
    }

    // Clean up.
    if (err != noErr && *bundlePtr != nil) {
        CFRelease(*bundlePtr);
        *bundlePtr = nil;
    }

    if (bundleURL != nil)
        CFRelease(bundleURL);

    if (baseURL != nil)
        CFRelease(baseURL);

    return err;
}

pascal void MacPoller::IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void* inUserData)
{
    switch (inState) {
    case kEventLoopIdleTimerStarted:
    case kEventLoopIdleTimerStopped:
        // Get invoked with this constant at the start of the idle period,
        // or whenever user activity cancels the idle.
        ((MacPoller*)inUserData)->m_secondsIdle = 0;
        ((MacPoller*)inUserData)->triggerResume();
        break;
    case kEventLoopIdleTimerIdling:
        // Called every time the timer fires (i.e. every second).
        ((MacPoller*)inUserData)->m_secondsIdle++;
        ((MacPoller*)inUserData)->poll();
        break;
    }
}

// Typedef for the function we're getting back from CFBundleGetFunctionPointerForName.
typedef OSStatus(*InstallEventLoopIdleTimerPtr)(EventLoopRef inEventLoop,
        EventTimerInterval   inFireDelay,
        EventTimerInterval   inInterval,
        EventLoopIdleTimerUPP    inTimerProc,
        void *               inTimerData,
        EventLoopTimerRef *  outTimer);

MacPoller::MacPoller(QWidget *parent)
        : AbstractSystemPoller(parent)
        , m_timerRef(0)
        , m_secondsIdle(0)
        , m_catch(false)
{
}

MacPoller::~MacPoller()
{
}

void MacPoller::unloadPoller()
{
    RemoveEventLoopTimer(m_timerRef);
}

bool MacPoller::isAvailable()
{
    return true;
}

bool MacPoller::setUpPoller()
{
    // May already be init'ed.
    if (m_timerRef) {
        return true;
    }

    // According to the docs, InstallEventLoopIdleTimer is new in 10.2.
    // According to the headers, it has been around since 10.0.
    // One of them is lying.  We'll play it safe and weak-link the function.

    // Load the "Carbon.framework" bundle.
    CFBundleRef carbonBundle;

    if (LoadFrameworkBundle(CFSTR("Carbon.framework"), &carbonBundle) != noErr) {
        return false;
    }

    // Load the Mach-O function pointers for the routine we will be using.
    InstallEventLoopIdleTimerPtr myInstallEventLoopIdleTimer =
        (InstallEventLoopIdleTimerPtr)CFBundleGetFunctionPointerForName(carbonBundle, CFSTR("InstallEventLoopIdleTimer"));

    if (myInstallEventLoopIdleTimer == 0) {
        return false;
    }

    EventLoopIdleTimerUPP timerUPP = NewEventLoopIdleTimerUPP(IdleTimerAction);
    if ((*myInstallEventLoopIdleTimer)(GetMainEventLoop(), kEventDurationSecond, kEventDurationSecond, timerUPP, 0, &m_timerRef)) {
        return true;
    }

    return false;
}

QList<int> MacPoller::timeouts() const
{
    return m_timeouts;
}

void MacPoller::addTimeout(int nextTimeout)
{
    m_timeouts.append(nextTimeout);
    poll();
}

int MacPoller::poll()
{
    int idle = m_secondsIdle * 1000;

    // Check if we reached a timeout..
    foreach(int i, m_timeouts) {
        if ((i - idle < 1000 && i > idle) || (idle - i < 1000 && idle > i)) {
            // Bingo!
            emit timeoutReached(i);
        }
    }

    return idle;
}

int MacPoller::forcePollRequest()
{
    return poll();
}

void MacPoller::removeTimeout(int timeout)
{
    m_timeouts.removeOne(timeout);
    poll();
}

void MacPoller::catchIdleEvent()
{
    m_catch = true;
}

void MacPoller::stopCatchingIdleEvents()
{
    m_catch = false;
}

void MacPoller::triggerResume()
{
    if (m_catch) {
        emit resumingFromIdle();
        stopCatchingIdleEvents();
    }
}

void MacPoller::simulateUserActivity()
{
    // TODO
}

#include "macpoller.moc"
