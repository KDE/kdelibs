// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ClassValue();
ClassValue(const ClassValue&);
ClassValue(const QCString&);
ClassValue & operator = (ClassValue&);
ClassValue & operator = (const QCString&);
bool operator ==(ClassValue&);
bool operator !=(ClassValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {ClassValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~ClassValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "ClassValue"; }

// End of automatically generated code           //
