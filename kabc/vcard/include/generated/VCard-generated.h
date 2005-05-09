// XXX Automatically generated. DO NOT EDIT! XXX //

public:
VCard();
VCard(const VCard&);
VCard(const Q3CString&);
VCard & operator = (VCard&);
VCard & operator = (const Q3CString&);
bool operator ==(VCard&);
bool operator !=(VCard& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {VCard a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~VCard();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "VCard"; }

// End of automatically generated code           //
