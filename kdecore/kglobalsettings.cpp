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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kglobalsettings.h"

#include <qdir.h>
#include <qpixmap.h>
#include <qfontdatabase.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kapplication.h>
#include <kipc.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kcharsets.h>
#include <kaccel.h>
#include <klocale.h>
#include <qfontinfo.h>
#include <stdlib.h>

#ifdef Q_WS_X11
//#include <X11/X.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
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
QColor *KGlobalSettings::kde2Blue = 0;
QColor *KGlobalSettings::kde2Gray = 0;
QColor *KGlobalSettings::kde2AlternateColor = 0;

KGlobalSettings::KMouseSettings *KGlobalSettings::s_mouseSettings = 0;

int KGlobalSettings::dndEventDelay()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "General" );
    return c->readNumEntry("StartDragDist", QApplication::startDragDistance());
}

bool KGlobalSettings::singleClick()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
}

KGlobalSettings::TearOffHandle KGlobalSettings::insertTearOffHandle()
{
    int tearoff;
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    tearoff =c->readNumEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return (TearOffHandle) tearoff;
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

bool KGlobalSettings::visualActivate()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("VisualActivate", KDE_DEFAULT_VISUAL_ACTIVATE);
}

unsigned int KGlobalSettings::visualActivateSpeed()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return
        c->readNumEntry(
            "VisualActivateSpeed",
            KDE_DEFAULT_VISUAL_ACTIVATE_SPEED
        );
}



int KGlobalSettings::autoSelectDelay()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readNumEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    int completion;
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "General" );
    completion = c->readNumEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
        (completion > (int) CompletionPopupAuto))
      {
        completion = (int) CompletionPopup; // Default
      }
  return (Completion) completion;
}

bool KGlobalSettings::showContextMenusOnPress ()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs (c, "ContextMenus");

    return cgs.config()->readBoolEntry("ShowOnPress", true);
}

int KGlobalSettings::contextMenuKey ()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs (c, "Shortcuts");

    KShortcut cut (cgs.config()->readEntry ("PopupMenuContext", "Menu"));
    return cut.keyCodeQt();
}

QColor KGlobalSettings::toolBarHighlightColor()
{
    initColors();
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("Toolbar style") );
    return c->readColorEntry("HighlightColor", kde2Blue);
}

QColor KGlobalSettings::inactiveTitleColor()
{
    if (!kde2Gray)
        kde2Gray = new QColor(220, 220, 220);
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("WM") );
    return c->readColorEntry( "inactiveBackground", kde2Gray );
}

QColor KGlobalSettings::inactiveTextColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("WM") );
    return c->readColorEntry( "inactiveForeground", &Qt::darkGray );
}

QColor KGlobalSettings::activeTitleColor()
{
    initColors();
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("WM") );
    return c->readColorEntry( "activeBackground", kde2Blue);
}

QColor KGlobalSettings::activeTextColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("WM") );
    return c->readColorEntry( "activeForeground", &Qt::white );
}

int KGlobalSettings::contrast()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("KDE") );
    return c->readNumEntry( "contrast", 7 );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::baseColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    return c->readColorEntry( "windowBackground", &Qt::white );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::textColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    return c->readColorEntry( "windowForeground", &Qt::black );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::highlightedTextColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    return c->readColorEntry( "selectForeground", &Qt::white );
}

// IMPORTANT:
//  This function should be get in sync with
//   KApplication::kdisplaySetPalette()
QColor KGlobalSettings::highlightColor()
{
    initColors();
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    return c->readColorEntry( "selectBackground", kde2Blue );
}

QColor KGlobalSettings::alternateBackgroundColor()
{
    initColors();
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    *kde2AlternateColor = calculateAlternateBackgroundColor( baseColor() );
    return c->readColorEntry( "alternateBackground", kde2AlternateColor );
}

QColor KGlobalSettings::calculateAlternateBackgroundColor(const QColor& base)
{
    if (base == Qt::white)
        return QColor(238,246,255);
    else
    {
        int h, s, v;
        base.hsv( &h, &s, &v );
        if (v > 128)
            return base.dark(106);
        else if (base != Qt::black)
            return base.light(110);

        return QColor(32,32,32);
    }
}

QColor KGlobalSettings::linkColor()
{
    initColors();
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    return c->readColorEntry( "linkColor", kde2Blue );
}

QColor KGlobalSettings::visitedLinkColor()
{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    return c->readColorEntry( "visitedLinkColor", &Qt::magenta );
}

QFont KGlobalSettings::generalFont()
{
    if (_generalFont)
        return *_generalFont;

    _generalFont = new QFont("helvetica", 12);
    _generalFont->setPixelSize(12);
    _generalFont->setStyleHint(QFont::SansSerif);

    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    *_generalFont = c->readFontEntry("font", _generalFont);

    return *_generalFont;
}

QFont KGlobalSettings::fixedFont()
{
    if (_fixedFont)
        return *_fixedFont;

    _fixedFont = new QFont("courier", 12);
    _fixedFont->setPixelSize(12);
    _fixedFont->setStyleHint(QFont::TypeWriter);

    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    *_fixedFont = c->readFontEntry("fixed", _fixedFont);

    return *_fixedFont;
}

QFont KGlobalSettings::toolBarFont()
{
    if(_toolBarFont)
        return *_toolBarFont;

    _toolBarFont = new QFont("helvetica", 10);
    _toolBarFont->setPixelSize(10);
    _toolBarFont->setStyleHint(QFont::SansSerif);

    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    *_toolBarFont = c->readFontEntry("toolBarFont", _toolBarFont);

    return *_toolBarFont;
}

