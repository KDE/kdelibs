// XXX Automatically generated. DO NOT EDIT! XXX //

public:
LangValue();
LangValue(const LangValue&);
LangValue(const Q3CString&);
LangValue & operator = (LangValue&);
LangValue & operator = (const Q3CString&);
bool operator ==(LangValue&);
bool operator !=(LangValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {LangValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~LangValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "LangValue"; }

// End of automatically generated code           //
