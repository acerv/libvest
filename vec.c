// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#include "vec.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct
{
	size_t unit_size;
	size_t capacity;
	size_t count;
	uint8_t data[];
} vec_obj_t;

static inline __attribute__((pure)) vec_obj_t *vec_object(vec_t self)
{
	assert(self);
	return (vec_obj_t *)((uintptr_t)self - offsetof(vec_obj_t, data));
}

static inline size_t vec_size(size_t unit_size, size_t capacity)
{
	return sizeof(vec_obj_t) + unit_size * capacity;
}

vec_t vec_new_len(const size_t unit_size, const size_t count)
{
	vec_obj_t *obj = malloc(vec_size(unit_size, VEC_INIT_CAPACITY));
	if (!obj)
		return NULL;

	obj->count = count;
	obj->capacity = VEC_INIT_CAPACITY;
	obj->unit_size = unit_size;

	memset(obj->data, 0, VEC_INIT_CAPACITY * unit_size);

	return (vec_t )obj->data;
}

vec_t vec_new(const size_t unit_size)
{
	return vec_new_len(unit_size, 0);
}

void vec_free(vec_t self)
{
	vec_obj_t *obj = vec_object(self);
	free(obj);
}

vec_t vec_resize(vec_t self, const size_t count)
{
	vec_obj_t *obj = vec_object(self);
	size_t old_size = obj->count;

	if (count >= obj->capacity) {
		obj->capacity *= 2;

		obj = realloc(obj, vec_size(obj->unit_size, obj->capacity));
		if (!obj)
			return NULL;

		memset(obj->data + (old_size * obj->unit_size),
			0, (obj->capacity  - old_size) * obj->unit_size);
	}

	obj->count = count;

	return (vec_t )obj->data;
}

vec_t vec_extend(vec_t self, const size_t count)
{
	return vec_resize(self, vec_count(self) + count);
}

size_t vec_unit_size(const vec_t self)
{
	return vec_object(self)->unit_size;
}

size_t vec_count(const vec_t self)
{
	return vec_object(self)->count;
}

size_t vec_capacity(const vec_t self)
{
	return vec_object(self)->capacity;
}

void *vec_ptr_at(vec_t self, size_t pos)
{
	if (!pos)
		return self;

	vec_obj_t *obj = vec_object(self);

	if (pos >= obj->count)
		return obj->data + (obj->count  - 1) * obj->unit_size;

	return obj->data + pos * obj->unit_size;
}

void vec_copy(vec_t self, const size_t pos, const void *items, size_t len)
{
	assert(items);

	if (!len)
		return;

	vec_obj_t *obj = vec_object(self);
	if (pos >= obj->count)
		return;

	size_t tocopy = obj->count - pos;
	if (len > tocopy)
		len = tocopy;

	/* memory can overlap, so we move data inside a buffer before copying */
	vec_t buff = vec_new_len(obj->unit_size, len);

	memcpy(buff, items, len * obj->unit_size);
	memcpy(vec_ptr_at(self, pos), buff, len * obj->unit_size);

	vec_free(buff);
}

void vec_set(vec_t self, const size_t pos, const void *item)
{
	vec_copy(self, pos, item, 1);
}

void vec_get(const vec_t self, const size_t pos, void *item)
{
	assert(item);

	vec_obj_t *obj = vec_object(self);
	void *ptr = vec_ptr_at(self, pos);

	memcpy(item, ptr, obj->unit_size);
}
