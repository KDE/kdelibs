/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>

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

#include "kglobalsettings.h"
#include <config.h>

#include <kconfig.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcharsets.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kcomponentdata.h>
#include <kcolorscheme.h>

#include <kstyle.h>

#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QDesktopWidget>
#include <QtCore/QDir>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QFontInfo>
#include <QtGui/QKeySequence>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>
//#include <q3stylesheet.h> // no equivalent in Qt4
#include <QApplication>
#include <QtDBus/QtDBus>
#include <QtGui/QStyleFactory>
#include <QDesktopServices>

// next two needed so we can set their palettes
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>

#ifdef Q_WS_WIN
#include <windows.h>
#include <kkernel_win.h>

static QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif
#include "fixx11h.h"
#include <QX11Info>
#endif

#include <stdlib.h>
#include <kconfiggroup.h>

static QFont *_generalFont = 0;
static QFont *_fixedFont = 0;
static QFont *_toolBarFont = 0;
static QFont *_menuFont = 0;
static QFont *_windowTitleFont = 0;
static QFont *_taskbarFont = 0;
static QFont *_largeFont = 0;
static QFont *_smallestReadableFont = 0;
//static QColor *_buttonBackground = 0;
static KGlobalSettings::GraphicEffects _graphicEffects = KGlobalSettings::NoEffects;

static KGlobalSettings::KMouseSettings *s_mouseSettings = 0;

class KGlobalSettings::Private
{
    public:
        Private(KGlobalSettings *q)
            : q(q)
        {
        }

        void _k_slotNotifyChange(int, int);

        void propagateSettings(SettingsCategory category);
        void kdisplaySetPalette();
        void kdisplaySetStyle();
        void kdisplaySetFont();
        void applyGUIStyle();

        /**
         * @internal
         *
         * Ensures that cursors are loaded from the theme KDE is configured
         * to use. Note that calling this function doesn't cause existing
         * cursors to be reloaded. Reloading already created cursors is
         * handled by the KCM when a cursor theme is applied.
         *
         * It is not necessary to call this function when KGlobalSettings
         * is initialized.
         */
        void applyCursorTheme();

        /**
         * drop cached values for fonts
         */
        static void rereadFontSettings();
        /**
         * drop cached values for mouse settings
         */
        static void rereadMouseSettings();
        /**
         * drop cached values for settings that aren't in any of the previous groups
         */
        static void rereadOtherSettings();

        KGlobalSettings *q;
};

KGlobalSettings* KGlobalSettings::self()
{
    K_GLOBAL_STATIC(KGlobalSettings, s_self)
    return s_self;
}

KGlobalSettings::KGlobalSettings()
    : QObject(0), d(new Private(this))
{
    d->kdisplaySetStyle();
    d->kdisplaySetFont();
    d->propagateSettings(SETTINGS_QT);

    QDBusConnection::sessionBus().connect( QString(), "/KGlobalSettings", "org.kde.KGlobalSettings",
                                           "notifyChange", this, SLOT(_k_slotNotifyChange(int,int)) );
}

KGlobalSettings::~KGlobalSettings()
{
    delete d;
}

int KGlobalSettings::dndEventDelay()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readEntry("StartDragDist", QApplication::startDragDistance());
}

bool KGlobalSettings::singleClick()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("SingleClick", KDE_DEFAULT_SINGLECLICK );
}

bool KGlobalSettings::smoothScroll()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("SmoothScroll", KDE_DEFAULT_SMOOTHSCROLL );
}

KGlobalSettings::TearOffHandle KGlobalSettings::insertTearOffHandle()
{
    int tearoff;
    bool effectsenabled;
    KConfigGroup g( KGlobal::config(), "KDE" );
    effectsenabled = g.readEntry( "EffectsEnabled", false);
    tearoff = g.readEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return effectsenabled ? (TearOffHandle) tearoff : Disable;
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

int KGlobalSettings::autoSelectDelay()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    int completion;
    KConfigGroup g( KGlobal::config(), "General" );
    completion = g.readEntry("completionMode", -1);
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
    return g.readEntry("ShowOnPress", true);
}

