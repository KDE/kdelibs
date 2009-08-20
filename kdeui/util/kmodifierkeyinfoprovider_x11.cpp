/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QX11Info>

#include "kmodifierkeyinfo.h"
#include "kmodifierkeyinfoprovider_p.h"

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

struct ModifierDefinition
{
    ModifierDefinition( Qt::Key _key, unsigned int _mask, const char * _name, KeySym _keysym ) {
       key = _key;
       mask = _mask;
       name = _name;
       keysym = _keysym;
    }
    Qt::Key key;
    unsigned int mask;
    const char *name; // virtual modifier name
    KeySym keysym;
};

/*
 * Get the real modifiers related to a virtual modifier.
 */
unsigned int xkbVirtualModifier(XkbDescPtr xkb, const char *name)
{
    Q_ASSERT(xkb != 0);

    unsigned int mask = 0;
    bool nameEqual;
    for (int i = 0; i < XkbNumVirtualMods; ++i) {
        char *modStr = XGetAtomName(xkb->dpy, xkb->names->vmods[i]);
        if (modStr != 0) {
            nameEqual = (strcmp(name, modStr) == 0);
            XFree(modStr);
            if (nameEqual) {
                XkbVirtualModsToReal(xkb, 1 << i, &mask);
                break;
            }
        }
    }
    return mask;
}

/*
 * Event filter to receive events from QAbstractEventDispatcher. All X11 events
 * are forwarded to all providers.
 */
bool kmodifierKeyInfoEventFilter(void *message)
{
    if (KModifierKeyInfoProvider::s_eventFilterEnabled) {
        XEvent *evt = reinterpret_cast<XEvent*>(message);
        if (evt) {
            QSet<KModifierKeyInfoProvider*>::const_iterator it =
                KModifierKeyInfoProvider::s_providerList.constBegin();
            QSet<KModifierKeyInfoProvider*>::const_iterator end =
                KModifierKeyInfoProvider::s_providerList.constEnd();
            for ( ; it != end; ++it) {
                if ((*it)->x11Event(evt)) {
                    // providers usually return don't consume events and return false.
                    // If under any circumstance an event is consumed, don't forward it to
                    // other event filters.
                    return true;
                }
            }
        }
    }
    
    if (KModifierKeyInfoProvider::s_nextFilter) {
        return KModifierKeyInfoProvider::s_nextFilter(message);
    }
    
    return false;
}

QSet<KModifierKeyInfoProvider*> KModifierKeyInfoProvider::s_providerList;
bool KModifierKeyInfoProvider::s_eventFilterInstalled = false;
bool KModifierKeyInfoProvider::s_eventFilterEnabled = false;
QAbstractEventDispatcher::EventFilter KModifierKeyInfoProvider::s_nextFilter = 0;

KModifierKeyInfoProvider::KModifierKeyInfoProvider()
    : QWidget(0)
{
    int code, xkberr, maj, min;
    m_xkbAvailable = XkbQueryExtension(QX11Info::display(), &code, &m_xkbEv, &xkberr, &maj, &min);
    if (m_xkbAvailable) {
        XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd,
                        XkbStateNotifyMask | XkbMapNotifyMask,
                        XkbStateNotifyMask | XkbMapNotifyMask);
        unsigned long int stateMask = XkbModifierStateMask | XkbModifierBaseMask |
                                      XkbModifierLatchMask | XkbModifierLockMask |
                                      XkbPointerButtonMask;
        XkbSelectEventDetails(QX11Info::display(), XkbUseCoreKbd, XkbStateNotifyMask,
                              stateMask, stateMask);
    }

    xkbUpdateModifierMapping();

    // add known pointer buttons
    m_xkbButtons.insert(Qt::LeftButton, Button1Mask);
    m_xkbButtons.insert(Qt::MidButton, Button2Mask);
    m_xkbButtons.insert(Qt::RightButton, Button3Mask);
    m_xkbButtons.insert(Qt::XButton1, Button4Mask);
    m_xkbButtons.insert(Qt::XButton2, Button5Mask);

    // get the initial state
    if (m_xkbAvailable) {
        XkbStateRec state;
        XkbGetState(QX11Info::display(), XkbUseCoreKbd, &state);
        xkbModifierStateChanged(state.mods, state.latched_mods, state.locked_mods);
        xkbButtonStateChanged(state.ptr_buttons);
    }
    
    if (!s_eventFilterInstalled) {
        // This is the first provider constructed. Install the event filter.
        s_nextFilter = QAbstractEventDispatcher::instance()->setEventFilter(kmodifierKeyInfoEventFilter);
        s_eventFilterInstalled = true;
    }
    s_eventFilterEnabled = true;
    s_providerList.insert(this);
}

