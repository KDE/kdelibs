/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "config.h"
#include "kglobalsettings.h"

#include <qcolor.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qdir.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qfontinfo.h>
#include <qpixmap.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kapplication.h>

#include <kipc.h>

#ifdef Q_WS_WIN
#include <windows.h>
#include "qt_windows.h"
#include <win32_utils.h>
static QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif

#include <kdebug.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kcharsets.h>
#include <kaccel.h>
#include <klocale.h>
#include <stdlib.h>
#include <kprotocolinfo.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

QString* KGlobalSettings::s_desktopPath = 0;
QString* KGlobalSettings::s_autostartPath = 0;
QString* KGlobalSettings::s_trashPath = 0;
QString* KGlobalSettings::s_documentPath = 0;
QFont *KGlobalSettings::_generalFont = 0;
QFont *KGlobalSettings::_fixedFont = 0;
QFont *KGlobalSettings::_toolBarFont = 0;
QFont *KGlobalSettings::_menuFont = 0;
QFont *KGlobalSettings::_windowTitleFont = 0;
QFont *KGlobalSettings::_taskbarFont = 0;
QFont *KGlobalSettings::_largeFont = 0;
QColor *KGlobalSettings::_kde34Blue = 0;
QColor *KGlobalSettings::_inactiveBackground = 0;
QColor *KGlobalSettings::_inactiveForeground = 0;
QColor *KGlobalSettings::_activeBackground = 0;
QColor *KGlobalSettings::_buttonBackground = 0;
QColor *KGlobalSettings::_selectBackground = 0;
QColor *KGlobalSettings::_linkColor = 0;
QColor *KGlobalSettings::_visitedLinkColor = 0;
QColor *KGlobalSettings::alternateColor = 0;

KGlobalSettings::KMouseSettings *KGlobalSettings::s_mouseSettings = 0;

int KGlobalSettings::dndEventDelay()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readNumEntry("StartDragDist", QApplication::startDragDistance());
}

bool KGlobalSettings::singleClick()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
}

KGlobalSettings::TearOffHandle KGlobalSettings::insertTearOffHandle()
{
    int tearoff;
    bool effectsenabled;
    KConfigGroup g( KGlobal::config(), "KDE" );
    effectsenabled = g.readBoolEntry( "EffectsEnabled", false);
    tearoff = g.readNumEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return effectsenabled ? (TearOffHandle) tearoff : Disable;
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

bool KGlobalSettings::visualActivate()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("VisualActivate", KDE_DEFAULT_VISUAL_ACTIVATE);
}

unsigned int KGlobalSettings::visualActivateSpeed()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return
        g.readNumEntry(
            "VisualActivateSpeed",
            KDE_DEFAULT_VISUAL_ACTIVATE_SPEED
        );
}



int KGlobalSettings::autoSelectDelay()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readNumEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    int completion;
    KConfigGroup g( KGlobal::config(), "General" );
    completion = g.readNumEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
        (completion > (int) CompletionPopupAuto))
      {
        completion = (int) CompletionPopup; // Default
      }
  return (Completion) completion;
}

bool KGlobalSettings::showContextMenusOnPress ()
{
    KConfigGroup g(KGlobal::config(), "ContextMenus");
    return g.readBoolEntry("ShowOnPress", true);
}

int KGlobalSettings::contextMenuKey ()
{
    KConfigGroup g(KGlobal::config(), "Shortcuts");
    KShortcut cut (g.readEntry ("PopupMenuContext", "Menu"));
    return cut.keyCodeQt();
}

QColor KGlobalSettings::toolBarHighlightColor()
{
    initColors();
    KConfigGroup g( KGlobal::config(), "Toolbar style" );
    return g.readColorEntry("HighlightColor", _kde34Blue);
}

QColor KGlobalSettings::inactiveTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION));
#else
    if (!_inactiveBackground)
        _inactiveBackground = new QColor(157, 170, 186);
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "inactiveBackground", _inactiveBackground );
#endif
}

QColor KGlobalSettings::inactiveTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
#else
    if (!_inactiveForeground)
       _inactiveForeground = new QColor(221,221,221);
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "inactiveForeground", _inactiveForeground );
#endif
}

QColor KGlobalSettings::activeTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION));
#else
    initColors();
    if (!_activeBackground)
      _activeBackground = new QColor(65,142,220);
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readColorEntry( "activeBackground", _activeBackground);
#endif
}

QColor KGlobalSettings::activeTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    QColor white(Qt::white);
    return g.readColorEntry( "activeForeground", &white );
#endif
}

int KGlobalSettings::contrast()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readNumEntry( "contrast", 7 );
}

