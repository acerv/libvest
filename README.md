# VeSt - Dynamic Vectors and Strings library

This C library is designed to handle dynamic vectors and strings, providing
functions to operate on them in easy way.

## Vectors

Vectors implementation provide a simple interface to dynamic vectors.
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

Each metadata is stored in memory and all functions implemented by `vec.c` use
a pointer to `data[]`, instead of passing the full object. We achieve this by
hiding vector metadata behind the `data[]` pointer.

An usage example is the following:

```c
// create an empty vector of size_t elements
vec_t vec = vec_empty(sizeof(size_t));

// resize the vector
vec = vec_resize(vec, 1);

// extend the vector so we have 2048 elements
vec = vec_extend(vec, 2047);

// set items via vec_set() ..
for (size_t i = 0; i < vec_count(vec); i++)
    vec_set(vec, i, &i);

// .. but also cast works
for (size_t i = 0; i < vec_count(vec); i++)
    *((size_t *)vec + i) = i;

// read items back ..
size_t data;

for (size_t i = 0; i < vec_count(vec); i++) {
    vec_get(vec, i, &data);
    assert(data == i);
}

// .. but also cast works
for (size_t i = 0; i < vec_count(vec); i++) {
    data = *((size_t *)vec + i);
    assert(data == i);
}

// release memory
vec_free(vec);
```

Every vector has a capacity and all the times we extend the vector size, we
eventually extend the capcity. This value can be read by calling
`vec_capacity()`.

## Strings

Strings implementation provides a simple interface for strings handling and it's
and extension of Vectors. Some of the functions include strings resize, append,
clear, find, etc. Also in this case we abstract strings handling by hiding the
metadata behind `data[]` pointer.

String terminator is always present and handled internally by the library and
every string has a capacity which can be read by calling `vec_capacity()`. All
the times we append new data to the string, we resize the memory data, ensuring
that there's enough space for the string terminator.

An usage example is the following:

```c
// create a string
str_t str = str_new("hello");

// append an another string
str = str_append(str, " world");

// check size
assert(str_length(str) == 11);

// replace the 'l' 2 times
str = str_replace(str, "l", "L", 2);

// print without the need of passing `str->data`
// will print 'heLLo world'
printf("%s", str);

// bring back the 'L'
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
str_list_free(str);
str_free(str);
```

All the other features can be found inside `str.h` header file.

## Include this library in your project

If you are using `meson` build system, you are lucky. The only thing that it's
needed is to define a subproject as following:

```ini
[wrap-git]
url = https://github.com/acerv/vest.git
revision = HEAD
depth = 1

[provide]
vest = vest
```

For all the others, `libvest.so` can be compiled as following:

```bash
meson setup builddir && cd builddir
meson compile
```