QFont KGlobalSettings::menuFont()
{
    if(_menuFont)
        return *_menuFont;

    _menuFont = new QFont("helvetica", 12);
    _menuFont->setPixelSize(12);
    _menuFont->setStyleHint(QFont::SansSerif);

    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    *_menuFont = c->readFontEntry("menuFont", _menuFont);

    return *_menuFont;
}

QFont KGlobalSettings::windowTitleFont()
{
    if(_windowTitleFont)
        return *_windowTitleFont;

    _windowTitleFont = new QFont("helvetica", 12, QFont::Bold);
    _windowTitleFont->setPixelSize(12);
    _windowTitleFont->setStyleHint(QFont::SansSerif);

    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("WM") );
    *_windowTitleFont = c->readFontEntry("activeFont", _windowTitleFont); // inconsistency

    return *_windowTitleFont;
}

QFont KGlobalSettings::taskbarFont()
{
    if(_taskbarFont)
        return *_taskbarFont;

    _taskbarFont = new QFont("helvetica", 11);
    _taskbarFont->setPixelSize(11);
    _taskbarFont->setStyleHint(QFont::SansSerif);

    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, QString::fromLatin1("General") );
    *_taskbarFont = c->readFontEntry("taskbarFont", _taskbarFont);

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
            for(unsigned int i = 0; i < text.length(); i++)
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

    KConfig *config = KGlobal::config();
    bool dollarExpansion = config->isDollarExpansion();
    config->setDollarExpansion(true);
    KConfigGroupSaver cgs( config, "Paths" );

    // Desktop Path
    *s_desktopPath = QDir::homeDirPath() + "/" + "Desktop" + "/";
    *s_desktopPath = config->readEntry( "Desktop", *s_desktopPath);
    if ( (*s_desktopPath)[0] != '/' )
      s_desktopPath->prepend( QDir::homeDirPath() + "/" );
    *s_desktopPath = QDir::cleanDirPath( *s_desktopPath );
    if ( s_desktopPath->right(1) != "/")
        *s_desktopPath += "/";

    // Trash Path
    *s_trashPath = *s_desktopPath + i18n("Trash") + "/";
    *s_trashPath = config->readEntry( "Trash" , *s_trashPath);
    if ( (*s_trashPath)[0] != '/' )
      s_trashPath->prepend( QDir::homeDirPath() + "/" );
    *s_trashPath = QDir::cleanDirPath( *s_trashPath );
    if ( s_trashPath->right(1) != "/")
        *s_trashPath += "/";
    // We need to save it in any case, in case the language changes later on,
    if ( !config->hasKey( "Trash" ) )
    {
      config->writePathEntry( "Trash", *s_trashPath, true, true );
      config->sync();
    }

    // Autostart Path
    *s_autostartPath = KGlobal::dirs()->localkdedir() + "Autostart" + "/";
    *s_autostartPath = config->readEntry( "Autostart" , *s_autostartPath);
    if ( (*s_autostartPath)[0] != '/' )
      s_autostartPath->prepend( QDir::homeDirPath() + "/" );
    *s_autostartPath = QDir::cleanDirPath( *s_autostartPath );
    if ( s_autostartPath->right(1) != "/")
        *s_autostartPath += "/";

    // Document Path
    *s_documentPath = QString::null;
    *s_documentPath = config->readEntry( "Documents" , *s_documentPath);
    if ( (*s_documentPath)[0] != '/' )
      s_documentPath->prepend( QDir::homeDirPath() + "/" );
    *s_documentPath = QDir::cleanDirPath( *s_documentPath );
    if ( s_documentPath->right(1) != "/")
        *s_documentPath += "/";

    config->setDollarExpansion(dollarExpansion);

    // Make sure this app gets the notifications about those paths
    if (kapp)
        kapp->addKipcEventMask(KIPC::SettingsChanged);
}

void KGlobalSettings::initColors()
{
    if (!kde2Blue) {
      if (QPixmap::defaultDepth() > 8)
        kde2Blue = new QColor(84, 112, 152);
      else
        kde2Blue = new QColor(0, 0, 192);
    }
    if (!kde2AlternateColor)
      kde2AlternateColor = new QColor(240, 240, 240);
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

        KConfigGroupSaver cgs( KGlobal::config(), "Mouse" );
        QString setting = KGlobal::config()->readEntry("MouseButtonMapping");
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
            unsigned char map[5];
            switch (XGetPointerMapping(kapp->getDisplay(), map, 5))
            {
                case 2:
                    if ( (int)map[0] == 1 && (int)map[1] == 2 )
                        s.handed = KMouseSettings::RightHanded;
                    else if ( (int)map[0] == 2 && (int)map[1] == 1 )
                        s.handed = KMouseSettings::LeftHanded;
                    break;
                case 3:
                case 5:
                    if ( (int)map[0] == 1 && (int)map[2] == 3 )
                        s.handed = KMouseSettings::RightHanded;
                    else if ( (int)map[0] == 3 && (int)map[2] == 1 )
                        s.handed = KMouseSettings::LeftHanded;
                    break;
                default:
                    break;
            }
#else
	    // FIXME(E): Implement in Qt Embedded
#endif
        }
    }
    return *s_mouseSettings;
}

void KGlobalSettings::rereadMouseSettings()
{
    delete s_mouseSettings;
    s_mouseSettings = 0L;
}

bool KGlobalSettings::isMultiHead()
{
    QCString multiHead = getenv("KDE_MULTIHEAD");
    if (!multiHead.isEmpty()) {
        return (multiHead.lower() == "true");
    }
    return false;
}

bool KGlobalSettings::wheelMouseZooms()
{
    KConfigGroup group( KGlobal::config(), "KDE" );
    return group.readBoolEntry( "WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM );
}