QColor KGlobalSettings::buttonBackground()
{
    if (!_buttonBackground)
      _buttonBackground = new QColor(221,223,228);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "buttonBackground", _buttonBackground );
}

QColor KGlobalSettings::buttonTextColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    QColor black(Qt::black);
    return g.readColorEntry( "buttonForeground", &black );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::baseColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    QColor white(Qt::white);
    return g.readColorEntry( "windowBackground", &white );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::textColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    QColor black(Qt::black);
    return g.readColorEntry( "windowForeground", &black );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::highlightedTextColor()
{
    KConfigGroup g( KGlobal::config(), "General" );
    QColor white(Qt::white);
    return g.readColorEntry( "selectForeground", &white );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::highlightColor()
{
    initColors();
    if (!_selectBackground)
        _selectBackground = new QColor(103,141,178);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "selectBackground", _selectBackground );
}

QColor KGlobalSettings::alternateBackgroundColor()
{
    initColors();
    KConfigGroup g( KGlobal::config(), "General" );
    *alternateColor = calculateAlternateBackgroundColor( baseColor() );
    return g.readColorEntry( "alternateBackground", alternateColor );
}

QColor KGlobalSettings::calculateAlternateBackgroundColor(const QColor& base)
{
    if (base == Qt::white)
        return QColor(238,246,255);
    else
    {
        int h, s, v;
        base.getHsv( &h, &s, &v );
        if (v > 128)
            return base.dark(106);
        else if (base != Qt::black)
            return base.light(110);

        return QColor(32,32,32);
    }
}

bool KGlobalSettings::shadeSortColumn()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readBoolEntry( "shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN );
}

QColor KGlobalSettings::linkColor()
{
    initColors();
    if (!_linkColor)
        _linkColor = new QColor(0,0,238);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "linkColor", _linkColor );
}

QColor KGlobalSettings::visitedLinkColor()
{
    if (!_visitedLinkColor)
        _visitedLinkColor = new QColor(82,24,139);
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readColorEntry( "visitedLinkColor", _visitedLinkColor );
}

QFont KGlobalSettings::generalFont()
{
    if (_generalFont)
        return *_generalFont;

    // Sync default with kdebase/kcontrol/fonts/fonts.cpp
    _generalFont = new QFont("Sans Serif", 10);
    _generalFont->setPointSize(10);
    _generalFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_generalFont = g.readFontEntry("font", _generalFont);

    return *_generalFont;
}

QFont KGlobalSettings::fixedFont()
{
    if (_fixedFont)
        return *_fixedFont;

    // Sync default with kdebase/kcontrol/fonts/fonts.cpp
    _fixedFont = new QFont("Monospace", 10);
    _fixedFont->setPointSize(10);
    _fixedFont->setStyleHint(QFont::TypeWriter);

    KConfigGroup g( KGlobal::config(), "General" );
    *_fixedFont = g.readFontEntry("fixed", _fixedFont);

    return *_fixedFont;
}

QFont KGlobalSettings::toolBarFont()
{
    if(_toolBarFont)
        return *_toolBarFont;

    // Sync default with kdebase/kcontrol/fonts/fonts.cpp
    _toolBarFont = new QFont("Sans Serif", 10);
    _toolBarFont->setPointSize(10);
    _toolBarFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_toolBarFont = g.readFontEntry("toolBarFont", _toolBarFont);

    return *_toolBarFont;
}

QFont KGlobalSettings::menuFont()
{
    if(_menuFont)
        return *_menuFont;

    // Sync default with kdebase/kcontrol/fonts/fonts.cpp
    _menuFont = new QFont("Sans Serif", 10);
    _menuFont->setPointSize(10);
    _menuFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_menuFont = g.readFontEntry("menuFont", _menuFont);

    return *_menuFont;
}

QFont KGlobalSettings::windowTitleFont()
{
    if(_windowTitleFont)
        return *_windowTitleFont;

    // Sync default with kdebase/kcontrol/fonts/fonts.cpp
    _windowTitleFont = new QFont("Sans Serif", 9, QFont::Bold);
    _windowTitleFont->setPointSize(10);
    _windowTitleFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "WM" );
    *_windowTitleFont = g.readFontEntry("activeFont", _windowTitleFont); // inconsistency

    return *_windowTitleFont;
}

QFont KGlobalSettings::taskbarFont()
{
    if(_taskbarFont)
        return *_taskbarFont;

    // Sync default with kdebase/kcontrol/fonts/fonts.cpp
    _taskbarFont = new QFont("Sans Serif", 10);
    _taskbarFont->setPointSize(10);
    _taskbarFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_taskbarFont = g.readFontEntry("taskbarFont", _taskbarFont);

    return *_taskbarFont;
}


