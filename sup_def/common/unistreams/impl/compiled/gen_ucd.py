#!/usr/bin/env python3

import typing
import sys
import os
import re
import requests
import argparse
import tqdm
import pprint
import cProfile, profile, pstats
from string import Template
from enum import Flag, unique

#url = "https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt"
#response = requests.get(url)
#if response.status_code != 200:
#    print("Error: unable to download UnicodeData.txt")
#    sys.exit(1)
#
## Parse UnicodeData.txt
#unicode_data = {}

def static_vars(**kwargs):
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate

class InternalError(Exception):
    pass

def parallel_for(start, end, body, num_procs = 4, *args, **kwargs):
    import multiprocessing
    with multiprocessing.Pool(num_procs) as pool:
        for i in range(start, end):
            pool.apply_async(body, (i, *args), kwargs)
        pool.close()
        pool.join()
    return None

UNICODE_DATABASE_URL = Template("https://www.unicode.org/Public/${version}/ucd/${filename}")

UNICODEDATA_PROPERTIES : list[tuple[str, str]] = [
    ("name", "const char*"),
    ("category", "const char*"),
    ("canonical_combining_class", "uint8_t"),
    ("bidi_class", "const char*"),
    ("decomposition_mapping", "const char*"),
    ("decimal_digit_value", "int"),
    ("digit_value", "int"),
    ("numeric_value", "double"),
    ("mirrored", "bool"),
    ("unicode_1_name", "const char*"),
    ("iso_comment", "const char*"),
    ("simple_uppercase_mapping", "const char*"),
    ("simple_lowercase_mapping", "const char*"),
    ("simple_titlecase_mapping", "const char*")
]

MAX_CODE_POINT = 0x10FFFF

CXX_CTYPE_BOOLPROP_TEMPLATE = Template(
    f"static constinit const std::bitset<size_t({hex(MAX_CODE_POINT)})> " +
    "${prop}_bitset" +
    "{\n" +
    "${values}" +
    "};\n"
)
CXX_CTYPE_MAPPINGPROP_TEMPLATE = Template(
    "static constinit const std::array<${type}" +
    f", size_t({hex(MAX_CODE_POINT)})> " +
    "${prop}_array" +
    "{\n" +
    "${values}" +
    "};\n"
)

def gen_c_ctype_bitset_def():
    return """
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

#undef  DETAIL_MAKE_C_BITSET
#define DETAIL_MAKE_C_BITSET(size)                                             \\
    struct {                                                                   \\
        uint8_t data_main [(size_t)(size) / (__CHAR_BIT__ * sizeof(uint8_t))]; \\
        bool    data_extra[(size_t)(size) % (__CHAR_BIT__ * sizeof(uint8_t))]; \\
    }

static inline bool detail_bitset_get_at(const void* bitset, size_t bitset_size, size_t index)
{
    typedef DETAIL_MAKE_C_BITSET(bitset_size) this_bitset_t;
    const this_bitset_t* this_bitset = (const this_bitset_t*)bitset;
    if (index >= bitset_size)
        return false;
    size_t max_main = bitset_size - (bitset_size % (__CHAR_BIT__ * sizeof(uint8_t)));
    if (index < max_main)
        return (this_bitset->data_main[index / (__CHAR_BIT__ * sizeof(uint8_t))] >> (index % (__CHAR_BIT__ * sizeof(uint8_t)))) & 1;
    return this_bitset->data_extra[index % (__CHAR_BIT__ * sizeof(uint8_t))];
}
"""

def gen_cxx_ctype_bitset_for(prop: str, values: list[bool | int]) -> str:
    if len(values) != MAX_CODE_POINT + 1:
        raise ValueError(f"Values for {prop} must have exactly {MAX_CODE_POINT + 1} elements")
    actual_values : str = "\t"
    actual_values      += "\"" # Start the string
    for i, value in enumerate(values):
        if i % 8 == 0:
            actual_values += "\"" # Close the string
            actual_values += "\n"
            actual_values += "\t"
            actual_values += "\"" # Reopen the string
        actual_values += "1" if bool(value) else "0"
    actual_values += "\"" # Close the string
    return CXX_CTYPE_BOOLPROP_TEMPLATE.substitute(prop=prop, values=actual_values)

