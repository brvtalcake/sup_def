#project('sup_def', ['c', 'cpp'], license: 'MIT', license_files: ['LICENSE'], meson_version: '>=1.1.0', version: '0.0.1')

project('sup_def', ['c', 'cpp'], license: 'MIT', version: '0.0.1')
add_global_arguments('-std=gnu++23', language: 'cpp')
add_global_arguments('-std=gnu17', language: 'c')
add_global_arguments('-ftrack-macro-expansion=0', language: 'cpp')
add_global_arguments('-ftrack-macro-expansion=0', language: 'c')

sdcommon_srcfiles=files('sup_def/common/engine.cpp', 'sup_def/common/parser.cpp', 'sup_def/common/pragmas.cpp', 'sup_def/common/util/errors.cpp')

static_library('sdcommon', sdcommon_srcfiles)