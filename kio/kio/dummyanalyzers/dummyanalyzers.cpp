/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
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
//#include <strigi/strigiconfig.h>
#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>
#include <strigi/streamsaxanalyzer.h>
#include <strigi/streamthroughanalyzer.h>
#include <strigi/streamlineanalyzer.h>
#include <strigi/streameventanalyzer.h>
#include <config-strigi.h>

using namespace Strigi;
using namespace std;

class DummyEndAnalyzerFactory;
class DummyThroughAnalyzerFactory;
class DummySaxAnalyzerFactory;
class DummyLineAnalyzerFactory;
class DummyEventAnalyzerFactory;

class STRIGI_PLUGIN_API DummyEndAnalyzer : public StreamEndAnalyzer {
public:
    DummyEndAnalyzer() {}
    bool checkHeader(const char*, int32_t) const {
        return false;
    }
    STRIGI_ENDANALYZER_RETVAL analyze(Strigi::AnalysisResult&, InputStream*) {
        return -1;
    }
    const char* name() const { return "DummyEndAnalyzer"; }
};
class STRIGI_PLUGIN_API DummyEndAnalyzerFactory : public StreamEndAnalyzerFactory {
    const char* name() const {
        return "DummyEndAnalyzerFactory";
    }
    void registerFields(Strigi::FieldRegister&)  {}
    StreamEndAnalyzer* newInstance() const {
        return new DummyEndAnalyzer();
    }
};
class STRIGI_PLUGIN_API DummyThroughAnalyzer : public StreamThroughAnalyzer {
public:
    DummyThroughAnalyzer() {}
    const char* name() const {
	return "DummyThroughAnalyzer";
    }
    void setIndexable(Strigi::AnalysisResult*) {}
    InputStream* connectInputStream(InputStream *in) {
        return in;
    }
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummyThroughAnalyzerFactory : public StreamThroughAnalyzerFactory {
    const char* name() const {
        return "DummyThroughAnalyzerFactory";
    }
    void registerFields(Strigi::FieldRegister&)  {}
    StreamThroughAnalyzer* newInstance() const {
        return new DummyThroughAnalyzer();
    }
};
class STRIGI_PLUGIN_API DummySaxAnalyzer : public StreamSaxAnalyzer {
public:
    DummySaxAnalyzer() {}
    const char* name() const { return "DummySaxAnalyzer"; }
    void startAnalysis(AnalysisResult*) {}
    void endAnalysis(bool /*complete*/) {}
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummySaxAnalyzerFactory : public StreamSaxAnalyzerFactory {
    const char* name() const {
        return "DummySaxAnalyzerFactory";
    }
    void registerFields(Strigi::FieldRegister&)  {}
    StreamSaxAnalyzer* newInstance() const {
        return new DummySaxAnalyzer();
    }
};
class STRIGI_PLUGIN_API DummyLineAnalyzer : public StreamLineAnalyzer {
public:
    DummyLineAnalyzer() {}
    const char* name() const { return "DummyLineAnalyzer"; }
    void startAnalysis(AnalysisResult*) {}
    void endAnalysis(bool /*complete*/) {}
    void handleLine(const char*, uint32_t) {}
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummyLineAnalyzerFactory : public StreamLineAnalyzerFactory {
    const char* name() const {
        return "DummyLineAnalyzerFactory";
    }
    void registerFields(Strigi::FieldRegister&)  {}
    StreamLineAnalyzer* newInstance() const {
        return new DummyLineAnalyzer();
    }
};
class STRIGI_PLUGIN_API DummyEventAnalyzer : public StreamEventAnalyzer {
public:
    DummyEventAnalyzer() {}
    const char* name() const { return "DummyEventAnalyzer"; }
    void startAnalysis(AnalysisResult*) {}
    void endAnalysis(bool /*complete*/) {}
    void handleData(const char*, uint32_t) {}
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummyEventAnalyzerFactory : public StreamEventAnalyzerFactory {
    const char* name() const {
        return "DummyEventAnalyzerFactory";
    }
    void registerFields(Strigi::FieldRegister&)  {}
    StreamEventAnalyzer* newInstance() const {
        return new DummyEventAnalyzer();
    }
};

class Factory : public AnalyzerFactoryFactory {
public:
    list<StreamEndAnalyzerFactory*>
    streamEndAnalyzerFactories() const {
        list<StreamEndAnalyzerFactory*> af;
        af.push_back(new DummyEndAnalyzerFactory());
        return af;
    }
    list<StreamThroughAnalyzerFactory*>
    streamThroughAnalyzerFactories() const {
        list<StreamThroughAnalyzerFactory*> af;
        af.push_back(new DummyThroughAnalyzerFactory());
        return af;
    }
    list<StreamSaxAnalyzerFactory*>
    streamSaxAnalyzerFactories() const {
        list<StreamSaxAnalyzerFactory*> af;
        af.push_back(new DummySaxAnalyzerFactory());
        return af;
    }
    list<StreamLineAnalyzerFactory*>
    streamLineAnalyzerFactories() const {
        list<StreamLineAnalyzerFactory*> af;
        af.push_back(new DummyLineAnalyzerFactory());
        return af;
    }
    list<StreamEventAnalyzerFactory*>
    streamEventAnalyzerFactories() const {
        list<StreamEventAnalyzerFactory*> af;
        af.push_back(new DummyEventAnalyzerFactory());
        return af;
    }
};

/*
 Register the AnalyzerFactoryFactory
*/
STRIGI_ANALYZER_FACTORY(Factory)
