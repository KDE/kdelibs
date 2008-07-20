/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kfilemetainfo_p.h"
#include "kglobal.h"
#include <strigi/streamanalyzer.h>
#include <strigi/analysisresult.h>
#include <strigi/analyzerconfiguration.h>
#include <QDebug>
using namespace Strigi;
using namespace std;

class PredicateProperties::Private : public QSharedData {
public:
    static const QString nullString;
    static const QStringList nullStringList;
    static const PredicateProperties nullPP;
    PredicateProperties parent;
    QString key;
    QString name;
    QVariant::Type type;
    uint attributes;
};
const QString PredicateProperties::Private::nullString;
const QStringList PredicateProperties::Private::nullStringList;
const PredicateProperties PredicateProperties::Private::nullPP;

PredicateProperties::PredicateProperties(const QString& predicate) {
    if (!predicate.isEmpty()) {
        p = new Private();
        p->key = predicate;
    }
}
PredicateProperties::PredicateProperties(const PredicateProperties& pp)
        :p(pp.p) {
}
PredicateProperties::~PredicateProperties() {
}
const PredicateProperties&
PredicateProperties::operator=(const PredicateProperties& pp) {
    p = pp.p;
    return pp;
}
const QString&
PredicateProperties::name() const {
    if (p == 0) return Private::nullString;
    return (p->name.isEmpty()) ?p->key :p->name;
}

const QStringList&
PredicateProperties::suggestedValues() const {
    return Private::nullStringList;
}

uint
PredicateProperties::minCardinality() const {
    return 0;
}

uint
PredicateProperties::maxCardinality() const {
    return 0;
}

uint
PredicateProperties::attributes() const {
    return (p) ?p->attributes :0;
}
QVariant::Type
PredicateProperties::type() const {
    return (p) ?p->type :QVariant::Invalid;
}
QValidator*
PredicateProperties::createValidator() const {
    return 0;
}
const PredicateProperties&
PredicateProperties::parent() const {
    return (p) ?p->parent :Private::nullPP;
}
bool
PredicateProperties::isValid() const {
    return p;
}

/// PredicatePropertyProvider

K_GLOBAL_STATIC(PredicatePropertyProvider, staticPredicatePropertyProvider)

PredicatePropertyProvider*
PredicatePropertyProvider::self() {
    return staticPredicatePropertyProvider;
}
PredicateProperties
PredicatePropertyProvider::getPredicateProperties(const QString& key) {
    PredicateProperties p(key);
    return p;
}

class ShallowAnalysisConfiguration : public Strigi::AnalyzerConfiguration { 
     /** Limit the amount of data we will read from a stream. 
       * This is a suggestion to analyzers which they should follow. 
       * Only index real files. We do not look at subfiles. 
       * This setting is needed because by default all subfiles are examined. 
       **/ 
     int64_t maximalStreamReadLength(const Strigi::AnalysisResult& ar) { 
         // 64k should be enough 
         return (ar.depth() == 0) ?65536 :0; 
     } 
}; 

class PredicatePropertyProvider::Private {
public:
    ShallowAnalysisConfiguration config;
    StreamAnalyzer indexer;
    Private() :indexer(config) {}
};
PredicatePropertyProvider::PredicatePropertyProvider() {
    p = new PredicatePropertyProvider::Private();
}
PredicatePropertyProvider::~PredicatePropertyProvider() {
    delete p;
}
StreamAnalyzer&
PredicatePropertyProvider::indexer() {
    return p->indexer;
}