int KGlobalSettings::contextMenuKey ()
{
    KConfigGroup g(KGlobal::config(), "Shortcuts");
    QString s = g.readEntry ("PopupMenuContext", "Menu");

    // this is a bit of a code duplication with KShortcut,
    // but seeing as that is all in kdeui these days there's little choice.
    // this is faster for what we're really after here anyways
    // (less allocations, only processing the first item always, etc)
    if (s == QLatin1String("none")) {
        return QKeySequence()[0];
    }

    const QStringList shortCuts = s.split(';');

    if (shortCuts.count() < 1) {
        return QKeySequence()[0];
    }

    s = shortCuts.at(0);

    if ( s.startsWith( "default(" ) ) {
        s = s.mid( 8, s.length() - 9 );
    }

    return QKeySequence::fromString(s)[0];
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::inactiveTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "inactiveBackground", QColor(224, 223, 222) );
#endif
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::inactiveTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "inactiveForeground", QColor(20, 19, 18) );
#endif
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::activeTitleColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "activeBackground", QColor(96, 148, 207));
#endif
}

// NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::activeTextColor()
{
#ifdef Q_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT));
#else
    KConfigGroup g( KGlobal::config(), "WM" );
    return g.readEntry( "activeForeground", QColor(255, 255, 255) );
#endif
}

int KGlobalSettings::contrast()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry( "contrast", 7 );
}

qreal KGlobalSettings::contrastF(const KSharedConfigPtr &config)
{
    if (config) {
        KConfigGroup g( config, "KDE" );
        return 0.1 * g.readEntry( "contrast", 7 );
    }
    return 0.1 * (qreal)contrast();
}

bool KGlobalSettings::shadeSortColumn()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readEntry( "shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN );
}

bool KGlobalSettings::allowDefaultBackgroundImages()
{
    KConfigGroup g( KGlobal::config(), "General" );
    return g.readEntry( "allowDefaultBackgroundImages", KDE_DEFAULT_ALLOW_DEFAULT_BACKGROUND_IMAGES );
}

QFont KGlobalSettings::generalFont()
{
    if (_generalFont)
        return *_generalFont;

#ifdef Q_WS_MAC
    _generalFont = new QFont("Lucida Grande", 13);
#else
    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _generalFont = new QFont("Sans Serif", 10);
#endif
    _generalFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_generalFont = g.readEntry("font", *_generalFont);

    return *_generalFont;
}

QFont KGlobalSettings::fixedFont()
{
    if (_fixedFont)
        return *_fixedFont;

#ifdef Q_WS_MAC
    _fixedFont = new QFont("Monaco", 10);
#else
    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _fixedFont = new QFont("Monospace", 10);
#endif
    _fixedFont->setStyleHint(QFont::TypeWriter);

    KConfigGroup g( KGlobal::config(), "General" );
    *_fixedFont = g.readEntry("fixed", *_fixedFont);

    return *_fixedFont;
}

QFont KGlobalSettings::toolBarFont()
{
    if(_toolBarFont)
        return *_toolBarFont;

#ifdef Q_WS_MAC
    _toolBarFont = new QFont("Lucida Grande", 11);
#else
    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _toolBarFont = new QFont("Sans Serif", 8);
#endif
    _toolBarFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_toolBarFont = g.readEntry("toolBarFont", *_toolBarFont);

    return *_toolBarFont;
}

QFont KGlobalSettings::menuFont()
{
    if(_menuFont)
        return *_menuFont;

#ifdef Q_WS_MAC
    _menuFont = new QFont("Lucida Grande", 13);
#else
    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _menuFont = new QFont("Sans Serif", 10);
#endif
    _menuFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_menuFont = g.readEntry("menuFont", *_menuFont);

    return *_menuFont;
}

QFont KGlobalSettings::windowTitleFont()
{
    if(_windowTitleFont)
        return *_windowTitleFont;

    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _windowTitleFont = new QFont("Sans Serif", 9, QFont::Bold);
    _windowTitleFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "WM" );
    *_windowTitleFont = g.readEntry("activeFont", *_windowTitleFont); // inconsistency

    return *_windowTitleFont;
}

QFont KGlobalSettings::taskbarFont()
{
    if(_taskbarFont)
        return *_taskbarFont;

    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _taskbarFont = new QFont("Sans Serif", 10);
    _taskbarFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_taskbarFont = g.readEntry("taskbarFont", *_taskbarFont);

    return *_taskbarFont;
}


