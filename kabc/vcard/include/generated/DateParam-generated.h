// XXX Automatically generated. DO NOT EDIT! XXX //

public:
DateParam();
DateParam(const DateParam&);
DateParam(const QCString&);
DateParam & operator = (DateParam&);
DateParam & operator = (const QCString&);
bool operator ==(DateParam&);
bool operator !=(DateParam& x) {return !(*this==x);}
bool operator ==(const QCString& s) {DateParam a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~DateParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "DateParam"; }

// End of automatically generated code           //
