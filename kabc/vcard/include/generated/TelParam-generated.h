// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TelParam();
TelParam(const TelParam&);
TelParam(const QCString&);
TelParam & operator = (TelParam&);
TelParam & operator = (const QCString&);
bool operator ==(TelParam&);
bool operator !=(TelParam& x) {return !(*this==x);}
bool operator ==(const QCString& s) {TelParam a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~TelParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TelParam"; }

// End of automatically generated code           //
