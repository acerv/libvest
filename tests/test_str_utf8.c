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


/* Existing byte-level functions with UTF-8 strings */

static void test_str_append_utf8(void)
{
	str_t str = str_new("caf");

	str = str_append(str, "\xc3\xa9");
	assert(str);
	assert(str_length(str) == 5);
	assert(memcmp(str, "caf\xc3\xa9", 5) == 0);

	str_free(str);
}

static void test_str_remove_utf8(void)
{
	str_t str = str_new("caf\xc3\xa9 caf\xc3\xa9");

	str = str_remove(str, "\xc3\xa9");
	assert(str);
	assert(memcmp(str, "caf caf", 7) == 0);

	str_free(str);
}

static void test_str_repeat_utf8(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_repeat(str, 3);
	assert(str);
	assert(str_length(str) == 15);
	assert(memcmp(str, "caf\xc3\xa9" "caf\xc3\xa9" "caf\xc3\xa9", 15) == 0);

	str_free(str);
}

static void test_str_startswith_utf8(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_startswith(str, "caf"));
	assert(str_startswith(str, "caf\xc3\xa9"));
	assert(!str_startswith(str, "\xc3\xa9"));
	assert(!str_startswith(str, "cafe"));

	str_free(str);
}

static void test_str_endswith_utf8(void)
{
	str_t str = str_new("caf\xc3\xa9");

	assert(str_endswith(str, "\xc3\xa9"));
	assert(str_endswith(str, "caf\xc3\xa9"));
	assert(!str_endswith(str, "caf"));
	assert(!str_endswith(str, "afe"));

	str_free(str);
}

static void test_str_split_utf8_separator(void)
{
	str_t str = str_new("caf\xc3\xa9,monde,bonjour");

	vec_str_t parts = str_split(str, ",");
	assert(parts);
	assert(vec_count(parts) == 3);
	assert(memcmp(parts[0], "caf\xc3\xa9", 5) == 0);
	assert(memcmp(parts[1], "monde", 5) == 0);
	assert(memcmp(parts[2], "bonjour", 7) == 0);

	str_list_free(parts);
	str_free(str);
}

static void test_str_split_utf8_tokens(void)
{
	str_t str = str_new("caf\xc3\xa9 monde");

	vec_str_t parts = str_split(str, " ");
	assert(parts);
	assert(vec_count(parts) == 2);
	assert(memcmp(parts[0], "caf\xc3\xa9", 5) == 0);
	assert(memcmp(parts[1], "monde", 5) == 0);

	str_list_free(parts);
	str_free(str);
}

static void test_str_format_utf8(void)
{
	str_t str = str_empty();

	str = str_format(str, "bonjour %s", "caf\xc3\xa9");
	assert(str);
	assert(memcmp(str, "bonjour caf\xc3\xa9", 13) == 0);

	str_free(str);
}

static void test_str_format_utf8_multiple(void)
{
	str_t str = str_empty();

	str = str_format(str, "%s et %s", "caf\xc3\xa9", "th\xc3\xa9");
	assert(str);
	assert(memcmp(str, "caf\xc3\xa9" " et th\xc3\xa9", 13) == 0);

	str_free(str);
}

static void test_str_find_utf8_multiple_byte_indices(void)
{
	str_t str = str_new("\xc3\xa9\xc3\xa9\xc3\xa9");

	vec_index_t pos = str_find(str, "\xc3\xa9");
	assert(pos);
	assert(vec_count(pos) == 3);
	assert(pos[0] == 0);
	assert(pos[1] == 2);
	assert(pos[2] == 4);

	vec_free(pos);
	str_free(str);
}

static void test_str_find_utf8_middle_of_string(void)
{
	str_t str = str_new("ab\xc3\xa9" "def\xc3\xa9" "ghi");

	vec_index_t pos = str_find(str, "\xc3\xa9");
	assert(pos);
	assert(vec_count(pos) == 2);
	assert(pos[0] == 2);
	assert(pos[1] == 7);

	vec_free(pos);
	str_free(str);
}

static void test_str_replace_utf8_grow(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_replace(str, "\xc3\xa9", "\xc3\xa9\xc3\xa9", -1);
	assert(str);
	assert(str_length(str) == 7);
	assert(memcmp(str, "caf\xc3\xa9\xc3\xa9", 7) == 0);

	str_free(str);
}

static void test_str_replace_utf8_shrink(void)
{
	str_t str = str_new("caf\xc3\xa9" "caf\xc3\xa9");

	str = str_replace(str, "\xc3\xa9", "e", -1);
	assert(str);
	assert(str_length(str) == 8);
	assert(memcmp(str, "cafecafe", 8) == 0);

	str_free(str);
}

