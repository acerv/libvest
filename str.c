// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#include "str.h"
#include "vec.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define BUFSIZE 64

static str_t str_resize(str_t self, const size_t size)
{
	// the following resize will eventually expand the vector
	// capacity in order to include the string terminator
	self = vec_resize(self, size + 1);
	if (!self)
		return NULL;

	self[size] = '\0';

	// The terminator at position size remains valid in memory
	// (within capacity), while vec_count correctly tracks length.
	// This design supports both str_length() and C-string compatibility.
	self = vec_resize(self, size);
	if (!self)
		return NULL;

	return self;
}

static str_t str_extend(str_t self, const size_t count)
{
	return str_resize(self, str_length(self) + count);
}

str_t str_empty(void)
{
	return vec_new(sizeof(char));
}

str_t str_new_len(const size_t count)
{
	return vec_new_len(sizeof(char), count);
}

str_t str_new(const char *str)
{
	assert(str);

	size_t len = strlen(str);
	str_t self = str_new_len(len);
	if (!self)
		return NULL;

	vec_copy(self, 0, str, len);

	return self;
}

void str_free(str_t self)
{
	vec_free(self);
}

size_t str_length(const str_t self)
{
	return vec_count(self);
}

str_t str_insert(str_t self, const size_t pos, const char *str)
{
	assert(self);
	assert(str);

	if (pos > str_length(self))
		return NULL;

	size_t str_len = strlen(str);

	self = str_extend(self, str_len);
	if (!self)
		return NULL;

	vec_copy(self, pos + str_len, vec_ptr_at(self, pos),
		str_length(self) - pos);

	vec_copy(self, pos, str, str_len);

	return self;
}

str_t str_append(str_t self, const char *str)
{
	return str_insert(self, str_length(self), str);
}

str_t str_clear(str_t self)
{
	return str_resize(self, 0);
}

vec_str_t str_split(str_t self, const char *sep)
{
	assert(self);
	assert(sep);

	vec_str_t vec_str;
	char *token;

	vec_str = vec_new(sizeof(str_t));
	if (!vec_str)
		return NULL;

	token = strtok(self, sep);
	while (token) {
		vec_str_t tmp = vec_extend(vec_str, 1);
		if (!tmp) {
			vec_free(vec_str);
			return NULL;
		}

		vec_str = tmp;

		vec_str[vec_count(vec_str) - 1] = str_new(token);
		token = strtok(NULL, sep);
	}

	return vec_str;
}

void str_list_free(vec_str_t list)
{
	assert(list);

	for (size_t i = 0; i < vec_count(list); i++)
		str_free(list[i]);

	vec_free(list);
}

bool str_startswith(const str_t self, const char *sub)
{
	assert(self);
	assert(sub);

	size_t str_len;
	size_t sub_len;

	str_len = str_length(self);
	sub_len = strlen(sub);

	if (sub_len > str_len)
		return false;

	if (strncmp(self, sub, sub_len))
		return false;

	return true;
}

bool str_endswith(const str_t self, const char *sub)
{
	assert(self);
	assert(sub);

	size_t str_len;
	size_t sub_len;

	str_len = str_length(self);
	sub_len = strlen(sub);

	if (sub_len > str_len)
		return false;

	if (strncmp(self + str_len - sub_len, sub, sub_len))
		return false;

	return true;
}

static void kmp_compute_lps(const char *pat, const size_t pat_len, size_t *lps)
{
	assert(pat);
	assert(lps);

	size_t i = 1;
	size_t len = 0;

	lps[0] = 0;

	while (i < pat_len) {
		if (pat[i] == pat[len]) {
			len++;
			vec_set(lps, i, &len);
			i++;
		} else {
			if (len != 0) {
				vec_get(lps, len - 1, &len);
			} else {
				vec_set(lps, i, &(size_t){0});
				i++;
			}
		}
	}
}

