// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#include "vec.h"
#include "utils.h"
#include <assert.h>

static void test_vec_new(void)
{
	int *vec = vec_new(sizeof(int));

	assert(vec_unit_size(vec) == sizeof(int));
	assert(vec_count(vec) == 0);
	assert(vec_capacity(vec) > 0);

	vec_free(vec);
}

static void test_vec_new_len(void)
{
	int *vec = vec_new_len(sizeof(int), 1024);

	assert(vec_unit_size(vec) == sizeof(int));
	assert(vec_count(vec) == 1024);
	assert(vec_capacity(vec) > 0);

	vec_free(vec);
}

static void test_vec_resize(void)
{
	int *vec = vec_new(sizeof(int));

	for (size_t size = 0; size < 2048; size++) {
		vec = vec_resize(vec, size);
		assert(vec);
		assert(vec_count(vec) == size);
	}

	for (size_t size = 2048; size > 0; size--) {
		vec = vec_resize(vec, size);
		assert(vec);
		assert(vec_count(vec) == size);
	}

	vec_free(vec);
}

static void test_vec_extend(void)
{
	int *vec = vec_new(sizeof(int));
	size_t old_count;

	for (size_t i = 0; i < 2048; i++) {
		old_count = vec_count(vec);
		vec = vec_extend(vec, 1);
		assert(vec);
		assert(vec_count(vec) == old_count + 1);
	}

	vec_free(vec);
}

static void test_vec_set_get(void)
{
	const size_t len = 10;

	vec_t *vec = vec_new_len(sizeof(size_t), len);
	size_t item;
	size_t count;

	for (size_t i = 0; i < len; i++)
		vec_set(vec, (size_t)i, &i);

	count = 0;
	for (size_t i = 0; i < len; i++) {
		vec_get(vec, (size_t)i, &item);
		if (item == i)
			count++;
	}

	assert(count == len);

	vec_free(vec);
}

static void test_vec_ptr_at(void)
{
	const int len = 64;

	size_t count;
	long long *item;

	vec_t vec = vec_new_len(sizeof(long long), (size_t)len);

	for (long long i = 0; i < len; i++)
		vec_set(vec, (size_t)i, &i);

	count = 0;
	for (int i = 0; i < len; i++) {
		item = (long long *)vec_ptr_at(vec, (size_t)i);
		if (i != *item)
			count++;
	}

	assert(count == 0);

	vec_free(vec);
}

#define VEC_COPY_LEN 20

static void test_vec_copy(void)
{
	size_t item;
	size_t count;
	size_t items[VEC_COPY_LEN];
	vec_t vec = vec_new_len(sizeof(size_t), VEC_COPY_LEN);

	for (size_t i = 0; i < VEC_COPY_LEN; i++)
		items[i] = i + 100;

	vec_copy(vec, 0, items, VEC_COPY_LEN);

	count = 0;
	for (size_t i = 0; i < vec_capacity(vec); i++) {
		item = *(((size_t *)vec) + i);
		if (item == (i + 100))
			count++;
	}

	assert(count == vec_count(vec));

	vec_free(vec);
}

#define VEC_COPY_OOB_LEN 20

static void test_vec_copy_out_of_bounds(void)
{
	size_t item;
	size_t count;
	size_t items[VEC_COPY_OOB_LEN];
	vec_t vec = vec_new_len(sizeof(size_t), VEC_COPY_OOB_LEN - 10);

	for (size_t i = 0; i < VEC_COPY_OOB_LEN; i++)
		items[i] = i + 200;

	/* pos > vec_count(vec) */
	vec_copy(vec, vec_count(vec) + 1, items, VEC_COPY_OOB_LEN);

	count = 0;
	for (size_t i = 0; i < vec_capacity(vec); i++) {
		item = *(((size_t *)vec) + i);
		if (item)
			count++;
	}

	assert(count == 0);

	/* items len > vec_count(vec) */
	vec_copy(vec, 0, items, VEC_COPY_OOB_LEN);

	count = 0;
	for (size_t i = 0; i < vec_capacity(vec); i++) {
		item = *(((size_t *)vec) + i);
		if (item)
			count++;
	}

	assert(count == vec_count(vec));

	vec_free(vec);
}

int main(void)
{
	RUN_TEST(test_vec_new);
	RUN_TEST(test_vec_new_len);
	RUN_TEST(test_vec_resize);
	RUN_TEST(test_vec_extend);
	RUN_TEST(test_vec_set_get);
	RUN_TEST(test_vec_ptr_at);
	RUN_TEST(test_vec_copy);
	RUN_TEST(test_vec_copy_out_of_bounds);

	return 0;
}
