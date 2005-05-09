// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TelParam();
TelParam(const TelParam&);
TelParam(const Q3CString&);
TelParam & operator = (TelParam&);
TelParam & operator = (const Q3CString&);
bool operator ==(TelParam&);
bool operator !=(TelParam& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {TelParam a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~TelParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TelParam"; }

// End of automatically generated code           //
