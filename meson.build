#project('sup_def', ['c', 'cpp'], license: 'MIT', license_files: ['LICENSE'], meson_version: '>=1.1.0', version: '0.0.1')

project('sup_def', ['c', 'cpp'], license: 'MIT', version: '0.0.1', default_options: ['warning_level=2', 'cpp_std=gnu++20', 'c_std=gnu17', 'buildtype=release'])

c_compiler = meson.get_compiler('c')
cpp_compiler = meson.get_compiler('cpp')

add_global_arguments('-ftrack-macro-expansion=0', language: 'c')
add_global_arguments('-flto', language: 'c')
add_global_arguments('-ftrack-macro-expansion=0', language: 'cpp')
add_global_arguments('-flto', language: 'cpp')

sdcommon_srcfiles=files('sup_def/common/engine.cpp', 'sup_def/common/parser.cpp', 'sup_def/common/pragmas.cpp', 'sup_def/common/util/errors.cpp')

sdcommon=static_library('sdcommon', sdcommon_srcfiles)

external_supdef_srcfiles=files('sup_def/external/main.cpp', 'sup_def/external/program.cpp', 'sup_def/external/cmdline.cpp')
executable('supdef', external_supdef_srcfiles, link_with: sdcommon)