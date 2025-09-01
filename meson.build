# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2025 Andrea Cervesato <andrea.cervesato@mailbox.org>

project(
    'vest',
    'c',
    license : 'GPL-2.0+',
    version : '1.0',
    default_options : [
        'c_std=c99',
        'warning_level=everything',
    ],
    meson_version: '>=1.3.1'
)

add_project_arguments([
        '-Wno-c++-compat',
    ], language : 'c'
)

library_sources = [
    'vec.c',
    'str.c',
]

library_include = include_directories('.')

my_library = library(
    'vest',
    library_sources,
    include_directories : library_include,
    install : true,
    install_dir : 'lib',
)

vest = declare_dependency(
    include_directories: library_include,
    link_with: [my_library]
)

if get_option('build_tests')
    subdir('tests')
endif
