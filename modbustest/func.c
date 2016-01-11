#include "func.h"

int isHexCode( char *code )
{
	int len = strlen(code);

	int i;

	for ( i=0 ; i<len ; i++ )
	{
		if ( code[i] <48 ||
			code[i] > 70 ||
			(code[i] >57 && code[i] <65))
		{
			return 0;		// this code is not Hex code.
		}
	}
	return 1;	// this code is Hex code.
}
