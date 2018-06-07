# jsonParser
lib to manage json

## need to be done:

Need to be done next:
- [x] support array
- [x] correct bug on display
- [x] prevent multiple element with same key in one object
- [ ] manage integers
  - [x] created by user by jsonSetObject()
  - [ ] in a read file / string
- [x] add function to parse string
- [x] add function to get one element
- [x] add function to set an element
- [ ] add function to set an object
- [x] add function to print JSON in file/string
- [x] add documentation in header
- [x] remove properly object when an element is removed form array
- [ ] **verify memory leak with valgrind**

## example:
### main.c:
```C
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "jsonParser/jsonParser.h"

int main ( void )
{
	json_el *data = NULL;
	uint32_t dataLength = 0;

	void * value = NULL;
	uint64_t length = 0;
	JSON_TYPE type = jT ( undefined );


	printf ( "\nFirst object:\n" );
	if ( jsonParseFile ( "file.json", &data, &dataLength ) )
	{
		printf ( "error\n");
	}
	jsonPrint ( data, 0, 0 );

	printf ( "\nElement:\n" );
	jsonGet ( data, 0, "h1", &value, &type );

	printf ( "h1 value  = %s type : %d\n", (char *)value, type );

	printf ( "\nFirst object with replacements:\n" );
	value = malloc ( 54 );
	sprintf ( ( char * ) value, "ceci est un long test qui va remplacer la valeur de G" );
	if ( jsonSet ( data, 0, "h", value, jT ( str ) ) )
	{
		printf ( "error\n" );
		free ( value );
	}
	else
	{
		value = NULL;
	}
	value = malloc ( 11 );
	sprintf ( ( char * ) value, "ajout en L" );
	if ( jsonSet ( data, 0, "l", value, jT ( str ) ) )
	{
		printf ( "error\n" );
		free ( value );
	}
	else
	{
		value = NULL;
	}
	jsonPrint ( data, 0, 0 );

	if ( data )
	{
		jsonFree ( &data, dataLength );
		data = NULL;
		dataLength = 0;
	}

	jsonParseString ( "[1,2,3,4,5,6,7,{\"alpha\":\"test\"}]", &data, &dataLength );

	printf ( "\nSecond object:\n" );
	jsonPrint ( data, 0, 0 );

	// get json in string
	if ( jsonPrintString ( data, 0, ( char ** )&value, NULL ) )
	{
		// probleme
	}
	else
	{
		printf ( "%s\n", ( char * ) value );
	}

	if ( value )
	{
		free ( value );
		value = NULL;
	}

	// try to put json in a small string it should be fail
	length = 20;
	if ( jsonPrintString ( data, 0, ( char ** )&value, &length ) )
	{ // normal case on failure
		if ( value )
		{ // failure but some work already done
			printf ( "%s %lu\n", ( char * ) value, strlen ( ( char * ) value ) );
		}
	}

	if ( value )
	{
		free ( value );
		value = NULL;
	}



	if ( data )
	{
		jsonFree ( &data, dataLength );
		data = NULL;
		dataLength = 0;
	}

	return ( 0 );
}
```

### file.json:
```Json
{
	"a":"bob",
	"b":"pour avoir un b",
	"c":"testé",
	"b":"second B\"",
	"d":12,
	"e":true,
	"f":false,
	"i":[ 1, 2 ],
	"g":
	{
		"g1":"ici",
		"g2":"bob",
		"g3":"testé",
		"g4":12,
		"g5":true,
		"g6":false,
		"g7":
		{

			"g71":"ici",
			"g72":{
				"g721":"B52",
				"g722":100,
				"g723":999
			}
		},
		"g8":
		{
			"g81":"rien",
			"g82":{
			}
		}
	},
	"h":{
		"h1":"B52",
		"h2":100,
		"h3":999
	},
	"k":
	[
		1,2,3,4,5,6,7,8,9,10,11,12,13
	],
	"g":"poney"
}

```

### display output:
```Shel
> gcc main.c jsonParser/jsonParser.c -Wall && ./a.out

First object:
{
        "a":"bob",
        "b":"second B\"",
        "c":"testé",
        "d":12.000000,
        "e":false,
        "f":true,
        "i":[
                1.000000,
                2.000000
        ],
        "g":"poney",
        "h":{
                "h1":"B52",
                "h2":100.000000,
                "h3":999.000000
        },
        "k":[
                1.000000,
                2.000000,
                3.000000,
                4.000000,
                5.000000,
                6.000000,
                7.000000,
                8.000000,
                9.000000,
                10.000000,
                11.000000,
                12.000000,
                13.000000
        ]
}

Element:
h1 value  = B52 type : 3

First object with replacements:
{
        "a":"bob",
        "b":"second B\"",
        "c":"testé",
        "d":12.000000,
        "e":false,
        "f":true,
        "i":[
                1.000000,
                2.000000
        ],
        "g":"poney",
        "h":"ceci est un long test qui va remplacer la valeur de G",
        "k":[
                1.000000,
                2.000000,
                3.000000,
                4.000000,
                5.000000,
                6.000000,
                7.000000,
                8.000000,
                9.000000,
                10.000000,
                11.000000,
                12.000000,
                13.000000
        ],
        "l":"ajout en L"
}

Second object:
[
        1.000000,
        2.000000,
        3.000000,
        4.000000,
        5.000000,
        6.000000,
        7.000000,
        {
                "alpha":"test"
        }
]
[1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,7.000000,{"alpha":"test"}]
[1.000000,2.000000, 19
```
