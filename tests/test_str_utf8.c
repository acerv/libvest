// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#include "str.h"
#include "vec.h"
#include "utils.h"
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* UTF-8 validation */

static void test_str_utf8_valid_ascii(void)
{
	str_t str = str_new("hello world");

	assert(str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_valid_multibyte(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_valid_emoji(void)
{
	str_t str = str_new("\xf0\x9f\x8c\x8d\xf0\x9f\x9a\x80");

	assert(str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_valid_empty(void)
{
	str_t str = str_new("");

	assert(str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_invalid_overlong(void)
{
	str_t str = str_new_len(2);

	str[0] = (char)0xC0;
	str[1] = (char)0x80;

	assert(!str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_invalid_surrogate(void)
{
	str_t str = str_new_len(3);

	str[0] = (char)0xED;
	str[1] = (char)0xA0;
	str[1] = (char)0xA0;
	str[2] = (char)0x80;

	assert(!str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_invalid_truncated(void)
{
	str_t str = str_new_len(2);

	str[0] = (char)0xE2;
	str[1] = (char)0x82;

	assert(!str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_invalid_continuation_start(void)
{
	str_t str = str_new_len(1);

	str[0] = (char)0x80;

	assert(!str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_invalid_too_large(void)
{
	str_t str = str_new_len(4);

	str[0] = (char)0xF4;
	str[1] = (char)0x90;
	str[2] = (char)0x80;
	str[3] = (char)0x80;

	assert(!str_utf8_valid(str));

	str_free(str);
}

static void test_str_utf8_invalid_null_ptr(void)
{
	assert(!str_utf8_valid(NULL));
}

/* Codepoint counting */

static void test_str_char_count_ascii(void)
{
	str_t str = str_new("hello");

	assert(str_char_count(str) == 5);

	str_free(str);
}

static void test_str_char_count_multibyte(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_char_count(str) == 4);

	str_free(str);
}

static void test_str_char_count_mixed(void)
{
	str_t str = str_new("a\xf0\x9f\x8c\x8d" "b");

	assert(str_char_count(str) == 3);

	str_free(str);
}

static void test_str_char_count_empty(void)
{
	str_t str = str_new("");

	assert(str_char_count(str) == 0);

	str_free(str);
}

static void test_str_char_count_emoji_sequence(void)
{
	str_t str = str_new("\xf0\x9f\x98\x80\xf0\x9f\x98\x81\xf0\x9f\x98\x82");

	assert(str_char_count(str) == 3);

	str_free(str);
}

/* Byte offset conversion */

static void test_str_utf8_byte_at_ascii(void)
{
	str_t str = str_new("hello");

	assert(str_utf8_byte_at(str, 0) == 0);
	assert(str_utf8_byte_at(str, 2) == 2);
	assert(str_utf8_byte_at(str, 5) == 5);

	str_free(str);
}

static void test_str_utf8_byte_at_multibyte(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_utf8_byte_at(str, 0) == 0);
	assert(str_utf8_byte_at(str, 1) == 1);
	assert(str_utf8_byte_at(str, 2) == 2);
	assert(str_utf8_byte_at(str, 3) == 3);
	assert(str_utf8_byte_at(str, 4) == 5);

	str_free(str);
}

static void test_str_utf8_byte_at_4byte(void)
{
	str_t str = str_new("a\xf0\x9f\x8c\x8d" "b");

	assert(str_utf8_byte_at(str, 0) == 0);
	assert(str_utf8_byte_at(str, 1) == 1);
	assert(str_utf8_byte_at(str, 2) == 5);
	assert(str_utf8_byte_at(str, 3) == 6);

	str_free(str);
}

static void test_str_utf8_byte_at_overflow(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_utf8_byte_at(str, 100) == str_length(str));

	str_free(str);
}

/* Navigation */

static void test_str_utf8_next_ascii(void)
{
	str_t str = str_new("abc");

	assert(str_utf8_next(str, 0) == 1);
	assert(str_utf8_next(str, 1) == 2);
	assert(str_utf8_next(str, 2) == 3);

	str_free(str);
}

static void test_str_utf8_next_multibyte(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_utf8_next(str, 0) == 1);
	assert(str_utf8_next(str, 3) == 5);

	str_free(str);
}

static void test_str_utf8_next_at_end(void)
{
	str_t str = str_new("abc");

	assert(str_utf8_next(str, 3) == 3);

	str_free(str);
}

static void test_str_utf8_next_beyond_end(void)
{
	str_t str = str_new("abc");

	assert(str_utf8_next(str, 10) == 10);

	str_free(str);
}

static void test_str_utf8_prev_ascii(void)
{
	str_t str = str_new("abc");

	assert(str_utf8_prev(str, 3) == 2);
	assert(str_utf8_prev(str, 2) == 1);
	assert(str_utf8_prev(str, 1) == 0);

	str_free(str);
}

static void test_str_utf8_prev_multibyte(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_utf8_prev(str, 5) == 3);
	assert(str_utf8_prev(str, 3) == 2);

	str_free(str);
}

static void test_str_utf8_prev_at_start(void)
{
	str_t str = str_new("abc");

	assert(str_utf8_prev(str, 0) == 0);

	str_free(str);
}

/* Decode */

static void test_str_utf8_decode_ascii(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode("A", &len);
	assert(cp == 'A');
	assert(len == 1);
}

static void test_str_utf8_decode_2byte(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode("\xc3\xa9", &len);
	assert(cp == 0xE9);
	assert(len == 2);
}

static void test_str_utf8_decode_3byte(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode("\xe2\x82\xac", &len);
	assert(cp == 0x20AC);
	assert(len == 3);
}

static void test_str_utf8_decode_4byte(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode("\xf0\x9f\x8c\x8d", &len);
	assert(cp == 0x1F30D);
	assert(len == 4);
}

static void test_str_utf8_decode_invalid_byte(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode("\xFF", &len);
	assert(cp == 0xFFFD);
	assert(len == 1);
}

static void test_str_utf8_decode_truncated(void)
{
	size_t len;
	uint32_t cp;
	char buf[2];

	buf[0] = (char)0xE2;
	buf[1] = '\0';

	cp = str_utf8_decode(buf, &len);
	assert(cp == 0xFFFD);
	assert(len == 1);
}

static void test_str_utf8_decode_null_ptr(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode(NULL, &len);
	assert(cp == 0xFFFD);
	assert(len == 0);
}

static void test_str_utf8_decode_null_out_len(void)
{
	uint32_t cp;

	cp = str_utf8_decode("A", NULL);
	assert(cp == 0xFFFD);
}

/* Encode */

static void test_str_utf8_encode_ascii(void)
{
	char buf[4];
	size_t len;

	len = str_utf8_encode(buf, 'A');
	assert(len == 1);
	assert(buf[0] == 'A');
}

static void test_str_utf8_encode_2byte(void)
{
	char buf[4];
	size_t len;

	len = str_utf8_encode(buf, 0xE9);
	assert(len == 2);
	assert(buf[0] == (char)0xC3);
	assert(buf[1] == (char)0xA9);
}

static void test_str_utf8_encode_3byte(void)
{
	char buf[4];
	size_t len;

	len = str_utf8_encode(buf, 0x20AC);
	assert(len == 3);
	assert(buf[0] == (char)0xE2);
	assert(buf[1] == (char)0x82);
	assert(buf[2] == (char)0xAC);
}

static void test_str_utf8_encode_4byte(void)
{
	char buf[4];
	size_t len;

	len = str_utf8_encode(buf, 0x1F30D);
	assert(len == 4);
	assert(buf[0] == (char)0xF0);
	assert(buf[1] == (char)0x9F);
	assert(buf[2] == (char)0x8C);
	assert(buf[3] == (char)0x8D);
}

static void test_str_utf8_encode_surrogate(void)
{
	char buf[4];
	size_t len;

	len = str_utf8_encode(buf, 0xD800);
	assert(len == 0);
}

static void test_str_utf8_encode_too_large(void)
{
	char buf[4];
	size_t len;

	len = str_utf8_encode(buf, 0x110000);
	assert(len == 0);
}

/* Integration: existing API + UTF-8 conversion */

static void test_str_utf8_integration_range(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str_t sub = str_range(str, str_utf8_byte_at(str, 3),
			      str_utf8_byte_at(str, 4));
	assert(sub);
	assert(str_length(sub) == 2);
	assert(sub[0] == (char)0xC3);
	assert(sub[1] == (char)0xA9);

	str_free(sub);
	str_free(str);
}

static void test_str_utf8_integration_insert(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_insert(str, str_utf8_byte_at(str, 3), " X");
	assert(str);
	assert(str_length(str) == 7);
	assert(memcmp(str, "caf X\xc3\xa9", 7) == 0);

	str_free(str);
}

static void test_str_utf8_integration_replace(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_replace(str, "\xc3\xa9", "e", -1);
	assert(str);
	assert(str_length(str) == 4);
	assert(memcmp(str, "cafe", 4) == 0);

	str_free(str);
}

static void test_str_utf8_integration_find(void)
{
	str_t str = str_new("caf\xc3\xa9 caf\xc3\xa9");

	vec_index_t pos = str_find(str, "\xc3\xa9");
	assert(pos);
	assert(vec_count(pos) == 2);

	vec_free(pos);
	str_free(str);
}

static void test_str_utf8_integration_iterate(void)
{
	str_t str = str_new("caf\xc3\xa9");
	size_t cp_count = 0;
	size_t byte_offset = 0;
	size_t len = str_length(str);

	while (byte_offset < len) {
		cp_count++;
		byte_offset = str_utf8_next(str, byte_offset);
	}

	assert(cp_count == 4);

	str_free(str);
}

int main(void)
{
	RUN_TEST(test_str_utf8_valid_ascii);
	RUN_TEST(test_str_utf8_valid_multibyte);
	RUN_TEST(test_str_utf8_valid_emoji);
	RUN_TEST(test_str_utf8_valid_empty);
	RUN_TEST(test_str_utf8_invalid_overlong);
	RUN_TEST(test_str_utf8_invalid_surrogate);
	RUN_TEST(test_str_utf8_invalid_truncated);
	RUN_TEST(test_str_utf8_invalid_continuation_start);
	RUN_TEST(test_str_utf8_invalid_too_large);
	RUN_TEST(test_str_utf8_invalid_null_ptr);
	RUN_TEST(test_str_char_count_ascii);
	RUN_TEST(test_str_char_count_multibyte);
	RUN_TEST(test_str_char_count_mixed);
	RUN_TEST(test_str_char_count_empty);
	RUN_TEST(test_str_char_count_emoji_sequence);
	RUN_TEST(test_str_utf8_byte_at_ascii);
	RUN_TEST(test_str_utf8_byte_at_multibyte);
	RUN_TEST(test_str_utf8_byte_at_4byte);
	RUN_TEST(test_str_utf8_byte_at_overflow);
	RUN_TEST(test_str_utf8_next_ascii);
	RUN_TEST(test_str_utf8_next_multibyte);
	RUN_TEST(test_str_utf8_next_at_end);
	RUN_TEST(test_str_utf8_next_beyond_end);
	RUN_TEST(test_str_utf8_prev_ascii);
	RUN_TEST(test_str_utf8_prev_multibyte);
	RUN_TEST(test_str_utf8_prev_at_start);
	RUN_TEST(test_str_utf8_decode_ascii);
	RUN_TEST(test_str_utf8_decode_2byte);
	RUN_TEST(test_str_utf8_decode_3byte);
	RUN_TEST(test_str_utf8_decode_4byte);
	RUN_TEST(test_str_utf8_decode_invalid_byte);
	RUN_TEST(test_str_utf8_decode_truncated);
	RUN_TEST(test_str_utf8_decode_null_ptr);
	RUN_TEST(test_str_utf8_decode_null_out_len);
	RUN_TEST(test_str_utf8_encode_ascii);
	RUN_TEST(test_str_utf8_encode_2byte);
	RUN_TEST(test_str_utf8_encode_3byte);
	RUN_TEST(test_str_utf8_encode_4byte);
	RUN_TEST(test_str_utf8_encode_surrogate);
	RUN_TEST(test_str_utf8_encode_too_large);
	RUN_TEST(test_str_utf8_integration_range);
	RUN_TEST(test_str_utf8_integration_insert);
	RUN_TEST(test_str_utf8_integration_replace);
	RUN_TEST(test_str_utf8_integration_find);
	RUN_TEST(test_str_utf8_integration_iterate);

	return 0;
}
