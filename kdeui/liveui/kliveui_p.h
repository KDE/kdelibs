#include <QtXml/QXmlContentHandler>

#include <kdelibs_export.h>

class KDEUI_EXPORT XmlGuiHandler : public QXmlContentHandler
{
public:
    XmlGuiHandler(KLiveUiBuilder *builder, QObject *component);
    
    virtual bool startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &attributes);
    virtual bool endElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes & /*attributes*/);
    virtual bool characters(const QString &text);

    virtual void setDocumentLocator(QXmlLocator*) { }
    virtual bool startDocument() { return true; }
    virtual bool endDocument() { return true; }
    virtual bool startPrefixMapping(const QString&, const QString&) { return true; }
     virtual bool endPrefixMapping(const QString&) { return true; }
     virtual bool endElement(const QString&, const QString&, const QString&) { return true; }
     virtual bool ignorableWhitespace(const QString&) { return true; }
    virtual bool processingInstruction(const QString&, const QString&) { return true; }
    virtual bool skippedEntity(const QString&) { return true; }
    virtual QString errorString() const { return QString(); }
private:
    KLiveUiBuilder *builder;
    QWidget *currentWidget;
    QObject *component;
};

