# jsonParser
lib to manage json

## need to be done:

Need to be done next:
- [x] support array
- [x] correct bug on display
- [x] prevent multiple element with same key in one object
- [ ] manage integers
- [ ] add function to parse string
- [x] add function to get one element
- [ ] add function to set an element
- [ ] add function to print JSON in file/string
- [ ] add documentation in header
- [ ] **verify memory leak with valgrind**

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

	void * value = NULL;
	JSON_TYPE type = jT ( undefined );


	if ( jsonParseFile ( "file.json", &data, &dataLength ) )
	{
		printf ( "error\n");
	}
	
	jsonPrint ( data, 0, 0 );

	jsonGet ( data, 0, "h1", &value, &type );

	printf ( "h1 value  = %s type : %d\n", (char *)value, type );

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