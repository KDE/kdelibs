// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Param();
Param(const Param&);
Param(const Q3CString&);
Param & operator = (Param&);
Param & operator = (const Q3CString&);
bool operator ==(Param&);
bool operator !=(Param& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {Param a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~Param();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "Param"; }

// End of automatically generated code           //
