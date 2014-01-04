
#ifndef _DEBTOOLS_H__
#define _DEBTOOLS_H__


// cannot get debug out running in Code::Blocks - here is my own...
#ifdef __WXDEBUG__

#include <string.h>
#include <stdio.h>

   #define DPRINT(f) ::fprintf(stderr, (f) )
   #define DPRINT1(f,s) ::fprintf(stderr, (f) , (s) )
   #define DPRINT2(f,s1,s2) ::fprintf(stderr, (f) , (s1) , (s2) )

#else
   #define DPRINT(f)
   #define DPRINT1(f,s)
   #define DPRINT2(f,s1,s2)
#endif

#endif // _DEBTOOLS_H__
