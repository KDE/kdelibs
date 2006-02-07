// testing the data kioslave
// (C) 2002, 2003 Leo Savernik
//
// invoke "make dataprotocoltest" to generate the binary inside KDE CVS
// invoke "make test" to generate the binary outside KDE CVS

// fix the symptoms, not the illness ;-)
#ifdef QT_NO_ASCII_CAST
#  undef QT_NO_ASCII_CAST
#endif

#ifdef DATAKIOSLAVE
#  undef DATAKIOSLAVE
#endif
#ifndef TESTKIO
#  define TESTKIO
#endif

#include <kio/global.h>

#include <qstring.h>

#include <iostream>
using namespace std;

class TestSlave {
public:
  TestSlave() {
  }
  virtual ~TestSlave() {
  }

  virtual void get(const KUrl &) = 0;
  virtual void mimetype(const KUrl &) = 0;

  void mimeType(const QString &type) {
    testStrings("MIME Type: ",mime_type_expected,type);
  }

  void totalSize(KIO::filesize_t bytes) {
//    cout << "content size: " << bytes << " bytes" << endl;
  }

  void setMetaData(const QString &key, const QString &value) {
//    meta_data[key] = value;
//    cout << "§ " << key << " = " << value << endl;
    QString prefix = "Metadata[\""+key+"\"]: ";
    KIO::MetaData::Iterator it = attributes_expected.find(key);
    if (it != attributes_expected.end()) {
      testStrings(prefix,it.data(),value);
      // remove key from map
      attributes_expected.remove(it);
    } else {
      cout << endl << prefix.latin1() << " no such key expected";
      total++;
    }
  }

  void sendMetaData() {
    // check here if attributes_expected contains any excess keys
    KIO::MetaData::ConstIterator it = attributes_expected.begin();
    KIO::MetaData::ConstIterator end = attributes_expected.end();
    for (; it != end; ++it) {
      cout << endl << "Metadata[\"" << it.key().latin1()
      		<< "\"] was expected but not defined";
      total++;
    }
  }

  void data(const QByteArray &a) {
    if (a.isEmpty())
/*      cout << "<no more data>" << endl*/;
    else {
      testStrings("Content: ",content_expected,a);
    }/*end if*/
  }

  void finished() {
  }

  void dispatchLoop() {
    // dummy to make kde_main happy
  }

  // == stuff for regression testing
private:
  int testcaseno;	// number of testcase
  bool failure;		// true if any testcase failed
  QMap<int,bool> failed_testcases;

  // -- testcase related members
  QString mime_type_expected;	// expected mime type
  /** contains all attributes and values the testcase has to set */
  KIO::MetaData attributes_expected;
  /** contains the content as it is expected to be returned */
  QByteArray content_expected;
  int passed;		// # of passed tests
  int total;		// # of total tests

  /**
   * compares two strings, printing an error message if they don't match.
   * @param prefix prefix string for output in case of mismatch
   * @param templat template string
   * @param s string to compare to template
   * @param casesensitive true if case sensitive compare (currently not used)
   */
  void testStrings(const QString &prefix, const QString &templat,
                   const QString &s, bool /*casesensitive*/ = true) {
    if (templat == s)
      passed++;
    else {
      cout << endl << prefix.latin1() << "expected \"" << templat.latin1() << "\", found \""
      		<< s.latin1() << "\"";
      failure = true;
    }/*end if*/
    total++;
  }

public:
  /** begins a testrun over all testcases */
  void initTestrun() {
    testcaseno = 0;
    failure = false;
  }

  /** reuturns true if any testcase failed
    */
  bool hasFailedTestcases() const { return failure; }

  /**
   * sets up a new testcase
   * @param name screen name for testcase
   */
  void beginTestcase(const char *name) {
    passed = 0;
    total = 0;
    testcaseno++;
    cout << "Testcase " << testcaseno << ": [" << name << "] ";
  }

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

  /**
   * closes testcase, printing out stats
   */
  void endTestcase() {
    bool failed = passed < total;
    if (failed) {
      failure = true;
      failed_testcases[testcaseno] = true;
      cout << endl;
    }
    cout << "(" << passed << " of " << total << ") " << (failed ? "failed"
    		: "passed") << endl;
  }

  void endTestrun() {
    if (failure) {
      QMap<int,bool>::ConstIterator it = failed_testcases.begin();
      for (; it != failed_testcases.end(); ++it) {
        cout << "Testcase " << it.key() << " failed" << endl;
      }
    }
  }
};

