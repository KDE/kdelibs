// testing the data kioslave
// (C) 2002, 2003 Leo Savernik
//
// invoke "make test" to generate the binary

#include <kio/global.h>

#include <qcstring.h>
#include <qstring.h>

#include <iostream.h>

class KURL;

class TestSlave {
public:
  TestSlave(const QCString &protocol,const QCString &,const QCString &) {
  }
  virtual ~TestSlave() {
  }

  virtual void get(const KURL &) = 0;
  virtual void mimetype(const KURL &) = 0;

  void mimeType(const QString &type) {
    cout << "mime type: " << type << endl;
  }

  void totalSize(KIO::filesize_t bytes) {
    cout << "content size: " << bytes << " bytes" << endl;
  }

  void setMetaData(const QString &key, const QString &value) {
//    meta_data[key] = value;
    cout << "§ " << key << " = " << value << endl;
  }

  void data(const QByteArray &a) {
    if (a.isEmpty())
      cout << "<no more data>" << endl;
    else {
      cout << (const char *)a << endl;
    }/*end if*/
  }

  void finished() {
  }

  void dispatchLoop() {
    // dummy to make kde_main happy
  }
};

#include "data.cpp"	// we need access to static data & functions

const char * const urls[] = {
	// -------------------- escape resolving
	"data:,blah%20blah",
	// -------------------- mime type, escape resolving
	"data:text/html,<div%20style=\"border:thin%20orange%20solid;"
	"padding:1ex;background-color:yellow;color:black\">Rich%20<b>text</b></div>",
	// -------------------- whitespace test I
	"data:text/css  ;  charset =  iso-8859-15 , body { color: yellow; "
	"background:darkblue; font-weight:bold }",
	// -------------------- out of spec argument order, base64 decoding,
	// whitespace test II
	"data: ;  base64 ; charset =  \"iso-8859-1\" ,cGFhYWFhYWFhc2QhIQo=",
	// -------------------- arbitrary keys, reserved names as keys,
	// whitespace test III
	"data: ;base64=nospace;key = onespaceinner; key2=onespaceouter ;"
	" charset = utf8 ; <<empty>>= ,Die, Allied Schweinehund "
	"(C) 1990 Wolfenstein 3D",
	// -------------------- string literal with escaped chars, testing
	// delimiters within string
	"data:;fortune-cookie=\"Master Leep say: \\\"Rabbit is humble, "
	"Rabbit is gentle; follow the Rabbit\\\"\",(C) 1997 Shadow Warrior ;-)",
};      

int main(int argc,char *argv[]) {
  DataProtocol kio_data("","");

  for (uint i = 0; i < sizeof urls/sizeof urls[0]; i++) {  
    kio_data.get(urls[i]);
  }/*next i*/

  return 0;
}

#include "testkio_data.moc"
