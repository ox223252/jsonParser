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
	char **key;
	void **value;
	uint8_t *type;
	uint32_t length;
	struct __json_el * parent;
}
json_el;

uint32_t jsonParseString ( char * const str, json_el ** out, uint32_t * length );

uint32_t jsonParseFile ( const char * file, json_el ** out, uint32_t * length );

uint32_t jsonPrint ( json_el * data, uint32_t id, uint8_t indent );

uint32_t jsonFree ( json_el ** data, uint32_t length );

void * jsonGet ( const json_el * const data, const uint32_t id, const char * const key, void ** const value, JSON_TYPE * const type );

#endif