// XXX Automatically generated. DO NOT EDIT! XXX //

public:
NValue();
NValue(const NValue&);
NValue(const QCString&);
NValue & operator = (NValue&);
NValue & operator = (const QCString&);
bool operator ==(NValue&);
bool operator !=(NValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {NValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~NValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "NValue"; }

// End of automatically generated code           //
