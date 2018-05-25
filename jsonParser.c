////////////////////////////////////////////////////////////////////////////////
/// \copiright ox223252, 2017
///
/// This program is free software: you can redistribute it and/or modify it
///     under the terms of the GNU General Public License published by the Free
///     Software Foundation, either version 2 of the License, or (at your
///     option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
///     ANY WARRANTY; without even the implied of MERCHANTABILITY or FITNESS FOR
///     A PARTICULAR PURPOSE. See the GNU General Public License for more
///     details.
///
/// You should have received a copy of the GNU General Public License along with
///     this program. If not, see <http://www.gnu.org/licenses/>
////////////////////////////////////////////////////////////////////////////////dz

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include "jsonParser.h"

static uint32_t jsonParseArray ( const char * content, uint64_t * const contentId, uint32_t numObj, json_el ** out, uint32_t * outLength );
static uint32_t jsonParseObject ( const char * content, uint64_t * const contentId, uint32_t numObj, json_el ** out, uint32_t * outLength );

static uint32_t getELement ( const char * str, void ** const value, JSON_TYPE type, uint64_t * const id )
{
	uint32_t i = 0;

	switch ( type )
	{
		case jT( str ):
		{
			// remove '\"'
			(*id)++;
			str++;

			// get str size
			i = 0;
			while ( str[ i ] != '\"' && 
				i < 0xfffffffe )
			{
				if ( str[ i ] == '\\' && 
					str[ i + 1 ] == '\"' )
				{
					i += 2;
				}
				else
				{
					i++;
				}
			}

			(*value) = malloc ( i + 1 );
			if ( !(*value) )
			{
				return ( __LINE__ );
			}
			(( char * )(*value))[ i ] = '\0';
			memcpy ( *value, str, i );


			(*id)+= i + 1;
			break;
		}
	}
	return ( 0 );
}

