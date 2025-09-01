// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#include "str.h"
#include "vec.h"
#include "utils.h"
#include <string.h>
#include <assert.h>

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

static void test_str_range(void)
{
	str_t str = str_new("ciao mondo ciao");

	str_t str_r = str_range(str, 5, 10); 
	assert(str_r);
	assert(memcmp(str_r, "mondo", str_length(str_r)) == 0);
	str_free(str_r);

	str_r = str_range(str, 10, 5); 
	assert(str_r);
	assert(memcmp(str_r, "mondo", str_length(str_r)) == 0);
	str_free(str_r);

	str_r = str_range(str, 1000, 5); 
	assert(str_r);
	assert(memcmp(str_r, "mondo ciao", str_length(str_r)) == 0);
	str_free(str_r);

	str_r = str_range(str, 5, 1000); 
	assert(str_r);
	assert(memcmp(str_r, "mondo ciao", str_length(str_r)) == 0);
	str_free(str_r);

	str_r = str_range(str, 5, 5); 
	assert(str_r);
	assert(memcmp(str_r, "", str_length(str_r)) == 0);
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

int main(void)
{
	RUN_TEST(test_str_empty);
	RUN_TEST(test_str_range);
	RUN_TEST(test_str_repeat);
	RUN_TEST(test_str_remove);
	RUN_TEST(test_str_replace);
	RUN_TEST(test_str_find);
	RUN_TEST(test_str_startswith);
	RUN_TEST(test_str_endswith);
	RUN_TEST(test_str_split);
	RUN_TEST(test_str_replace_count);
	RUN_TEST(test_str_replace_bigger);
	RUN_TEST(test_str_replace_smaller);
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

	return 0;
}
