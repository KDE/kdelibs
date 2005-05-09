// XXX Automatically generated. DO NOT EDIT! XXX //

public:
FloatValue();
FloatValue(const FloatValue&);
FloatValue(const Q3CString&);
FloatValue & operator = (FloatValue&);
FloatValue & operator = (const Q3CString&);
bool operator ==(FloatValue&);
bool operator !=(FloatValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {FloatValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~FloatValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "FloatValue"; }

// End of automatically generated code           //
