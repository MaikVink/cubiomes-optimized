#ifdef INCLUDE_STACKTRACE
#define STARTFUNC(name) printf("s "); printf(name); printf("\n")
    #define ENDFUNC(name) printf("r "); printf(name); printf("\n")
#else
#define STARTFUNC(name) ;
#define ENDFUNC(name) ;
#endif