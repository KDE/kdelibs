// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Group();
Group(const Group&);
Group(const Q3CString&);
Group & operator = (Group&);
Group & operator = (const Q3CString&);
bool operator ==(Group&);
bool operator !=(Group& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {Group a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~Group();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "Group"; }

// End of automatically generated code           //
