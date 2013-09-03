/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>
    Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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

#include "kmodifierkeyinfo.h"
#include "kmodifierkeyinfoprovider_p.h"

#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <xcb/xcb.h>

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

// same as QX11Info::display, reimplemented to not have to link QtX11Extras
Display *display()
{
    if (!qApp)
        return NULL;
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native)
        return NULL;

    void *display = native->nativeResourceForScreen(QByteArray("display"), QGuiApplication::primaryScreen());
    return reinterpret_cast<Display *>(display);
}

KModifierKeyInfoProvider::KModifierKeyInfoProvider()
    : QObject(0)
    , QAbstractNativeEventFilter()
    , m_xkbEv(0)
    , m_xkbAvailable(false)
{
    if (qApp) {
        if (qApp->platformName() == QStringLiteral("xcb")) {
            int code, xkberr, maj, min;
            m_xkbAvailable = XkbQueryExtension(display(), &code, &m_xkbEv, &xkberr, &maj, &min);
        }
    }
    if (m_xkbAvailable) {
        XkbSelectEvents(display(), XkbUseCoreKbd,
                        XkbStateNotifyMask | XkbMapNotifyMask,
                        XkbStateNotifyMask | XkbMapNotifyMask);
        unsigned long int stateMask = XkbModifierStateMask | XkbModifierBaseMask |
                                      XkbModifierLatchMask | XkbModifierLockMask |
                                      XkbPointerButtonMask;
        XkbSelectEventDetails(display(), XkbUseCoreKbd, XkbStateNotifyMask,
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
        XkbGetState(display(), XkbUseCoreKbd, &state);
        xkbModifierStateChanged(state.mods, state.latched_mods, state.locked_mods);
        xkbButtonStateChanged(state.ptr_buttons);

        QCoreApplication::instance()->installNativeEventFilter(this);
    }
}

KModifierKeyInfoProvider::~KModifierKeyInfoProvider()
{
    if (m_xkbAvailable) {
        QCoreApplication::instance()->removeNativeEventFilter(this);
    }
}

bool KModifierKeyInfoProvider::setKeyLatched(Qt::Key key, bool latched)
{
    if (!m_xkbModifiers.contains(key)) return false;

    return XkbLatchModifiers(display(), XkbUseCoreKbd,
                             m_xkbModifiers[key], latched ? m_xkbModifiers[key] : 0);
}

bool KModifierKeyInfoProvider::setKeyLocked(Qt::Key key, bool locked)
{
    if (!m_xkbModifiers.contains(key)) return false;

    return XkbLockModifiers(display(), XkbUseCoreKbd,
                            m_xkbModifiers[key], locked ? m_xkbModifiers[key] : 0);
}

// HACK: xcb-xkb is not yet a public part of xcb. Because of that we have to include the event structure.
namespace {
    typedef struct _xcb_xkb_map_notify_event_t {
        uint8_t         response_type;
        uint8_t         xkbType;
        uint16_t        sequence;
        xcb_timestamp_t time;
        uint8_t         deviceID;
        uint8_t         ptrBtnActions;
        uint16_t        changed;
        xcb_keycode_t   minKeyCode;
        xcb_keycode_t   maxKeyCode;
        uint8_t         firstType;
        uint8_t         nTypes;
        xcb_keycode_t   firstKeySym;
        uint8_t         nKeySyms;
        xcb_keycode_t   firstKeyAct;
        uint8_t         nKeyActs;
        xcb_keycode_t   firstKeyBehavior;
        uint8_t         nKeyBehavior;
        xcb_keycode_t   firstKeyExplicit;
        uint8_t         nKeyExplicit;
        xcb_keycode_t   firstModMapKey;
        uint8_t         nModMapKeys;
        xcb_keycode_t   firstVModMapKey;
        uint8_t         nVModMapKeys;
        uint16_t        virtualMods;
        uint8_t         pad0[2];
    } _xcb_xkb_map_notify_event_t;
    typedef struct _xcb_xkb_state_notify_event_t {
        uint8_t         response_type;
        uint8_t         xkbType;
        uint16_t        sequence;
        xcb_timestamp_t time;
        uint8_t         deviceID;
        uint8_t         mods;
        uint8_t         baseMods;
        uint8_t         latchedMods;
        uint8_t         lockedMods;
        uint8_t         group;
        int16_t         baseGroup;
        int16_t         latchedGroup;
        uint8_t         lockedGroup;
        uint8_t         compatState;
        uint8_t         grabMods;
        uint8_t         compatGrabMods;
        uint8_t         lookupMods;
        uint8_t         compatLoockupMods;
        uint16_t        ptrBtnState;
        uint16_t        changed;
        xcb_keycode_t   keycode;
        uint8_t         eventType;
        uint8_t         requestMajor;
        uint8_t         requestMinor;
    } _xcb_xkb_state_notify_event_t;
    typedef union {
        /* All XKB events share these fields. */
        struct {
            uint8_t response_type;
            uint8_t xkbType;
            uint16_t sequence;
            xcb_timestamp_t time;
            uint8_t deviceID;
        } any;
        _xcb_xkb_map_notify_event_t map_notify;
        _xcb_xkb_state_notify_event_t state_notify;
    } _xkb_event;
}

bool KModifierKeyInfoProvider::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
{
    Q_UNUSED(result)
    if (!m_xkbAvailable || eventType != "xcb_generic_event_t") {
        return false;
    }
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t*>(message);
    if ((event->response_type & ~0x80) == m_xkbEv + XkbEventCode) {
        _xkb_event *kbevt = reinterpret_cast<_xkb_event *>(event);
        unsigned int stateMask = XkbModifierStateMask | XkbModifierBaseMask |
                                 XkbModifierLatchMask | XkbModifierLockMask;
        if (kbevt->any.xkbType == XkbMapNotify) {
            xkbUpdateModifierMapping();
        } else if (kbevt->any.xkbType == XkbStateNotify) {
            if (kbevt->state_notify.changed & stateMask) {
                xkbModifierStateChanged(kbevt->state_notify.mods, kbevt->state_notify.latchedMods,
                                        kbevt->state_notify.lockedMods);
            } else if (kbevt->state_notify.changed & XkbPointerButtonMask) {
                xkbButtonStateChanged(kbevt->state_notify.ptrBtnState);
            }
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
    if (!m_xkbAvailable) {
        return;
    }
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

    XkbDescPtr xkb = XkbGetKeyboard(display(), XkbAllComponentsMask, XkbUseCoreKbd);

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
                mask = XkbKeysymToModifiers(display(), it->keysym);
            } else if (mask == 0) {
                // special case for AltGr
                mask = XkbKeysymToModifiers(display(), XK_Mode_switch) |
                       XkbKeysymToModifiers(display(), XK_ISO_Level3_Shift) |
                       XkbKeysymToModifiers(display(), XK_ISO_Level3_Latch) |
                       XkbKeysymToModifiers(display(), XK_ISO_Level3_Lock);
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
