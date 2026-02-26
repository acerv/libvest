// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#ifndef LIBVEST_STR_H
#define LIBVEST_STR_H

#include <stddef.h>
#include <stdbool.h>

/** @brief A simple string. */
typedef char* str_t;

/** @brief An array of strings. */
typedef str_t* vec_str_t;

/** @brief An array of indices. */
typedef size_t* vec_index_t;

/** @brief Create an empty string.
 *
 * @return Pointer to the first character of the string that is a terminator.
 */
str_t str_empty(void);

/** @brief Create a string with a specific length.
 *
 * Create a string with `count` length.
 *
 * @param count Number of characters.
 * @return Pointer to the first character of the string.
 * */
str_t str_new_len(const size_t count);

/** @brief Convert C-string into a string.
 *
 * Copy the content of the `str` into a new memory handled by a string.
 *
 * @param str C-string to copy.
 * @return Pointer to the first character of the string.
 */
str_t str_new(const char* str);

/** @brief Release string memory */
void str_free(str_t self);

/** @brief Return the length of a string.
 *
 * @param self The string.
 * @return Number of characters inside the string.
 */
size_t str_length(const str_t self) __attribute__((pure));

/** @brief Insert a C-string in a specific position of a string.
 *
 * Insert `str` at `pos` of `self`.
 *
 * @param self The string.
 * @param pos Position where we want to insert the C-string.
 * @param str C-string to insert.
 * @return Pointer to the first character of the string having `str` in `pos`.
 */
str_t str_insert(str_t self, const size_t pos, const char *str);

/** @brief Append a C-string to a string.
 *
 * Append `str` at the end of `self`.
 *
 * @param self The string.
 * @param str C-string to add.
 * @return Pointer to the first character of the string having `str` at the end.
 */
str_t str_append(str_t self, const char *str);

/** @brief Clear a string.
 *
 * Resize the string to 0 length.
 *
 * @param self The string.
 * @return Pointer to the first character of the string.
 */
str_t str_clear(str_t self);

/** @brief Split a string into multiple substrings according to the separator.
 *
 * Split `self` into multiple substrings according to `sep`.
 *
 * @param self The string.
 * @param sep Separator.
 * @return Array of strings.
 */
vec_str_t str_split(const str_t self, const char *sep);

/** @brief Release an array of strings.
 *
 * @param list Array of strings.
 */
void str_list_free(vec_str_t list);

/** @brief Return true if a string starts with a certain substring.
 *
 * Return true if `self` starts with `sub`. False otherwise.
 *
 * @param self The string.
 * @param sub Substring of the string.
 * @return True if `sub` is at the beginning of `self`. False otherwise.
 */
bool str_startswith(const str_t self, const char *sub) __attribute__((pure));

/** @brief Return true if a string ends with a certain substring.
 *
 * Return true if `self` ends with `sub`. False otherwise.
 *
 * @param self The string.
 * @param sub Substring of the string.
 * @return True if `sub` is at the end of `self`. False otherwise.
 */
bool str_endswith(const str_t self, const char *sub) __attribute__((pure));

/** @brief Find a substring inside a string.
 *
 * Find `pat` inside `self`.
 *
 * @param self The string.
 * @param pat Substring of the string.
 * @return Indices where `pat` is located inside `self`.
 */
vec_index_t str_find(const str_t self, const char *pat);

/** @brief Replace a substring with an another substring inside a string.
 *
 * Replace `old_str` with `new_str` inside `self` for `count` times.
 *
 * @param self The string.
 * @param old_str Substring to replace.
 * @param new_str Substring that will replace `old_str`.
 * @param count Number of occurrences we want to replace. -1 means all the
 *              occurrences.
 * @return String with replaced substrings.
 */
str_t str_replace(str_t self, const char *old_str, const char *new_str,
		  const int count);

/** @brief Remove a substring from a string.
 *
 * Remove all `str` inside `self`.
 *
 * @param self The string.
 * @param str Substring to remove.
 * @return String with removed substrings.
 */
str_t str_remove(str_t self, const char *str);

/** @brief Repeat a string n-times.
 *
 * Repeat `self` a `count` of times and return a new string.
 *
 * @param self The string.
 * @param count Number of times we want to repeat `self`.
 * @return String with repeated strings.
 */
str_t str_repeat(str_t self, const size_t count);

/** @brief Return a substring in within a range of a string.
 *
 * @param self The string.
 * @param start Lower location in `self`.
 * @param end Upper location in `self`.
 * @return Substring of `self` within `start` and `end` range.
 */
str_t str_range(const str_t self, size_t start, size_t end);

/** @brief Create a string according to formatter syntax.
 *
 * Uses a printf-like format syntax to create a string. String is cleared
 * before usage. Supported % formatters are:
 *
 * - %s : string
 * - %i : integer numbers
 * - %l : long numbers
 * - %u : unsigned numbers
 * - %f : floating point numbers
 *
 * @param self The string.
 * @param fmt print-like formatter string.
 * @param ... Variadic items to format inside string.
 * @return Formatted string.
 */
str_t str_format(str_t self, const char *fmt, ...);

#endif
