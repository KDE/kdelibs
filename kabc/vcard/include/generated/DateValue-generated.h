// XXX Automatically generated. DO NOT EDIT! XXX //

public:
DateValue();
DateValue(const DateValue&);
DateValue(const Q3CString&);
DateValue & operator = (DateValue&);
DateValue & operator = (const Q3CString&);
bool operator ==(DateValue&);
bool operator !=(DateValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {DateValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~DateValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "DateValue"; }

// End of automatically generated code           //