def gen_cxx_ctype_mapping_for(prop: str, typeof_values: str, values: list[int]) -> str:
    if len(values) != MAX_CODE_POINT + 1:
        raise ValueError(f"Values for {prop} must have exactly {MAX_CODE_POINT + 1} elements")
    actual_values : str = ""
    for value in values:
        actual_values += f"\t{typeof_values}({value}),\n"
    return CXX_CTYPE_MAPPINGPROP_TEMPLATE.substitute(prop=prop, type=typeof_values, values=actual_values)

C_CTYPE_BOOLPROP_TEMPLATE = Template(
    gen_c_ctype_bitset_def() +
    f"\nstatic constexpr const DETAIL_MAKE_C_BITSET({hex(MAX_CODE_POINT)}) " +
    "${prop}_bitset" +
    " = {\n" +
    "${values}" +
    "};\n"
)
C_CTYPE_MAPPINGPROP_TEMPLATE = Template(
    f"static constexpr const " + "${type} ${prop}_array" + "[(size_t)({hex(MAX_CODE_POINT)})] = {\n" + "${values}" + "};\n"
)

def gen_c_ctype_bitset_for(prop: str, values: list[bool | int]) -> str:
    if len(values) != MAX_CODE_POINT + 1:
        raise ValueError(f"Values for {prop} must have exactly {MAX_CODE_POINT + 1} elements")
    actual_data_main_values : str = ""
    actual_data_extra_values : str = ""
    # Fill everything up
    main_values  : int = 0
    extra_values : int = 0
    for i, value in enumerate(values):
        if i < MAX_CODE_POINT - (MAX_CODE_POINT % (8 * 1)):
            actual_data_main_values += f"\t{value},\n"
            main_values += 1
        else:
            actual_data_extra_values += f"\t{value},\n"
            extra_values += 1
    if main_values != MAX_CODE_POINT // (8 * 1):
        raise InternalError(f"main_values = {main_values} != {MAX_CODE_POINT // (8 * 1)}")
    if extra_values != MAX_CODE_POINT % (8 * 1):
        raise InternalError(f"extra_values = {extra_values} != {MAX_CODE_POINT % (8 * 1)}")
    actual_values : str = "\t.data_main = {\n" + actual_data_main_values + "\t},\n" + "\t.data_extra = {\n" + actual_data_extra_values + "\t}\n"
    return C_CTYPE_BOOLPROP_TEMPLATE.substitute(prop=prop, values=actual_values)

def gen_c_ctype_mapping_for(prop: str, typeof_values: str, values: list[int]) -> str:
    if len(values) != MAX_CODE_POINT + 1:
        raise ValueError(f"Values for {prop} must have exactly {MAX_CODE_POINT + 1} elements")
    actual_values : str = ""
    for value in values:
        actual_values += f"\t({typeof_values}){value},\n"
    return C_CTYPE_MAPPINGPROP_TEMPLATE.substitute(prop=prop, type=typeof_values, values=actual_values)

def interpret_as(type, value, language):
    nullptr = "nullptr" if language == "c++" else "((void*)0)"
    trueval = "true" if language == "c++" else "1"
    falseval = "false" if language == "c++" else "0"
    match type:
        case "const char*":
            return f"\"{value}\"" if value else nullptr
        case "uint8_t":
            return f"{value}"
        case "int":
            return f"{value}"
        case "double":
            return f"{value}"
        case "bool":
            return trueval if value else falseval
        case _:
            raise ValueError(f"Unknown type {type}")
        
def make_table_name(prop, lang, prefix, suffix):
    return f"{prop}" if lang == "c++" else f"{prefix}{prop}{suffix}"

def flags_bits(*bits: int):
    result : int = 0
    for bit in bits:
        result |= (1 << bit)
    return result

