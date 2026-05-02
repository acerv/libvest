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

static void test_str_empty(void)
{
	str_t str = str_empty();

	assert(str);
	assert(memcmp(str, "", 1) == 0);
	assert(str_length(str) == 0);

	str_free(str);
}

static void test_str_create(void)
{
	str_t str = str_new("ciao");

	assert(str);
	assert(memcmp(str, "ciao", str_length(str)) == 0);
	assert(str_length(str) == 4);

	str_free(str);
}

static void test_str_new_len(void)
{
	str_t str = str_new_len(1024);

	assert(str);
	assert(memcmp(str, "", 1) == 0);
	assert(str_length(str) == 1024);

	str_free(str);
}

static void test_str_append(void)
{
	str_t str = str_new("ciao");

	str = str_append(str, " mondo");
	assert(str);
	assert(memcmp(str, "ciao mondo", str_length(str)) == 0);
	assert(str_length(str) == 10);

	str_free(str);
}

static void test_str_clear(void)
{
	str_t str = str_new("ciao mondo");

	str = str_clear(str);

	assert(str);
	assert(str_length(str) == 0);

	str_free(str);
}

static void test_str_insert(void)
{
	str_t str = str_new("ciao");

	str = str_insert(str, 0, "mondo ");

	assert(str);
	assert(memcmp(str, "mondo ciao", str_length(str)) == 0);
	assert(str_length(str) == 10);

	str_free(str);
}

static void test_str_split(void)
{
	str_t str = str_new("hi this is a message");
	vec_str_t tok;

	assert(str);

	tok = str_split(str, " "); 
	assert(tok);

	assert(vec_count(tok) == 5);
	assert(memcmp(tok[0], "hi", str_length(tok[0])) == 0);
	assert(memcmp(tok[1], "this", str_length(tok[1])) == 0);
	assert(memcmp(tok[2], "is", str_length(tok[2])) == 0);
	assert(memcmp(tok[3], "a", str_length(tok[3])) == 0);
	assert(memcmp(tok[4], "message", str_length(tok[4])) == 0);

	str_list_free(tok);
	str_free(str);
}

static void test_str_startswith(void)
{
	str_t str = str_new("ciao mondo");
	assert(str_startswith(str, "ciao"));
	str_free(str);
}

static void test_str_endswith(void)
{
	str_t str = str_new("ciao mondo");
	assert(str_endswith(str, "mondo"));
	str_free(str);
}

static void test_str_find(void)
{
	str_t str = str_new("ABABACCABA");
	const char *pat = "ABA";

	vec_index_t pos;

	pos = str_find(str, pat);

	assert(vec_count(pos) == 3);
	assert(pos[0] == 0);
	assert(pos[1] == 2);
	assert(pos[2] == 7);

	str_free(str);
	vec_free(pos);
}

