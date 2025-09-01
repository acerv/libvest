// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#ifndef LIBVEST_VEC_H 
#define LIBVEST_VEC_H

#include <stddef.h>

/** @brief Initial vector capacity. */
#define VEC_INIT_CAPACITY 128

/** @brief An abstract vector.
 *
 * A vector is a set of items of the same type that can be resized as needed.
 * Each vector has a default capacity that is increased when it's needed,
 * according to the requests, and a number of items of the same type, which is
 * variable.
 */
typedef void* vec_t;

/** @brief Create a new vector with a specific number of items.
 *
 * @param unit_size Size of a single item.
 * @param count Number of items.
 * @return New vector with `count` items of `unit_size` size.
 */
vec_t vec_new_len(const size_t unit_size, const size_t count);

/** @brief Create a new vector.
 *
 * @param unit_size Size of a single item.
 * @return New vector.
 */
vec_t vec_new(const size_t unit_size);

/** @brief Release the vector memory.
 */
void vec_free(vec_t self);

/** @brief Resize the vector.
 *
 * Change the vector size according to the items `count`.
 * @param self Vector object.
 * @param count Number of items.
 * @return Resized vector.
 */
vec_t vec_resize(vec_t self, const size_t count);

/** @brief Extend the vector by `count`.
 *
 * Extend the vector size according to the items `count`.
 * @param self Vector object.
 * @param count Number of items to add.
 * @return Extended vector.
 */
vec_t vec_extend(vec_t self, const size_t count);

/** @brief Return the size of a single item.
 *
 * @param self Vector object.
 * @return Single item size.
 */
size_t vec_unit_size(const vec_t self) __attribute__((pure));

/** @brief Return the number of items inside the vector.
 *
 * @param self Vector object.
 * @return Number of items inside the vector.
 */
size_t vec_count(const vec_t self) __attribute__((pure));

/** @brief Return the capacity of the vector.
 *
 * Capacity of the vector is the maximum number of items that the vector has
 * pre-allocated.
 *
 * @param self Vector object.
 * @return Capacity of the vector.
 */
size_t vec_capacity(const vec_t self) __attribute__((pure));

/** @brief Return the pointer to where item at `pos` is located.
 *
 * @param self Vector object.
 * @param pos Position of the item inside vector.
 * @return Item pointer.
 */
void *vec_ptr_at(vec_t self, size_t pos) __attribute__((pure));

/** @brief Set a set of `items` in `pos`.
 *
 * Copy the memory of `items` inside `data[]` on given position. The function
 * doesn't know what is the actual size of `items`, so **BEWARE** when you use
 * it.
 *
 * @param self Vector object.
 * @param pos Vector position.
 * @param items Pointer to the items.
 * @param len Number of items.
 */
void vec_copy(vec_t self, const size_t pos, const void *items, size_t len);

/** @brief Set a vector `item` in `pos`.
 *
 * Copy the memory of `item` inside `data[]` on given position. The function
 * doesn't know what is the actual type of `item`, so **BEWARE** when you use
 * it.
 *
 * @param self Vector object.
 * @param pos Vector position.
 * @param item Pointer to the item.
 */
void vec_set(vec_t self, const size_t pos, const void *item);

/** @brief Get a vector `item` in `pos`.
 *
 * Copy the memory of `data[]` at `pos` inside `item`.The function doesn't know
 * what is the actual type of `item`, so **BEWARE** when you use it.
 *
 * @param self Vector object.
 * @param pos Vector position.
 * @param item Pointer to the item.
 */
void vec_get(const vec_t self, const size_t pos, void *item);

#endif
