file(GLOB_RECURSE SDTHIRDPARTY_MAP_SOURCES "*.h" "*.hpp" "*.cpp" "*.c")
add_library(sdthirdparty_map INTERFACE)
target_sources(sdthirdparty_map INTERFACE ${SDTHIRDPARTY_MAP_SOURCES})