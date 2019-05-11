#undef ClassHeaderDecl
#undef HeaderDecl
#undef ItemDecl
#undef ItemDecl2
#undef TailDecl

#ifdef Declare_String

	#define ClassHeaderDecl(Class, Name)		char* Class::m_sz##Name[CountOf##Name]={
	#define HeaderDecl(Name)					char* g_sz##Name[CountOf##Name]={
	#define ItemDecl(Item)						#Item,
	#define ItemDecl2(Item,String)				#String,
	#define TailDecl(Name)						};

#else

	#ifdef Declare_Extern

		#define ClassHeaderDecl(Class, Name)	static char* m_sz##Name[CountOf##Name];
		#define HeaderDecl(Name)				extern char* g_sz##Name[CountOf##Name];
		#define ItemDecl(Item)
		#define ItemDecl2(Item,String)
		#define TailDecl(Name)

	#else

		#define ClassHeaderDecl(Class, Name)	typedef enum {
		#define HeaderDecl(Name)				typedef enum {
		#define ItemDecl(Item)					Item,
		#define ItemDecl2(Item,String)			Item,
		#define TailDecl(Name)					CountOf##Name} Name;

	#endif
#endif

#undef Declare_String
#undef Declare_Extern
