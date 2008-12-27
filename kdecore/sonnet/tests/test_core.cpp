/**
 *
 * Copyright 2007  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "speller.h"
#include "globals.h"
#include "test_core.h"

#include <qtest_kde.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

QTEST_KDEMAIN_CORE( SonnetCoreTest )

using namespace Sonnet;

void SonnetCoreTest::testCore()
{
    Speller dict("en_US");

    kDebug()<< "Clients are "   << dict.availableBackends();
    kDebug()<< "Languages are " << dict.availableLanguages();
    kDebug()<< "Language names are " << dict.availableLanguageNames();
    kDebug()<< "Language dicts " << dict.availableDictionaries();

    QStringList words;

    words << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted";

    QTime mtime;
    mtime.start();
    for (QStringList::Iterator itr = words.begin();
         itr != words.end(); ++itr) {
        if (!dict.isCorrect(*itr)) {
            //kDebug()<<"Word " << *itr <<" is misspelled";
            QStringList sug = dict.suggest(*itr);
            //kDebug()<<"Suggestions : "<<sug;
        }
    }
    //mtime.stop();
    kDebug()<<"Elapsed time is "<<mtime.elapsed();

    kDebug()<<"Detecting language ...";
    QString sentence = QString::fromLatin1("QClipboard features some convenience functions to access common data types: setText() allows the exchange of Unicode text and setPixmap() and setImage() allows the exchange of QPixmaps and QImages between applications.");
    kDebug()<<"\tlang is "<<Sonnet::detectLanguage(sentence);
}

void SonnetCoreTest::testCore2()
{
    Speller dict("de_DE");
    if (!dict.availableDictionaries().contains("German")) {
        QSKIP("This test requires a german spelling dictionary", SkipSingle);
        return; 
    }
    kDebug()<< "Clients are "   << dict.availableBackends();
    kDebug()<< "Languages are " << dict.availableLanguages();
    kDebug()<< "Language names are " << dict.availableLanguageNames();
    kDebug()<< "Language dicts " << dict.availableDictionaries();

    QStringList words;

    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";
    words << "Hallo" << "halo" << "Umgebunp" << "Regirung" << "bet";

    QTime mtime;
    mtime.start();
    for (QStringList::Iterator itr = words.begin();
         itr != words.end(); ++itr) {
        if (!dict.isCorrect(*itr)) {
            //kDebug()<<"Word " << *itr <<" is misspelled";
            QStringList sug = dict.suggest(*itr);
            //kDebug()<<"Suggestions : "<<sug;
        }
    }
    //mtime.stop();
    kDebug()<<"Elapsed time is "<<mtime.elapsed();

    kDebug()<<"Detecting language ...";
    QString sentence = QString::fromLatin1("Die K Desktop Environment (KDE; auf Deutsch K-Arbeitsumgebung; früher: Kool Desktop Environment) ist eine frei verfügbare Arbeitsumgebung, das heißt eine grafische Benutzeroberfläche mit vielen Zusatzprogrammen für den täglichen Gebrauch.");
    kDebug()<<"\tlang is "<<Sonnet::detectLanguage(sentence);
}

#include "test_core.moc"
