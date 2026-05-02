// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#include "vec.h"
#include "utils.h"
#include <assert.h>
#include <stdint.h>

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

static void test_vec_ptr_at_empty_zero(void)
{
	vec_t vec = vec_new(sizeof(int));

	assert(vec_ptr_at(vec, 0) == NULL);

	vec_free(vec);
}

static void test_vec_ptr_at_empty_large_index(void)
{
	vec_t vec = vec_new(sizeof(int));

	assert(vec_ptr_at(vec, 999) == NULL);

	vec_free(vec);
}

static void test_vec_ptr_at_oob(void)
{
	const size_t len = 10;
	size_t *vec = vec_new_len(sizeof(size_t), len);
	size_t *item;

	for (size_t i = 0; i < len; i++)
		vec_set(vec, i, &i);

	item = vec_ptr_at(vec, len + 100);
	assert(*item == (len - 1));

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

static void test_vec_new_len_overflow(void)
{
	vec_t vec = vec_new_len(sizeof(size_t), SIZE_MAX);

	assert(vec == NULL);
}

static void test_vec_resize_overflow(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 1);

	vec_t result = vec_resize(vec, SIZE_MAX);
	assert(result == NULL);

	vec_free(vec);
}

static void test_vec_extend_overflow(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 1);

	vec_t result = vec_extend(vec, SIZE_MAX);
	assert(result == NULL);

	vec_free(vec);
}

static void test_vec_resize_large_unit_overflow(void)
{
	vec_t vec = vec_new_len(1, 1);

	size_t *result = vec_resize(vec, SIZE_MAX / 2 + 1);
	assert(result == NULL);
}

static void test_vec_resize_shrink(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 100);

	for (size_t i = 0; i < 100; i++)
		vec_set(vec, i, &i);

	vec = vec_resize(vec, 50);
	assert(vec);
	assert(vec_count(vec) == 50);

	size_t item;
	for (size_t i = 0; i < 50; i++) {
		vec_get(vec, i, &item);
		assert(item == i);
	}

	vec_free(vec);
}

static void test_vec_resize_same_size(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 10);

	vec = vec_resize(vec, 10);
	assert(vec);
	assert(vec_count(vec) == 10);

	vec_free(vec);
}

static void test_vec_extend_multiple(void)
{
	size_t *vec = vec_new(sizeof(size_t));

	vec = vec_extend(vec, 5);
	assert(vec);
	assert(vec_count(vec) == 5);

	vec_free(vec);
}

static void test_vec_get_empty(void)
{
	size_t *vec = vec_new(sizeof(size_t));
	size_t item = 42;

	vec_get(vec, 0, &item);
	assert(item == 42);

	vec_free(vec);
}

static void test_vec_copy_zero_len(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 10);
	size_t data[] = {99, 98, 97};

	vec_copy(vec, 0, data, 0);
	assert(vec_count(vec) == 10);

	size_t item;
	for (size_t i = 0; i < 10; i++) {
		vec_get(vec, i, &item);
		assert(item == 0);
	}

	vec_free(vec);
}

static void test_vec_copy_partial(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 5);
	size_t data[] = {10, 20, 30, 40, 50};

	for (size_t i = 0; i < 5; i++)
		vec_set(vec, i, &i);

	vec_copy(vec, 2, data, 2);

	size_t item;
	vec_get(vec, 2, &item);
	assert(item == 10);
	vec_get(vec, 3, &item);
	assert(item == 20);
	vec_get(vec, 4, &item);
	assert(item == 4);

	vec_free(vec);
}

static void test_vec_resize_preserves_data(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 10);

	for (size_t i = 0; i < 10; i++)
		vec_set(vec, i, &i);

	size_t old_cap = vec_capacity(vec);

	vec = vec_resize(vec, 200);
	assert(vec);
	assert(vec_count(vec) == 200);
	assert(vec_capacity(vec) > old_cap);

	size_t item;
	for (size_t i = 0; i < 10; i++) {
		vec_get(vec, i, &item);
		assert(item == i);
	}

	vec_free(vec);
}

static void test_vec_new_len_zero_count(void)
{
	vec_t vec = vec_new_len(sizeof(int), 0);

	assert(vec_count(vec) == 0);
	assert(vec_capacity(vec) == VEC_INIT_CAPACITY);

	vec_free(vec);
}

static void test_vec_set_get_first_last(void)
{
	const size_t len = 3;
	size_t *vec = vec_new_len(sizeof(size_t), len);

	size_t zero = 0;
	size_t last = 999;
	vec_set(vec, 0, &zero);
	vec_set(vec, 2, &last);

	size_t item;
	vec_get(vec, 0, &item);
	assert(item == 0);
	vec_get(vec, 1, &item);
	assert(item == 0);
	vec_get(vec, 2, &item);
	assert(item == 999);

	vec_free(vec);
}

static void test_vec_free(void)
{
	size_t *vec = vec_new_len(sizeof(size_t), 5);

	for (size_t i = 0; i < 5; i++)
		vec_set(vec, i, &i);

	vec_free(vec);
}

static void test_vec_copy_into_empty(void)
{
	size_t *vec = vec_new(sizeof(size_t));
	size_t data[] = {42};

	vec_copy(vec, 0, data, 1);
	assert(vec_count(vec) == 0);

	vec_free(vec);
}

static void test_vec_extend_zero(void)
{
	size_t *vec = vec_new(sizeof(size_t));

	vec = vec_extend(vec, 0);
	assert(vec);
	assert(vec_count(vec) == 0);

	vec_free(vec);
}

int main(void)
{
	RUN_TEST(test_vec_new);
	RUN_TEST(test_vec_new_len);
	RUN_TEST(test_vec_new_len_overflow);
	RUN_TEST(test_vec_resize);
	RUN_TEST(test_vec_resize_overflow);
	RUN_TEST(test_vec_resize_large_unit_overflow);
	RUN_TEST(test_vec_extend);
	RUN_TEST(test_vec_extend_overflow);
	RUN_TEST(test_vec_set_get);
	RUN_TEST(test_vec_ptr_at);
	RUN_TEST(test_vec_ptr_at_empty_zero);
	RUN_TEST(test_vec_ptr_at_empty_large_index);
	RUN_TEST(test_vec_ptr_at_oob);
	RUN_TEST(test_vec_copy);
	RUN_TEST(test_vec_copy_out_of_bounds);
	RUN_TEST(test_vec_resize_shrink);
	RUN_TEST(test_vec_resize_same_size);
	RUN_TEST(test_vec_extend_multiple);
	RUN_TEST(test_vec_get_empty);
	RUN_TEST(test_vec_copy_zero_len);
	RUN_TEST(test_vec_copy_partial);
	RUN_TEST(test_vec_resize_preserves_data);
	RUN_TEST(test_vec_new_len_zero_count);
	RUN_TEST(test_vec_set_get_first_last);
	RUN_TEST(test_vec_free);
	RUN_TEST(test_vec_copy_into_empty);
	RUN_TEST(test_vec_extend_zero);

	return 0;
}