static void test_str_replace(void)
{
	str_t str = str_new("ABABACCABA");
	size_t len = str_length(str);

	str = str_replace(str, "A", "F", -1);
	assert(str);
	assert(str_length(str) == len);
	assert(memcmp(str, "FBFBFCCFBF", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_smaller(void)
{
	str_t str = str_new("ciao mondo ciao");

	str = str_replace(str, "ciao", "hi", -1);
	assert(str);
	assert(memcmp(str, "hi mondo hi", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_bigger(void)
{
	str_t str = str_new("hi mondo hi");

	str = str_replace(str, "hi", "ciao", -1);
	assert(str);
	assert(memcmp(str, "ciao mondo ciao", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_count(void)
{
	str_t str = str_new("ABABACCABA");
	size_t len = str_length(str);

	str = str_replace(str, "A", "F", 1);
	assert(str);
	assert(str_length(str) == len);
	assert(memcmp(str, "FBABACCABA", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_empty(void)
{
	str_t str = str_new("hello world hello world");

	str = str_replace(str, " world", "", -1);
	assert(str);
	assert(memcmp(str, "hello hello", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_count_limit_partial(void)
{
	str_t str = str_new("aaa bbb aaa bbb aaa");

	str = str_replace(str, "aaa", "ccc", 2);
	assert(str);
	assert(memcmp(str, "ccc bbb ccc bbb aaa", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_not_found(void)
{
	str_t str = str_new("hello world");

	str_t result = str_replace(str, "xyz", "abc", -1);
	assert(result == str);
	assert(memcmp(result, "hello world", str_length(result)) == 0);

	str_free(str);
}

static void test_str_replace_shrink_multiple(void)
{
	str_t str = str_new("abcXXXdefXXXghiXXXjkl");

	str = str_replace(str, "XXX", "-", -1);
	assert(str);
	assert(memcmp(str, "abc-def-ghi-jkl", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_grow_multiple(void)
{
	str_t str = str_new("a_b_a_b_a");

	str = str_replace(str, "_", "---", -1);
	assert(str);
	assert(memcmp(str, "a---b---a---b---a", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_at_boundaries(void)
{
	str_t str = str_new("XhelloX");

	str = str_replace(str, "X", "", -1);
	assert(str);
	assert(memcmp(str, "hello", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_consecutive(void)
{
	str_t str = str_new("XXhelloXX");

	str = str_replace(str, "XX", "-", -1);
	assert(str);
	assert(memcmp(str, "-hello-", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_overlapping(void)
{
	str_t str = str_new("XXXXX");

	str = str_replace(str, "XX", "-", -1);
	assert(str);
	assert(memcmp(str, "--X", str_length(str)) == 0);

	str_free(str);
}

static void test_str_remove(void)
{
	str_t str = str_new("hello world hello");

	str = str_remove(str, "hello");
	assert(str);
	assert(memcmp(str, " world ", str_length(str)) == 0);

	str_free(str);
}

static void test_str_repeat(void)
{
	str_t str = str_new("hi ");

	str = str_repeat(str, 6); 
	assert(str);
	assert(memcmp(str, "hi hi hi hi hi hi ", str_length(str)) == 0);

	str_free(str);
}

static void test_str_repeat_overflow(void)
{
	str_t str = str_new("a");

	str_t result = str_repeat(str, SIZE_MAX);
	assert(result == NULL);

	str_free(str);
}

static void test_str_range_forward(void)
{
	str_t str = str_new("ciao mondo ciao");

	str_t str_r = str_range(str, 5, 10);
	assert(str_r);
	assert(memcmp(str_r, "mondo", str_length(str_r)) == 0);

	str_free(str_r);
	str_free(str);
}

static void test_str_range_reversed(void)
{
	str_t str = str_new("ciao mondo ciao");

	str_t str_r = str_range(str, 10, 5);
	assert(str_r);
	assert(memcmp(str_r, "mondo", str_length(str_r)) == 0);

	str_free(str_r);
	str_free(str);
}

static void test_str_range_start_overflow(void)
{
	str_t str = str_new("ciao mondo ciao");

	str_t str_r = str_range(str, 1000, 5);
	assert(str_r);
	assert(memcmp(str_r, "mondo ciao", str_length(str_r)) == 0);

	str_free(str_r);
	str_free(str);
}

static void test_str_range_end_overflow(void)
{
	str_t str = str_new("ciao mondo ciao");

	str_t str_r = str_range(str, 5, 1000);
	assert(str_r);
	assert(memcmp(str_r, "mondo ciao", str_length(str_r)) == 0);

	str_free(str_r);
	str_free(str);
}

static void test_str_format_string(void)
{
	const char *end = "ciao -> mondo";
	const size_t end_size = strlen(end);

	str_t str = str_empty();

	str = str_format(str, "%s -> %s", "ciao", "mondo");

	assert(str_length(str) == end_size);
	assert(memcmp(str, end, end_size) == 0);

	str_free(str);
}

static void test_str_format_int(void)
{
	const char *end = "64 :: 1024";
	const size_t end_size = strlen(end);

	str_t str = str_empty();

	str = str_format(str, "%i :: %i", 64, 1024);

	assert(str_length(str) == end_size);
	assert(memcmp(str, end, end_size) == 0);

	str_free(str);
}

static void test_str_format_double(void)
{
	const char *end = "64.1 :: 314.023";
	const size_t end_size = strlen(end);

	str_t str = str_empty();

	str = str_format(str, "%f :: %f", 64.1, 314.023);

	assert(str_length(str) == end_size);
	assert(memcmp(str, end, end_size) == 0);

	str_free(str);
}

static void test_str_format_long(void)
{
	const char *end = "64 :: 1024";
	const size_t end_size = strlen(end);

	str_t str = str_empty();

	str = str_format(str, "%l :: %l", (long)64, (long)1024);

	assert(str_length(str) == end_size);
	assert(memcmp(str, end, end_size) == 0);

	str_free(str);
}

static void test_str_format_unsigned_long(void)
{
	const char *end = "64 :: 1024";
	const size_t end_size = strlen(end);

	str_t str = str_empty();

	str = str_format(str, "%u :: %u", (unsigned long)64, (unsigned long)1024);

	assert(str_length(str) == end_size);
	assert(memcmp(str, end, end_size) == 0);

	str_free(str);
}

static void test_str_format_unknown(void)
{
	const char *end = "???";
	const size_t end_size = strlen(end);

	str_t str = str_empty();

	str = str_format(str, "%K", "ciao");

	assert(str_length(str) == end_size);
	assert(memcmp(str, end, end_size) == 0);

	str_free(str);
}

static void test_str_format_trailing_percent(void)
{
	str_t str = str_empty();

	str = str_format(str, "hello%");
	assert(str);
	assert(memcmp(str, "hello%", str_length(str)) == 0);

	str_free(str);
}

static void test_str_format_double_percent(void)
{
	str_t str = str_empty();

	str = str_format(str, "100%% done");
	assert(str);
	assert(memcmp(str, "100% done", str_length(str)) == 0);

	str_free(str);
}

static void test_str_startswith_exact(void)
{
	str_t str = str_new("hello");

	assert(str_startswith(str, "hello"));

	str_free(str);
}

static void test_str_startswith_empty(void)
{
	str_t str = str_new("hello");

	assert(str_startswith(str, ""));

	str_free(str);
}

static void test_str_startswith_too_long(void)
{
	str_t str = str_new("hello");

	assert(!str_startswith(str, "helloo"));

	str_free(str);
}

static void test_str_startswith_no_match(void)
{
	str_t str = str_new("hello");

	assert(!str_startswith(str, "world"));

	str_free(str);
}

static void test_str_endswith_exact(void)
{
	str_t str = str_new("hello");

	assert(str_endswith(str, "hello"));

	str_free(str);
}

static void test_str_endswith_empty(void)
{
	str_t str = str_new("hello");

	assert(str_endswith(str, ""));

	str_free(str);
}

static void test_str_endswith_too_long(void)
{
	str_t str = str_new("hello");

	assert(!str_endswith(str, "hhello"));

	str_free(str);
}

static void test_str_endswith_no_match(void)
{
	str_t str = str_new("hello");

	assert(!str_endswith(str, "world"));

	str_free(str);
}

static void test_str_find_no_match(void)
{
	str_t str = str_new("hello");

	vec_index_t pos = str_find(str, "xyz");
	assert(pos);
	assert(vec_count(pos) == 0);

	vec_free(pos);
	str_free(str);
}

static void test_str_find_empty_pattern(void)
{
	str_t str = str_new("hello");

	vec_index_t pos = str_find(str, "");
	assert(pos == NULL);

	str_free(str);
}

static void test_str_insert_at_end(void)
{
	str_t str = str_new("hello");

	str = str_insert(str, 5, " world");
	assert(str);
	assert(memcmp(str, "hello world", str_length(str)) == 0);

	str_free(str);
}

static void test_str_range_empty(void)
{
	str_t str = str_new("hello");

	str_t empty = str_range(str, 0, 0);
	assert(empty);
	assert(str_length(empty) == 0);
	str_free(empty);

	str_free(str);
}

static void test_str_insert_beyond_length(void)
{
	str_t str = str_new("hello");

	str_t result = str_insert(str, 10, "world");
	assert(result == NULL);

	str_free(str);
}

static void test_str_insert_empty(void)
{
	str_t str = str_new("hello");

	str = str_insert(str, 3, "");
	assert(str);
	assert(memcmp(str, "hello", str_length(str)) == 0);

	str_free(str);
}

static void test_str_insert_middle(void)
{
	str_t str = str_new("abcde");

	str = str_insert(str, 2, "XY");
	assert(str);
	assert(memcmp(str, "abXYcde", str_length(str)) == 0);

	str_free(str);
}

static void test_str_append_empty(void)
{
	str_t str = str_new("hello");

	str = str_append(str, "");
	assert(str);
	assert(memcmp(str, "hello", str_length(str)) == 0);

	str_free(str);
}

static void test_str_clear_then_append(void)
{
	str_t str = str_new("hello");

	str = str_clear(str);
	assert(str_length(str) == 0);

	str = str_append(str, "world");
	assert(str);
	assert(memcmp(str, "world", str_length(str)) == 0);

	str_free(str);
}

static void test_str_split_single_token(void)
{
	str_t str = str_new("hello");

	vec_str_t tok = str_split(str, ",");
	assert(tok);
	assert(vec_count(tok) == 1);
	assert(memcmp(tok[0], "hello", str_length(tok[0])) == 0);

	str_list_free(tok);
	str_free(str);
}

static void test_str_split_all_separators(void)
{
	str_t str = str_new(",,,");

	vec_str_t tok = str_split(str, ",");
	assert(tok);
	assert(vec_count(tok) == 0);

	str_list_free(tok);
	str_free(str);
}

static void test_str_split_empty_string(void)
{
	str_t str = str_new("");

	vec_str_t tok = str_split(str, ",");
	assert(tok);
	assert(vec_count(tok) == 0);

	str_list_free(tok);
	str_free(str);
}

static void test_str_remove_no_match(void)
{
	str_t str = str_new("hello");

	str = str_remove(str, "xyz");
	assert(str);
	assert(memcmp(str, "hello", str_length(str)) == 0);

	str_free(str);
}

static void test_str_remove_entire_string(void)
{
	str_t str = str_new("hello");

	str = str_remove(str, "hello");
	assert(str);
	assert(str_length(str) == 0);

	str_free(str);
}

static void test_str_repeat_once(void)
{
	str_t str = str_new("hi");

	str = str_repeat(str, 1);
	assert(str);
	assert(memcmp(str, "hi", str_length(str)) == 0);

	str_free(str);
}

static void test_str_repeat_empty(void)
{
	str_t str = str_new("");

	str = str_repeat(str, 5);
	assert(str);
	assert(str_length(str) == 0);

	str_free(str);
}

static void test_str_range_full_string(void)
{
	str_t str = str_new("hello");

	str_t str_r = str_range(str, 0, 5);
	assert(str_r);
	assert(memcmp(str_r, "hello", str_length(str_r)) == 0);

	str_free(str_r);
	str_free(str);
}

static void test_str_range_single_char(void)
{
	str_t str = str_new("hello");

	str_t str_r = str_range(str, 2, 3);
	assert(str_r);
	assert(memcmp(str_r, "l", str_length(str_r)) == 0);

	str_free(str_r);
	str_free(str);
}

static void test_str_format_empty(void)
{
	str_t str = str_new("keep me");

	str = str_format(str, "");
	assert(str);
	assert(memcmp(str, "keep me", str_length(str)) == 0);

	str_free(str);
}

static void test_str_format_literals_only(void)
{
	str_t str = str_empty();

	str = str_format(str, "hello world");
	assert(str);
	assert(memcmp(str, "hello world", str_length(str)) == 0);

	str_free(str);
}

static void test_str_format_mixed(void)
{
	str_t str = str_empty();

	str = str_format(str, "%s has %i items", "box", 42);
	assert(str);
	assert(memcmp(str, "box has 42 items", str_length(str)) == 0);

	str_free(str);
}

static void test_str_find_single_char(void)
{
	str_t str = str_new("abacada");

	vec_index_t pos = str_find(str, "a");
	assert(pos);
	assert(vec_count(pos) == 4);
	assert(pos[0] == 0);
	assert(pos[1] == 2);
	assert(pos[2] == 4);
	assert(pos[3] == 6);

	vec_free(pos);
	str_free(str);
}

static void test_str_find_full_string(void)
{
	str_t str = str_new("hello");

	vec_index_t pos = str_find(str, "hello");
	assert(pos);
	assert(vec_count(pos) == 1);
	assert(pos[0] == 0);

	vec_free(pos);
	str_free(str);
}

static void test_str_find_at_end(void)
{
	str_t str = str_new("hello world");

	vec_index_t pos = str_find(str, "world");
	assert(pos);
	assert(vec_count(pos) == 1);
	assert(pos[0] == 6);

	vec_free(pos);
	str_free(str);
}

static void test_str_replace_same_size(void)
{
	str_t str = str_new("hello world");

	str = str_replace(str, "world", "earth", -1);
	assert(str);
	assert(memcmp(str, "hello earth", str_length(str)) == 0);

	str_free(str);
}

static void test_str_replace_count_zero(void)
{
	str_t str = str_new("aaa");

	str = str_replace(str, "a", "b", 0);
	assert(str);
	assert(memcmp(str, "aaa", str_length(str)) == 0);

	str_free(str);
}

static void test_str_length_empty(void)
{
	str_t str = str_empty();

	assert(str_length(str) == 0);

	str_free(str);
}

static void test_str_format_negative_int(void)
{
	str_t str = str_empty();

	str = str_format(str, "val=%i", -42);
	assert(str);
	assert(memcmp(str, "val=-42", str_length(str)) == 0);

	str_free(str);
}

static void test_str_format_negative_float(void)
{
	str_t str = str_empty();

	str = str_format(str, "%f", -3.14);
	assert(str);
	assert(memcmp(str, "-3.14", str_length(str)) == 0);

	str_free(str);
}

static void test_str_format_multiple_percents(void)
{
	str_t str = str_empty();

	str = str_format(str, "100%% of %i%% is %i%%", 100, 100);
	assert(str);
	assert(memcmp(str, "100% of 100% is 100%", str_length(str)) == 0);

	str_free(str);
}

static void test_str_format_reused_string_null_terminated(void)
{
	str_t str = str_new("hello world");

	str = str_format(str, "ab");
	assert(str);
	assert(str_length(str) == 2);
	assert(strcmp(str, "ab") == 0);

	str_free(str);
}

static void test_str_format_reused_string_with_specifiers(void)
{
	str_t str = str_new("xxxxxxxxxxxxxxxx");

	str = str_format(str, "%s=%i", "val", 42);
	assert(str);
	assert(str_length(str) == 6);
	assert(strcmp(str, "val=42") == 0);

	str_free(str);
}

static void test_str_insert_empty_into_empty(void)
{
	str_t str = str_empty();

	str = str_insert(str, 0, "");
	assert(str);
	assert(str_length(str) == 0);

	str_free(str);
}

static void test_str_split_no_leak_verify(void)
{
	str_t str = str_new("one,two,three");

	vec_str_t tok = str_split(str, ",");
	assert(tok);
	assert(vec_count(tok) == 3);
	assert(memcmp(tok[0], "one", str_length(tok[0])) == 0);
	assert(memcmp(tok[1], "two", str_length(tok[1])) == 0);
	assert(memcmp(tok[2], "three", str_length(tok[2])) == 0);

	str_list_free(tok);
	str_free(str);
}

int main(void)
{
	RUN_TEST(test_str_range_forward);
	RUN_TEST(test_str_range_reversed);
	RUN_TEST(test_str_range_start_overflow);
	RUN_TEST(test_str_range_end_overflow);
	RUN_TEST(test_str_range_empty);
	RUN_TEST(test_str_repeat);
	RUN_TEST(test_str_repeat_overflow);
	RUN_TEST(test_str_remove);
	RUN_TEST(test_str_replace);
	RUN_TEST(test_str_replace_smaller);
	RUN_TEST(test_str_replace_bigger);
	RUN_TEST(test_str_replace_count);
	RUN_TEST(test_str_replace_empty);
	RUN_TEST(test_str_replace_count_limit_partial);
	RUN_TEST(test_str_replace_not_found);
	RUN_TEST(test_str_replace_shrink_multiple);
	RUN_TEST(test_str_replace_grow_multiple);
	RUN_TEST(test_str_replace_at_boundaries);
	RUN_TEST(test_str_replace_consecutive);
	RUN_TEST(test_str_replace_overlapping);
	RUN_TEST(test_str_find);
	RUN_TEST(test_str_find_no_match);
	RUN_TEST(test_str_find_empty_pattern);
	RUN_TEST(test_str_startswith);
	RUN_TEST(test_str_endswith);
	RUN_TEST(test_str_split);
	RUN_TEST(test_str_insert);
	RUN_TEST(test_str_clear);
	RUN_TEST(test_str_append);
	RUN_TEST(test_str_new_len);
	RUN_TEST(test_str_create);
	RUN_TEST(test_str_format_string);
	RUN_TEST(test_str_format_int);
	RUN_TEST(test_str_format_double);
	RUN_TEST(test_str_format_long);
	RUN_TEST(test_str_format_unsigned_long);
	RUN_TEST(test_str_format_unknown);
	RUN_TEST(test_str_format_trailing_percent);
	RUN_TEST(test_str_format_double_percent);
	RUN_TEST(test_str_startswith_exact);
	RUN_TEST(test_str_startswith_empty);
	RUN_TEST(test_str_startswith_too_long);
	RUN_TEST(test_str_startswith_no_match);
	RUN_TEST(test_str_endswith_exact);
	RUN_TEST(test_str_endswith_empty);
	RUN_TEST(test_str_endswith_too_long);
	RUN_TEST(test_str_endswith_no_match);
	RUN_TEST(test_str_insert_at_end);
	RUN_TEST(test_str_empty);
	RUN_TEST(test_str_insert_beyond_length);
	RUN_TEST(test_str_insert_empty);
	RUN_TEST(test_str_insert_middle);
	RUN_TEST(test_str_append_empty);
	RUN_TEST(test_str_clear_then_append);
	RUN_TEST(test_str_split_single_token);
	RUN_TEST(test_str_split_all_separators);
	RUN_TEST(test_str_split_empty_string);
	RUN_TEST(test_str_remove_no_match);
	RUN_TEST(test_str_remove_entire_string);
	RUN_TEST(test_str_repeat_once);
	RUN_TEST(test_str_repeat_empty);
	RUN_TEST(test_str_range_full_string);
	RUN_TEST(test_str_range_single_char);
	RUN_TEST(test_str_format_empty);
	RUN_TEST(test_str_format_literals_only);
	RUN_TEST(test_str_format_mixed);
	RUN_TEST(test_str_find_single_char);
	RUN_TEST(test_str_find_full_string);
	RUN_TEST(test_str_find_at_end);
	RUN_TEST(test_str_replace_same_size);
	RUN_TEST(test_str_replace_count_zero);
	RUN_TEST(test_str_length_empty);
	RUN_TEST(test_str_format_negative_int);
	RUN_TEST(test_str_format_negative_float);
	RUN_TEST(test_str_format_multiple_percents);
	RUN_TEST(test_str_format_reused_string_null_terminated);
	RUN_TEST(test_str_format_reused_string_with_specifiers);
	RUN_TEST(test_str_insert_empty_into_empty);
	RUN_TEST(test_str_split_no_leak_verify);

	return 0;
}
