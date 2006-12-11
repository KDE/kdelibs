/*
    kcodecaction.cpp

    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyrigth (c) 2006      by Michel Hermier         <michel.hermier@gmail.com>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kcodecaction.h"

#include <kcharsets.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtextcodec.h>

// Acording to http://www.iana.org/assignments/ianacharset-mib
// the default/unknown mib value is 2. 
#define MIB_DEFAULT 2

class KCodecAction::Private
{
public:
    Private()
        : defaultAction(0)
        , configureAction(0)
    {
    }

    QAction *defaultAction;
    QAction *configureAction;
};

KCodecAction::KCodecAction( KActionCollection *parent, const QString &name )
    : KSelectAction( parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::KCodecAction( const QString &text, KActionCollection *parent, const QString &name )
    : KSelectAction( text, parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::KCodecAction( const KIcon &icon, const QString &text, KActionCollection *parent, const QString &name )
    : KSelectAction( icon, text, parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::~KCodecAction()
{
    delete d;
}

void KCodecAction::init()
{
    d->defaultAction = addAction(i18n("Default"));
    foreach(QString encodingName, KGlobal::charsets()->descriptiveEncodingNames())
       addAction(encodingName);
#if 0
    addSeparator();
    d->configureAction = addAction(i18n("Configure"));
    d->configureAction->setCheckable(false);
#endif
    setCurrentItem(0);

//    setEditable(true);
}

int KCodecAction::mibForName(const QString &codecName, bool *ok) const
{
    // FIXME logic is good but code is ugly

    bool success = false;
    int mib = MIB_DEFAULT;
    KCharsets *charsets = KGlobal::charsets();

    if (codecName == d->defaultAction->text())
        success = true;
    else
    {    
        QTextCodec *codec = charsets->codecForName(codecName, success);
        if (!success)
        {
            // Maybe we got a description name instead
            codec = charsets->codecForName(charsets->encodingForName(codecName), success);
        }
        
        if (codec)
            mib = codec->mibEnum();
    }

    if (ok)
        *ok = success;

    if (success)
        return mib;

    kWarning() << k_funcinfo << "Invalid codec name: "  << codecName << endl;
    return MIB_DEFAULT;
}

QTextCodec *KCodecAction::codecForMib(int mib) const
{
    if (mib == MIB_DEFAULT)
    {
        // FIXME offer to change the default codec
        return QTextCodec::codecForLocale();
    }
    else
        return QTextCodec::codecForMib(mib);
}

void KCodecAction::actionTriggered(QAction *action)
{
#if 0
    if (action == d->configureAction)
    {
        // Configure the menu content
        return;
    }
#endif

    bool ok = false;
    int mib = mibForName(action->text(), &ok);
    if (ok)
    {
        KSelectAction::actionTriggered(action);

//	emit triggeredMib(mib);
        emit triggered(codecForMib(mib));
    }
#if 0
    else
    {
//        Warn the user in the gui somehow ?
    }
#endif
}

QTextCodec *KCodecAction::currentCodec() const
{
    return codecForMib(currentCodecMib());
}

bool KCodecAction::setCurrentCodec( QTextCodec *codec )
{
    if (codec)
        return setCurrentCodec(QLatin1String(codec->name()));
    else
    {
        kWarning() << k_funcinfo << "Codec is not selectable." << endl;
        return false;
    }
}

QString KCodecAction::currentCodecName() const
{
    return currentText();
}

bool KCodecAction::setCurrentCodec( const QString &codecName )
{
    if (setCurrentAction(codecName, Qt::CaseInsensitive))
        return true;

    // Maybe we got an encoding , not a description name
    KCharsets *charsets = KGlobal::charsets();
    if (setCurrentAction(charsets->encodingForName(codecName), Qt::CaseInsensitive))
        return true;

    return false;
}

int KCodecAction::currentCodecMib() const
{
    return mibForName(currentCodecName());
}

bool KCodecAction::setCurrentCodec( int mib )
{
    if (mib == MIB_DEFAULT)
        return setCurrentAction(d->defaultAction);
    else
        return setCurrentCodec(codecForMib(mib));
}

#include "kcodecaction.moc"