vec_index_t str_find(const str_t self, const char *pat)
{
	assert(self);
	assert(pat);

	size_t n;
	size_t m;
	size_t i = 0;
	size_t j = 0;
	size_t *lps = NULL;
	vec_index_t pos;

	n = str_length(self);
	m = strlen(pat);
	if (!m)
		return NULL;

	pos = vec_new(sizeof(size_t));
	if (!pos)
		goto exit;

	lps = vec_new_len(sizeof(size_t), m);
	if (!lps)
		goto exit;

	kmp_compute_lps(pat, m, lps);

	while (i < n) {
		if (self[i] == pat[j]) {
			i++;
			j++;
		}

		if (j == m) {
			pos = vec_extend(pos, 1);
			if (!pos)
				goto exit;

			vec_set(pos, vec_count(pos) - 1, &(size_t){i - j});
			vec_get(lps, j - 1, &j);
		} else if (i < n && self[i] != pat[j]) {
			if (j != 0)
				vec_get(lps, j - 1, &j);
			else
				i++;
		}
	}

exit:
	if (lps)
		vec_free(lps);

	return pos;
}

str_t str_replace(str_t self, const char *old_str, const char *new_str,
		  const int count)
{
	assert(self);
	assert(old_str);
	assert(new_str);

	size_t len_old = strlen(old_str);
	if (len_old > str_length(self))
		return NULL;

	vec_index_t pos = str_find(self, old_str);
	if (!pos)
		return NULL;

	size_t pos_count = vec_count(pos);
	if (!pos_count)
		goto exit;

	size_t len_new = strlen(new_str);
	size_t shift = len_new - len_old;
	size_t index, val;

	for (size_t i = 0; i < pos_count; i++) {
		if (i >= (size_t)count)
			break;

		vec_get(pos, i, &index);

		if (shift) {
			self = str_extend(self, shift);
			if (!self)
				goto exit;

			vec_copy(self, index + len_new,
				vec_ptr_at(self, index + len_old),
				str_length(self) - index);

			for (size_t j = i + 1; j < pos_count; j++) {
				vec_get(pos, j, &val);
				val += shift;
				vec_set(pos, j, &val);
			}
		}

		for (size_t j = 0; j < len_new; j++)
			vec_set(self, index + j, new_str + j);
	}

exit:
	if (pos)
		vec_free(pos);

	return self;
}

str_t str_remove(str_t self, const char *str)
{
	return str_replace(self, str, "", -1);
}

str_t str_repeat(str_t self, const size_t count)
{
	assert(self);

	if (!count)
		return self;

	size_t len = str_length(self);

	self = str_extend(self, len * count);
	if (!self)
		return NULL;

	for (size_t i = 0; i < count; i++)
		vec_copy(self, i * len, self, len);

	return self;
}

str_t str_range(const str_t self, size_t start, size_t end)
{
	assert(self);

	str_t str = str_empty();
	if (!str)
		return NULL;

	if (start == end)
		return str;

	size_t len = str_length(self);
	size_t temp;
	
	start = start > len ? len : start;
	end = end > len ? len : end;

	if (start > end) {
		temp = start;
		start = end;
		end = temp;
	}

	size_t range_size = end - start;

	str = str_extend(str, range_size);
	if (!str)
		return NULL;

	vec_copy(str, 0, vec_ptr_at(self, start), range_size);

	return str;
}

str_t str_format(str_t self, const char *fmt, ...)
{
	assert(fmt);

	if (*fmt == '\0')
		return self;

	self = str_clear(self);

	char *s;
	int d_num;
	double f_num;
	long long l_num;
	unsigned long long u_num;
	va_list ap;
	size_t pos = 0;
	char buf[BUFSIZE];

	va_start(ap, fmt);

	do {
		if (*fmt != '%') {
			self = vec_extend(self, 1);
			vec_set(self, pos, fmt);
			pos++;
			continue;
		}

		fmt++;

		if (*fmt == '\0')
			break;

		switch (*fmt) {
		case 's':
			s = va_arg(ap, char *);
			self = str_append(self, s);
			pos += strlen(s);
			break;
		case 'i':
			d_num = va_arg(ap, int);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%d", d_num);
			self = str_append(self, buf);
			break;
		case 'l':
			l_num = va_arg(ap, long long);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%lld", l_num);
			self = str_append(self, buf);
			break;
		case 'u':
			u_num = va_arg(ap, unsigned long long);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%llu", u_num);
			self = str_append(self, buf);
			break;
		case 'f':
			f_num = va_arg(ap, double);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%g", f_num);
			self = str_append(self, buf);
			break;
		default:
			self = str_append(self, "???");
			pos += strlen("???");
			continue;
		}
	} while (*(++fmt) != '\0');

	va_end(ap);

	return self;
}
