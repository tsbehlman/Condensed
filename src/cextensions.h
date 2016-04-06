#pragma once

#define likely(x)      __builtin_expect( !!( x ), 1 )
#define unlikely(x)    __builtin_expect( !!( x ), 0 )

#define unless( condition ) if( !( likely( condition ) ) )
#define until( condition ) while( !( likely( condition ) ) )

#define new( type ) ( (type*) malloc( sizeof(type) ) )
#define Array( type, number ) ( (type*) malloc( number * sizeof(type) ) )

#define String( length ) ( (String) calloc( length, sizeof(char) ) )

#define min( a, b ) ( likely( a < b ) ? a : b )
#define max( a, b ) ( likely( a > b ) ? a : b )

typedef char* String;
typedef struct tm Time;