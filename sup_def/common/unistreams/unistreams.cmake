include("${SUPDEF_PROJECT_CMAKE_DIR}/get_header_list.cmake")

set(
    UNISTREAMS_SOURCES_LIST

    "${CMAKE_CURRENT_LIST_DIR}/unistreams.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/types.hpp"

    "${CMAKE_CURRENT_LIST_DIR}/static_test/detail_optional.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/static_test/detail_tuple.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/static_test/utf_chars.ipp"

    "${CMAKE_CURRENT_LIST_DIR}/impl/char_traits.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/detail_optional.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/detail_tuple.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/file.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/mmaped_file.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/utf_chars.ipp"

    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_char8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_char32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_utf16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_utf8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_wchar.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/wchar_char32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char16_utf8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char8_utf16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/utf16_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/wchar_utf16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char32_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char16_char32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char32_utf8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/wchar_utf8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char8_char32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char8_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/wchar_char8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char8_utf8.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/utf8_utf16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char8_char16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/utf8_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char16_utf16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/wchar_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char32_utf16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char16_utf32.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/char_char16.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/impl/string_conversions/wchar_char16.ipp"

    PARENT_SCOPE
)

get_header_list(
    OUTPUT_VAR TMP_UNISTREAMS_HEADERS_LIST
    SOURCES_LIST ${UNISTREAMS_SOURCES_LIST}
    VERIFY_EXISTS
)

set(UNISTREAMS_HEADERS_LIST ${TMP_UNISTREAMS_HEADERS_LIST} PARENT_SCOPE)