static void test_str_replace_utf8_same_size(void)
{
	str_t str = str_new("caf\xc3\xa9" "caf\xc3\xa9");

	str = str_replace(str, "\xc3\xa9", "\xc3\xa0", -1);
	assert(str);
	assert(str_length(str) == 10);
	assert(memcmp(str, "caf\xc3\xa0" "caf\xc3\xa0", 10) == 0);

	str_free(str);
}

static void test_str_replace_utf8_count_limited(void)
{
	str_t str = str_new("\xc3\xa9\xc3\xa9\xc3\xa9");

	str = str_replace(str, "\xc3\xa9", "X", 2);
	assert(str);
	assert(memcmp(str, "XX\xc3\xa9", 3) == 0);

	str_free(str);
}

static void test_str_replace_utf8_multi_byte_old_and_new(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_replace(str, "caf\xc3\xa9", "th\xc3\xa9", -1);
	assert(str);
	assert(str_length(str) == 4);
	assert(memcmp(str, "th\xc3\xa9", 4) == 0);

	str_free(str);
}

static void test_str_insert_utf8_at_beginning(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_insert(str, 0, "mon ");
	assert(str);
	assert(memcmp(str, "mon caf\xc3\xa9", 9) == 0);

	str_free(str);
}

static void test_str_insert_utf8_at_end(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_insert(str, 5, " monde");
	assert(str);
	assert(memcmp(str, "caf\xc3\xa9 monde", 11) == 0);

	str_free(str);
}

static void test_str_insert_utf8_at_codepoint_boundary(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_insert(str, str_utf8_byte_at(str, 3), " X");
	assert(str);
	assert(memcmp(str, "caf X" "\xc3\xa9", 7) == 0);

	str_free(str);
}

static void test_str_range_utf8_extract_multi_byte(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str_t sub = str_range(str, str_utf8_byte_at(str, 3),
			      str_utf8_byte_at(str, 4));
	assert(sub);
	assert(str_length(sub) == 2);
	assert(memcmp(sub, "\xc3\xa9", 2) == 0);

	str_free(sub);
	str_free(str);
}

static void test_str_range_utf8_full_string(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str_t sub = str_range(str, 0, str_length(str));
	assert(sub);
	assert(memcmp(sub, "caf\xc3\xa9", 5) == 0);

	str_free(sub);
	str_free(str);
}

static void test_str_range_utf8_empty(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str_t sub = str_range(str, str_utf8_byte_at(str, 3),
			      str_utf8_byte_at(str, 3));
	assert(sub);
	assert(str_length(sub) == 0);

	str_free(sub);
	str_free(str);
}

static void test_str_clear_utf8_then_append(void)
{
	str_t str = str_new("caf\xc3\xa9");

	str = str_clear(str);
	assert(str_length(str) == 0);

	str = str_append(str, "th\xc3\xa9");
	assert(str);
	assert(memcmp(str, "th\xc3\xa9", 5) == 0);

	str_free(str);
}

static void test_str_new_utf8_roundtrip(void)
{
	const char *original = "caf\xc3\xa9 monde";

	str_t str = str_new(original);
	assert(str);
	assert(str_length(str) == 11);
	assert(memcmp(str, original, 11) == 0);

	str_free(str);
}

static void test_str_utf8_combined_find_and_replace(void)
{
	str_t str = str_new("\xc3\xa9l\xc3\xa8ve \xc3\xa9tudiant \xc3\xa9l\xc3\xa8ve");

	vec_index_t pos = str_find(str, "\xc3\xa9l\xc3\xa8ve");
	assert(pos);
	assert(vec_count(pos) == 2);
	assert(pos[0] == 0);
	assert(pos[1] == 18);

	str = str_replace(str, "\xc3\xa9l\xc3\xa8ve", "student", -1);
	assert(str);
	assert(memcmp(str, "student \xc3\xa9tudiant student", 25) == 0);

	vec_free(pos);
	str_free(str);
}

static void test_str_replace_utf8_shrink_multiple(void)
{
	str_t str = str_new("\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9");

	str = str_replace(str, "\xc3\xa9", "e", -1);
	assert(str);
	assert(str_length(str) == 4);
	assert(memcmp(str, "eeee", 4) == 0);

	str_free(str);
}

