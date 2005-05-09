// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AdrValue();
AdrValue(const AdrValue&);
AdrValue(const Q3CString&);
AdrValue & operator = (AdrValue&);
AdrValue & operator = (const Q3CString&);
bool operator ==(AdrValue&);
bool operator !=(AdrValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {AdrValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~AdrValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "AdrValue"; }

// End of automatically generated code           //
