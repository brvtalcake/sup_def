# sdthirdparty_empty_macro
file(GLOB_RECURSE SDTHIRDPARTY_EMPTY_MACRO_SOURCES "*.h" "*.hpp" "*.cpp" "*.c")
add_library(sdthirdparty_empty_macro INTERFACE)
target_sources(sdthirdparty_empty_macro INTERFACE ${SDTHIRDPARTY_EMPTY_MACRO_SOURCES})