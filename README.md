# jsonParser
lib to manage json

## bug:
If you execute the main test programm, you will see only 2 levels of json instead of 3 or 4 levels existing.
At the second level, the length is shorter than reality of x units.

## need to be done:

Need to be done next:
- [ ] support array
- [x] correct bug on display
- [ ] prevent multiple element with same key in one object
- [ ] add function to get one element
- [ ] add function to set on element
 -[ ] add function to print JSON in file/string 

## example:
### main.c:
```C
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "jsonParser/jsonParser.h"

int main ( void )
{
	json_el *data = NULL;
	uint32_t dataLength = 0;
	uint8_t i = 0;

	// while ( ++i )
	{
		if ( data )
		{
			jsonFree ( &data, dataLength );
			data = NULL;
			dataLength = 0;
		}

		printf ( "\e[A\e[2K");
		printf ( "test n°%3d ", i );
		if ( jsonParseFile ( "file.json", &data, &dataLength ) )
		{
			printf ( "error\n");
		}
		printf ( "done\n");

		usleep ( 10 );
	}
	
	jsonPrint ( data, 0 );

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
			"g81":"rien"
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
	]
}
```