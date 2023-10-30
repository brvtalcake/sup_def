/* 
 * MIT License
 * 
 * Copyright (c) 2023 Axel PASCON
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if !INCLUDED_FROM_SUPDEF_SOURCE
    #error "Don't include this yourself"
#endif

#if !DEFINED(CharacterType)
template <typename T>
concept CharacterType = std::same_as<char, std::remove_cv_t<T>>       ||
                        std::same_as<wchar_t, std::remove_cv_t<T>>    ||
                        std::same_as<char8_t, std::remove_cv_t<T>>    ||
                        std::same_as<char16_t, std::remove_cv_t<T>>   ||
                        std::same_as<char32_t, std::remove_cv_t<T>>;
#define CharacterType_DEFINED 1
#endif

#if !DEFINED(CStrType)
template <typename T>
concept CStrType = (std::is_pointer_v<T> || std::is_array_v<T>) &&
                   CharacterType<
                       std::remove_cv_t<std::remove_pointer_t<
                           std::remove_cv_t<std::decay_t<
                               T
                            >>
                        >>
                    >;
#define CStrType_DEFINED 1
#endif

#if !DEFINED(PtrCStr)
template <typename T>
concept PtrCStr = std::is_pointer_v<T>                        &&
                  CharacterType<
                      std::remove_cv_t<std::remove_pointer_t<
                          std::remove_cv_t<T>
                      >>
                  >;
#define PtrCStr_DEFINED 1
#endif

#if !DEFINED(UnboundedArrayCStr)
template <typename T>
concept UnboundedArrayCStr = std::is_unbounded_array_v<T>                &&
                             CharacterType<
                                 std::remove_cv_t<std::remove_pointer_t<
                                     std::remove_cv_t<std::decay_t<
                                         T
                                     >>
                                 >>
                             >;
#define UnboundedArrayCStr_DEFINED 1
#endif

#if !DEFINED(BoundedArrayCStr)
template <typename T>
concept BoundedArrayCStr = std::is_bounded_array_v<T>                    &&
                           CharacterType<
                               std::remove_cv_t<std::remove_pointer_t<
                                   std::remove_cv_t<std::decay_t<
                                       T
                                   >>
                               >>
                           >;
#define BoundedArrayCStr_DEFINED 1
#endif

#if !DEFINED(ArrayCStr)
template <typename T>
concept ArrayCStr = UnboundedArrayCStr<T> || BoundedArrayCStr<T>;
#define ArrayCStr_DEFINED 1
#endif

#if !DEFINED(StdStringType)
template <typename T>
concept StdStringType = requires(T)
{
    typename std::remove_cvref_t<T>::value_type;
    typename std::remove_cvref_t<T>::size_type;
    typename std::remove_cvref_t<T>::traits_type;
    typename std::remove_cvref_t<T>::allocator_type;
};
#define StdStringType_DEFINED 1
#endif

#if !DEFINED(StringType)
template <typename T>
concept StringType = CStrType<T>                                                ||
                     ( StdStringType<std::remove_cv_t<T>>                       &&
                     CharacterType<typename std::remove_cv_t<T>::value_type> );
#define StringType_DEFINED 1
#endif

#if !DEFINED(FilePath)
template <typename T>
concept FilePath = std::same_as<std::remove_cvref_t<std::filesystem::path>, T>   || 
                   StringType<T>;

#define FilePath_DEFINED 1
#endif
