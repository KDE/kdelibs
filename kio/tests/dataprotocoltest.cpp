/*
 * Copyright (C) 2002,2003 Leo Savernik <l.savernik@aon.at>
 * Copyright (C) 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
 */

#ifdef DATAKIOSLAVE
#  undef DATAKIOSLAVE
#endif
#ifndef TESTKIO
#  define TESTKIO
#endif

#include "dataprotocoltest.h"

#include <kio/global.h>

QTEST_KDEMAIN(DataProtocolTest, NoGUI)

class TestSlave {
public:
  TestSlave() {
  }
  virtual ~TestSlave() {
  }

  void mimeType(const QString &type) {
    QCOMPARE(mime_type_expected, type);
  }

  void totalSize(KIO::filesize_t /*bytes*/) {
//    qDebug() << "content size: " << bytes << " bytes";
  }

  void setMetaData(const QString &key, const QString &value) {
    KIO::MetaData::Iterator it = attributes_expected.find(key);
    QVERIFY(it != attributes_expected.end());
    QCOMPARE(it.value(), value);
    attributes_expected.erase(it);
  }

  void setAllMetaData(const KIO::MetaData &md) {
    KIO::MetaData::ConstIterator it = md.begin();
    KIO::MetaData::ConstIterator end = md.end();

    for (; it != end; ++it) {
      KIO::MetaData::Iterator eit = attributes_expected.find(it.key());
      QVERIFY(eit != attributes_expected.end());
      QCOMPARE(eit.value(), it.value());
      attributes_expected.erase(eit);
    }
  }

  void sendMetaData() {
    // check here if attributes_expected contains any excess keys
    KIO::MetaData::ConstIterator it = attributes_expected.constBegin();
    KIO::MetaData::ConstIterator end = attributes_expected.constEnd();
    for (; it != end; ++it) {
      qDebug() << "Metadata[\"" << it.key()
               << "\"] was expected but not defined";
    }
    QVERIFY(attributes_expected.isEmpty());
  }

  void data(const QByteArray &a) {
    if (a.isEmpty()) {
//    qDebug() << "<no more data>";
    } else {
      QCOMPARE(content_expected, a);
    }/*end if*/
  }

  void dispatch_data(const QByteArray &a) {
    data(a);
  }

  void finished() {
  }

  void dispatch_finished() {
  }

  void ref() {}
  void deref() {}

private:
  // -- testcase related members
  QString mime_type_expected;	// expected mime type
  /** contains all attributes and values the testcase has to set */
  KIO::MetaData attributes_expected;
  /** contains the content as it is expected to be returned */
  QByteArray content_expected;

public:
  /**
   * sets the mime type that this testcase is expected to return
   */
  void setExpectedMimeType(const QString &mime_type) {
    mime_type_expected = mime_type;
  }

  /**
   * sets all attribute-value pairs the testcase must deliver.
   */
  void setExpectedAttributes(const KIO::MetaData &attres) {
    attributes_expected = attres;
  }

  /**
   * sets content as expected to be delivered by the testcase.
   */
  void setExpectedContent(const QByteArray &content) {
    content_expected = content;
  }
};

#include "dataprotocol.cpp"	// we need access to static data & functions

void runTest(const QByteArray &mimetype, const QStringList &metalist, const QByteArray &content, const KUrl &url)
{
    DataProtocol kio_data;

    kio_data.setExpectedMimeType(mimetype);
    MetaData exp_attrs;
    foreach (const QString &meta, metalist) {
        const QStringList metadata = meta.split(QLatin1Char('='));
        Q_ASSERT(metadata.count() == 2);
        exp_attrs[metadata[0]] = metadata[1];
    }
    kio_data.setExpectedAttributes(exp_attrs);
    kio_data.setExpectedContent(content);

    // check that mimetype(url) returns the same value as the complete parsing
    kio_data.mimetype(url);

    kio_data.get(url);
}

void DataProtocolTest::runAllTests()
{
    QFETCH(QByteArray, expected_mime_type);
    QFETCH(QString, metadata);
    QFETCH(QByteArray, exp_content);
    QFETCH(QByteArray, url);

    const QStringList metalist = metadata.split(QLatin1Char('\n'));

    runTest(expected_mime_type, metalist, exp_content, KUrl(url));
}

