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
- [x] add function to set an object
- [x] add function to print JSON in file/string
- [x] add documentation in header
- [x] remove properly object when an element is removed form array
- [x] **verify memory leak with valgrind**

## Exemple:
You can found a fully functionnal exemple int `jsonParser.c` at the of file in a `week main`

## Behind :
The json was stored in a `json_el` array, each new element is stored in a `json_el` and added at the en of array.

For exemple a json like : "[{key:value},2]" was stored as it :
```
array[ 0 ] =  { 
	.key = NULL, 
	.value = (void**)[2], // array of pointer sized by array[0].length
		// [0] = pointer on int what contain the index of object in the array, in this case (1)
		// [1] = pointeur on double that contain `2`
	.type = (JSON_TYPE*)[2] // array of type of value element, sized by array[0].length
		// [0] = jT(array)
		// [1] = jT(double)
	.length = 2,
}
array[ 1 ] = {
	.key = (char**)[1], // array of string that contain key, sized by array[1].length
		// [0] = key
	.value = (void**)[1], // array of pointer sized by array[1].length
		// [0] = value
	.type = (JSON_TYPE*)[1] // sized by array[1].length
		// [0] = jT(string)
	.length = 1
}
```
