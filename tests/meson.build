# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>

new_tests = [
    'test_str.c',
    'test_vec.c',
]

foreach src : new_tests
    bin = src.split('.')[0]
    exe = executable(
        bin,
        src,
        include_directories : [library_include, '.'],
        link_with : my_library,
    )
    test(bin, exe)
endforeach