@unique
class CtypeCategory(Flag):
    Lu = flags_bits(0)
    Ll = flags_bits(1)
    Lt = flags_bits(2)
    LC = Lu | Ll | Lt
    Lm = flags_bits(3)
    Lo = flags_bits(4)
    L  = Lu | Ll | Lt | Lm | Lo
    Mn = flags_bits(5)
    Mc = flags_bits(6)
    Me = flags_bits(7)
    M  = Mn | Mc | Me
    Nd = flags_bits(8)
    Nl = flags_bits(9)
    No = flags_bits(10)
    N  = Nd | Nl | No
    Pc = flags_bits(11)
    Pd = flags_bits(12)
    Ps = flags_bits(13)
    Pe = flags_bits(14)
    Pi = flags_bits(15)
    Pf = flags_bits(16)
    Po = flags_bits(17)
    P  = Pc | Pd | Ps | Pe | Pi | Pf | Po
    Sm = flags_bits(18)
    Sc = flags_bits(19)
    Sk = flags_bits(20)
    So = flags_bits(21)
    S  = Sm | Sc | Sk | So
    Zs = flags_bits(22)
    Zl = flags_bits(23)
    Zp = flags_bits(24)
    Z  = Zs | Zl | Zp
    Cc = flags_bits(25)
    Cf = flags_bits(26)
    Cs = flags_bits(27)
    Co = flags_bits(28)
    Cn = flags_bits(29)
    C  = Cc | Cf | Cs | Co | Cn
    @staticmethod
    def from_string(s: str):
        dic = CtypeCategory.as_string()
        if s not in dic.values():
            raise ValueError(f"Unknown category {s}")
        for cat, cat_str in dic.items():
            if cat_str == s:
                return cat
        raise InternalError(f"Unreachable code reached")
    @staticmethod
    def as_string():
        dic = {
            CtypeCategory.Lu: "Lu",
            CtypeCategory.Ll: "Ll",
            CtypeCategory.Lt: "Lt",
            CtypeCategory.LC: "LC",
            CtypeCategory.Lm: "Lm",
            CtypeCategory.Lo: "Lo",
            CtypeCategory.L : "L" ,
            CtypeCategory.Mn: "Mn",
            CtypeCategory.Mc: "Mc",
            CtypeCategory.Me: "Me",
            CtypeCategory.M : "M" ,
            CtypeCategory.Nd: "Nd",
            CtypeCategory.Nl: "Nl",
            CtypeCategory.No: "No",
            CtypeCategory.N : "N" ,
            CtypeCategory.Pc: "Pc",
            CtypeCategory.Pd: "Pd",
            CtypeCategory.Ps: "Ps",
            CtypeCategory.Pe: "Pe",
            CtypeCategory.Pi: "Pi",
            CtypeCategory.Pf: "Pf",
            CtypeCategory.Po: "Po",
            CtypeCategory.P : "P" ,
            CtypeCategory.Sm: "Sm",
            CtypeCategory.Sc: "Sc",
            CtypeCategory.Sk: "Sk",
            CtypeCategory.So: "So",
            CtypeCategory.S : "S" ,
            CtypeCategory.Zs: "Zs",
            CtypeCategory.Zl: "Zl",
            CtypeCategory.Zp: "Zp",
            CtypeCategory.Z : "Z" ,
            CtypeCategory.Cc: "Cc",
            CtypeCategory.Cf: "Cf",
            CtypeCategory.Cs: "Cs",
            CtypeCategory.Co: "Co",
            CtypeCategory.Cn: "Cn",
            CtypeCategory.C : "C"
        }
        if len(dic) != 38:
            raise InternalError(f"len(dic) = {len(dic)} != 38")
        return dic
    @staticmethod
    def gen_all():
        xs = [
                CtypeCategory.Lu, CtypeCategory.Ll, CtypeCategory.Lt, CtypeCategory.LC,
                CtypeCategory.Lm, CtypeCategory.Lo, CtypeCategory.L,  CtypeCategory.Mn,
                CtypeCategory.Mc, CtypeCategory.Me, CtypeCategory.M,  CtypeCategory.Nd,
                CtypeCategory.Nl, CtypeCategory.No, CtypeCategory.N,  CtypeCategory.Pc,
                CtypeCategory.Pd, CtypeCategory.Ps, CtypeCategory.Pe, CtypeCategory.Pi,
                CtypeCategory.Pf, CtypeCategory.Po, CtypeCategory.P,  CtypeCategory.Sm,
                CtypeCategory.Sc, CtypeCategory.Sk, CtypeCategory.So, CtypeCategory.S ,
                CtypeCategory.Zs, CtypeCategory.Zl, CtypeCategory.Zp, CtypeCategory.Z ,
                CtypeCategory.Cc, CtypeCategory.Cf, CtypeCategory.Cs, CtypeCategory.Co,
                CtypeCategory.Cn, CtypeCategory.C
            ]
        if len(xs) != 38:
            raise InternalError(f"len(xs) = {len(xs)} != 38")
        for cat in xs:
            yield cat

