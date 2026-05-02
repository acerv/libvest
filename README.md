# VeSt - Dynamic Vectors and Strings library

This C library is designed to handle dynamic vectors and strings, providing
functions to operate on them in an easy way.

## Vectors

Vectors implementation provides a simple interface to dynamic vectors.
We abstract a single vector with the following metadata:

```c
typedef struct
{
    size_t unit_size; /* single vector item size */
    size_t capacity;  /* total capacity of the vector */
    size_t count;     /* number of items */
    uint8_t data[];   /* items memory allocation */
} vec_obj_t;
```

Each metadata field is stored in memory and all functions implemented by `vec.c` use
a pointer to `data[]`, instead of passing the full object. We achieve this by
hiding vector metadata behind the `data[]` pointer.

A usage example is the following:

```c
// create an empty vector of size_t elements
vec_t vec = vec_new(sizeof(size_t));

// resize the vector
vec = vec_resize(vec, 1);

// extend the vector so we have 2048 elements
vec = vec_extend(vec, 2047);

// set items via vec_set() ..
for (size_t i = 0; i < vec_count(vec); i++)
    vec_set(vec, i, &i);

// .. but direct cast works too
for (size_t i = 0; i < vec_count(vec); i++)
    *((size_t *)vec + i) = i;

// read items back ..
size_t data;

for (size_t i = 0; i < vec_count(vec); i++) {
    vec_get(vec, i, &data);
    assert(data == i);
}

// .. but direct cast works too
for (size_t i = 0; i < vec_count(vec); i++) {
    data = *((size_t *)vec + i);
    assert(data == i);
}

// release memory
vec_free(vec);
```

Every vector has a capacity and every time we extend the vector size, we
eventually extend the capacity. This value can be read by calling
`vec_capacity()`.

## Strings

Strings implementation provides a simple interface for string handling and is
an extension of Vectors. Some of the functions include string resize, append,
clear, find, etc. Also in this case we abstract string handling by hiding the
metadata behind the `data[]` pointer.

The string terminator is always present and handled internally by the library and
every string has a capacity which can be read by calling `vec_capacity()`. Every
time we append new data to the string, we resize the memory allocation, ensuring
that there's enough space for the string terminator.

A usage example is the following:

```c
// create a string
str_t str = str_new("hello");

// append another string
str = str_append(str, " world");

// check size
assert(str_length(str) == 11);

// replace 'l' 2 times
str = str_replace(str, "l", "L", 2);

// print directly — no need to access an internal data field
// will print 'heLLo world'
printf("%s", str);

// bring back the 'l'
str = str_replace(str, "L", "l");

// repeat string 10 times
str = str_repeat(str, 10);

// we can find all 10 "hello" occurrences
vec_index_t index = str_find(str, "hello");
for (int i = 0; i < vec_count(index); i++)
    printf("%i\n", index[i]);

// split string into words
vec_str_t words = str_split(str, " ");
for (int i = 0; i < vec_count(words); i++)
    printf("%s\n", words[i]);

// release memory
vec_free(index);
str_list_free(words);
str_free(str);
```

All the other features can be found inside the `str.h` header file.

## UTF-8 Support

All existing string functions (`str_find`, `str_replace`, `str_range`, etc.)
operate at the byte level and work correctly with UTF-8 strings — valid UTF-8
byte sequences are unambiguous, so byte-level search and replacement produces
the same results as codepoint-level operations.

The only place byte-level semantics differ from codepoint-level semantics is
**positioning**: passing a codepoint index where a byte offset is expected will
produce incorrect results with multi-byte characters. For this purpose the
library provides UTF-8 conversion utilities.

### Validation

```c
str_t str = str_new("café");

if (str_utf8_valid(str)) {
    // string is well-formed UTF-8
}
```

### Counting codepoints

`str_length()` returns the byte count. `str_char_count()` returns the number
of Unicode codepoints.

```c
str_t str = str_new("café");

str_length(str);     // returns 5 (bytes: c a f é(2 bytes))
str_char_count(str); // returns 4 (codepoints)
```

### Converting codepoint indices to byte offsets

Use `str_utf8_byte_at()` to translate a codepoint index into the correct
byte offset for existing functions.

```c
str_t str = str_new("café");

// Extract "é" (codepoint index 3) by converting to byte offsets
str_t sub = str_range(str,
    str_utf8_byte_at(str, 3),  // byte offset 3
    str_utf8_byte_at(str, 4)   // byte offset 5
);
// sub == "é"
```

### Navigation

Iterate through a string one codepoint at a time:

```c
str_t str = str_new("café");
size_t offset = 0;

while (offset < str_length(str)) {
    // process codepoint at byte offset
    offset = str_utf8_next(str, offset);
}
```

### Encode / Decode

Low-level primitives for converting between codepoint values and UTF-8 bytes:

```c
// Decode: bytes → codepoint
const char *bytes = "\xc3\xa9";
size_t len;
uint32_t cp = str_utf8_decode(bytes, &len);
// cp == 0xE9 (é), len == 2

// Encode: codepoint → bytes
char buf[4];
size_t written = str_utf8_encode(buf, 0x1F30D);
// buf == "\xf0\x9f\x8c\x8d" (🌍), written == 4
```

## Include this library in your project

If you are using the `meson` build system, you can define a subproject as follows:

```ini
[wrap-git]
url = https://github.com/acerv/vest.git
revision = HEAD
depth = 1

[provide]
vest = vest
```

For everyone else, `libvest.so` can be compiled as follows:

```bash
meson setup builddir && cd builddir
meson compile
```
