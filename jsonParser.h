#ifndef __JSONPARSER_H__
#define __JSONPARSER_H__

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

////////////////////////////////////////////////////////////////////////////////
/// \file jsonParser.c
/// \brief library used to manage json
/// \author ox223252
/// \date 2018-05
/// \copyright GPLv2
/// \version 0.1
/// \warning work in progress
/// \bug array not supported
/// \bug when prinJSON called, only 2 lvl of json can be displayed
////////////////////////////////////////////////////////////////////////////////

#define jT(type) JSON_TYPE_##type

typedef enum
{
	JSON_TYPE_undefined,  ///< not defined yet
	JSON_TYPE_bool,       ///< boolean
	JSON_TYPE_float,      ///< double
	JSON_TYPE_str,        ///< char *
	JSON_TYPE_obj,        ///< new json object
	JSON_TYPE_array       ///< new json array
}JSON_TYPE;

typedef struct __json_el
{
	char **key;           ///<
	void **value;         ///<
	uint8_t *type;        ///<
	uint32_t length;      ///<
	struct __json_el * parent;///<
}
json_el;

////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t jsonParseString ( char * const str, json_el ** out, 
///     uint32_t * length );
/// \param[ in ] str: string need to parsed,
/// \param[ in ] out: output data pointer, (*out) should be null,
/// \param[ in ] length: length of the output data field,
/// \brief this function will parse the arg string to create a json data object.
/// \details that will parse string to verify format and validity
/// \return if 0 then OK else that failed, you can see errno for more details
////////////////////////////////////////////////////////////////////////////////
uint32_t jsonParseString ( char * const str, json_el ** out, uint32_t * length );

////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t jsonParseFile ( const char * file, json_el ** out, 
///     uint32_t * length );
/// \param[ in ] file: file need to be parsed,
/// \param[ in ] out: output data pointer, (*out) should be null,
/// \param[ in ] length: length of the output data field,
/// \brief this function will read file and parse data by jsonParseString() fnc.
/// \details this function will read all file and pass data to jsonParseString()
///     to parse data read.
/// \return if 0 then OK else that failed, you can see errno for more details
////////////////////////////////////////////////////////////////////////////////
uint32_t jsonParseFile ( const char * file, json_el ** out, uint32_t * length );

////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t jsonPrint ( json_el * data, uint32_t id, uint8_t indent );
/// \param[ in ] data: obj what contain JSON (created by jsonParseFile/String),
/// \param[ in ] id: object id in the data array, set to 0 in most cases,
/// \param[ in ] indent: indentation level for the recursive call, set to 0,
/// \brief this function will parse JSON obj and print it
/// \return if 0 the OK else that failed, see errno for more details
////////////////////////////////////////////////////////////////////////////////
uint32_t jsonPrint ( json_el * data, uint32_t id, uint8_t indent );

////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t jsonFree ( json_el ** data, uint32_t length );
/// \param[ in ] data: JSON objct data's array
/// \param[ in ] length: length of data's array
/// \brief this function will free each sub element of the data array, and the 
///     array itself
/// \return if 0 then OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
uint32_t jsonFree ( json_el ** data, uint32_t length );

////////////////////////////////////////////////////////////////////////////////
/// \fn void * jsonGet ( const json_el * const data, const uint32_t id,
///     const char * const key, void ** const value, JSON_TYPE * const type );
/// \param[ in ] data: JSON objct data's array,
/// \param[ in ] id: object id in the data array, set to 0 in most cases,
/// \param[ in ] key: key of the wanted element,
/// \param[ in ] value: a pointer on the found object value,
/// \param[ in ] type: the type of object value
/// \brief that will parse all obj to found the first element with this key
/// \return NULL if key not found else pointer on value
////////////////////////////////////////////////////////////////////////////////
void * jsonGet ( const json_el * const data, const uint32_t id, 
	const char * const key, void ** const value, JSON_TYPE * const type );

////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t jsonSet ( json_el * const data, const uint32_t id, 
///     const char * const key, void * const value, const JSON_TYPE type );
/// \param[ in ] data: JSON objct data's array,
/// \param[ in ] id: object id in the data array, set to 0 in most cases,
/// \param[ in ] key: key of the wanted element,
/// \param[ in ] valeu: a pointer on the new value,
/// \param[ in ] type: type of the new obj
/// \brief that will remove the existing obj and replace it by the new
/// \return  if 0 then OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
uint32_t jsonSet ( json_el * const data, const uint32_t id, 
	const char * const key, void * const value, const JSON_TYPE type );

#endif