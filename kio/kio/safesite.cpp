/* This file is part of the KDE project
 *
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "safesite.h"
#include <QApplication>
#include <QTimer>
#include <assert.h>

using namespace SafeSite;

namespace SafeSite {
class Service {
public:
    Service(const QString& name, const QString& url, const QString& details) { info._name = name; info._url = KUrl(url); info._details = details; }
    virtual ~Service() {}

    virtual void cancel(Report *r) = 0;
    virtual void run(Report *r) = 0;
    void report(Report *r) {
        r->serviceReported(this);
    }

    Report::Result result;
    QString reportText;
    QString extendedText;
    QMap<QString, QString> metaData;
    ServiceInfo info;
};


class DummyService : public Service {
public:
    DummyService()
        : Service("Dummy Service", "http://www.example.com/", "Foo") {
    }

    ~DummyService() {
    }

    void cancel(Report *r) {
        Q_UNUSED(r);
    }

    void run(Report *r) {
        const KUrl u = r->url();
        if (u.url() == "http://www.example.com/") {
            result = Report::KnownGood;
            reportText = "Known good site.";
            extendedText = "This site has been verified and has a valid SSL certificate.";
        } else {
            result = Report::Indeterminate;
            reportText = "Site is unknown.";
            extendedText = "Please report back to the dummy service to help build a report and profile for this site.";
        }

        report(r);
    }

};
}

namespace {

static QList<Service*> *servicesList = 0;

static void cleanupServicesList() {
    qDeleteAll(*servicesList);
    delete servicesList;
    servicesList = 0;
}

static QList<Service*> *services() {
    if (!servicesList) {
        servicesList = new QList<Service*>;
        servicesList->append(new DummyService);
        qAddPostRoutine(cleanupServicesList);
    }
    return servicesList;
}

}


namespace SafeSite {
ServiceInfo::ServiceInfo()
    : d(0) {
}


ServiceInfo::~ServiceInfo() {
}


const QString& ServiceInfo::name() const {
    return _name;
}


const KUrl& ServiceInfo::url() const {
    return _url;
}


const QString& ServiceInfo::details() const {
    return _details;
}


ServiceInfo& ServiceInfo::operator=(const ServiceInfo& x) {
    _name = x._name;
    _url = x._url;
    _details = x._details;
    // d = 0;
    return *this;
}


Report::Report()
    : d(0) {
    _currentService = -1;
    _result = Unknown;
}


Report::~Report() {
}


Report::Result Report::result(const QString& service) const {
    if (service.isNull()) {
        return _result;
    }
    return _jobResults[service];
}


QString Report::extendedResultText(const QString& service) const {
    if (service.isNull()) {
        return _extendedResults[services()[0]];
    }
    return _extendedResults[service];
}


QString Report::report(const QString& service) const {
    if (service.isNull()) {
        return _reports[services()[0]];
    }
    return _reports[service];
}


QMap<QString, QString> Report::metaData(const QString& service) const {
    if (service.isNull()) {
        MetaData md;
        foreach (MetaData i, _metaData) {
            md.unite(i);
        }
        return md;
    }
    return _metaData[service];
}


QStringList Report::services() const {
    return Agent::services();
}


ServiceInfo Report::serviceInfo(const QString& service) {
    return Agent::serviceInfo(service);
}


void Report::abort() {
    if (_currentService != -1) {
        Service *s = servicesList->at(_currentService);
        s->cancel(this);
        _currentService = -1;
        _result = Aborted;
    }
}


// FIXME: parallelize this?  What are the implications?
bool Report::run() {
    if (_currentService != -1) {
        return false;
    }
    _result = Working;
    _currentService = 0;
    Service *s = servicesList->at(_currentService);
    s->run(this);
    return true;
}


void Report::serviceReported(Service *s) {
    assert(_currentService >= 0);
    _jobResults[s->info.name()] = s->result;
    _extendedResults[s->info.name()] = s->extendedText;
    _reports[s->info.name()] = s->reportText;
    _metaData[s->info.name()] = s->metaData;
    emit serviceReported(s->info.name());
    QTimer::singleShot(0, this, SLOT(next()));
}


void Report::next() {
    if (_currentService + 1 < servicesList->size()) {
        Service *s = servicesList->at(++_currentService);
        s->run(this);
    } else {
        _currentService = -1;
        _result = Indeterminate;
        uint knownPhish = 0, knownGood = 0, errors = 0;
        for (QMap<QString, Result>::ConstIterator i = _jobResults.begin(); i != _jobResults.end(); ++i) {
            if (i.value() == KnownPhishing) {
                ++knownPhish;
            } else if (i.value() == KnownGood) {
                ++knownGood;
            } else if (i.value() == Error) {
                ++errors;
            }
        }
        if (knownPhish) {
            if (knownGood) {
                _result = Inconsistent;
            } else {
                _result = KnownPhishing;
            }
        } else if (knownGood) {
            _result = KnownGood;
            // FIXME: deal with errors
            // Maybe set this to Indeterminate if there were errors?
            // Could be a DoS attack
        } else if (errors) {
            _result = Error;
        }
        emit done();
    }
}


const KUrl& Report::url() const {
    return _url;
}


Report *Agent::obtainReport(const KUrl& url) {
    Report *r = new Report;
    r->_url = url;
    if (r->run()) {
        return r;
    }

    delete r;
    return 0;
}


QStringList Agent::services() {
    QStringList rc;
    QList<Service*> *s = ::services();
    foreach (Service *i, *s) {
        rc += i->info.name();
    }
    return rc;
}


ServiceInfo Agent::serviceInfo(const QString& service) {
    QList<Service*> *s = ::services();
    foreach (Service *i, *s) {
        if (i->info.name() == service) {
            return i->info;
        }
    }
    return ServiceInfo();
}


Agent::Agent() {
}


Agent::~Agent() {
}

}

#include "safesite.moc"
