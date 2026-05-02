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
#include <stdint.h>

#define BUFSIZE 64

static str_t str_resize(str_t self, const size_t size)
{
	self = vec_resize(self, size + 1);
	if (!self)
		return NULL;

	self[size] = '\0';

	/* Shrinking never triggers realloc, so this cannot fail */
	self = vec_resize(self, size);

	return self;
}

static str_t str_extend(str_t self, const size_t count)
{
	size_t len = str_length(self);
	if (len > SIZE_MAX - count)
		return NULL;

	return str_resize(self, len + count);
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

	size_t old_len = str_length(self);

	if (pos > old_len)
		return NULL;

	size_t str_len = strlen(str);

	self = str_extend(self, str_len);
	if (!self)
		return NULL;

	vec_copy(self, pos + str_len, vec_ptr_at(self, pos), old_len - pos);
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

vec_str_t str_split(const str_t self, const char *sep)
{
	assert(self);
	assert(sep);

	str_t copy;
	vec_str_t vec_str;
	char *token;

	copy = str_new(self);
	if (!copy)
		return NULL;

	vec_str = vec_new(sizeof(str_t));
	if (!vec_str)
		goto error;

	token = strtok(copy, sep);
	while (token) {
		vec_str_t v_tmp = vec_extend(vec_str, 1);
		if (!v_tmp)
			goto error;

		vec_str = v_tmp;

		str_t s_tmp = str_new(token);
		if (!s_tmp)
			goto error;

		vec_str[vec_count(vec_str) - 1] = s_tmp;
		token = strtok(NULL, sep);
	}

	str_free(copy);

	return vec_str;

error:
	if (copy)
		str_free(copy);
	if (vec_str)
		str_list_free(vec_str);

	return NULL;
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
	vec_index_t pos = NULL;

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
	long long shift = (long long)len_new - (long long)len_old;
	size_t index, val, self_len, last_end = 0;

	for (size_t i = 0; i < pos_count; i++) {
		if (i >= (size_t)count)
			break;

		vec_get(pos, i, &index);

		if (index < last_end)
			continue;

		self_len = str_length(self);

	if (shift >= 0) {
		self = str_extend(self, (size_t)shift);
		if (!self)
			goto exit;

		vec_copy(self, index + len_new,
			vec_ptr_at(self, index + len_old),
			self_len - index - len_old);

			for (size_t j = i + 1; j < pos_count; j++) {
				vec_get(pos, j, &val);
				val += (size_t)shift;
				vec_set(pos, j, &val);
			}
		} else {
		    size_t remaining = self_len - index - len_old;

		    vec_copy(self, index + len_new,
			vec_ptr_at(self, index + len_old),
			remaining);

		    self = str_resize(self, self_len - (size_t)(-1 * shift));
		    if (!self)
			goto exit;

		    for (size_t j = i + 1; j < pos_count; j++) {
			vec_get(pos, j, &val);
			val -= (size_t)(-shift);
			vec_set(pos, j, &val);
		    }
		}

		for (size_t j = 0; j < len_new; j++)
			vec_set(self, index + j, new_str + j);

		last_end = index + len_new;
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

	if (len > SIZE_MAX / count)
		return NULL;

	size_t total = len * count;

	self = str_extend(self, total - len);
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
			self = str_extend(self, 1);
			if (!self)
				goto exit;

			vec_set(self, pos, &*fmt);
			pos++;
			continue;
		}

		fmt++;

		if (*fmt == '\0') {
			self = vec_extend(self, 1);
			if (!self)
				goto exit;

			char pct = '%';
			vec_set(self, pos, &pct);
			pos++;
			goto exit;
		}

		switch (*fmt) {
		case '%':
			self = str_extend(self, 1);
			if (!self)
				goto exit;

			vec_set(self, pos, fmt);
			pos++;
			break;
		case 's':
			s = va_arg(ap, char *);
			self = str_append(self, s);
			if (!self)
				goto exit;

			pos += strlen(s);
			break;
		case 'i':
			d_num = va_arg(ap, int);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%d", d_num);
			self = str_append(self, buf);
			if (!self)
				goto exit;

			break;
		case 'l':
			l_num = va_arg(ap, long long);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%lld", l_num);
			self = str_append(self, buf);
			if (!self)
				goto exit;

			break;
		case 'u':
			u_num = va_arg(ap, unsigned long long);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%llu", u_num);
			self = str_append(self, buf);
			if (!self)
				goto exit;

			break;
		case 'f':
			f_num = va_arg(ap, double);
			memset(buf, 0, BUFSIZE);
			pos += (size_t)snprintf(buf, BUFSIZE, "%g", f_num);
			self = str_append(self, buf);
			if (!self)
				goto exit;

			break;
		default:
			self = str_append(self, "???");
			if (!self)
				goto exit;

			pos += strlen("???");
			continue;
		}
	} while (*(++fmt) != '\0');

exit:
	va_end(ap);

	return self;
}



/* UTF-8 implementation (RFC 3629) */

static inline int utf8_byte_len(uint8_t byte)
{
	if (byte <= 0x7F)
		return 1;
	if ((byte & 0xE0) == 0xC0)
		return 2;
	if ((byte & 0xF0) == 0xE0)
		return 3;
	if ((byte & 0xF8) == 0xF0)
		return 4;
	return 0;
}