def unicode_ctype_is(cat: CtypeCategory, code_point: int, parsed_unicode_data: list[dict[str, typing.Any]], udata_length) -> bool:
    @static_vars(prev_line=0,maybeinrange=(False,0,0,"Cn"))
    def lookup_cat(_parsed_unicode_dat: list[dict[str, typing.Any]], cp: int) -> str:
        line = lookup_cat.prev_line - 1 if lookup_cat.prev_line > 0 else 0
        if lookup_cat.maybeinrange[0] and lookup_cat.maybeinrange[1] <= cp <= lookup_cat.maybeinrange[2]:
            return lookup_cat.maybeinrange[3]
        elif lookup_cat.maybeinrange[0]:
            lookup_cat.maybeinrange = (False, 0, 0, "Cn")
        while 0 <= line < udata_length and _parsed_unicode_dat[line]["code_point"] < cp:
            line += 1
        if line >= udata_length:
            lookup_cat.prev_line = udata_length - 1
            return "Cn"
        if _parsed_unicode_dat[line]["code_point"] == cp:
            lookup_cat.prev_line = line
            return _parsed_unicode_dat[line]["category"]
        else: # _parsed_unicode_dat[line]["code_point"] > cp
            to_test_props = ["name", "iso_comment", "unicode_1_name"]
            matches_first1 : map[bool] = map(
                lambda match: True if match else False,
                map(
                    lambda prop: re.match(
                        r".*First.*",
                        _parsed_unicode_dat[line - 1][prop],
                        re.IGNORECASE
                    ),
                    to_test_props
                )
            )
            matches_last2 : map[bool] = map(
                lambda match: True if match else False,
                map(
                    lambda prop: re.match(
                        r".*Last.*",
                        _parsed_unicode_dat[line][prop],
                        re.IGNORECASE
                    ),
                    to_test_props
                )
            )
            if any(matches_first1) and all(
                map(lambda b1, b2: b1 == b2, matches_first1, matches_last2)
            ):
                lookup_cat.prev_line = line - 1
                lookup_cat.maybeinrange = (True, _parsed_unicode_dat[line - 1]["code_point"], _parsed_unicode_dat[line]["code_point"], _parsed_unicode_dat[line - 1]["category"])
                assert _parsed_unicode_dat[line - 1]["category"] == _parsed_unicode_dat[line]["category"]
                return _parsed_unicode_dat[line - 1]["category"]
            else:
                lookup_cat.prev_line = line - 1
                lookup_cat.maybeinrange = (True, code_point, _parsed_unicode_dat[line]["code_point"] - 1, "Cn")
                return "Cn"
        typing.assert_never(None)
    if code_point < 0 or code_point > MAX_CODE_POINT:
        return False
    actual_cat = lookup_cat(parsed_unicode_data, code_point)
    return CtypeCategory.from_string(actual_cat) & cat in CtypeCategory
    #return str(actual_cat) in str(CtypeCategory.as_string()[cat]), ret_line

def dump_unicode_ctype_is_foreach_cat(parsed_unicode_data: list[dict[str, typing.Any]], dir: str, hu: bool):
    def dump_unicode_ctype_is(cat: CtypeCategory, parsed_unicode_data: list[dict[str, typing.Any]]):
        path = os.path.join(dir, f"unicode_ctype_is_{CtypeCategory.as_string()[cat]}.dat")
        pathcompressed = path + ".compressed"
        print(f"Dumping {CtypeCategory.as_string()[cat]} property for each code point to {path}")
        if os.path.exists(path):
            os.remove(path)
        if os.path.exists(pathcompressed):
            os.remove(pathcompressed)
        compressed_ranges : list[tuple[bool, int]] = []
        compressed_ranges_last_index : int = -1
        raw_string : str = ""
        raw_string_bool_list : list[bool] = []
        udata_length = len(parsed_unicode_data)
        for i in tqdm.tqdm(range(MAX_CODE_POINT + 1)):
            tmp_is = unicode_ctype_is(cat, i, parsed_unicode_data, udata_length)
            if compressed_ranges_last_index == -1:
                compressed_ranges.append((tmp_is, 1))
                compressed_ranges_last_index += 1
            elif tmp_is == compressed_ranges[compressed_ranges_last_index][0]:
                compressed_ranges[compressed_ranges_last_index] = (tmp_is, compressed_ranges[compressed_ranges_last_index][1] + 1)
            else:
                compressed_ranges.append((tmp_is, 1))
                compressed_ranges_last_index += 1
            raw_string_bool_list.append(tmp_is)
        if not hu:
            with open(path, "wb") as f:
                raw_string = "".join(map(lambda b: "\x01" if b else "\x00", raw_string_bool_list))
                f.write(raw_string.encode("utf-8"))
                f.close()
            with open(pathcompressed, "wb") as fcompressed:
                for is_true, count in compressed_ranges:
                    truefalse_bit = b"\x01" if is_true else b"\x00"
                    bytecount = count.to_bytes(4, 'big')
                    fcompressed.write(truefalse_bit)
                    fcompressed.write(bytecount)
                    fcompressed.write(b"\n")
                fcompressed.close()
        else:
            with open(path, "w") as f:
                to_write = "".join(map(lambda b: "1\n" if b else "0\n", raw_string_bool_list))
                f.write(to_write)
                f.close()
            with open(pathcompressed, "w") as fcompressed:
                for is_true, count in compressed_ranges:
                    fcompressed.write(f"{1 if is_true else 0}\n")
                    fcompressed.write(f"{count}\n")
                fcompressed.close()
        return None
    dump_unicode_ctype_is(CtypeCategory.Lu, parsed_unicode_data)
    #for cat in CtypeCategory.gen_all():
    #    dump_unicode_ctype_is(cat, parsed_unicode_data)
    #import multiprocessing
    #with multiprocessing.Pool(10) as pool:
    #    for cat in CtypeCategory.gen_all():
    #        pool.apply_async(dump_unicode_ctype_is, (cat, parsed_unicode_data))
    #    pool.close()
    #    pool.join()
    return None

