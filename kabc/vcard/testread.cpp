#include <iostream>
#include <stdlib.h>
#include <assert.h>

#include <qfile.h>
#include <qtextstream.h>

#include <VCard.h>

using namespace std;

int main(int argc, char * argv[])
{
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <filename>" << endl;
		exit(1);
	}
	
	QFile f(argv[1]);
	
	QCString str;
	
	if (!f.open(IO_ReadOnly)) {
		cerr << "Couldn't open file \"" << argv[1] << endl;
		exit(1);
	}
	
	QTextStream t(&f);
	
	while (!t.eof())
		str += t.readLine().utf8() + '\n';
	
	using namespace VCARD; 

	// Iterate through all vCards in the file.

        cout << "--------- begin ----------" << endl;
        cout << str.data();
        cout << "---------  end  ----------" << endl;

	VCardEntity e(str);
	
	VCardListIterator it(e.cardList());
	
	for (; it.current(); ++it) {
		
		cerr << "****************** VCARD ********************" << endl;
		
		// Create a vcard using the string representation.
		VCard & v (*it.current());

		if (v.has(EntityEmail)) {
			cerr << "Email parameter found" << endl;
			
			QCString s = v.contentLine(EntityEmail)->value()->asString();
			
			cerr << "Email value == " << s << endl;
		}
		
		if (v.has(EntityNickname)) {
			cerr << "Nickname parameter found" << endl;
			
			cerr << "Nickname value == " <<
				v.contentLine(EntityNickname)->value()->asString() <<
				endl;
		}
		
		if (v.has(EntityRevision)) {
			
			cerr << "Revision parameter found" << endl;
			
			DateValue * d =
				(DateValue *)
				v.contentLine(EntityRevision)->value();
			
			assert(d != 0);
			
			cerr << "Revision date: " << endl;
			cerr << "Day   : " << d->day()		<< endl;
			cerr << "Month : " << d->month()	<< endl;
			cerr << "Year  : " << d->year()		<< endl;
			
			if (d->hasTime()) {
				cerr << "Revision date has a time component" << endl;
				cerr << "Revision time: " << endl;
				cerr << "Hour   : " << d->hour()	<< endl;
				cerr << "Minute : " << d->minute()	<< endl;
				cerr << "Second : " << d->second()	<< endl;

			}
			else cerr << "Revision date does NOT have a time component" << endl;
		}
		
		if (v.has(EntityURL)) {
			cerr << "URL Parameter found" << endl;
			
			cerr << "URL Value == " <<
				v.contentLine(EntityURL)->value()->asString() <<
				endl;
			
			URIValue * urlVal =
				(URIValue *)v.contentLine(EntityURL)->value();

			assert(urlVal != 0);
			
			cerr << "URL scheme == " <<
				urlVal->scheme() << endl;
			
			cerr << "URL scheme specific part == " <<
				urlVal->schemeSpecificPart() << endl;
		}
		
		if (v.has(EntityN)) {
			cerr << "N Parameter found" << endl;
			
			NValue * n =
				(NValue *)(v.contentLine(EntityN)->value());
			
			cerr << "Family name  == " << n->family()	<< endl;
			cerr << "Given  name  == " << n->given()	<< endl;
			cerr << "Middle name  == " << n->middle()	<< endl;
			cerr << "Prefix       == " << n->prefix()	<< endl;
			cerr << "Suffix       == " << n->suffix()	<< endl;
		}
		
		cerr << "***************** END VCARD ******************" << endl;
	}
}