uint32_t str_utf8_decode(const char *bytes, size_t *out_len)
{
	uint8_t b;
	int len;
	uint32_t cp;

	if (!bytes || !out_len) {
		if (out_len)
			*out_len = 0;
		return 0xFFFD;
	}

	b = (uint8_t)*bytes;

	len = utf8_byte_len(b);
	if (len == 0) {
		*out_len = 1;
		return 0xFFFD;
	}

	if (len >= 2 && ((uint8_t)bytes[1] < 0x80 || (uint8_t)bytes[1] > 0xBF)) {
		*out_len = 1;
		return 0xFFFD;
	}
	if (len >= 3 && ((uint8_t)bytes[2] < 0x80 || (uint8_t)bytes[2] > 0xBF)) {
		*out_len = 2;
		return 0xFFFD;
	}
	if (len == 4 && ((uint8_t)bytes[3] < 0x80 || (uint8_t)bytes[3] > 0xBF)) {
		*out_len = 3;
		return 0xFFFD;
	}

	*out_len = (size_t)len;

	switch (len) {
	case 1:
		cp = (uint32_t)b;
		break;
	case 2:
		cp = ((uint32_t)(b & 0x1F) << 6) |
		     ((uint32_t)(bytes[1] & 0x3F));
		if (cp < 0x80) {
			*out_len = 1;
			return 0xFFFD;
		}
		break;
	case 3:
		cp = ((uint32_t)(b & 0x0F) << 12) |
		     ((uint32_t)(bytes[1] & 0x3F) << 6) |
		     ((uint32_t)(bytes[2] & 0x3F));
		if (cp < 0x800) {
			*out_len = 1;
			return 0xFFFD;
		}
		if (cp >= 0xD800 && cp <= 0xDFFF) {
			*out_len = 1;
			return 0xFFFD;
		}
		break;
	case 4:
		cp = ((uint32_t)(b & 0x07) << 18) |
		     ((uint32_t)(bytes[1] & 0x3F) << 12) |
		     ((uint32_t)(bytes[2] & 0x3F) << 6) |
		     ((uint32_t)(bytes[3] & 0x3F));
		if (cp < 0x10000) {
			*out_len = 1;
			return 0xFFFD;
		}
		if (cp > 0x10FFFF) {
			*out_len = 1;
			return 0xFFFD;
		}
		break;
	default:
		cp = 0xFFFD;
		break;
	}

	return cp;
}

size_t str_utf8_encode(char *out, uint32_t codepoint)
{
	if (codepoint <= 0x7F) {
		out[0] = (char)codepoint;
		return 1;
	}
	if (codepoint <= 0x7FF) {
		out[0] = (char)(0xC0 | (codepoint >> 6));
		out[1] = (char)(0x80 | (codepoint & 0x3F));
		return 2;
	}
	if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
		return 0;
	if (codepoint <= 0xFFFF) {
		out[0] = (char)(0xE0 | (codepoint >> 12));
		out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		out[2] = (char)(0x80 | (codepoint & 0x3F));
		return 3;
	}
	if (codepoint <= 0x10FFFF) {
		out[0] = (char)(0xF0 | (codepoint >> 18));
		out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
		out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		out[3] = (char)(0x80 | (codepoint & 0x3F));
		return 4;
	}
	return 0;
}

bool str_utf8_valid(str_t self)
{
	size_t i, len, byte_len;
	uint32_t cp;

	if (!self)
		return false;

	len = str_length(self);
	i = 0;
	while (i < len) {
		cp = str_utf8_decode(self + i, &byte_len);
		if (cp == 0xFFFD)
			return false;
		i += byte_len;
	}

	return true;
}

size_t str_char_count(str_t self)
{
	size_t i, len, byte_len, count;
	uint32_t cp;

	if (!self)
		return 0;

	len = str_length(self);
	count = 0;
	i = 0;
	while (i < len) {
		cp = str_utf8_decode(self + i, &byte_len);
		if (byte_len == 0)
			break;
		if (cp == 0xFFFD) {
			i++;
			continue;
		}
		count++;
		i += byte_len;
	}

	return count;
}

size_t str_utf8_byte_at(str_t self, size_t cp_index)
{
	size_t i, len, byte_len, count;
	uint32_t cp;

	if (!self)
		return 0;

	len = str_length(self);
	count = 0;
	i = 0;
	while (i < len) {
		cp = str_utf8_decode(self + i, &byte_len);
		if (byte_len == 0)
			break;
		if (cp == 0xFFFD) {
			i++;
			continue;
		}
		if (count == cp_index)
			return i;
		count++;
		i += byte_len;
	}

	return i;
}

size_t str_utf8_next(str_t self, size_t byte_offset)
{
	size_t len, byte_len;
	uint32_t cp;

	if (!self)
		return byte_offset;

	len = str_length(self);
	if (byte_offset >= len)
		return byte_offset;

	cp = str_utf8_decode(self + byte_offset, &byte_len);
	if (byte_len == 0)
		return byte_offset + 1;
	if (cp == 0xFFFD)
		return byte_offset + 1;

	return byte_offset + byte_len;
}

size_t str_utf8_prev(str_t self, size_t byte_offset)
{
	size_t i, prev;
	uint32_t cp;
	size_t byte_len;

	if (!self || byte_offset == 0)
		return 0;

	prev = 0;
	i = 0;
	while (i < byte_offset) {
		cp = str_utf8_decode(self + i, &byte_len);
		if (byte_len == 0)
			break;
		if (cp == 0xFFFD) {
			i++;
			continue;
		}
		prev = i;
		i += byte_len;
	}

	return prev;
}
