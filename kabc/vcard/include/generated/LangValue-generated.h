// XXX Automatically generated. DO NOT EDIT! XXX //

public:
LangValue();
LangValue(const LangValue&);
LangValue(const QCString&);
LangValue & operator = (LangValue&);
LangValue & operator = (const QCString&);
bool operator ==(LangValue&);
bool operator !=(LangValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {LangValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~LangValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "LangValue"; }

// End of automatically generated code           //
