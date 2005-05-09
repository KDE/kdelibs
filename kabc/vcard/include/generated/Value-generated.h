// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Value();
Value(const Value&);
Value(const Q3CString&);
Value & operator = (Value&);
Value & operator = (const Q3CString&);
bool operator ==(Value&);
bool operator !=(Value& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {Value a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~Value();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "Value"; }

// End of automatically generated code           //
