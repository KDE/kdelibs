// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextListValue();
TextListValue(const TextListValue&);
TextListValue(const Q3CString&);
TextListValue & operator = (TextListValue&);
TextListValue & operator = (const Q3CString&);
bool operator ==(TextListValue&);
bool operator !=(TextListValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {TextListValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~TextListValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextListValue"; }

// End of automatically generated code           //