static uint32_t jsonParseArray ( const char * content, uint64_t * const contentId, uint32_t numObj, json_el ** out, uint32_t * outLength )
{
	void * tmp = NULL;
	char str[ 256 ];
	double value;
	uint32_t i; // loop counter

	(*out)[ numObj ].key = NULL;

	switch ( content [ *contentId ] )
	{
		case '[':
		{
			if ( content[ (*contentId) + 1 ] != ']' )
			{
				do
				{
					(*contentId)++;

					// get next value
					tmp = realloc ( (*out)[ numObj ].value, sizeof ( void * ) * (*out)[ numObj ].length + 1 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					(*out)[ numObj ].value = tmp;
					tmp = NULL;

					// set next element type
					tmp = realloc ( (*out)[ numObj ].type, sizeof ( uint8_t ) * (*out)[ numObj ].length + 1 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					(*out)[ numObj ].type = tmp;
					(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( undefined );
					tmp = NULL;


					switch ( content [ *contentId ] )
					{
						case '{':
						case '[':
						{
							if ( content [ *contentId ] == '{')
							{
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( obj );
							}
							else
							{
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( array );
							}
							
							(*outLength)++;

							tmp = realloc ( *out, sizeof ( json_el ) * (*outLength) );
							if ( !tmp )
							{
								return ( __LINE__ );
							}
							*out = tmp;
							tmp = NULL;

							(*out)[ (*outLength) - 1 ].key = NULL;
							(*out)[ (*outLength) - 1 ].value = NULL;
							(*out)[ (*outLength) - 1 ].type = NULL;
							(*out)[ (*outLength) - 1 ].length = 0;
							(*out)[ (*outLength) - 1 ].parent = NULL;

							(*out)[ numObj ].value[ (*out)[ numObj ].length ] = &(*out)[ (*outLength) - 1 ];
							(*out)[ (*outLength) - 1 ].parent = &(*out)[ numObj ];

							if ( ( content [ *contentId ] == '{') &&
								jsonParseObject ( content, contentId, (*outLength) - 1, out, outLength ) ||
								( content [ *contentId ] == '[') &&
								jsonParseArray ( content, contentId, (*outLength) - 1, out, outLength ) )
							{
								return ( __LINE__ );
							}
							(*contentId)++;
							break;
						}
						case '"':
						{ // str element
							(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( str );
							
							getELement ( 
								&content [ *contentId ], 
								&(*out)[ numObj ].value[ (*out)[ numObj ].length ], 
								jT( str ),
								contentId );
							break;
						}
						default:
						{ // number of boolean
							if ( sscanf ( &content [ *contentId ], "%[true|false]", str ) )
							{ // boolean
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( bool );

								// get next value
								tmp = malloc ( sizeof ( uint8_t ) );
								if ( !tmp )
								{
									return ( __LINE__ );
								}
								(*out)[ numObj ].value[ (*out)[ numObj ].length ] = tmp;
								tmp = NULL;

								*( uint8_t * )( (*out)[ numObj ].value[ (*out)[ numObj ].length ] ) = ( strcmp ( "true", str )?1:0 );
								(*contentId) += strlen ( str );
							}
							else if ( ( content [ *contentId ] >= '0' ) &&
								( content [ *contentId ] <= '9' ) ||
								( content [ *contentId ] == '.' ) )
							{ // it's a number
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( float );

								if ( !sscanf ( &content [ *contentId ], "%lf", &value ) )
								{
									return ( __LINE__ );
								}

								// get next value
								tmp = malloc ( sizeof ( double ) );
								if ( !tmp )
								{
									return ( __LINE__ );
								}
								(*out)[ numObj ].value[ (*out)[ numObj ].length ] = tmp;
								tmp = NULL;

								*( double * )( (*out)[ numObj ].value[ (*out)[ numObj ].length ] ) = value;

								while ( ( content [ *contentId ] >= '0' ) &&
									( content [ *contentId ] <= '9' ) ||
									( content [ *contentId ] == '.' ) )
								{
									(*contentId)++;
								}
							}
							break;
						}
					}

					(*out)[ numObj ].length++;
				}
				while ( content[ *contentId ] == ',' );
			}
			else
			{
				(*contentId) += 2;
			}

			break;
		}
		case '{':
		{
			return ( jsonParseObject ( content, contentId, (*outLength) - 1, out, outLength ) );
		}
	}

	return ( 0 );
}

static uint32_t jsonParseObject ( const char * content, uint64_t * const contentId, uint32_t numObj, json_el ** out, uint32_t * outLength )
{
	void * tmp = NULL;
	char str[ 256 ];
	double value;
	uint32_t i, j; // loop counter

	switch ( content [ *contentId ] )
	{
		case '{':
		{

			if ( content[ (*contentId) + 1 ] != '}' )
			{
				do
				{
					// remove '{' or ','
					(*contentId)++;

					if ( sscanf ( &content[ *contentId ], "\"%256[^\"]\":", str ) == 0 )
					{
						return ( __LINE__ );
					}
					
					// get key of next element
					tmp = realloc ( (*out)[ numObj ].key, sizeof ( char * ) * (*out)[ numObj ].length + 1 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					(*out)[ numObj ].key = tmp;
					tmp = NULL;

					// init obj
					(*out)[ numObj ].key[ (*out)[ numObj ].length ] = NULL;

					tmp = malloc ( strlen ( str ) + 1 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					(*out)[ numObj ].key[ (*out)[ numObj ].length ] = tmp;
					strcpy ( (*out)[ numObj ].key[ (*out)[ numObj ].length ], str );
					tmp = NULL;
					
					// remove '"<str>":'
					(*contentId) += strlen ( str ) + 3;

					// get next value
					tmp = realloc ( (*out)[ numObj ].value, sizeof ( void * ) * (*out)[ numObj ].length + 1 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					(*out)[ numObj ].value = tmp;
					tmp = NULL;

					// set next element type
					tmp = realloc ( (*out)[ numObj ].type, sizeof ( uint8_t ) * (*out)[ numObj ].length + 1 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					(*out)[ numObj ].type = tmp;
					(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( undefined );
					tmp = NULL;

					switch ( content [ *contentId ] )
					{
						case '{':
						case '[':
						{
							if ( content [ *contentId ] == '{')
							{
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( obj );
							}
							else
							{
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( array );
							}
							
							(*outLength)++;

							tmp = realloc ( *out, sizeof ( json_el ) * (*outLength) );
							if ( !tmp )
							{
								return ( __LINE__ );
							}
							*out = tmp;
							tmp = NULL;

							(*out)[ (*outLength) - 1 ].key = NULL;
							(*out)[ (*outLength) - 1 ].value = NULL;
							(*out)[ (*outLength) - 1 ].type = NULL;
							(*out)[ (*outLength) - 1 ].length = 0;
							(*out)[ (*outLength) - 1 ].parent = NULL;

							tmp = malloc ( sizeof ( uint32_t ) );
							if ( !tmp )
							{
								return ( __LINE__ );
							}
							(*out)[ numObj ].value[ (*out)[ numObj ].length ] = tmp;
							tmp = NULL;

							*( uint32_t * )((*out)[ numObj ].value[ (*out)[ numObj ].length ]) = (*outLength) - 1;
							(*out)[ (*outLength) - 1 ].parent = &(*out)[ numObj ];

							if ( ( content [ *contentId ] == '{') &&
								jsonParseObject ( content, contentId, (*outLength) - 1, out, outLength ) ||
								( content [ *contentId ] == '[') &&
								jsonParseArray ( content, contentId, (*outLength) - 1, out, outLength ) )
							{
								return ( __LINE__ );
							}
							(*contentId)++;
							break;
						}
						case '"':
						{ // str element
							(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( str );

							getELement ( 
								&content [ *contentId ], 
								&((*out)[ numObj ].value[ (*out)[ numObj ].length ]),
								jT( str ),
								contentId );
							break;
						}
						default:
						{ // number of boolean
							if ( sscanf ( &content [ *contentId ], "%[true|false]", str ) )
							{ // boolean
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( bool );
								// get next value
								tmp = malloc ( sizeof ( uint8_t ) );
								if ( !tmp )
								{
									return ( __LINE__ );
								}
								(*out)[ numObj ].value[ (*out)[ numObj ].length ] = tmp;
								tmp = NULL;


								*( uint8_t * )( (*out)[ numObj ].value[ (*out)[ numObj ].length ] ) = ( strcmp ( "true", str )?1:0 );
								(*contentId) += strlen ( str );
							}
							else if ( ( content [ *contentId ] >= '0' ) &&
								( content [ *contentId ] <= '9' ) ||
								( content [ *contentId ] == '.' ) )
							{ // it's a number
								(*out)[ numObj ].type[ (*out)[ numObj ].length ] = jT( float );

								if ( !sscanf ( &content [ *contentId ], "%lf", &value ) )
								{
									return ( __LINE__ );
								}

								// get next value
								tmp = malloc ( sizeof ( double ) );
								if ( !tmp )
								{
									return ( __LINE__ );
								}
								(*out)[ numObj ].value[ (*out)[ numObj ].length ] = tmp;
								tmp = NULL;

								*( double * )( (*out)[ numObj ].value[ (*out)[ numObj ].length ] ) = value;

								while ( ( content [ *contentId ] >= '0' ) &&
									( content [ *contentId ] <= '9' ) ||
									( content [ *contentId ] == '.' ) )
								{
									(*contentId)++;
								}
							}
						}
					}


					(*out)[ numObj ].length++;
				}
				while ( content [ *contentId ] == ',' );
			}
			else
			{
				(*contentId) += 2;
			}

			break;
		}
		case '[':
		{
			return ( jsonParseArray ( content, contentId, (*outLength) - 1, out, outLength ) );
		}
	}

	return ( 0 );
}

uint32_t jsonParseFile ( const char * file, json_el ** out, uint32_t * length )
{
	FILE * f = NULL;
	uint64_t fileSize = 0;
	char * fileContent = NULL;
	uint32_t bracketLvl = 0;
	uint32_t squareLvl = 0;

	char str[ 10 ];

	json_el *current;

	uint64_t i = 0; // loop counter
	uint64_t j = 0; // loop counter

	struct
	{
		uint8_t string:1,
			value:1,
			array:1,
			obj:1,
			key:1;
	}
	flag = { 0 };

	if ( !out ||
		*out ||
		!length ||
		*length )
	{ // out pointer is null or *out pointer is not null
		return ( __LINE__ );
	}

	f = fopen ( file, "r" );
	if ( !f )
	{ // file not open
		return ( __LINE__ );
	}

	// get file size
	fseek ( f, 0, SEEK_END );
	fileSize = ftell ( f );

	fseek ( f, 0, SEEK_SET );

	fileContent = malloc ( fileSize );
	if ( !fileContent )
	{
		fclose ( f );
		return ( __LINE__ );
	}

	if ( fread ( fileContent, 1, fileSize, f ) != fileSize )
	{ // not read all
		fclose ( f );
		free ( fileContent );
		return ( __LINE__ );
	}

	// not need to keep file open
	fclose ( f );

	// remove useless bytes (space/tab/newlines)
	for ( i = fileSize - 1; i > 0; i-- )
	{
		if ( ( fileContent[ i ] == ' ' ) ||
			( fileContent[ i ] == '\n' ) ||
			( fileContent[ i ] == '\t' ) )
		{
			for ( j = i; j < fileSize - 1; j++ )
			{
				fileContent[ j ] = fileContent[ j + 1 ];
			}
			fileContent[ j ] = '\0';
			fileSize--;
		}
	}

	// verify format
	for ( i = 0; i < fileSize; i++ )
	{
		if ( flag.string && 
			fileContent[ i ] != '"' )
		{
			continue;
		}
		switch ( fileContent[ i ] )
		{
			case '"':
			{ // set if next element is in stirng or not 
				flag.string ^= 0x01;
				break;
			}
			case '{':
			{
				bracketLvl++;
				break;
			}
			case '}':
			{
				bracketLvl--;
				break;
			}
			case '[':
			{
				squareLvl++;
				break;
			}
			case ']':
			{
				squareLvl--;
				break;
			}
			case ':':
			case ',':
			{
				if ( sscanf ( &fileContent[ i + 1 ], "%[true|false]", str ) )
				{ // boolean
					i += strlen ( str );
				}
				else if ( ( fileContent[ i + 1 ] != '"' ) && // string
					( fileContent[ i + 1 ] != '.' ) && // number
					( fileContent[ i + 1 ] != '{' ) && // obj
					( fileContent[ i + 1 ] != '[' ) && // array
					( ( fileContent[ i + 1 ] < '0' ) ||
					( fileContent[ i + 1 ] > '9' ) ) )
				{
					printf ( "format problem ...%c%c%c...\n", fileContent[ i - 1 ], fileContent[ i ],fileContent[ i + 1 ] );
					printf ( "%s\n", fileContent );
					free ( fileContent );
					return ( __LINE__ );
				}
				break;
			}
			default:
			{
				if ( ( fileContent[ i ] != '.' ) && // number
					( ( fileContent[ i ] < '0' ) || // number
					( fileContent[ i ] > '9' ) ) )
				{
					printf ( "format problem ...%c%c%c...\n", fileContent[ i - 1 ], fileContent[ i ], fileContent[ i + 1 ] );
					free ( fileContent );
					return ( __LINE__ );
				}
				break;
			}
		}
	}

	if ( ( bracketLvl != 0 ) ||
		( squareLvl != 0 ) ||
		( flag.string ) )
	{
		printf ( "format problem [ ] = %d, { } = %d \" \" = %s\n",
			squareLvl,
			bracketLvl,
			( flag.string )? "error" : "ok" );
		free ( fileContent );
		return ( __LINE__ );
	}

	*out = malloc ( sizeof ( json_el ) * 1 );

	if ( !*out )
	{
		free ( fileContent );
		return ( __LINE__ );
	}
	(*length)++;
	(*out)->key = NULL;
	(*out)->value = NULL;
	(*out)->type = NULL;
	(*out)->length = 0;
	(*out)->parent = NULL;

	i = 0;
	jsonParseObject ( fileContent, &i, 0, out, length );
	
	free ( fileContent );
	return ( 0 );
}

uint32_t jsonPrint ( json_el * data, uint32_t id, uint8_t indent )
{
	uint32_t i = 0; // loop counter
	uint8_t j = 0;

	if ( !data ||
		!data[ id ].type &&
		!data[ id ].value &&
		!data[ id ].key )
	{ // data pointer is null
		return ( __LINE__ );
	}

	if ( !indent )
	{
		printf ( "{\n" );
	}

	for ( j = 0; j < indent + 1; j++ )
	{
		printf ( "\t" );
	}
	printf ( "length:%d\n", data[ id ].length );

	for ( i = 0; i < data[ id ].length; i++ )
	{
		for ( j = 0; j < indent + 1; j++ )
		{
			printf ( "\t" );
		}

		if ( data[ id ].key &&
			data[ id ].key[ i ] )
		{
			printf ( "\"%s\":", data[ id ].key[ i ] );
		}

		if ( !data[ id ].type )
		{
			printf ( "no data\n");
		}
		else switch ( data[ id ].type[ i ] )
		{
			case jT( undefined ):
			{
				printf ( "undefined" );
				break;
			}
			case jT( bool ):
			{
				printf ( "%s", ( *( uint8_t * )data[ id ].value[ i ] )?"true":"false" );
				break;
			}
			case jT( float ):
			{
				printf ( "%lf", *( double * )data[ id ].value[ i ] );
				break;
			}
			case jT( str ):
			{
				printf ( "\"%s\"", ( char * )(data[ id ].value[ i ]) );
				break;
			}
			case jT( obj ):
			{
				printf ( "{\n" );
				jsonPrint ( data, *( uint32_t * )(data[ id ].value[ i ]), indent + 1 );
				for ( j = 0; j < indent + 1; j++ )
				{
					printf ( "\t" );
				}
				printf ( "}" );
				break;
			}
			case jT( array ):
			{
				printf ( "[\n" );
				jsonPrint ( data, *( uint32_t * )(data[ id ].value[ i ]), indent + 1 );
				for ( j = 0; j < indent + 1; j++ )
				{
					printf ( "\t" );
				}
				printf ( "]" );
				break;
			}
		}

		if ( i < data[ id ].length - 1 )
		{
			printf ( "," );
		}

		printf ( "\n" );
	}

	if ( !indent )
	{
		printf ( "}\n" );
	}
}

uint32_t jsonFree ( json_el ** data, uint32_t length )
{
	uint32_t i = 0, j = 0;

	if ( !data )
	{
		return ( __LINE__ );
	}

	for ( i = 0; i < length; i++ )
	{
		for ( j = 0; j < (*data)[ i ].length; j++ )
		{
			if ( (*data)[ i ].key &&
				(*data)[ i ].key[ j ] )
			{
				free ( (*data)[ i ].key[ j ] );
				(*data)[ i ].key[ j ] = NULL;
			}

			if ( (*data)[ i ].value &&
				(*data)[ i ].value[ j ] )
			{
				if ( ( (*data)[ i ].type[ j ] == jT( float ) ) ||
					( (*data)[ i ].type[ j ] == jT( str ) ) ||
					( (*data)[ i ].type[ j ] == jT( bool ) ) )
				{
					free ( (*data)[ i ].value[ j ] );
				}
				else
				{ // undefined / array / obj
					// nothing to be done
				}
				(*data)[ i ].value[ j ] = NULL;
			}
		}
		free ( (*data)[ i ].key );
		free ( (*data)[ i ].value );
		free ( (*data)[ i ].type );
		(*data)[ i ].key = NULL;
		(*data)[ i ].value = NULL;
		(*data)[ i ].type = NULL;
	}

	free ( *data );

	return ( 0 );
}