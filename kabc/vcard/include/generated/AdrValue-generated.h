// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AdrValue();
AdrValue(const AdrValue&);
AdrValue(const QCString&);
AdrValue & operator = (AdrValue&);
AdrValue & operator = (const QCString&);
bool operator ==(AdrValue&);
bool operator !=(AdrValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {AdrValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~AdrValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "AdrValue"; }

// End of automatically generated code           //