#def dump_unicode_data_properties_tables(filename, unidata_lines, lang, namespace, prefix, suffix, verify=True):
#    print(f"Dumping Unicode data properties tables to {filename}")
#    if os.path.exists(filename):
#        os.remove(filename)
#    with open(filename, "w") as f:
#        if lang == "c++" and namespace:
#            f.write(f"namespace {namespace}\n")
#            f.write("{\n")
#        for tuple in UNICODEDATA_PROPERTIES:
#            curr_type = tuple[1]
#            curr_prop = tuple[0]
#            f.write(f"static const {curr_type} {make_table_name(curr_prop, lang, prefix, suffix)}[] = ")
#            f.write("{\n")
#            curr_cp = unidata_lines[0]["code_point"]
#            prev_cp = curr_cp
#            prev_line = None
#            for line in unidata_lines:
#                curr_cp = line["code_point"]
#                if not prev_line or prev_cp + 1 == curr_cp:
#                    f.write(f"    [ {curr_cp} ] = {interpret_as(curr_type, line[curr_prop], lang)},\n")
#                else:
#                    for i in range(prev_cp + 1, curr_cp):
#                        to_write = f"    [ {i} ] = {interpret_as(curr_type, prev_line[curr_prop], lang)},\n"
#                        if curr_prop == "name" or curr_prop == "iso_comment" or curr_prop == "unicode_1_name":
#                            # Replace the ", first" or ", First" with ""
#                            to_write = re.sub(r",\s+First", "", to_write)
#                            to_write = re.sub(r",\s+first", "", to_write)
#                            # If what has to be repeated contains ", Last" or ", last", replace everything with "unassigned or private use"
#                            if re.search(r",\s+Last", to_write) or re.search(r",\s+last", to_write):
#                                to_write = f"    [ {i} ] = \"unassigned or private use\",\n"
#                        f.write(to_write)
#                    f.write(f"    [ {curr_cp} ] = {interpret_as(curr_type, line[curr_prop], lang)},\n")
#                prev_cp = curr_cp
#                prev_line = line
#            f.write("};\n")
#        if lang == "c++" and namespace:
#            f.write("}\n")
#        f.close()
#    if verify:
#        print(f"Verifying correct generation of {filename}")
#        with open(filename, "r") as f:
#            CONTENT = f.read()
#            f.close()
#            def verif(*locvars):
#                n, cont, unidata_props, fname = locvars
#                if cont.count(f"[ {n} ]") != len(unidata_props):
#                    print(f"Error: verification failed for {fname} at code point {hex(n)}: {cont.count(f'[ {n} ]')} != {len(unidata_props)}")
#                    sys.exit(1)
#                return None
#            parallel_for(0, 1114109, verif, 10, (CONTENT, UNICODEDATA_PROPERTIES, filename))
#    return None

