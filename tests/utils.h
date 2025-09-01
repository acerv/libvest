// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define RUN_TEST(test) do {\
	printf(">>> %s\n", #test); \
	test(); \
} while(0)

#endif