KModifierKeyInfoProvider::~KModifierKeyInfoProvider()
{
    s_providerList.remove(this);
    if (s_providerList.isEmpty()) {
        // disable filtering events
        s_eventFilterEnabled = false;
    }
}

bool KModifierKeyInfoProvider::setKeyLatched(Qt::Key key, bool latched)
{
    if (!m_xkbModifiers.contains(key)) return false;

    return XkbLatchModifiers(QX11Info::display(), XkbUseCoreKbd,
                             m_xkbModifiers[key], latched ? m_xkbModifiers[key] : 0);
}

bool KModifierKeyInfoProvider::setKeyLocked(Qt::Key key, bool locked)
{
    if (!m_xkbModifiers.contains(key)) return false;

    return XkbLockModifiers(QX11Info::display(), XkbUseCoreKbd,
                            m_xkbModifiers[key], locked ? m_xkbModifiers[key] : 0);
}

bool KModifierKeyInfoProvider::x11Event(XEvent *event)
{
    if (m_xkbAvailable) {
        XkbEvent *kbevt;
        unsigned int stateMask = XkbModifierStateMask | XkbModifierBaseMask |
                                 XkbModifierLatchMask | XkbModifierLockMask;
        if (event->type == m_xkbEv + XkbEventCode &&
            (kbevt = (XkbEvent*)event) != 0)
        {
            if (kbevt->any.xkb_type == XkbMapNotify) {
                xkbUpdateModifierMapping();
            } else if (kbevt->any.xkb_type == XkbStateNotify) {
                XkbStateNotifyEvent *snevent = (XkbStateNotifyEvent*)event;
                if (snevent->changed & stateMask) {
                    xkbModifierStateChanged(snevent->mods, snevent->latched_mods,
                                            snevent->locked_mods);
                } else if (snevent->changed & XkbPointerButtonMask) {
                    xkbButtonStateChanged(snevent->ptr_buttons);
                }
            }
            return false;
        }
    }

    return false;
}

void KModifierKeyInfoProvider::xkbModifierStateChanged(unsigned char mods,
                                                       unsigned char latched_mods,
                                                       unsigned char locked_mods)
{
    // detect keyboard modifiers
    ModifierStates oldState;
    ModifierStates newState;
    
    QHash<Qt::Key, unsigned int>::const_iterator it;
    QHash<Qt::Key, unsigned int>::const_iterator end = m_xkbModifiers.constEnd();
    for (it = m_xkbModifiers.constBegin(); it != end; ++it) {
        if (!m_modifierStates.contains(it.key())) continue;
        newState = Nothing;
        oldState = m_modifierStates[it.key()];

        // determine the new state
        if (mods & it.value()) {
            newState |= Pressed;
        }
        if (latched_mods & it.value()) {
            newState |= Latched;
        }
        if (locked_mods & it.value()) {
            newState |= Locked;
        }

        if (newState != oldState) {
            m_modifierStates[it.key()] = newState;

            if ((newState ^ oldState) & Pressed) {
                emit keyPressed(it.key(), newState & Pressed);
            }
            if ((newState ^ oldState) & Latched) {
                emit keyLatched(it.key(), newState & Latched);
            }
            if ((newState ^ oldState) & Locked) {
                emit keyLocked(it.key(), newState & Locked);
            }
        }
    }
}

