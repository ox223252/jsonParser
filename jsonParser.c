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
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include "jsonParser.h"

#ifdef DEBUG_MODE
#define DEBUG(...) {printf("\e[1;34m%s \e[33m%d\e[0m : ", __func__, __LINE__ );printf(__VA_ARGS__);}
#else
#define DEBUG(...)
#endif

static uint32_t getELement ( const char * str, void ** const value, JSON_TYPE type, uint32_t * const id );
static uint32_t jsonParseObject ( const char * content, uint32_t * const contentId, uint32_t numObj, json_el ** out, uint32_t * outLength );
static uint32_t jsonVerifyFormat ( char * const str, uint64_t * const strSize );
static uint32_t jsonRemoveSpaces ( char * const str, uint64_t * const strSize );
static uint32_t jsonFreeOne ( json_el * const data, uint32_t id );


static uint32_t getELement ( const char * str, void ** const value, JSON_TYPE type, uint32_t * const id )
{
	uint32_t i = 0;

	if ( !str ||
		!value ||
		!id )
	{
		return ( __LINE__ );
	}

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
		default:
		{
			return ( __LINE__ );
		}
	}
	return ( 0 );
}

static uint32_t jsonParseObject ( const char * content, uint32_t * const contentId, uint32_t numObj, json_el ** out, uint32_t * outLength )
{
	void * tmp = NULL;
	char str[ 256 ];
	double value;
	uint32_t nextValueId = 0;


	if ( content [ *contentId ] == '[' )
	{
		(*out)[ numObj ].key = NULL;
	}

	switch ( content [ *contentId ] )
	{
		case '{':
		case '[':
		{
			if ( ( content[ (*contentId) + 1 ] != '}' ) &&
				( content[ (*contentId) + 1 ] != ']' ) )
			{
				do
				{

					// remove '{' or ','
					(*contentId)++;

					nextValueId = (*out)[ numObj ].length;
					if ( (*out)[ numObj ].key ||
						( content [ (*contentId) - 1 ] == '{' ) )
					{ // for obj manage keys
						if ( sscanf ( &content[ *contentId ], "\"%256[^\"]\":", str ) == 0 )
						{
							return ( __LINE__ );
						}

						if ( (*out)[ numObj ].key )
						{ // if key table is set  -> we'll try to find new key in existing table
							for ( nextValueId = 0; nextValueId < (*out)[ numObj ].length; nextValueId++ )
							{
								if ( !strcmp ( str, (*out)[ numObj ].key[ nextValueId ] ) )
								{ // new key already exist
									break;
								}
							}
						}

						if ( nextValueId == (*out)[ numObj ].length )
						{ // if next id is last -> key is not in the table
							// get key of next element
							tmp = realloc ( (*out)[ numObj ].key, sizeof ( char * ) * ( (*out)[ numObj ].length + 1 ) );
							if ( !tmp )
							{
								return ( __LINE__ );
							}
							(*out)[ numObj ].key = tmp;
							tmp = NULL;

							(*out)[ numObj ].key[ nextValueId ] = NULL;
						}

						if ( !(*out)[ numObj ].key[ nextValueId ] )
						{ // if next key is null -> key not exist -> need to be created
							// init obj
							tmp = malloc ( strlen ( str ) + 1 );
							if ( !tmp )
							{
								return ( __LINE__ );
							}
							(*out)[ numObj ].key[ nextValueId ] = tmp;
							strcpy ( (*out)[ numObj ].key[ nextValueId ], str );
							tmp = NULL;
						}

						// remove '"<str>":'
						(*contentId) += strlen ( str ) + 3;
					}

					if ( nextValueId == (*out)[ numObj ].length )
					{ // it's a new object
						// get next value
						tmp = realloc ( (*out)[ numObj ].value, sizeof ( void * ) * ( nextValueId + 1 ) );
						if ( !tmp )
						{
							return ( __LINE__ );
						}
						(*out)[ numObj ].value = tmp;
						(*out)[ numObj ].value[ nextValueId ] = NULL;
						tmp = NULL;

						// set next element type
						tmp = realloc ( (*out)[ numObj ].type, sizeof ( uint8_t ) * ( nextValueId + 1 ) );
						if ( !tmp )
						{
							return ( __LINE__ );
						}
						(*out)[ numObj ].type = tmp;
						(*out)[ numObj ].type[ nextValueId ] = jT( undefined );
						tmp = NULL;
					}
					else
					{ // this object replace another one
						switch ( (*out)[ numObj ].type[ nextValueId ] )
						{
							case jT ( bool ):
							case jT ( double ):
							case jT ( str ):
							case jT ( obj ):
							case jT ( array ):
							{
								free ( (*out)[ numObj ].value[ nextValueId ] );
								(*out)[ numObj ].value[ nextValueId ] = NULL;
								break;
							}
						}
					}

					switch ( content [ *contentId ] )
					{
						case '{':
						case '[':
						{
							if ( content [ *contentId ] == '{')
							{
								(*out)[ numObj ].type[ nextValueId ] = jT( obj );
							}
							else
							{
								(*out)[ numObj ].type[ nextValueId ] = jT( array );
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

							(*out)[ numObj ].value[ nextValueId ] = malloc ( sizeof ( uint32_t ) );

							*( uint32_t * )((*out)[ numObj ].value[ nextValueId ]) = (*outLength) - 1;

							if ( jsonParseObject ( content, contentId, (*outLength) - 1, out, outLength ) )
							{
								return ( __LINE__ );
							}
							(*contentId)++;
							break;
						}
						case '"':
						{ // str element
							(*out)[ numObj ].type[ nextValueId ] = jT( str );

							getELement (
								&content [ *contentId ],
								&((*out)[ numObj ].value[ nextValueId ]),
								jT( str ),
								contentId );
							break;
						}
						default:
						{ // number of boolean
							if ( sscanf ( &content [ *contentId ], "%[true|false]", str ) )
							{ // boolean
								(*out)[ numObj ].type[ nextValueId ] = jT( bool );
								// get next value
								tmp = malloc ( sizeof ( uint8_t ) );
								if ( !tmp )
								{
									return ( __LINE__ );
								}
								(*out)[ numObj ].value[ nextValueId ] = tmp;
								tmp = NULL;


								*( uint8_t * )( (*out)[ numObj ].value[ nextValueId ] ) = ( strcmp ( "true", str )?1:0 );
								(*contentId) += strlen ( str );
							}
							else if ( ( ( content [ *contentId ] >= '0' ) &&
								( content [ *contentId ] <= '9' ) ) ||
								( content [ *contentId ] == '.' ) )
							{ // it's a number
								(*out)[ numObj ].type[ nextValueId ] = jT( double );

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
								(*out)[ numObj ].value[ nextValueId ] = tmp;
								tmp = NULL;

								*( double * )( (*out)[ numObj ].value[ nextValueId ] ) = value;

								while ( ( ( content [ *contentId ] >= '0' ) &&
									( content [ *contentId ] <= '9' ) ) ||
									( content [ *contentId ] == '.' ) )
								{
									(*contentId)++;
								}
							}
						}
					}

					if ( nextValueId == (*out)[ numObj ].length )
					{
						(*out)[ numObj ].length++;
					}
				}
				while ( content [ *contentId ] == ',' );
			}
			else
			{
				(*contentId)++;
			}

			break;
		}
		default:
		{ // never shoud occured once you get a '{' or '['
			(*contentId)++;
			break;
		}
	}

	return ( 0 );
}

static uint32_t jsonVerifyFormat ( char * const str, uint64_t * const strSize )
{
	uint64_t i = 0; // loop counter
	uint8_t string = 0; // flag to store if char is in string or not
	uint32_t bracketLvl = 0;
	uint32_t squareLvl = 0;
	char tmp[ 6 ];

	if ( !str ||
		!strSize )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	// verify format
	for ( i = 0; i < (*strSize); i++ )
	{
		if ( string &&
			str[ i ] != '"' )
		{
			continue;
		}
		switch ( str[ i ] )
		{
			case '"':
			{ // set if next element is in stirng or not
				if ( string &&
					( str[ i -1 ] == '\\' ) )
				{
					continue;
				}
				string ^= 0x01;
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
				if ( sscanf ( &str[ i + 1 ], "%[true|false]", tmp ) )
				{ // boolean
					i += strlen ( tmp );
				}
				else if ( ( str[ i + 1 ] != '"' ) && // string
					( str[ i + 1 ] != '.' ) && // number
					( str[ i + 1 ] != '{' ) && // obj
					( str[ i + 1 ] != '[' ) && // array
					( ( str[ i + 1 ] < '0' ) ||
					( str[ i + 1 ] > '9' ) ) )
				{
					printf ( "%d: format problem ...%c%c%c...\n", __LINE__, str[ i - 1 ], str[ i ],str[ i + 1 ] );
					printf ( "%s\n", str );
					free ( str );
					return ( __LINE__ );
				}
				break;
			}
			default:
			{
				if ( ( str[ i ] != '.' ) && // number
					( ( str[ i ] < '0' ) || // number
					( str[ i ] > '9' ) ) )
				{
					printf ( "%d: format problem ...%c%c%c...\n", __LINE__, str[ i - 1 ], str[ i ], str[ i + 1 ] );
					free ( str );
					return ( __LINE__ );
				}
				break;
			}
		}
	}
	return ( 0 );
}

static uint32_t jsonRemoveSpaces ( char * const str, uint64_t * const strSize )
{
	uint64_t i = 0; // loop counter
	uint64_t j = 0; // loop counter
	uint8_t string = 0; // flag to store if char is in string or not

	if ( !str ||
		!strSize )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	// remove useless bytes (space/tab/newlines)
	for ( i = (*strSize) - 1; i > 0; i-- )
	{
		if ( string &&
			str[ i ] != '"' )
		{ // if it's a string don't remove space or brake line
			continue;
		}

		if ( str[ i ] == '"' )
		{ // if it's beging or end of string store information
			if ( string &&
				( str[ i -1 ] == '\\' ) )
			{
				continue;
			}
			string ^= 0x01;
			continue;
		}

		if ( ( str[ i ] == ' ' ) ||
			( str[ i ] == '\n' ) ||
			( str[ i ] == '\r' ) ||
			( str[ i ] == '\t' ) )
		{ // else remove space, brake line, or tab
			for ( j = i; j < (*strSize) - 1; j++ )
			{
				str[ j ] = str[ j + 1 ];
			}
			str[ j ] = '\0';
			(*strSize)--;
		}
	}
	return ( 0 );
}

static uint32_t jsonFreeOne ( json_el * const data, uint32_t id )
{
	uint32_t j = 0;

	if ( !data )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	for ( j = 0; j < data[ id ].length; j++ )
	{
		if (  data[ id ].key &&
			 data[ id ].key[ j ] )
		{
			free (  data[ id ].key[ j ] );
			 data[ id ].key[ j ] = NULL;
		}

		if (  data[ id ].value &&
			 data[ id ].value[ j ]  &&
			( data[ id ].type[ j ] != jT ( ptrBool ) ) &&
			( data[ id ].type[ j ] != jT ( ptrUint8_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrUint16_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrUint32_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrUint64_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrInt8_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrInt16_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrInt32_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrInt64_t ) ) &&
			( data[ id ].type[ j ] != jT ( ptrFloat ) ) &&
			( data[ id ].type[ j ] != jT ( ptrDouble ) ) &&
			( data[ id ].type[ j ] != jT ( ptrStr ) ) )
		{
			free (  data[ id ].value[ j ] );
			data[ id ].value[ j ] = NULL;
		}
	}

	if ( data[ id ].key )
	{
		free (  data[ id ].key );
		 data[ id ].key = NULL;
	}

	if ( data[ id ].value )
	{
		free (  data[ id ].value );
		 data[ id ].value = NULL;
	}

	if ( data[ id ].type )
	{
		free (  data[ id ].type );
		 data[ id ].type = NULL;
	}

	return ( 0 );
}

uint32_t jsonParseString ( char * const str, json_el ** out, uint32_t * length )
{
	uint32_t objId = 0;
	uint64_t strSize = strlen ( str );

	if ( !out ||
		*out ||
		!length ||
		*length ||
		!str )
	{ // out pointer is null or *out pointer is not null
		errno = EINVAL;
		return ( __LINE__ );
	}

	if ( jsonRemoveSpaces ( str, &strSize ) )
	{
		return ( __LINE__ );
	}

	if ( jsonVerifyFormat ( str, &strSize ) )
	{
		printf ( "format problem width brackets\n" );
		free ( str );
		return ( __LINE__ );
	}

	*out = malloc ( sizeof ( json_el ) * 1 );

	if ( !*out )
	{
		free ( str );
		return ( __LINE__ );
	}
	(*length)++;
	(*out)->key = NULL;
	(*out)->value = NULL;
	(*out)->type = NULL;
	(*out)->length = 0;

	return ( jsonParseObject ( str, &objId, 0, out, length ) );
}

uint32_t jsonParseFile ( const char * file, json_el ** out, uint32_t * length )
{
	FILE * f = NULL;
	uint64_t fileSize = 0;
	char * fileContent = NULL;

	uint32_t ret = 0; // return value

	if ( !out ||
		*out ||
		!length ||
		*length ||
		!file )
	{ // out pointer is null or *out pointer is not null
		errno = EINVAL;
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

	fileContent = malloc ( fileSize + 1 );
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

	fileContent[ fileSize ] = '\0';

	// not need to keep file open
	fclose ( f );
	ret = jsonParseString ( fileContent, out, length );
	free ( fileContent );
	return ( ret );
}

#pragma GCC diagnostic ignored "-Wformat"
uint32_t jsonPrintFile ( json_el * const data, const uint32_t id, const uint8_t indent, FILE * const outFile )
{
	uint32_t i = 0; // loop counter
	uint8_t j = 0;

	if ( !outFile ||
		!data ||
		( !data[ id ].type &&
		!data[ id ].value &&
		!data[ id ].key ) )
	{ // data pointer is null
		errno = EINVAL;
		return ( __LINE__ );
	}

	if ( indent > 2 )
	{
		return ( 0 );
	}

	if ( !indent )
	{
		if ( data[ id ].key )
		{
			fprintf ( outFile, "{\n" );
		}
		else
		{
			fprintf ( outFile, "[\n" );
		}
	}

	for ( i = 0; i < data[ id ].length; i++ )
	{
		for ( j = 0; j < indent + 1; j++ )
		{
			fprintf ( outFile, "\t" );
		}

		if ( data[ id ].key &&
			data[ id ].key[ i ] )
		{
			fprintf ( outFile, "\"%s\":", data[ id ].key[ i ] );
		}

		if ( !data[ id ].type )
		{
			fprintf ( outFile, "no data\n");
		}
		else switch ( data[ id ].type[ i ] )
		{
			case jT( undefined ):
			{
				fprintf ( outFile, "undefined" );
				break;
			}
			case jT( ptrBool ):
			case jT( bool ):
			{
				fprintf ( outFile, "%s", ( *( uint8_t * )data[ id ].value[ i ] )?"true":"false" );
				break;
			}
			case jT( ptrUint8_t ):
			case jT( uint8_t ):
			{
				fprintf ( outFile, "%u", *( uint8_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrUint16_t ):
			case jT( uint16_t ):
			{
				fprintf ( outFile, "%u", *( uint16_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrUint32_t ):
			case jT( uint32_t ):
			{
				fprintf ( outFile, "%u", *( uint32_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrUint64_t ):
			case jT( uint64_t ):
			{
				fprintf ( outFile, "%llu", *( uint64_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrInt8_t ):
			case jT( int8_t ):
			{
				fprintf ( outFile, "%d", *( int8_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrInt16_t ):
			case jT( int16_t ):
			{
				fprintf ( outFile, "%d", *( int16_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrInt32_t ):
			case jT( int32_t ):
			{
				fprintf ( outFile, "%d", *( int32_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrInt64_t ):
			case jT( int64_t ):
			{
				fprintf ( outFile, "%lld", *( int64_t * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrFloat ):
			case jT( float ):
			{
				fprintf ( outFile, "%f", *( float * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrDouble ):
			case jT( double ):
			{
				fprintf ( outFile, "%lf", *( double * )data[ id ].value[ i ] );
				break;
			}
			case jT( ptrStr ):
			case jT( str ):
			{
				fprintf ( outFile, "\"%s\"", ( char * )(data[ id ].value[ i ]) );
				break;
			}
			case jT( obj ):
			{
				fprintf ( outFile, "{\n" );
				jsonPrintFile ( data, *( uint32_t * )(data[ id ].value[ i ]), indent + 1, outFile );
				for ( j = 0; j < indent + 1; j++ )
				{
					printf ( "\t" );
				}
				fprintf ( outFile, "}" );
				break;
			}
			case jT( array ):
			{
				fprintf ( outFile, "[\n" );
				jsonPrintFile ( data, *( uint32_t * )(data[ id ].value[ i ]), indent + 1, outFile );
				for ( j = 0; j < indent + 1; j++ )
				{
					printf ( "\t" );
				}
				fprintf ( outFile, "]" );
				break;
			}
		}

		if ( i < data[ id ].length - 1 )
		{
			fprintf ( outFile, "," );
		}

		fprintf ( outFile, "\n" );
	}

	if ( !indent )
	{
		if ( data[ id ].key )
		{
			fprintf ( outFile, "}\n" );
		}
		else
		{
			fprintf ( outFile, "]\n" );
		}
	}

	return ( 0 );
}

uint32_t jsonPrintString ( json_el * const data, const uint32_t id, char ** const outStr, uint64_t * const outLength )
{
	uint32_t i = 0; // loop counter
	void * tmp = NULL;
	char tmpStr[ 48 ];

	enum
	{
		JPS_UNDEFINED = 0,
		JPS_OUT_NULL = 0x01,
		JPS_OUT_SET = 0x02,
		JPS_LEN_NULL = 0x04,
		JPS_LEN_ZERO = 0x08,
		JPS_LEN_SET = 0x10
	}
	flag = JPS_UNDEFINED;

	if ( !outStr ||
		!data ||
		( !data[ id ].type &&
		!data[ id ].value &&
		!data[ id ].key ) )
	{ // data pointer is null
		errno = EINVAL;
		return ( __LINE__ );
	}

	// manage the differents cases available for outStr and outLength
	if ( !*outStr )
	{ // store the initial state of outStr
		flag |= JPS_OUT_NULL;
	}
	else
	{
		flag |= JPS_OUT_SET;
	}

	if ( !outLength )
	{ // store the initial state of outLength
		flag |= JPS_LEN_NULL;
	}
	else if ( *outLength == 0 )
	{
		flag |= JPS_LEN_ZERO;
	}
	else
	{
		flag |= JPS_LEN_SET;
	}

	if ( flag & JPS_OUT_NULL )
	{ // if outStr is null need to allocate memory
		if( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) )
		{
			*outStr = malloc ( 1024 );
		}
		else
		{ // flag & JPS_LEN_SET
			*outStr = malloc ( *outLength );
		}

		if ( !*outStr )
		{
			return ( __LINE__ );
		}
		*outStr[ 0 ] = '\0';
	}

	if ( data[ id ].key )
	{ // if key is not null it's an object
		sprintf ( &(*outStr)[ strlen ( *outStr ) ], "{" );
	}
	else
	{ // else it's an array
		sprintf ( &(*outStr)[ strlen ( *outStr ) ], "[" );
	}

	for ( i = 0; i < data[ id ].length; i++ )
	{
		if ( data[ id ].key &&
			data[ id ].key[ i ] )
		{ // if key exist print it
			// manage cases when outStr's size need to be  increase
			if ( ( flag & JPS_OUT_NULL ) &&
				( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) ) )
			{
				tmp = realloc ( *outStr, strlen ( *outStr ) + strlen ( ( char * )(data[ id ].key[ i ]) ) + 128 );
				if ( !tmp )
				{
					return ( __LINE__ );
				}
				*outStr = tmp;
			}
			else if ( flag & JPS_LEN_SET )
			{
				// verify if JSON is not too big to outStr 
				if ( ( strlen( *outStr ) + strlen ( data[ id ].key[ i ] ) + 2 ) > *outLength )
				{
					return ( __LINE__ );
				}
			}
			sprintf ( &(*outStr)[ strlen( *outStr ) ], "\"%s\":", data[ id ].key[ i ] );
		}
		else
		{ // if key is null but need to incerase string size
			// manage cases when outStr's size need to be increase
			if ( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) )
			{
				tmp = realloc ( *outStr, strlen ( *outStr ) +  128 );
				if ( !tmp )
				{
					return ( __LINE__ );
				}
				*outStr = tmp;
			}
		}

		if ( !data[ id ].type )
		{ // if data not exist
			if ( flag & JPS_LEN_SET )
			{
				// verify if JSON is not too big to outStr
				if ( ( strlen( *outStr ) + 7 + 2 ) > *outLength )
				{
					return ( __LINE__ );
				}
			}
			sprintf ( &(*outStr)[ strlen( *outStr ) ], "no data");
		}
		else switch ( data[ id ].type[ i ] )
		{ // if data exist
			case jT( undefined ):
			{
				if ( flag & JPS_LEN_SET ) // if outLength is set verify string size
				{
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + 9 + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
				}
				sprintf ( &(*outStr)[ strlen( *outStr ) ], "undefined" );
				break;
			}
			case jT( ptrBool ):
			case jT( bool ):
			{
				if ( flag & JPS_LEN_SET ) // if outLength is set verify string size
				{
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + 5 + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
				}
				sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", ( *( uint8_t * )data[ id ].value[ i ] )?"true":"false" );
				break;
			}
			case jT( ptrInt8_t ):
			case jT( int8_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%d", *( int8_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%d", *( int8_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrInt16_t ):
			case jT( int16_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%d", *( int16_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%d", *( int16_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrInt32_t ):
			case jT( int32_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%d", *( int32_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%d", *( int32_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrInt64_t ):
			case jT( int64_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%lld", *( int64_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%lld", *( int64_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrUint8_t ):
			case jT( uint8_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%u", *( uint8_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%u", *( uint8_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrUint16_t ):
			case jT( uint16_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%u", *( uint16_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%u", *( uint16_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrUint32_t ):
			case jT( uint32_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%u", *( uint32_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%u", *( uint32_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrUint64_t ):
			case jT( uint64_t ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%llu", *( uint64_t * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%llu", *( uint64_t * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrFloat ):
			case jT( float ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%f", *( float * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr
					if ( ( strlen ( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%f", *( float * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrDouble ):
			case jT( double ):
			{
				if ( flag & JPS_LEN_SET )
				{
					sprintf ( tmpStr, "%lf", *( double * )data[ id ].value[ i ] );
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( tmpStr ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%s", tmpStr );
				}
				else
				{
					sprintf ( &(*outStr)[ strlen( *outStr ) ], "%lf", *( double * )data[ id ].value[ i ] );
				}
				break;
			}
			case jT( ptrStr ):
			case jT( str ):
			{
				if ( ( flag & JPS_OUT_NULL ) &&
					flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) )
				{
					tmp = realloc ( *outStr, strlen ( *outStr ) + strlen ( ( char * )(data[ id ].value[ i ]) ) + 2 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					*outStr = tmp;
				}
				else if ( flag & JPS_LEN_SET )
				{
					// verify if JSON is not too big to outStr 
					if ( ( strlen( *outStr ) + strlen ( ( char * )(data[ id ].value[ i ]) ) + 2 ) > *outLength )
					{
						return ( __LINE__ );
					}
				}
				sprintf ( &(*outStr)[ strlen( *outStr ) ], "\"%s\"", ( char * )(data[ id ].value[ i ]) );
				break;
			}
			case jT( obj ):
			{

				if ( flag & JPS_LEN_SET ) // if outLength is set verify string size
				{
					// verify if JSON is not too big to outStr
					if ( ( strlen( *outStr ) + 3 ) > *outLength )
					{
						return ( __LINE__ );
					}
				}
				jsonPrintString ( data, *( uint32_t * )(data[ id ].value[ i ]), outStr, outLength );
				break;
			}
			case jT( array ):
			{
				if ( flag & JPS_LEN_SET ) // if outLength is set verify string size
				{
					// verify if JSON is not too big to outStr
					if ( ( strlen( *outStr ) + 3 ) > *outLength )
					{
						return ( __LINE__ );
					}
				}
				jsonPrintString ( data, *( uint32_t * )(data[ id ].value[ i ]), outStr, outLength );
				break;
			}
		}

		if ( i < data[ id ].length - 1 )
		{
			sprintf ( &(*outStr)[ strlen( *outStr ) ], "," );
		}
	}

	if ( data[ id ].key )
	{
		sprintf ( &(*outStr)[ strlen( *outStr ) ], "}" );
	}
	else
	{
		sprintf ( &(*outStr)[ strlen( *outStr ) ], "]" );
	}

	if ( ( flag & JPS_OUT_NULL ) &&
		( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) ) )
	{
		tmp = realloc ( *outStr, strlen ( *outStr ) + 2 );
		if ( !tmp )
		{
			return ( __LINE__ );
		}
		*outStr = tmp;
	}
	if ( flag & JPS_LEN_ZERO )
	{
		*outLength = strlen ( *outStr );
	}

	return ( 0 );
}
#pragma GCC diagnostic pop

uint32_t jsonPrint ( json_el * data, uint32_t id, uint8_t indent )
{
	return ( jsonPrintFile ( data, id, indent, stdout ) );
}

uint32_t jsonFree ( json_el ** data, uint32_t length )
{
	uint32_t i = 0;
	if ( !data )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}
	for ( i = 0; i < length; i++ )
	{
		if ( jsonFreeOne ( *data, i ) )
		{
			return ( __LINE__ );
		}
	}

	free ( *data );
	*data = NULL;

	return ( 0 );
}

void * jsonGet ( const json_el * const data, const uint32_t id, const char * const key, void ** const value, JSON_TYPE * const type )
{
	uint32_t i = 0;

	for ( i = 0; i < data[ id ].length; i++ )
	{
		if ( !strcmp ( data[ id ].key[ i ], key ) )
		{
			if ( type )
			{ // if type pointer defined return type value
				*type = data[ id ].type[ i ];
			}

			if ( !value )
			{ // if value NULL
				return ( data[ id ].value[ i ] );
			}

			*value = data[ id ].value[ i ];
			return ( *value );
		}
	}
	return ( NULL );
}

void * jsonGetRecursive ( const json_el * const data, const uint32_t id, const char * const key, void ** const value, JSON_TYPE * const type )
{
	uint32_t i = 0;
	void * tmp = NULL;


	for ( i = 0; i < data[ id ].length; i++ )
	{
		if ( !strcmp ( data[ id ].key[ i ], key ) )
		{
			if ( type )
			{ // if type pointer defined return type value
				*type = data[ id ].type[ i ];
			}

			if ( !value )
			{ // if value NULL
				return ( data[ id ].value[ i ] );
			}

			*value = data[ id ].value[ i ];
			return ( *value );
		}
		else if ( data[ id ].type[ i ] == jT ( obj ) )
		{
			tmp = jsonGet ( data, *( uint32_t * )data[ id ].value[ i ], key, value, type );

			if ( tmp )
			{
				return ( tmp );
			}
		}
	}
	return ( NULL );
}

uint32_t jsonSet ( json_el * const data, const uint32_t id, const char * const key, void * const value, const JSON_TYPE type )
{
	uint32_t elID = 0;
	void * tmp = NULL;

	if ( ( type == jT ( obj ) ) ||
		( type == jT ( array ) ) )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	if ( data[ id ].key )
	{ // parrent is obj
		for ( elID = 0; elID < data[ id ].length; elID++ )
		{
			if ( !strcmp ( data[ id ].key[ elID ], key ) )
			{
				break;
			}
		}
	}
	else
	{ // parrent is array
		if ( key )
		{
			#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
			// the array count is in 32 bits
			elID = (uint32_t)( key );
			#pragma GCC diagnostic pop
		}
		else
		{
			elID = data[ id ].length;
		}
	}

	tmp = 0;

	if ( elID >= data[ id ].length )
	{
		if ( key )
		{ // only need if parrent is obj
			tmp = realloc ( data[ id ].key, sizeof ( char * ) * ( data[ id ].length + 1 ) );
			if ( !tmp )
			{
				return ( __LINE__ );
			}
			data[ id ].key = tmp;
			tmp = NULL;

			data[ id ].key[ data[ id ].length ] = NULL;

			tmp = malloc ( strlen ( key ) + 1 );
			if ( !tmp )
			{
				return ( __LINE__ );
			}
			data[ id ].key[ data[ id ].length ] = tmp;
			strcpy ( data[ id ].key[ data[ id ].length ], key );
			tmp = NULL;
		}

		// get next value
		tmp = realloc (data[ id ].value, sizeof ( void * ) * ( data[ id ].length + 1 ) );
		if ( !tmp )
		{
			return ( __LINE__ );
		}
		data[ id ].value = tmp;
		tmp = NULL;

		// if value pointer provided
		if ( ( value == NULL ) ||
			( type == jT ( ptrBool ) ) ||
			( type == jT ( ptrUint8_t ) ) ||
			( type == jT ( ptrUint16_t ) ) ||
			( type == jT ( ptrUint32_t ) ) ||
			( type == jT ( ptrUint64_t ) ) ||
			( type == jT ( ptrInt8_t ) ) ||
			( type == jT ( ptrInt16_t ) ) ||
			( type == jT ( ptrInt32_t ) ) ||
			( type == jT ( ptrInt64_t ) ) ||
			( type == jT ( ptrFloat ) ) ||
			( type == jT ( ptrDouble ) ) ||
			( type == jT ( ptrStr ) ) )
		{ // if it's a pointer or NULL value soo no need memory alloc

			data[ id ].value[ data[ id ].length ] = value;
			if ( value )
			{
				printf ( "%lf\n", *(float *)value );
			}
		}
		else
		{ // taht need alloc
			// get the memory size need to be allocated
			switch ( type )
			{
				case jT ( bool ):
				case jT ( int8_t ):
				case jT ( uint8_t ):
				{
					tmp = ( void * )( 1 );
					break;
				}
				case jT ( int16_t ):
				case jT ( uint16_t ):
				{
					tmp = ( void * )( sizeof ( int16_t ) );
					break;
				}
				case jT ( int32_t ):
				case jT ( uint32_t ):
				case jT ( float ):
				{
					tmp = ( void * )( sizeof ( int32_t ) );
					break;
				}
				case jT ( int64_t ):
				case jT ( uint64_t ):
				case jT ( double ):
				{
					tmp = ( void * )( sizeof ( int64_t ) );
					break;
				}
				case jT ( str ):
				{
					tmp = ( void * )( strlen ( value ) + 1 );
					break;
				}
				default:
				{
					break;
				}
			}

			// alloc memory
			#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
			// tmp was used as buffer to don't define another var
			data[ id ].value[ data[ id ].length ] = malloc ( (uint32_t)tmp );
			#pragma GCC diagnostic pop

			if ( !data[ id ].value[ data[ id ].length ] )
			{ // in case of memory allocation failled
				return ( __LINE__ );
			}

			// copy data
			#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
			// tmp was used as buffer to don't define another var
			memcpy ( data[ id ].value[ data[ id ].length ], value, (uint32_t)tmp );
			#pragma GCC diagnostic pop
		}

		// set next element type
		tmp = realloc ( data[ id ].type, sizeof ( uint8_t ) * ( data[ id ].length + 1 ) );
		if ( !tmp )
		{
			return ( __LINE__ );
		}
		data[ id ].type = tmp;
		data[ id ].type[ data[ id ].length ] = type;
		tmp = NULL;

		data[ id ].length++;
	}
	else
	{
		free ( data[ id ].value[ elID ] );
		data[ id ].value[ elID ] = NULL;

		// if value pointer provided
		if ( ( value == NULL ) ||
			( type == jT ( ptrBool ) ) ||
			( type == jT ( ptrUint8_t ) ) ||
			( type == jT ( ptrUint16_t ) ) ||
			( type == jT ( ptrUint32_t ) ) ||
			( type == jT ( ptrUint64_t ) ) ||
			( type == jT ( ptrInt8_t ) ) ||
			( type == jT ( ptrInt16_t ) ) ||
			( type == jT ( ptrInt32_t ) ) ||
			( type == jT ( ptrInt64_t ) ) ||
			( type == jT ( ptrFloat ) ) ||
			( type == jT ( ptrDouble ) ) ||
			( type == jT ( ptrStr ) ) )
		{ // if it's a pointer or NULL value soo no need memory alloc
			data[ id ].value[ data[ id ].length ] = value;
		}
		else
		{ // taht need alloc
			// get the memory size need to be allocated
			switch ( type )
			{
				case jT ( bool ):
				case jT ( int8_t ):
				case jT ( uint8_t ):
				{
					tmp = ( void * )( 1 );
					break;
				}
				case jT ( int16_t ):
				case jT ( uint16_t ):
				{
					tmp = ( void * )( sizeof ( int16_t ) );
					break;
				}
				case jT ( int32_t ):
				case jT ( uint32_t ):
				case jT ( float ):
				{
					tmp = ( void * )( sizeof ( int32_t ) );
					break;
				}
				case jT ( int64_t ):
				case jT ( uint64_t ):
				case jT ( double ):
				{
					tmp = ( void * )( sizeof ( int64_t ) );
					break;
				}
				case jT ( str ):
				{
					tmp = ( void * )( strlen ( value ) + 1 );
					break;
				}
				default:
				{
					break;
				}
			}

			// alloc memory
			#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
			// tmp was used as buffer to don't define another var
			data[ id ].value[ elID ] = malloc ( (uint32_t)tmp );
			#pragma GCC diagnostic pop

			if ( !data[ id ].value[ elID ] )
			{ // in case of memory allocation failled
				return ( __LINE__ );
			}

			// copy data
			#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
			// tmp was used as buffer to don't define another var
			memcpy ( data[ id ].value[ elID ], value, (uint32_t)tmp );
			#pragma GCC diagnostic pop
		}

		data[ id ].type[ elID ] = type;
	}

	return ( 0 );
}

uint32_t jsonSetObj ( json_el ** const data, const uint32_t id, const char * const key, const JSON_TYPE type, uint32_t * const length )
{
	uint32_t elID = 0;
	void * tmp = NULL;

	if ( !( type == jT ( obj ) ) &&
		!( type == jT ( array ) ) )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}


	if ( jsonSet ( *data, id, key, NULL, jT ( uint8_t ) ) )
	{
		return ( __LINE__ );
	}

	for ( elID = 0; elID < (*data)[ id ].length; elID++ )
	{
		if ( !strcmp ( (*data)[ id ].key[ elID ], key ) )
		{
			break;
		}
	}

	if ( elID >= (*data)[ id ].length )
	{
		return ( __LINE__ );
	}
	else
	{
		tmp = realloc ( (*data), sizeof ( json_el ) * ( *length + 1 ) );
		if ( !tmp )
		{
			return ( __LINE__ );
		}
		(*data) = tmp;
		tmp = NULL;

		(*data)[ id ].value[ elID ] = malloc ( sizeof ( *length ) );
		*( uint32_t * )((*data)[ id ].value[ elID ]) = *length;
		(*data)[ id ].type[ elID ] = type;

		(*data)[ *length ].key = NULL;
		(*data)[ *length ].value = NULL;
		(*data)[ *length ].type = NULL;
		(*data)[ *length ].length = 0;

		(*length)++;
	}

	return ( 0 );
}

int __attribute__((weak)) main ( void )
{
	json_el *data = NULL;
	uint32_t dataLength = 0;

	uint8_t bob = 6;
	uint8_t alice = 6;
	uint8_t alice2 = 7;
	float test = 12.3;

	void * value = NULL;
	uint64_t length = 0;
	JSON_TYPE type = jT ( undefined );

	// read file part
	printf ( "Object from file:\n" );
	if ( jsonParseFile ( "file.json", &data, &dataLength ) )
	{
		printf ( "error\n");
	}
	else
	{
		jsonPrint ( data, 0, 0 );

		printf ( "\nElement:\n" );
		jsonGetRecursive ( data, 0, "h1", &value, &type );

		printf ( "h1 value  = %s type : %d\n", (char *)value, type );

		printf ( "\nFirst object with replacements:\n" );
		if ( jsonSet ( data, 0, "h", "ceci est un long test qui va remplacer la valeur de G", jT ( str ) ) )
		{
			printf ( "error\n" );
		}

		if ( jsonSet ( data, 0, "l", "ajout en L" , jT ( str ) ) )
		{
			printf ( "error\n" );
		}
		jsonPrint ( data, 0, 0 );

		//value non need to be freed, is only a pointer on data array
		value = NULL;

		if ( data )
		{
			jsonFree ( &data, dataLength );
			dataLength = 0;
		}
		printf ( "\n" );
	}

	// read json from string
	printf ( "Object form string:\n" );
	if ( jsonParseString ( "[1,2,3,4,5,6,7,{\"alpha\":\"test\"}]", &data, &dataLength ) )
	{
		printf ( "error\n");
	}
	else
	{
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

		if ( data )
		{
			jsonFree ( &data, dataLength );
			dataLength = 0;
		}

		if ( value )
		{
			free ( value );
			value = NULL;
		}
		printf ( "\n" );
	}

	// try to put json in a small string it should be fail
	length = 20;
	if ( jsonPrintString ( data, 0, ( char ** )&value, &length ) )
	{ // normal case on failure
		if ( value )
		{ // failure but some work already done
			printf ( "%s %lu\n", ( char * ) value, strlen ( ( char * ) value ) );
		}
		
		if ( value )
		{
			free ( value );
			value = NULL;
		}
	}

	// create json and feed it
	if ( jsonParseString ( "{}", &data, &dataLength ) )
	{
		printf ( "error\n" );
	}
	else
	{
		jsonSet ( data, 0, "bob", &bob, jT ( uint8_t ) );
		jsonSet ( data, 0, "alice", &alice, jT ( uint8_t ) );
		jsonSet ( data, 0, "alice", "au pays de merveilles", jT ( str ) );

		jsonSetObj ( &data, 0, "mike", jT ( obj ), &dataLength );

		jsonSet ( data, dataLength - 1, "alice2", &alice2, jT ( uint8_t ) );
		jsonSet ( data, dataLength - 1, "alice3", &test, jT ( float ) );
		jsonSetObj ( &data, dataLength - 1, "array", jT ( array ), &dataLength );
		jsonSet ( data, dataLength - 1, NULL, &test, jT ( float ) );

		jsonPrint ( data, 0, 0 );

		// here an exemple of using pointer in json, usefull for
		// periodic messages from json like MQTT publish
		if ( jsonGetRecursive ( data, 0, "array", &value, &type ) )
		{
			jsonSet ( data, dataLength - 1, NULL, &test, jT ( ptrFloat ) );
			if ( type == jT ( array ) ||
				type == jT ( obj ) )
			{
				jsonPrint ( data, *(uint32_t*)value, 0 );
			}
			test = 17.0;
			if ( type == jT ( array ) ||
				type == jT ( obj ) )
			{
				jsonPrint ( data, *(uint32_t*)value, 0 );
			}
		}

		if ( data )
		{
			jsonFree ( &data, dataLength );
			dataLength = 0;
		}
	}

	return ( 0 );
}