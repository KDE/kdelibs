// XXX Automatically generated. DO NOT EDIT! XXX //

public:
OrgValue();
OrgValue(const OrgValue&);
OrgValue(const QCString&);
OrgValue & operator = (OrgValue&);
OrgValue & operator = (const QCString&);
bool operator ==(OrgValue&);
bool operator !=(OrgValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {OrgValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~OrgValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "OrgValue"; }

// End of automatically generated code           //