QFont KGlobalSettings::largeFont(const QString &text)
{
    QFontDatabase db;
    QStringList fam = db.families();

    // Move a bunch of preferred fonts to the front.
    if (fam.removeAll("Arial")>0)
       fam.prepend("Arial");
    if (fam.removeAll("Sans Serif")>0)
       fam.prepend("Sans Serif");
    if (fam.removeAll("Verdana")>0)
       fam.prepend("Verdana");
    if (fam.removeAll("Tahoma")>0)
       fam.prepend("Tahoma");
    if (fam.removeAll("Lucida Sans")>0)
       fam.prepend("Lucida Sans");
    if (fam.removeAll("Lucidux Sans")>0)
       fam.prepend("Lucidux Sans");
    if (fam.removeAll("Nimbus Sans")>0)
       fam.prepend("Nimbus Sans");
    if (fam.removeAll("Gothic I")>0)
       fam.prepend("Gothic I");

    if (_largeFont)
        fam.prepend(_largeFont->family());

    for(QStringList::ConstIterator it = fam.constBegin();
        it != fam.constEnd(); ++it)
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

QFont KGlobalSettings::smallestReadableFont()
{
    if(_smallestReadableFont)
        return *_smallestReadableFont;

    // NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
    _smallestReadableFont = new QFont("Sans Serif", 8);
    _smallestReadableFont->setStyleHint(QFont::SansSerif);

    KConfigGroup g( KGlobal::config(), "General" );
    *_smallestReadableFont = g.readEntry("smallestReadableFont", *_smallestReadableFont);

    return *_smallestReadableFont;
}

void KGlobalSettings::Private::rereadFontSettings()
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
    delete _smallestReadableFont;
    _smallestReadableFont = 0L;
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
        // FIXME: Implement on other platforms
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

void KGlobalSettings::Private::rereadMouseSettings()
{
#ifndef Q_WS_WIN
    delete s_mouseSettings;
    s_mouseSettings = 0L;
#endif
}

QString KGlobalSettings::desktopPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::DesktopLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

// Autostart is not a XDG path, so we keep with old kdelibs code code
QString KGlobalSettings::autostartPath()
{
    QString s_autostartPath;
    KConfigGroup g( KGlobal::config(), "Paths" );
    s_autostartPath = KGlobal::dirs()->localkdedir() + "Autostart/";
    s_autostartPath = g.readPathEntry( "Autostart" , s_autostartPath );
    s_autostartPath = QDir::cleanPath( s_autostartPath );
    if ( !s_autostartPath.endsWith( '/' ) ) {
        s_autostartPath.append( QLatin1Char( '/' ) );
    }
    return s_autostartPath;
}

QString KGlobalSettings::documentPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::DocumentsLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

QString KGlobalSettings::downloadPath()
{
    // Qt 4.4.1 does not have DOWNLOAD, so we based on old code for now
    QString downloadPath = QDir::homePath();
#ifndef Q_WS_WIN
    QString xdgUserDirs = QDir::homePath() + QLatin1String( "/.config/user-dirs.dirs" );
    if( QFile::exists( xdgUserDirs ) ) {
        KConfig xdgUserConf( xdgUserDirs, KConfig::SimpleConfig );
        KConfigGroup g( &xdgUserConf, "" );
        downloadPath  = g.readPathEntry( "XDG_DOWNLOAD_DIR", downloadPath ).remove(  '"' );
    }
#endif
    downloadPath = QDir::cleanPath( downloadPath );
    if ( !downloadPath.endsWith( '/' ) ) {
        downloadPath.append( QLatin1Char(  '/' ) );
    }
    return downloadPath;
}

QString KGlobalSettings::videosPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::MoviesLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

QString KGlobalSettings::picturesPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::PicturesLocation );
    return path.isEmpty() ? QDir::homePath() :path;
}

QString KGlobalSettings::musicPath()
{
    QString path = QDesktopServices::storageLocation( QDesktopServices::MusicLocation );
    return path.isEmpty() ? QDir::homePath() : path;
}

bool KGlobalSettings::isMultiHead()
{
#ifdef Q_WS_WIN
    return GetSystemMetrics(SM_CMONITORS) > 1;
#else
    QByteArray multiHead = qgetenv("KDE_MULTIHEAD");
    if (!multiHead.isEmpty()) {
        return (multiHead.toLower() == "true");
    }
    return false;
#endif
}

bool KGlobalSettings::wheelMouseZooms()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry( "WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM );
}