static void test_str_replace_utf8_grow_multiple(void)
{
	str_t str = str_new("a\xc3\xa9" "b\xc3\xa9" "c");

	str = str_replace(str, "\xc3\xa9", "\xc3\xa9\xc3\xa9", -1);
	assert(str);
	assert(str_length(str) == 11);
	assert(memcmp(str, "a\xc3\xa9" "\xc3\xa9" "b\xc3\xa9" "\xc3\xa9" "c", 8) == 0);

	str_free(str);
}

static void test_str_replace_utf8_4byte_char(void)
{
	str_t str = str_new("hello\xf0\x9f\x8c\x8dworld");

	str = str_replace(str, "\xf0\x9f\x8c\x8d", "earth", -1);
	assert(str);
	assert(str_length(str) == 15);
	assert(memcmp(str, "helloearthworld", 15) == 0);

	str_free(str);
}

static void test_str_find_utf8_4byte_char(void)
{
	str_t str = str_new("a" "\xf0\x9f\x8c\x8d" "b" "\xf0\x9f\x8c\x8d" "c");

	vec_index_t pos = str_find(str, "\xf0\x9f\x8c\x8d");
	assert(pos);
	assert(vec_count(pos) == 2);
	assert(pos[0] == 1);
	assert(pos[1] == 6);

	vec_free(pos);
	str_free(str);
}

static void test_str_range_utf8_4byte_char(void)
{
	str_t str = str_new("a" "\xf0\x9f\x8c\x8d" "b");

	str_t sub = str_range(str, str_utf8_byte_at(str, 1),
			      str_utf8_byte_at(str, 2));
	assert(sub);
	assert(str_length(sub) == 4);
	assert(memcmp(sub, "\xf0\x9f\x8c\x8d", 4) == 0);

	str_free(sub);
	str_free(str);
}

static void test_str_split_utf8_multi_byte_separator(void)
{
	str_t str = str_new("caf\xc3\xa9" "\xc3\xa9" "monde\xc3\xa9" "bonjour");

	vec_str_t parts = str_split(str, "\xc3\xa9");
	assert(parts);
	assert(vec_count(parts) == 3);

	str_list_free(parts);
	str_free(str);
}

static void test_str_insert_utf8_4byte_at_boundary(void)
{
	str_t str = str_new("a" "\xf0\x9f\x8c\x8d" "b");

	str = str_insert(str, str_utf8_byte_at(str, 1), "X");
	assert(str);
	assert(memcmp(str, "aX" "\xf0\x9f\x8c\x8d" "b", 7) == 0);

	str_free(str);
}

static void test_str_startswith_utf8_multi_byte_prefix(void)
{
	str_t str = str_new("\xc3\xa9l\xc3\xa8ve");

	assert(str_startswith(str, "\xc3\xa9"));
	assert(str_startswith(str, "\xc3\xa9l"));
	assert(str_startswith(str, "\xc3\xa9l\xc3\xa8"));
	assert(!str_startswith(str, "\xc3\xa9l\xc3\xa8vX"));

	str_free(str);
}

static void test_str_endswith_utf8_multi_byte_suffix(void)
{
	str_t str = str_new("\xc3\xa9l\xc3\xa8ve");

	assert(str_endswith(str, "\xc3\xa8ve"));
	assert(str_endswith(str, "ve"));
	assert(str_endswith(str, "\xc3\xa9l\xc3\xa8ve"));
	assert(!str_endswith(str, "X\xc3\xa8ve"));

	str_free(str);
}

/* Bug fix tests — decode edge cases */

static void test_str_utf8_decode_null_byte(void)
{
	size_t len;
	uint32_t cp;

	cp = str_utf8_decode("\x00", &len);
	assert(cp == 0x0000);
	assert(len == 1);
}

static void test_str_utf8_decode_surrogate_byte_len(void)
{
	size_t len;
	uint32_t cp;
	char buf[3];

	buf[0] = (char)0xED;
	buf[1] = (char)0xA0;
	buf[2] = (char)0x80;

	cp = str_utf8_decode(buf, &len);
	assert(cp == 0xFFFD);
	assert(len == 1);
}

static void test_str_utf8_decode_out_of_range_byte_len(void)
{
	size_t len;
	uint32_t cp;
	char buf[4];

	buf[0] = (char)0xF4;
	buf[1] = (char)0x90;
	buf[2] = (char)0x80;
	buf[3] = (char)0x80;

	cp = str_utf8_decode(buf, &len);
	assert(cp == 0xFFFD);
	assert(len == 1);
}

/* Bug fix tests — invalid sequence handling in higher-level functions */

