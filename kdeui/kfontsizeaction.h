/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KFONTSIZEACTION_H
#define KFONTSIZEACTION_H

#include <kselectaction.h>

class KDEUI_EXPORT KFontSizeAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY( int fontSize READ fontSize WRITE setFontSize )

public:
    KFontSizeAction(KActionCollection* parent, const QString& name);
    KFontSizeAction(const QString& text, KActionCollection* parent, const QString& name);
    KFontSizeAction(const KIcon& icon, const QString& text, KActionCollection* parent, const QString& name);
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction(const QString& icon, const QString& text, KActionCollection* parent, const QString& name);

    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                     const QString& name = QString() );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const KShortcut& cut, const QObject* receiver,
                     const char* slot, KActionCollection* parent, const QString& name = QString() );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                     KActionCollection* parent = 0, const QString& name = QString() );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                     KActionCollection* parent = 0, const QString& name = QString() );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     KActionCollection* parent, const QString& name = QString() );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     KActionCollection* parent, const QString& name = QString() );

    virtual ~KFontSizeAction();

    int fontSize() const;

    void setFontSize( int size );

Q_SIGNALS:
    void fontSizeChanged( int );

protected Q_SLOTS:
    /**
     * This function is called whenever an action from the selections is triggered.
     */
    virtual void actionTriggered(QAction* action);

private:
    void init();

    class KFontSizeActionPrivate* const d;
};

#endif
