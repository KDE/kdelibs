/*
    kcodecaction.cpp

    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyrigth (c) 2006      by Michel Hermier         <michel.hermier@gmail.com>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>
    Copyright (C) 2007 Nick Shaforostoff              <shafff@ukr.net>

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
#include <kglobal.h>

#include <QMenu>
#include <QVariant>
#include <QtCore/QTextCodec>

// Acording to http://www.iana.org/assignments/ianacharset-mib
// the default/unknown mib value is 2.
#define MIB_DEFAULT 2

class KCodecAction::Private
{
public:
    Private()
        : defaultAction(0)
        , currentSubAction(0)
//         , configureAction(0)
    {
    }

    QAction *defaultAction;
    QAction *currentSubAction;
//     QAction *configureAction;
};

KCodecAction::KCodecAction(QObject *parent,bool showAutoOptions)
    : KSelectAction(parent)
    , d(new Private)
{
    init(showAutoOptions);
}

KCodecAction::KCodecAction(const QString &text, QObject *parent,bool showAutoOptions)
    : KSelectAction(text, parent)
    , d(new Private)
{
    init(showAutoOptions);
}

KCodecAction::KCodecAction(const KIcon &icon, const QString &text, QObject *parent,bool showAutoOptions)
    : KSelectAction(icon, text, parent)
    , d(new Private)
{
    init(showAutoOptions);
}

KCodecAction::~KCodecAction()
{
    delete d;
}

void KCodecAction::init(bool showAutoOptions)
{
    setToolBarMode(MenuMode);
    d->defaultAction = addAction(i18n("Default"));
//     if (showAutoOptions)
//         d->defaultAction->setData(QVariant((uint)KEncodingDetector::SemiautomaticDetection));


    //kWarning() << "d->defaultAction.menu()" << (int)d->defaultAction->menu() << endl;
//     int localeCodec=QTextCodec::codecForLocale()->mibEnum();
//     bool success;
    int i;
    foreach(const QStringList &encodingsForScript, KGlobal::charsets()->encodingsByScript())
    {
        KSelectAction* tmp = new KSelectAction(encodingsForScript.at(0),this);
        if (showAutoOptions)
        {
            KEncodingDetector::AutoDetectScript scri=KEncodingDetector::scriptForName(encodingsForScript.at(0));
            if (KEncodingDetector::hasAutoDetectionForScript(scri))
            {
                tmp->addAction(i18nc("Encodings menu","Autodetect"))->setData(QVariant((uint)scri));
                tmp->menu()->addSeparator();
            }
        }
        for (i=1; i<encodingsForScript.size(); ++i)
        {
//            tmp->addAction(encodingsForScript.at(i))/*.setChecked( localeCodec==mibForName(encodingsForScript.at(i),&success))*/;
            //kWarning() << "encodingsForScript.at(i) "<< success << endl;
            tmp->addAction(encodingsForScript.at(i));
        }
        connect(tmp,SIGNAL(triggered(QAction*)),this,SLOT(subActionTriggered(QAction*)));
        tmp->setCheckable(true);
        addAction(tmp);
    }
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
//we don't want to emit any signals from top-level items
//except for the default one
    if (action==d->defaultAction)
    {
        emit triggered(KEncodingDetector::SemiautomaticDetection);
        emit defaultItemTriggered();
    }
/*
    if (action == d->configureAction)
    {
        // Configure the menu content
        return;
    }
*/
}

void KCodecAction::subActionTriggered(QAction *action)
{
    if (d->currentSubAction==action)
        return;
    d->currentSubAction=action;
    bool ok = false;
    int mib = mibForName(action->text(), &ok);
    if (ok)
    {
        emit triggered(action->text());
        emit triggered(codecForMib(mib));
    }
    else
    {
        if (!action->data().isNull())
            emit triggered((KEncodingDetector::AutoDetectScript) action->data().toUInt());
        //emit triggered(action->text());
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
//     if (codec)
//         return setCurrentCodec(QLatin1String(codec->name()));
//     else
//     {
//         kWarning() << k_funcinfo << "Codec is not selectable." << endl;
//         return false;
//     }
    if (!codec)
        return false;

    int i,j;
    for (i=0;i<actions().size();++i)
    {
        if (actions().at(i)->menu())
        {
            for (j=0;j<actions().at(i)->menu()->actions().size();++j)
            {
                if (!j && !actions().at(i)->menu()->actions().at(j)->data().isNull())
                    continue;
//                 kWarning() << "codec " << (int)codec << "  "<< actions().at(i)->menu()->actions().at(j)->text() << (int)KGlobal::charsets()->codecForName(actions().at(i)->menu()->actions().at(j)->text())<<endl;
//                 kWarning() << "codecMib " << codec->mibEnum() << "  "<< KGlobal::charsets()->codecForName(actions().at(i)->menu()->actions().at(j)->text())->mibEnum()<<endl;
                if (codec==KGlobal::charsets()->codecForName(actions().at(i)->menu()->actions().at(j)->text()))
                {
                    d->currentSubAction=actions().at(i)->menu()->actions().at(j);
                    d->currentSubAction->trigger();
                    return true;
                }
            }
        }
    }
    return false;

}

QString KCodecAction::currentCodecName() const
{
    return d->currentSubAction->text();
}

bool KCodecAction::setCurrentCodec( const QString &codecName )
{
    return setCurrentCodec(KGlobal::charsets()->codecForName(codecName));

//     if (setCurrentAction(codecName, Qt::CaseInsensitive))
//         return true;
//// Maybe we got an encoding , not a description name
    //QString name=KGlobal::charsets()->encodingForName(codecName).toLower();
    /*QString name=QTextCodec::codecForName(codecName.toLatin1())->name().toLower();
    kWarning() << "setCurrentCodec. was: " << codecName << " searching for: " << name << endl;

    int i,j;
    for (i=0;i<actions().size();++i)
    {
        if (actions().at(0)->menu())
        {
            for (j=0;j<actions().at(0)->menu()->actions().size();++j)
            {
                if (name==actions().at(0)->menu()->actions().at(j)->text().toLower())
                {
                    d->currentSubAction=actions().at(0)->menu()->actions().at(j);
                    d->currentSubAction->trigger();
                    return true;
                }
            }
        }
    }
    return false;*/
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

KEncodingDetector::AutoDetectScript KCodecAction::currentAutoDetectScript() const
{
    return d->currentSubAction->data().isNull()?
            KEncodingDetector::None            :
            (KEncodingDetector::AutoDetectScript)d->currentSubAction->data().toUInt();
}

bool KCodecAction::setCurrentAutoDetectScript(KEncodingDetector::AutoDetectScript scri)
{
    if (scri==KEncodingDetector::SemiautomaticDetection)
    {
        d->currentSubAction=d->defaultAction;
        d->currentSubAction->trigger();
        return true;
    }

    int i;
    for (i=0;i<actions().size();++i)
    {
        if (actions().at(i)->menu())
        {
            if (!actions().at(i)->menu()->actions().isEmpty()
                 &&!actions().at(i)->menu()->actions().at(0)->data().isNull()
                 &&actions().at(i)->menu()->actions().at(0)->data().toUInt()==(uint)scri
               )
            {
                d->currentSubAction=actions().at(i)->menu()->actions().at(0);
                d->currentSubAction->trigger();
                return true;
            }
        }
    }
    return false;
}


#include "kcodecaction.moc"