QRect KGlobalSettings::splashScreenDesktopGeometry()
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        int scr = group.readEntry("Unmanaged", -3);
        if (group.readEntry("XineramaEnabled", true) && scr != -2) {
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
        if (group.readEntry("XineramaEnabled", true) &&
            group.readEntry("XineramaPlacementEnabled", true)) {
            return dw->screenGeometry(dw->screenNumber(point));
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

QRect KGlobalSettings::desktopGeometry(const QWidget* w)
{
    QDesktopWidget *dw = QApplication::desktop();

    if (dw->isVirtualDesktop()) {
        KConfigGroup group(KGlobal::config(), "Windows");
        if (group.readEntry("XineramaEnabled", true) &&
            group.readEntry("XineramaPlacementEnabled", true)) {
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
    return g.readEntry("ShowIconsOnPushButtons",
                       KDE_DEFAULT_ICON_ON_PUSHBUTTON);
}

KGlobalSettings::GraphicEffects KGlobalSettings::graphicEffectsLevel()
{
    // This variable stores whether _graphicEffects has the default value because it has not been
    // loaded yet, or if it has been loaded from the user settings or defaults and contains a valid
    // value.
    static bool _graphicEffectsInitialized = false;

    if (!_graphicEffectsInitialized) {
        _graphicEffectsInitialized = true;
        Private::rereadOtherSettings();
    }

    return _graphicEffects;
}

KGlobalSettings::GraphicEffects KGlobalSettings::graphicEffectsLevelDefault()
{
    // For now, let always enable animations by default. The plan is to make
    // this code a bit smarter. (ereslibre)

    return ComplexAnimationEffects;
}

bool KGlobalSettings::showFilePreview(const KUrl &url)
{
    KConfigGroup g(KGlobal::config(), "PreviewSettings");
    QString protocol = url.protocol();
    bool defaultSetting = KProtocolInfo::showFilePreview( protocol );
    return g.readEntry(protocol, defaultSetting );
}

bool KGlobalSettings::opaqueResize()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("OpaqueResize", KDE_DEFAULT_OPAQUE_RESIZE);
}

int KGlobalSettings::buttonLayout()
{
    KConfigGroup g( KGlobal::config(), "KDE" );
    return g.readEntry("ButtonLayout", KDE_DEFAULT_BUTTON_LAYOUT);
}

void KGlobalSettings::emitChange(ChangeType changeType, int arg)
{
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange" );
    QList<QVariant> args;
    args.append(static_cast<int>(changeType));
    args.append(arg);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

void KGlobalSettings::Private::_k_slotNotifyChange(int changeType, int arg)
{
    switch(changeType) {
    case StyleChanged:
        KGlobal::config()->reparseConfiguration();
        kdisplaySetStyle();
        break;

    case ToolbarStyleChanged:
        KGlobal::config()->reparseConfiguration();
        emit q->toolbarAppearanceChanged(arg);
        break;

    case PaletteChanged:
        KGlobal::config()->reparseConfiguration();
        kdisplaySetPalette();
        break;

    case FontChanged:
        KGlobal::config()->reparseConfiguration();
        KGlobalSettings::Private::rereadFontSettings();
        kdisplaySetFont();
        break;

    case SettingsChanged: {
        KGlobal::config()->reparseConfiguration();
        rereadOtherSettings();
        SettingsCategory category = static_cast<SettingsCategory>(arg);
        if (category == SETTINGS_MOUSE) {
            KGlobalSettings::Private::rereadMouseSettings();
        }
        propagateSettings(category);
        break;
    }
    case IconChanged:
        QPixmapCache::clear();
        KGlobal::config()->reparseConfiguration();
        emit q->iconChanged(arg);
        break;

    case CursorChanged:
        applyCursorTheme();
        break;

    case BlockShortcuts:
        // FIXME KAccel port
        //KGlobalAccel::blockShortcuts(arg);
        emit q->blockShortcuts(arg); // see kwin
        break;

    default:
        kWarning(101) << "Unknown type of change in KGlobalSettings::slotNotifyChange: " << changeType;
    }
}

// Set by KApplication
QString kde_overrideStyle;

void KGlobalSettings::Private::applyGUIStyle()
{
    const QLatin1String currentStyleName(qApp->style()->metaObject()->className());

    if (kde_overrideStyle.isEmpty()) {
        const QString &defaultStyle = KStyle::defaultStyle();
        const KConfigGroup pConfig(KGlobal::config(), "General");
        const QString &styleStr = pConfig.readEntry("widgetStyle", defaultStyle);

        if (styleStr.isEmpty() ||
                // check whether we already use the correct style to return then
                // (workaround for Qt misbehavior to avoid double style initialization)
                0 == (styleStr + QLatin1String("Style")).compare(currentStyleName, Qt::CaseInsensitive) ||
                0 == styleStr.compare(currentStyleName, Qt::CaseInsensitive)) {
            return;
        }

        QStyle* sp = QStyleFactory::create( styleStr );
        if (currentStyleName == sp->metaObject()->className()) {
            delete sp;
            return;
        }

        // If there is no default style available, try falling back any available style
        if ( !sp && styleStr != defaultStyle)
            sp = QStyleFactory::create( defaultStyle );
        if ( !sp )
            sp = QStyleFactory::create( QStyleFactory::keys().first() );
        qApp->setStyle(sp);
    } else if (0 == kde_overrideStyle.compare(currentStyleName, Qt::CaseInsensitive) ||
            0 == (kde_overrideStyle + QLatin1String("Style")).compare(currentStyleName, Qt::CaseInsensitive)) {
        qApp->setStyle(kde_overrideStyle);
    }
    emit q->kdisplayStyleChanged();
}

QPalette KGlobalSettings::createApplicationPalette(const KSharedConfigPtr &config)
{
    QPalette palette;

    QPalette::ColorGroup states[3] = { QPalette::Active, QPalette::Inactive,
                                       QPalette::Disabled };

    // TT thinks tooltips shouldn't use active, so we use our active colors for all states
    KColorScheme schemeTooltip(QPalette::Active, KColorScheme::Tooltip, config);

    for ( int i = 0; i < 3 ; i++ ) {
        QPalette::ColorGroup state = states[i];
        KColorScheme schemeView(state, KColorScheme::View, config);
        KColorScheme schemeWindow(state, KColorScheme::Window, config);
        KColorScheme schemeButton(state, KColorScheme::Button, config);
        KColorScheme schemeSelection(state, KColorScheme::Selection, config);

        palette.setBrush( state, QPalette::WindowText, schemeWindow.foreground() );
        palette.setBrush( state, QPalette::Window, schemeWindow.background() );
        palette.setBrush( state, QPalette::Base, schemeView.background() );
        palette.setBrush( state, QPalette::Text, schemeView.foreground() );
        palette.setBrush( state, QPalette::Button, schemeButton.background() );
        palette.setBrush( state, QPalette::ButtonText, schemeButton.foreground() );
        palette.setBrush( state, QPalette::Highlight, schemeSelection.background() );
        palette.setBrush( state, QPalette::HighlightedText, schemeSelection.foreground() );
        palette.setBrush( state, QPalette::ToolTipBase, schemeTooltip.background() );
        palette.setBrush( state, QPalette::ToolTipText, schemeTooltip.foreground() );

        palette.setColor( state, QPalette::Light, schemeWindow.shade( KColorScheme::LightShade ) );
        palette.setColor( state, QPalette::Midlight, schemeWindow.shade( KColorScheme::MidlightShade ) );
        palette.setColor( state, QPalette::Mid, schemeWindow.shade( KColorScheme::MidShade ) );
        palette.setColor( state, QPalette::Dark, schemeWindow.shade( KColorScheme::DarkShade ) );
        palette.setColor( state, QPalette::Shadow, schemeWindow.shade( KColorScheme::ShadowShade ) );

        palette.setBrush( state, QPalette::AlternateBase, schemeView.background( KColorScheme::AlternateBackground) );
        palette.setBrush( state, QPalette::Link, schemeView.foreground( KColorScheme::LinkText ) );
        palette.setBrush( state, QPalette::LinkVisited, schemeView.foreground( KColorScheme::VisitedText ) );
    }

    return palette;
}

void KGlobalSettings::Private::kdisplaySetPalette()
{
    // Added by Sam/Harald (TT) for Mac OS X initially, but why?
    KConfigGroup cg( KGlobal::config(), "General" );
    if (cg.readEntry("nopaletteChange", false))
        return;

    if (qApp && qApp->type() == QApplication::GuiClient) {
        QApplication::setPalette( q->createApplicationPalette() );
        emit q->kdisplayPaletteChanged();
        emit q->appearanceChanged();
    }
}


void KGlobalSettings::Private::kdisplaySetFont()
{
    if (qApp && qApp->type() == QApplication::GuiClient) {
        QApplication::setFont(KGlobalSettings::generalFont());
        QApplication::setFont(KGlobalSettings::menuFont(), "QMenuBar");
        QApplication::setFont(KGlobalSettings::menuFont(), "QMenu");
        QApplication::setFont(KGlobalSettings::menuFont(), "KPopupTitle");
        QApplication::setFont(KGlobalSettings::toolBarFont(), "QToolBar");

#if 0
        // "patch" standard QStyleSheet to follow our fonts
        Q3StyleSheet* sheet = Q3StyleSheet::defaultSheet();
        sheet->item (QLatin1String("pre"))->setFontFamily (KGlobalSettings::fixedFont().family());
        sheet->item (QLatin1String("code"))->setFontFamily (KGlobalSettings::fixedFont().family());
        sheet->item (QLatin1String("tt"))->setFontFamily (KGlobalSettings::fixedFont().family());
#endif

        emit q->kdisplayFontChanged();
        emit q->appearanceChanged();
    }
}


void KGlobalSettings::Private::kdisplaySetStyle()
{
    if (qApp && qApp->type() == QApplication::GuiClient) {
        applyGUIStyle();

        // Reread palette from config file.
        kdisplaySetPalette();
    }
}


void KGlobalSettings::Private::rereadOtherSettings()
{
    KConfigGroup g( KGlobal::config(), "KDE-Global GUI Settings" );

    // Asking for hasKey we do not ask for graphicEffectsLevelDefault() that can
    // contain some very slow code. If we can save that time, do it. (ereslibre)

    if (g.hasKey("GraphicEffectsLevel")) {
        _graphicEffects = ((GraphicEffects) g.readEntry("GraphicEffectsLevel", QVariant((int) NoEffects)).toInt());

        return;
    }

    _graphicEffects = KGlobalSettings::graphicEffectsLevelDefault();
}


void KGlobalSettings::Private::applyCursorTheme()
{
#if defined(Q_WS_X11) && defined(HAVE_XCURSOR)
    KConfig config("kcminputrc");
    KConfigGroup g(&config, "Mouse");

    QString theme = g.readEntry("cursorTheme", QString());
    int size      = g.readEntry("cursorSize", -1);

    // Default cursor size is 16 points
    if (size == -1)
    {
        QApplication *app = static_cast<QApplication*>(QApplication::instance());
        size = app->desktop()->screen(0)->logicalDpiY() * 16 / 72;
    }

    // Note that in X11R7.1 and earlier, calling XcursorSetTheme()
    // with a NULL theme would cause Xcursor to use "default", but
    // in 7.2 and later it will cause it to revert to the theme that
    // was configured when the application was started.
    XcursorSetTheme(QX11Info::display(), theme.isNull() ?
                    "default" : QFile::encodeName(theme));
    XcursorSetDefaultSize(QX11Info::display(), size);

    emit q->cursorChanged();
#endif
}


void KGlobalSettings::Private::propagateSettings(SettingsCategory arg)
{
    KConfigGroup cg( KGlobal::config(), "KDE" );

    int num = cg.readEntry("CursorBlinkRate", QApplication::cursorFlashTime());
    if ((num != 0) && (num < 200))
        num = 200;
    if (num > 2000)
        num = 2000;
    QApplication::setCursorFlashTime(num);
    num = cg.readEntry("DoubleClickInterval", QApplication::doubleClickInterval());
    QApplication::setDoubleClickInterval(num);
    num = cg.readEntry("StartDragTime", QApplication::startDragTime());
    QApplication::setStartDragTime(num);
    num = cg.readEntry("StartDragDist", QApplication::startDragDistance());
    QApplication::setStartDragDistance(num);
    num = cg.readEntry("WheelScrollLines", QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    bool b = cg.readEntry("EffectAnimateMenu", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, b);
    b = cg.readEntry("EffectFadeMenu", false);
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, b);
    b = cg.readEntry("EffectAnimateCombo", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, b);
    b = cg.readEntry("EffectAnimateTooltip", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, b);
    b = cg.readEntry("EffectFadeTooltip", false);
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, b);
    //b = !cg.readEntry("EffectNoTooltip", false);
    //QToolTip::setGloballyEnabled( b ); ###

    emit q->settingsChanged(arg);
}

#include "kglobalsettings.moc"
