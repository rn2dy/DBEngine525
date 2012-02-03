#ifdef DEBUGX
#define LOC std::cout<<"Debug:"<<__FILE__<<":"<<__LINE__<<" ";
#define DEBUG_VAR(text) LOC std::cout<<(#text)<<"="<<text<<std::endl;
#define MSG_VAR(text,var) LOC std::cout<<text<<"="<<var<<std::endl;
#define MSG(text) LOC std::cout<<text<<std::endl;
#else
#define LOC
#define DEBUG_VAR(text)
#define MSG_VAR(text,var)
#define MSG(text)
#endif