void DataProtocolTest::runAllTests_data()
{
    QTest::addColumn<QByteArray>( "expected_mime_type" );
    QTest::addColumn<QString>( "metadata" );
    QTest::addColumn<QByteArray>( "exp_content" );
    QTest::addColumn<QByteArray>( "url" );

    const QByteArray textplain = "text/plain";
    const QString usascii = QLatin1String( "charset=us-ascii" );

    QTest::newRow( "escape resolving" ) <<
        textplain <<
        usascii <<
        QByteArray( "blah blah" ) <<
        QByteArray( "data:,blah%20blah" );

    QTest::newRow( "mime type, escape resolving" ) <<
        QByteArray( "text/html" ) <<
        usascii <<
        QByteArray( "<div style=\"border:thin orange solid;padding:1ex;background-color:yellow;color:black\">Rich <b>text</b></div>" ) <<
        QByteArray( "data:text/html,<div%20style=\"border:thin%20orange%20solid;"
                    "padding:1ex;background-color:yellow;color:black\">Rich%20<b>text</b>"
                    "</div>" );

    QTest::newRow( "whitespace test I" ) <<
        QByteArray( "text/css" ) <<
        QString(QLatin1String( "charset=iso-8859-15" )) <<
        QByteArray( " body { color: yellow; background:darkblue; font-weight:bold }" ) <<
        QByteArray( "data:text/css  ;  charset =  iso-8859-15 , body { color: yellow; "
                    "background:darkblue; font-weight:bold }" );

    QTest::newRow( "out of spec argument order, base64 decoding, whitespace test II" ) <<
        textplain <<
        QString(QLatin1String( "charset=iso-8859-1" )) <<
        QByteArray( "paaaaaaaasd!!\n" ) <<
        QByteArray( "data: ;  base64 ; charset =  \"iso-8859-1\" ,cGFhYWFhYWFhc2QhIQo=" );

    QTest::newRow( "arbitrary keys, reserved names as keys, whitespace test III" ) <<
        textplain <<
        QString(QLatin1String( "base64=nospace\n"
                               "key=onespaceinner\n"
                               "key2=onespaceouter\n"
                               "charset=utf8\n"
                               "<<empty>>=" )) <<
        QByteArray( "Die, Allied Schweinehund (C) 1990 Wolfenstein 3D" ) <<
        QByteArray( "data: ;base64=nospace;key = onespaceinner; key2=onespaceouter ;"
                    " charset = utf8 ; <<empty>>= ,Die, Allied Schweinehund "
                    "(C) 1990 Wolfenstein 3D" );

    QTest::newRow( "string literal with escaped chars, testing delimiters within string" ) <<
        textplain <<
        QString(QLatin1String( "fortune-cookie=Master Leep say: \"Rabbit is humble, Rabbit is gentle; follow the Rabbit\"\n"
                               "charset=us-ascii" )) <<
        QByteArray( "(C) 1997 Shadow Warrior ;-)" ) <<
        QByteArray( "data:;fortune-cookie=\"Master Leep say: \\\"Rabbit is humble, "
                    "Rabbit is gentle; follow the Rabbit\\\"\",(C) 1997 Shadow Warrior "
                    ";-)" );

    // the "greenbytes" tests are taken from http://greenbytes.de/tech/tc/datauri/
    QTest::newRow( "greenbytes-simplewfrag" ) <<
        textplain <<
        usascii <<
        QByteArray( "test" ) <<
        QByteArray( "data:,test#foo" );

    QTest::newRow( "greenbytes-simplefrag" ) <<
        QByteArray( "text/html" ) <<
        usascii <<
        QByteArray( "<p>foo</p>" ) <<
        QByteArray( "data:text/html,%3Cp%3Efoo%3C%2Fp%3E#bar" );

    QTest::newRow( "greenbytes-svg" ) <<
        QByteArray( "image/svg+xml" ) <<
        usascii <<
        QByteArray( "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
                    "  <circle cx=\"100\" cy=\"100\" r=\"25\" stroke=\"black\" stroke-width=\"1\" fill=\"green\"/>\n"
                    "</svg>\n") <<
        QByteArray( "data:image/svg+xml,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20version%3D%221.1"
                    "%22%3E%0A%20%20%3Ccircle%20cx%3D%22100%22%20cy%3D%22100%22%20r%3D%2225%22%20stroke%3D%22black%22%20"
                    "stroke-width%3D%221%22%20fill%3D%22green%22%2F%3E%0A%3C%2Fsvg%3E%0A#bar" );
}

#include "dataprotocoltest.moc"