#include "dataprotocol.cpp"	// we need access to static data & functions

// == general functionality
const struct {
const char * name;
const char * exp_mime_type; // 0 means "text/plain"
const struct {
	const char * key;
	const char * value;
} exp_attrs[10];	// ended with a key==0, value==0 pair
const char * exp_content;
const char * url;
} testcases[] = {
	// -----------------------------------------------------------------
	{ "escape resolving", 0, {}, "blah blah", "data:,blah%20blah" },
	// --------------------
	{ "mime type, escape resolving", "text/html", {},
	"<div style=\"border:thin orange solid;padding:1ex;background-color:"
	"yellow;color:black\">Rich <b>text</b></div>",
	"data:text/html,<div%20style=\"border:thin%20orange%20solid;"
	"padding:1ex;background-color:yellow;color:black\">Rich%20<b>text</b>"
	"</div>" },
	// -------------------- whitespace test I
	{ "whitespace test I", "text/css", {
		{ "charset", "iso-8859-15" }, { 0,0 } },
	" body { color: yellow; background:darkblue; font-weight:bold }",
	"data:text/css  ;  charset =  iso-8859-15 , body { color: yellow; "
	"background:darkblue; font-weight:bold }" },
	// -------------------- out of spec argument order, base64 decoding,
	// whitespace test II
	{ "out of spec argument order, base64 decoding, whitespace test II",
	0, {
		{ "charset", "iso-8859-1" }, { 0,0 } },
	"paaaaaaaasd!!\n",
	"data: ;  base64 ; charset =  \"iso-8859-1\" ,cGFhYWFhYWFhc2QhIQo=" },
	// -------------------- arbitrary keys, reserved names as keys,
	// whitespace test III
	{ "arbitrary keys, reserved names as keys, whitespace test III", 0, {
		{ "base64", "nospace" }, { "key", "onespaceinner" },
		{ "key2", "onespaceouter" }, { "charset", "utf8" },
		{ "<<empty>>", "" }, { 0,0 } },
	"Die, Allied Schweinehund (C) 1990 Wolfenstein 3D",
	"data: ;base64=nospace;key = onespaceinner; key2=onespaceouter ;"
	" charset = utf8 ; <<empty>>= ,Die, Allied Schweinehund "
	"(C) 1990 Wolfenstein 3D" },
	// -------------------- string literal with escaped chars, testing
	// delimiters within string
	{ "string literal with escaped chars, testing delimiters within "
	"string", 0, {
		{ "fortune-cookie", "Master Leep say: \"Rabbit is humble, "
		"Rabbit is gentle; follow the Rabbit\"" }, { 0,0 } },
	"(C) 1997 Shadow Warrior ;-)",
	"data:;fortune-cookie=\"Master Leep say: \\\"Rabbit is humble, "
	"Rabbit is gentle; follow the Rabbit\\\"\",(C) 1997 Shadow Warrior "
	";-)" },
};

#if 0
// == charset tests
	// -------------------- string
const QChar
const QChar * const charset_urls[] = {
#endif

int main(int /*argc*/,char* /*argv*/[]) {
  DataProtocol kio_data;

  kio_data.initTestrun();
  for (uint i = 0; i < sizeof testcases/sizeof testcases[0]; i++) {
    kio_data.beginTestcase(testcases[i].name);
    kio_data.setExpectedMimeType(testcases[i].exp_mime_type != 0
    		? testcases[i].exp_mime_type : "text/plain");

    bool has_charset = false;
    MetaData exp_attrs;
    if (testcases[i].exp_attrs != 0) {
      for (uint j = 0; testcases[i].exp_attrs[j].key != 0; j++) {
        exp_attrs[testcases[i].exp_attrs[j].key] = testcases[i].exp_attrs[j].value;
	if (strcmp(testcases[i].exp_attrs[j].key,"charset") == 0)
	  has_charset = true;
      }/*next j*/
    }
    if (!has_charset) exp_attrs["charset"] = "us-ascii";
    kio_data.setExpectedAttributes(exp_attrs);

    QByteArray exp_content;
    uint exp_content_len = strlen(testcases[i].exp_content);
    exp_content.setRawData(testcases[i].exp_content,exp_content_len);
    kio_data.setExpectedContent(exp_content);

    kio_data.get(KUrl(testcases[i].url));

    kio_data.endTestcase();
    exp_content.resetRawData(testcases[i].exp_content,exp_content_len);
  }/*next i*/
  kio_data.endTestrun();

  return kio_data.hasFailedTestcases() ? 1 : 0;
}

