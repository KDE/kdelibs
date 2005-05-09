// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AgentParam();
AgentParam(const AgentParam&);
AgentParam(const Q3CString&);
AgentParam & operator = (AgentParam&);
AgentParam & operator = (const Q3CString&);
bool operator ==(AgentParam&);
bool operator !=(AgentParam& x) {return !(*this==x);}
bool operator ==(const Q3CString& s) {AgentParam a(s);return(*this==a);} 
bool operator != (const Q3CString& s) {return !(*this == s);}

virtual ~AgentParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "AgentParam"; }

// End of automatically generated code           //
