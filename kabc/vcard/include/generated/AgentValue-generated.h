// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AgentValue();
AgentValue(const AgentValue&);
AgentValue(const QCString&);
AgentValue & operator = (AgentValue&);
AgentValue & operator = (const QCString&);
bool operator ==(AgentValue&);
bool operator !=(AgentValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {AgentValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~AgentValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "AgentValue"; }

// End of automatically generated code           //
