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
							tmp = realloc ( (*out)[ numObj ].key, sizeof ( char * ) * (*out)[ numObj ].length + 1 );
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

					if ( nextValueId ==  (*out)[ numObj ].length )
					{ // it's a new object
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
					}
					else
					{
						switch ( (*out)[ numObj ].type[ nextValueId ] )
						{
							case jT ( bool ):
							case jT ( float ):
							case jT ( str ):
							{
								free ( (*out)[ numObj ].value[ nextValueId ] );
								(*out)[ numObj ].value[ nextValueId ] = NULL;
								break;
							}
							case jT ( obj ):
							case jT ( array ):
							{
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
							(*out)[ (*outLength) - 1 ].parent = NULL;

							tmp = malloc ( sizeof ( uint32_t ) );
							if ( !tmp )
							{
								return ( __LINE__ );
							}
							(*out)[ numObj ].value[ nextValueId ] = tmp;
							tmp = NULL;

							*( uint32_t * )((*out)[ numObj ].value[ nextValueId ]) = (*outLength) - 1;
							(*out)[ (*outLength) - 1 ].parent = &(*out)[ numObj ];

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
								(*out)[ numObj ].type[ nextValueId ] = jT( float );

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
			 data[ id ].value[ j ] )
		{
			if ( (  data[ id ].type[ j ] == jT( float ) ) ||
				(  data[ id ].type[ j ] == jT( str ) ) ||
				(  data[ id ].type[ j ] == jT( bool ) ) ||
				(  data[ id ].type[ j ] == jT( array ) ) ||
				(  data[ id ].type[ j ] == jT( obj ) ) )
			{
				free (  data[ id ].value[ j ] );
			}
			else
			{ // undefined
			}
			 data[ id ].value[ j ] = NULL;
		}
	}

	if (  data[ id ].key )
	{
		free (  data[ id ].key );
		 data[ id ].key = NULL;
	}

	if (  data[ id ].value )
	{
		free (  data[ id ].value );
		 data[ id ].value = NULL;
	}

	if (  data[ id ].type )
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
	(*out)->parent = NULL;

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
			case jT( bool ):
			{
				fprintf ( outFile, "%s", ( *( uint8_t * )data[ id ].value[ i ] )?"true":"false" );
				break;
			}
			case jT( float ):
			{
				fprintf ( outFile, "%lf", *( double * )data[ id ].value[ i ] );
				break;
			}
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
	{
		flag |= JPS_OUT_NULL;
	}
	else
	{
		flag |= JPS_OUT_SET;
	}

	if ( !outLength )
	{
		flag |= JPS_LEN_NULL;
	}
	else if ( outLength == 0 )
	{
		flag |= JPS_LEN_ZERO;
	}
	else
	{
		flag |= JPS_OUT_SET;
	}


	if ( ( flag & JPS_OUT_NULL ) &&
		( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) ) )
	{
		*outStr = malloc ( 1024 );
		*outStr[ 0 ] = '\0';
	}
	else if ( ( flag & JPS_OUT_NULL ) &&
		( flag & JPS_OUT_SET ) )
	{
		*outStr = malloc ( *outLength );
		if ( !*outStr )
		{
			return ( __LINE__ );
		}
		(*outStr)[ 0 ] = '\0';
	}
	else if ( ( flag & JPS_OUT_SET ) &&
		( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) ) )
	{
		// nothing to be done
	}
	else if ( ( flag & JPS_OUT_SET ) &&
		( flag & JPS_OUT_SET ) )
	{
		// nothing to be done
	}
	if ( data[ id ].key )
	{
		sprintf ( &(*outStr)[ strlen ( *outStr ) ], "{" );
	}
	else
	{
		sprintf ( &(*outStr)[ strlen ( *outStr ) ], "[" );
	}

	for ( i = 0; i < data[ id ].length; i++ )
	{

		if ( data[ id ].key &&
			data[ id ].key[ i ] )
		{
			// manage cases when outStr's size need to be  increase
			if ( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) )
			{
				tmp = realloc ( *outStr, strlen ( *outStr ) + strlen ( ( char * )(data[ id ].key[ i ]) ) + 128 );
				if ( !tmp )
				{
					return ( __LINE__ );
				}
				*outStr = tmp;
			}
			else if ( flag & JPS_OUT_SET )
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
		{
			// manage cases when outStr's size need to be  increase
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
		{
			if ( flag & JPS_OUT_SET )
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
		{
			case jT( undefined ):
			{
				if ( flag & JPS_OUT_SET )
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
			case jT( bool ):
			{
				if ( flag & JPS_OUT_SET )
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
			case jT( float ):
			{
				if ( flag & JPS_OUT_SET )
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
			case jT( str ):
			{

				if ( flag & ( JPS_LEN_NULL | JPS_LEN_ZERO ) )
				{
					tmp = realloc ( *outStr, strlen ( *outStr ) + strlen ( ( char * )(data[ id ].value[ i ]) ) + 2 );
					if ( !tmp )
					{
						return ( __LINE__ );
					}
					*outStr = tmp;
				}
				else if ( flag & JPS_OUT_SET )
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

				if ( flag & JPS_OUT_SET )
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

				if ( flag & JPS_OUT_SET )
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

	return ( 0 );
}

void * jsonGet ( const json_el * const data, const uint32_t id, const char * const key, void ** const value, JSON_TYPE * const type )
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
	uint32_t i = 0;
	void * tmp = NULL;
		
	if ( ( type == jT ( obj ) ) ||
		( type == jT ( array ) ) )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}
	for ( i = 0; i < data[ id ].length; i++ )
	{
		if ( !strcmp ( data[ id ].key[ i ], key ) )
		{
			break;
		}
		else if ( data[ id ].type[ i ] == jT ( obj ) )
		{
			if ( !jsonSet ( data, *( uint32_t * )data[ id ].value[ i ], key, value, type ) )
			{
				return ( 0 );
			}
		}
	}

	if ( i >= data[ id ].length )
	{
		tmp = realloc ( data[ id ].key, sizeof ( char * ) * data[ id ].length + 1 );
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

		// get next value
		tmp = realloc (data[ id ].value, sizeof ( void * ) * data[ id ].length + 1 );
		if ( !tmp )
		{
			return ( __LINE__ );
		}
		data[ id ].value = tmp;
		data[ id ].value[ data[ id ].length ] = value;
		tmp = NULL;

		// set next element type
		tmp = realloc ( data[ id ].type, sizeof ( uint8_t ) * data[ id ].length + 1 );
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
		switch ( data[ id ].type[ i ] )
		{
			case jT ( bool ):
			case jT ( float ):
			case jT ( str ):
			{
				free ( data[ id ].value[ i ] );
				break;
			}
			case jT ( obj ):
			case jT ( array ):
			{
				jsonFreeOne ( data, *( uint32_t * )data[ id ].value[ i ] );
				data[ id ].value[ i ] = NULL;
				data[ id ].type[ i ] = jT ( undefined );
				break;
			}
		}

		data[ id ].value[ i ] = value;
		data[ id ].type[ i ] = type;
	}
	return ( 0 );
}