static void test_str_char_count_skip_invalid(void)
{
	str_t str = str_new("hello");
	str[2] = (char)0xFF;

	assert(str_char_count(str) == 5);

	str_free(str);
}

static void test_str_char_count_embedded_null(void)
{
	str_t str = str_new_len(3);

	str[0] = 'a';
	str[1] = '\0';
	str[2] = 'b';

	assert(str_char_count(str) == 3);

	str_free(str);
}

static void test_str_utf8_byte_at_skip_invalid(void)
{
	str_t str = str_new("hello");
	str[2] = (char)0xFF;

	assert(str_utf8_byte_at(str, 0) == 0);
	assert(str_utf8_byte_at(str, 1) == 1);
	assert(str_utf8_byte_at(str, 2) == 3);
	assert(str_utf8_byte_at(str, 3) == 4);

	str_free(str);
}

static void test_str_utf8_next_invalid_byte(void)
{
	str_t str = str_new("hello");
	str[2] = (char)0xFF;

	assert(str_utf8_next(str, 1) == 2);
	assert(str_utf8_next(str, 2) == 3);

	str_free(str);
}

static void test_str_utf8_prev_skip_invalid(void)
{
	str_t str = str_new("hello");
	str[2] = (char)0xFF;

	assert(str_utf8_prev(str, 5) == 4);
	assert(str_utf8_prev(str, 4) == 3);
	assert(str_utf8_prev(str, 3) == 2);

	str_free(str);
}

static void test_str_utf8_valid_embedded_null(void)
{
	str_t str = str_new_len(3);

	str[0] = 'a';
	str[1] = '\0';
	str[2] = 'b';

	assert(str_utf8_valid(str));

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
	RUN_TEST(test_str_append_utf8);
	RUN_TEST(test_str_remove_utf8);
	RUN_TEST(test_str_repeat_utf8);
	RUN_TEST(test_str_startswith_utf8);
	RUN_TEST(test_str_endswith_utf8);
	RUN_TEST(test_str_split_utf8_separator);
	RUN_TEST(test_str_split_utf8_tokens);
	RUN_TEST(test_str_format_utf8);
	RUN_TEST(test_str_format_utf8_multiple);
	RUN_TEST(test_str_find_utf8_multiple_byte_indices);
	RUN_TEST(test_str_find_utf8_middle_of_string);
	RUN_TEST(test_str_replace_utf8_grow);
	RUN_TEST(test_str_replace_utf8_shrink);
	RUN_TEST(test_str_replace_utf8_same_size);
	RUN_TEST(test_str_replace_utf8_count_limited);
	RUN_TEST(test_str_replace_utf8_multi_byte_old_and_new);
	RUN_TEST(test_str_insert_utf8_at_beginning);
	RUN_TEST(test_str_insert_utf8_at_end);
	RUN_TEST(test_str_insert_utf8_at_codepoint_boundary);
	RUN_TEST(test_str_range_utf8_extract_multi_byte);
	RUN_TEST(test_str_range_utf8_full_string);
	RUN_TEST(test_str_range_utf8_empty);
	RUN_TEST(test_str_clear_utf8_then_append);
	RUN_TEST(test_str_new_utf8_roundtrip);
	RUN_TEST(test_str_utf8_combined_find_and_replace);
	RUN_TEST(test_str_replace_utf8_shrink_multiple);
	RUN_TEST(test_str_replace_utf8_grow_multiple);
	RUN_TEST(test_str_replace_utf8_4byte_char);
	RUN_TEST(test_str_find_utf8_4byte_char);
	RUN_TEST(test_str_range_utf8_4byte_char);
	RUN_TEST(test_str_split_utf8_multi_byte_separator);
	RUN_TEST(test_str_insert_utf8_4byte_at_boundary);
	RUN_TEST(test_str_startswith_utf8_multi_byte_prefix);
	RUN_TEST(test_str_endswith_utf8_multi_byte_suffix);
	RUN_TEST(test_str_utf8_decode_null_byte);
	RUN_TEST(test_str_utf8_decode_surrogate_byte_len);
	RUN_TEST(test_str_utf8_decode_out_of_range_byte_len);
	RUN_TEST(test_str_char_count_skip_invalid);
	RUN_TEST(test_str_char_count_embedded_null);
	RUN_TEST(test_str_utf8_byte_at_skip_invalid);
	RUN_TEST(test_str_utf8_next_invalid_byte);
	RUN_TEST(test_str_utf8_prev_skip_invalid);
	RUN_TEST(test_str_utf8_valid_embedded_null);

	return 0;
}
