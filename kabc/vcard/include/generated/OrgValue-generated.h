// XXX Automatically generated. DO NOT EDIT! XXX //

public:
OrgValue();
OrgValue(const OrgValue&);
OrgValue(const Q3CString&);
OrgValue & operator = (OrgValue&);
OrgValue & operator = (const Q3CString&);
bool operator ==(OrgValue&);
bool operator !=(OrgValue& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {OrgValue a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~OrgValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "OrgValue"; }

// End of automatically generated code           //
