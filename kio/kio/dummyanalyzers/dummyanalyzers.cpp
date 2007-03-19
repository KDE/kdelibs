#include <strigi/jstreamsconfig.h>
#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>
#include <strigi/streamsaxanalyzer.h>
#include <strigi/streamthroughanalyzer.h>
#include <strigi/streamlineanalyzer.h>
#include <strigi/streameventanalyzer.h>

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
    char analyze(Strigi::AnalysisResult&, jstreams::InputStream*) {
        return -1;
    }
    const char* getName() const { return "DummyEndAnalyzer"; }
};
class STRIGI_PLUGIN_API DummyEndAnalyzerFactory : public StreamEndAnalyzerFactory {
    const char* getName() const {
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
    void setIndexable(Strigi::AnalysisResult*) {}
    jstreams::InputStream* connectInputStream(jstreams::InputStream *in) {
        return in;
    }
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummyThroughAnalyzerFactory : public StreamThroughAnalyzerFactory {
    const char* getName() const {
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
    const char* getName() const { return "DummySaxAnalyzer"; }
    void startAnalysis(AnalysisResult*) {}
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummySaxAnalyzerFactory : public StreamSaxAnalyzerFactory {
    const char* getName() const {
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
    const char* getName() const { return "DummyLineAnalyzer"; }
    void startAnalysis(AnalysisResult*) {}
    void handleLine(const char*, uint32_t) {}
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummyLineAnalyzerFactory : public StreamLineAnalyzerFactory {
    const char* getName() const {
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
    const char* getName() const { return "DummyEventAnalyzer"; }
    void startAnalysis(AnalysisResult*) {}
    void handleData(const char*, uint32_t) {}
    bool isReadyWithStream() { return true; }
};
class STRIGI_PLUGIN_API DummyEventAnalyzerFactory : public StreamEventAnalyzerFactory {
    const char* getName() const {
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
    getStreamEndAnalyzerFactories() const {
        list<StreamEndAnalyzerFactory*> af;
        af.push_back(new DummyEndAnalyzerFactory());
        return af;
    }
    list<StreamThroughAnalyzerFactory*>
    getStreamThroughAnalyzerFactories() const {
        list<StreamThroughAnalyzerFactory*> af;
        af.push_back(new DummyThroughAnalyzerFactory());
        return af;
    }
    list<StreamSaxAnalyzerFactory*>
    getStreamSaxAnalyzerFactories() const {
        list<StreamSaxAnalyzerFactory*> af;
        af.push_back(new DummySaxAnalyzerFactory());
        return af;
    }
    list<StreamLineAnalyzerFactory*>
    getStreamLineAnalyzerFactories() const {
        list<StreamLineAnalyzerFactory*> af;
        af.push_back(new DummyLineAnalyzerFactory());
        return af;
    }
    list<StreamEventAnalyzerFactory*>
    getStreamEventAnalyzerFactories() const {
        list<StreamEventAnalyzerFactory*> af;
        af.push_back(new DummyEventAnalyzerFactory());
        return af;
    }
};

/*
 Register the AnalyzerFactoryFactory
*/
STRIGI_ANALYZER_FACTORY(Factory)
