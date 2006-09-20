/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __KIconLoader_p_h_Included__
#define __KIconLoader_p_h_Included__

#include <qobject.h>
#include <qstringlist.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <qdict.h>

class KIconThemeNode
{
public:
    KIconThemeNode(KIconTheme *_theme);
    ~KIconThemeNode();

    void queryIcons(QStringList *lst, int size, KIcon::Context context) const;
    void queryIconsByContext(QStringList *lst, int size, KIcon::Context context) const;
    KIcon findIcon(const QString& name, int size, KIcon::MatchType match) const;
    void printTree(QString& dbgString) const;

    KIconTheme *theme;
};

class KIconLoaderPrivate : public QObject
{
    Q_OBJECT
public:
    QStringList mThemesInTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconLoader *q;
    KIconEffect mpEffect;
    QDict<QImage> imgDict;
    QImage lastImage; // last loaded image without effect applied
    QString lastImageKey; // key for icon without effect
    QString appname;
    int lastIconType; // see KIcon::type
    int lastIconThreshold; // see KIcon::threshold
    QPtrList<KIconThemeNode> links;
    bool extraDesktopIconsLoaded;
    bool delayedLoading;

public slots:
    void reconfigure();
};

#endif // __KIconLoader_p_h_Included__