QFont KGlobalSettings::largeFont(const QString &text)
{
    QFontDatabase db;
    QStringList fam = db.families();

    // Move a bunch of preferred fonts to the front.
    if (fam.remove("Arial"))
       fam.prepend("Arial");
    if (fam.remove("Verdana"))
       fam.prepend("Verdana");
    if (fam.remove("Tahoma"))
       fam.prepend("Tahoma");
    if (fam.remove("Lucida Sans"))
       fam.prepend("Lucida Sans");
    if (fam.remove("Lucidux Sans"))
       fam.prepend("Lucidux Sans");
    if (fam.remove("Nimbus Sans"))
       fam.prepend("Nimbus Sans");
    if (fam.remove("Gothic I"))
       fam.prepend("Gothic I");

    if (_largeFont)
        fam.prepend(_largeFont->family());

    for(QStringList::ConstIterator it = fam.begin();
        it != fam.end(); ++it)
    {
        if (db.isSmoothlyScalable(*it) && !db.isFixedPitch(*it))
        {
            QFont font(*it);
            font.setPixelSize(75);
            QFontMetrics metrics(font);
            int h = metrics.height();
            if ((h < 60) || ( h > 90))
                continue;

            bool ok = true;
            for(int i = 0; i < text.length(); i++)
            {
                if (!metrics.inFont(text[i]))
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            font.setPointSize(48);
            _largeFont = new QFont(font);
            return *_largeFont;
        }
    }
    _largeFont = new QFont(KGlobalSettings::generalFont());
    _largeFont->setPointSize(48);
    return *_largeFont;
}

void KGlobalSettings::initStatic() // should be called initPaths(). Don't put anything else here.
{
    if ( s_desktopPath != 0 )
        return;

    s_desktopPath = new QString();
    s_autostartPath = new QString();
    s_trashPath = new QString();
    s_documentPath = new QString();

    KConfigGroup g( KGlobal::config(), "Paths" );

    // Desktop Path
    *s_desktopPath = QDir::homePath() + "/Desktop/";
    *s_desktopPath = g.readPathEntry( "Desktop", *s_desktopPath);
    *s_desktopPath = QDir::cleanPath( *s_desktopPath );
    if ( !s_desktopPath->endsWith("/") )
      s_desktopPath->append(QLatin1Char('/'));

    // Trash Path - TODO remove in KDE4 (kio_trash can't use it for interoperability reasons)
    *s_trashPath = *s_desktopPath + i18n("Trash") + "/";
    *s_trashPath = g.readPathEntry( "Trash" , *s_trashPath);
    *s_trashPath = QDir::cleanPath( *s_trashPath );
    if ( !s_trashPath->endsWith("/") )
      s_trashPath->append(QLatin1Char('/'));
    // We need to save it in any case, in case the language changes later on,
    if ( !g.hasKey( "Trash" ) )
    {
      g.writePathEntry( "Trash", *s_trashPath, true, true );
      g.sync();
    }

    // Autostart Path
    *s_autostartPath = KGlobal::dirs()->localkdedir() + "Autostart/";
    *s_autostartPath = g.readPathEntry( "Autostart" , *s_autostartPath);
    *s_autostartPath = QDir::cleanPath( *s_autostartPath );
    if ( !s_autostartPath->endsWith("/") )
      s_autostartPath->append(QLatin1Char('/'));

    // Document Path
    *s_documentPath = g.readPathEntry( "Documents",
#ifdef Q_WS_WIN
        getWin32ShellFoldersPath("Personal")
#else
        QDir::homePath()
#endif
    );
    *s_documentPath = QDir::cleanPath( *s_documentPath );
    if ( !s_documentPath->endsWith("/"))
      s_documentPath->append(QLatin1Char('/'));

    // Make sure this app gets the notifications about those paths
    if (kapp)
        kapp->addKipcEventMask(KIPC::SettingsChanged);
}

void KGlobalSettings::initColors()
{
    if (!_kde34Blue) {
      if (QPixmap::defaultDepth() > 8)
        _kde34Blue = new QColor(103,141,178);
      else
        _kde34Blue = new QColor(0, 0, 192);
    }
    if (!alternateColor)
      alternateColor = new QColor(237, 244, 249);
}

void KGlobalSettings::rereadFontSettings()
{
    delete _generalFont;
    _generalFont = 0L;
    delete _fixedFont;
    _fixedFont = 0L;
    delete _menuFont;
    _menuFont = 0L;
    delete _toolBarFont;
    _toolBarFont = 0L;
    delete _windowTitleFont;
    _windowTitleFont = 0L;
    delete _taskbarFont;
    _taskbarFont = 0L;
}

void KGlobalSettings::rereadPathSettings()
{
    kdDebug() << "KGlobalSettings::rereadPathSettings" << endl;
    delete s_autostartPath;
    s_autostartPath = 0L;
    delete s_trashPath;
    s_trashPath = 0L;
    delete s_desktopPath;
    s_desktopPath = 0L;
    delete s_documentPath;
    s_documentPath = 0L;
}

KGlobalSettings::KMouseSettings & KGlobalSettings::mouseSettings()
{
    if ( ! s_mouseSettings )
    {
        s_mouseSettings = new KMouseSettings;
        KMouseSettings & s = *s_mouseSettings; // for convenience

#ifndef Q_WS_WIN
        KConfigGroup g( KGlobal::config(), "Mouse" );
        QString setting = g.readEntry("MouseButtonMapping");
        if (setting == "RightHanded")
            s.handed = KMouseSettings::RightHanded;
        else if (setting == "LeftHanded")
            s.handed = KMouseSettings::LeftHanded;
        else
        {
#ifdef Q_WS_X11
            // get settings from X server
            // This is a simplified version of the code in input/mouse.cpp
            // Keep in sync !
            s.handed = KMouseSettings::RightHanded;
            unsigned char map[20];
            int num_buttons = XGetPointerMapping(QX11Info::display(), map, 20);
            if( num_buttons == 2 )
            {
                if ( (int)map[0] == 1 && (int)map[1] == 2 )
                    s.handed = KMouseSettings::RightHanded;
                else if ( (int)map[0] == 2 && (int)map[1] == 1 )
                    s.handed = KMouseSettings::LeftHanded;
            }
            else if( num_buttons >= 3 )
            {
                if ( (int)map[0] == 1 && (int)map[2] == 3 )
                    s.handed = KMouseSettings::RightHanded;
                else if ( (int)map[0] == 3 && (int)map[2] == 1 )
                    s.handed = KMouseSettings::LeftHanded;
            }
#else
        // FIXME(E): Implement in Qt Embedded
#endif
        }
#endif //Q_WS_WIN
    }
#ifdef Q_WS_WIN
    //not cached
    s_mouseSettings->handed = (GetSystemMetrics(SM_SWAPBUTTON) ? KMouseSettings::LeftHanded : KMouseSettings::RightHanded);
#endif
    return *s_mouseSettings;
}

void KGlobalSettings::rereadMouseSettings()
{
#ifndef Q_WS_WIN
    delete s_mouseSettings;
    s_mouseSettings = 0L;
#endif
}

bool KGlobalSettings::isMultiHead()
{
#ifdef Q_WS_WIN
    return GetSystemMetrics(SM_CMONITORS) > 1;
#else
    QByteArray multiHead = getenv("KDE_MULTIHEAD");
    if (!multiHead.isEmpty()) {
        return (multiHead.toLower() == "true");
    }
    return false;
#endif
}

bool KGlobalSettings::wheelMouseZooms()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry( "WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM );
}

QRect KGlobalSettings::splashScreenDesktopGeometry()
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        int scr = group.readNumEntry("Unmanaged", -3);
        if (group.readBoolEntry("XineramaEnabled", true) && scr != -2) {
            if (scr == -3)
                scr = dw->screenNumber(QCursor::pos());
            return dw->screenGeometry(scr);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

QRect KGlobalSettings::desktopGeometry(const QPoint& point)
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readBoolEntry("XineramaEnabled", true) &&
            group.readBoolEntry("XineramaPlacementEnabled", true)) {
            return dw->screenGeometry(dw->screenNumber(point));
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

QRect KGlobalSettings::desktopGeometry(QWidget* w)
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readBoolEntry("XineramaEnabled", true) &&
            group.readBoolEntry("XineramaPlacementEnabled", true)) {
            if (w)
                return dw->screenGeometry(dw->screenNumber(w));
            else return dw->screenGeometry(-1);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

bool KGlobalSettings::showIconsOnPushButtons()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("ShowIconsOnPushButtons",
        KDE_DEFAULT_ICON_ON_PUSHBUTTON);
}

bool KGlobalSettings::showFilePreview(const KURL &url)
{
    KConfigGroup g(KGlobal::config(), "PreviewSettings");
    QString protocol = url.protocol();
    bool defaultSetting = KProtocolInfo::showFilePreview( protocol );
    return g.readBoolEntry(protocol, defaultSetting );
}

bool KGlobalSettings::opaqueResize()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readBoolEntry("OpaqueResize",
        KDE_DEFAULT_OPAQUE_RESIZE);
}

int KGlobalSettings::buttonLayout()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readNumEntry("ButtonLayout",
        KDE_DEFAULT_BUTTON_LAYOUT);
}
