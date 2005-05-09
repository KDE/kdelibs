// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TelValue();
TelValue(const TelValue&);
TelValue(const Q3CString&);
TelValue & operator = (TelValue&);
TelValue & operator = (const Q3CString&);
bool operator ==(TelValue&);
bool operator !=(TelValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {TelValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~TelValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TelValue"; }

// End of automatically generated code           //
