// XXX Automatically generated. DO NOT EDIT! XXX //

public:
DateValue();
DateValue(const DateValue&);
DateValue(const QCString&);
DateValue & operator = (DateValue&);
DateValue & operator = (const QCString&);
bool operator ==(DateValue&);
bool operator !=(DateValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {DateValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~DateValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "DateValue"; }

// End of automatically generated code           //