def parse_unicode_data_line(line):
    fields = line.split(";")
    code_point = int(fields[0], 16)
    name = fields[1]
    category = fields[2]
    canonical_combining_class = int(fields[3])
    bidi_class = fields[4]
    decomposition_mapping = fields[5]
    decimal_digit_value = fields[6]
    digit_value = fields[7]
    numeric_value = fields[8]
    mirrored = fields[9]
    unicode_1_name = fields[10]
    iso_comment = fields[11]
    simple_uppercase_mapping = fields[12]
    simple_lowercase_mapping = fields[13]
    simple_titlecase_mapping = fields[14]
    return {
        "code_point": code_point,
        "name": name,
        "category": category,
        "canonical_combining_class": canonical_combining_class,
        "bidi_class": bidi_class,
        "decomposition_mapping": decomposition_mapping,
        "decimal_digit_value": decimal_digit_value,
        "digit_value": digit_value,
        "numeric_value": numeric_value,
        "mirrored": mirrored,
        "unicode_1_name": unicode_1_name,
        "iso_comment": iso_comment,
        "simple_uppercase_mapping": simple_uppercase_mapping,
        "simple_lowercase_mapping": simple_lowercase_mapping,
        "simple_titlecase_mapping": simple_titlecase_mapping
    }

def parse_cmd_line():
    parser = argparse.ArgumentParser(
        prog=sys.argv[0],
        description="Generate Unicode related data tables for C and C++ programs."
    )
    parser.add_argument(
        "-u", "--unicode-version",
        help="Unicode version to use",
        action="store",
        default="15.1.0",
        dest="unicode_version"
    )
    parser.add_argument(
        "-o", "--output-dir",
        help="Output directory",
        action="store",
        default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "generated/"),
        dest="output_dir"
    )
    parser.add_argument(
        "-d", "--debug",
        help="Enable debug mode",
        action="store_true",
        default=False,
        dest="debug"
    )
    parser.add_argument(
        "-l", "--language",
        help="Output language",
        action="store",
        default="c++",
        dest="language"
    )
    parser.add_argument(
        "-n", "--namespace",
        help="Namespace for C++ output",
        action="store",
        default="uni::detail::unicodedata",
        dest="namespace"
    )
    parser.add_argument(
        "-p", "--prefix",
        help="Prefix for C output",
        action="store",
        default="uni_",
        dest="prefix"
    )
    parser.add_argument(
        "-s", "--suffix",
        help="Suffix for C output",
        action="store",
        default="",
        dest="suffix"
    )
    parser.add_argument(
        "-r", "--human-readable",
        help="Generate human-readable output",
        action="store_true",
        default=False,
        dest="human_readable"
    )
    return parser.parse_args()

def download_unicode_file(unicode_version, file):
    url = UNICODE_DATABASE_URL.substitute(version=unicode_version, filename=file)
    response = requests.get(url)
    if response.status_code != 200:
        print(f"Error: unable to download {file} from {url}")
        sys.exit(1)
    return response.text

def main():
    args = parse_cmd_line()
    if args.debug:
        print("Debug mode enabled")
        print("Command-line arguments:")
        pprint.pprint(args)
        pr = cProfile.Profile()
        pr.enable()
    print(f"Generating Unicode data tables for {args.language} in {args.output_dir}")
    unicode_data = download_unicode_file(args.unicode_version, "UnicodeData.txt")
    unicode_data_lines = unicode_data.split("\n")
    unicode_data_lines = [line for line in unicode_data_lines if line]
    unicode_data_lines = [parse_unicode_data_line(line) for line in unicode_data_lines]
    parsedudata_length = len(unicode_data_lines)
    print(f"Downloaded {parsedudata_length} lines of Unicode data")
    dump_unicode_ctype_is_foreach_cat(unicode_data_lines, args.output_dir, args.human_readable)
    if args.debug:
        pr.disable()
        ps = pstats.Stats(pr)
        ps.print_stats()
    #fileext = "cpp" if args.language == "c++" else "c"
    #dump_unicode_data_properties_tables(
    #    os.path.join(args.output_dir, f"unicode_data_properties_table.{fileext}"),
    #    unicode_data_lines,
    #    args.language,
    #    args.namespace,
    #    args.prefix,
    #    args.suffix
    #)

if __name__ == "__main__":
    main()
    #raise InternalError("Not implemented")
    #with open("test_c_bitset.h", "w") as f:
    #    f.write(gen_c_ctype_bitset_def())
    #    f.close()
    #print("Done")