void KModifierKeyInfoProvider::xkbButtonStateChanged(unsigned short ptr_buttons)
{
    // detect mouse button states
    bool newButtonState;

    QHash<Qt::MouseButton, unsigned short>::const_iterator it;
    QHash<Qt::MouseButton, unsigned short>::const_iterator end = m_xkbButtons.constEnd();
    for (it = m_xkbButtons.constBegin(); it != end; ++it) {
        newButtonState = (ptr_buttons & it.value());
        if (newButtonState != m_buttonStates[it.key()]) {
            m_buttonStates[it.key()] = newButtonState;
            emit buttonPressed(it.key(), newButtonState);
        }
    }
}

void KModifierKeyInfoProvider::xkbUpdateModifierMapping()
{
    m_xkbModifiers.clear();

    QList<ModifierDefinition> srcModifiers;
    srcModifiers << ModifierDefinition(Qt::Key_Shift, ShiftMask, 0, 0)
                 << ModifierDefinition( Qt::Key_Control, ControlMask, 0, 0)
                 << ModifierDefinition(Qt::Key_Alt, 0, "Alt", XK_Alt_L)
                 // << { 0, 0, I18N_NOOP("Win"), "superkey", "" }
                 << ModifierDefinition(Qt::Key_Meta, 0, "Meta", XK_Meta_L)
                 << ModifierDefinition(Qt::Key_Super_L, 0, "Super", XK_Super_L)
                 << ModifierDefinition(Qt::Key_Hyper_L, 0, "Hyper", XK_Hyper_L)
                 << ModifierDefinition(Qt::Key_AltGr, 0, "AltGr", 0)
                 << ModifierDefinition(Qt::Key_NumLock, 0, "NumLock", XK_Num_Lock)
                 << ModifierDefinition(Qt::Key_CapsLock, LockMask, 0, 0)
                 << ModifierDefinition( Qt::Key_ScrollLock, 0, "ScrollLock", XK_Scroll_Lock);

    XkbDescPtr xkb = XkbGetKeyboard(QX11Info::display(), XkbAllComponentsMask, XkbUseCoreKbd);

    QList<ModifierDefinition>::const_iterator it;
    QList<ModifierDefinition>::const_iterator end = srcModifiers.constEnd();
    for (it = srcModifiers.constBegin(); it != end; ++it) {
        unsigned int mask = it->mask;
        if (mask == 0 && xkb != 0) {
            // try virtual modifier first
            if (it->name != 0) {
                mask = xkbVirtualModifier(xkb, it->name);
            }
            if (mask == 0 && it->keysym != 0) {
                mask = XkbKeysymToModifiers(QX11Info::display(), it->keysym);
            } else if (mask == 0) {
                // special case for AltGr
                mask = XkbKeysymToModifiers(QX11Info::display(), XK_Mode_switch) |
                       XkbKeysymToModifiers(QX11Info::display(), XK_ISO_Level3_Shift) |
                       XkbKeysymToModifiers(QX11Info::display(), XK_ISO_Level3_Latch) |
                       XkbKeysymToModifiers(QX11Info::display(), XK_ISO_Level3_Lock);
            }
        }

        if (mask != 0) {
            m_xkbModifiers.insert(it->key, mask);
            // previously unknown modifier
            if (!m_modifierStates.contains(it->key)) {
                m_modifierStates.insert(it->key, Nothing);
                emit keyAdded(it->key);
            }
        }
    }

    // remove modifiers which are no longer available
    QMutableHashIterator<Qt::Key, ModifierStates> i(m_modifierStates);
    while (i.hasNext()) {
        i.next();
        if (!m_xkbModifiers.contains(i.key())) {
            Qt::Key key = i.key();
            i.remove();
            emit keyRemoved(key);
        }
    }

    if (xkb != 0) {
        XkbFreeKeyboard(xkb, 0, true);
    }
}
