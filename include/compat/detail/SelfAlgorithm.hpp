#pragma once

#include "../Config.hpp"
#include "SelfRanges.hpp"

#include <utility>
#include <type_traits>
#include <iterator>
#include <algorithm>

namespace compat {

    /// <summary>
    /// ISO C++20 投影恆等式函數物件 (對齊 std::identity)。
    /// </summary>
    struct identity {
        template <typename T>
        constexpr T&& operator()(T&& t) const noexcept {
            return std::forward<T>(t);
        }
        using is_transparent = void;
    };

    namespace detail {

        // --- C++11 invoke 輔助實作 ---
        template <typename F, typename... Args>
        constexpr auto invoke_impl(int, F&& f, Args&&... args)
            -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
            return std::forward<F>(f)(std::forward<Args>(args)...);
        }

        template <typename Base, typename T, typename Derived, typename... Args>
        constexpr auto invoke_impl(char, T Base::*pmf, Derived&& ref, Args&&... args)
            -> decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)) {
            return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
        }

        template <typename Base, typename T, typename Derived>
        constexpr auto invoke_impl(short, T Base::*pmd, Derived&& ref)
            -> decltype(std::forward<Derived>(ref).*pmd) {
            return std::forward<Derived>(ref).*pmd;
        }

        template <typename F, typename... Args>
        constexpr auto invoke(F&& f, Args&&... args)
            -> decltype(invoke_impl(0, std::forward<F>(f), std::forward<Args>(args)...)) {
            return invoke_impl(0, std::forward<F>(f), std::forward<Args>(args)...);
        }

        struct less_fn {
            template <typename T, typename U>
            constexpr bool operator()(T&& a, U&& b) const {
                return std::forward<T>(a) < std::forward<U>(b);
            }
        };

        struct equal_to_fn {
            template <typename T, typename U>
            constexpr bool operator()(T&& a, U&& b) const {
                return std::forward<T>(a) == std::forward<U>(b);
            }
        };

    } // namespace detail

    namespace detail {
    namespace self_algo {
    namespace ranges {

        // --- Tagged Result Types (ISO C++20 std::ranges::*_result) ---
        template <typename I, typename F>
        struct in_fun_result {
            I in;
            F fun;
        };

        template <typename I1, typename I2>
        struct in_in_result {
            I1 in1;
            I2 in2;
        };

        template <typename I, typename O>
        struct in_out_result {
            I in;
            O out;
        };

        template <typename I1, typename I2, typename O>
        struct in_in_out_result {
            I1 in1;
            I2 in2;
            O out;
        };

        template <typename I, typename O1, typename O2>
        struct in_out_out_result {
            I in;
            O1 out1;
            O2 out2;
        };

        template <typename T>
        struct min_max_result {
            T min;
            T max;
        };

        template <typename I>
        struct in_found_result {
            I in;
            bool found;
        };

        template <typename I, typename F>
        using for_each_result = in_fun_result<I, F>;

        template <typename I, typename O>
        using copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using copy_n_result = in_out_result<I, O>;

        template <typename I, typename O>
        using copy_backward_result = in_out_result<I, O>;

        template <typename I, typename O>
        using move_result = in_out_result<I, O>;

        template <typename I, typename O>
        using move_backward_result = in_out_result<I, O>;

        template <typename I1, typename I2>
        using mismatch_result = in_in_result<I1, I2>;

        template <typename I, typename O>
        using unary_transform_result = in_out_result<I, O>;

        template <typename I1, typename I2, typename O>
        using binary_transform_result = in_in_out_result<I1, I2, O>;

        template <typename I1, typename I2>
        using swap_ranges_result = in_in_result<I1, I2>;

        template <typename I, typename O>
        using reverse_copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using rotate_copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using unique_copy_result = in_out_result<I, O>;

        template <typename I, typename O1, typename O2>
        using partition_copy_result = in_out_out_result<I, O1, O2>;

        template <typename T>
        using minmax_result = min_max_result<T>;

        template <typename I>
        using minmax_element_result = min_max_result<I>;

        template <typename I>
        using next_permutation_result = in_found_result<I>;

        template <typename I>
        using prev_permutation_result = in_found_result<I>;

        using compat::detail::self_ranges::ranges::range;
        using compat::detail::self_ranges::ranges::borrowed_range;
        using compat::detail::self_ranges::ranges::borrowed_iterator_t;
        using compat::detail::self_ranges::ranges::borrowed_subrange_t;
        using compat::detail::self_ranges::ranges::range_value_t;
        using compat::detail::self_ranges::ranges::range_difference_t;
        using compat::detail::self_ranges::ranges::range_reference_t;
        using compat::detail::self_ranges::ranges::subrange;
        using compat::detail::self_ranges::ranges::iterator_t;
        using compat::detail::self_ranges::ranges::sentinel_t;
        using compat::detail::self_ranges::ranges::iter_difference_t;
        using compat::detail::self_ranges::ranges::iter_value_t;
        using compat::detail::self_ranges::ranges::iter_reference_t;
        using compat::detail::self_ranges::ranges::dangling;

        namespace detail {

            namespace range_detail = compat::detail::self_ranges::ranges::detail;
            using namespace compat::detail::self_ranges::ranges;

            // --- Non-modifying Sequence Algorithms ---

            struct all_of_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return false;
                        }
                    }
                    return true;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct any_of_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return true;
                        }
                    }
                    return false;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct none_of_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return false;
                        }
                    }
                    return true;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct for_each_fn {
                template <typename I, typename S, typename F, typename Proj = compat::identity>
                constexpr for_each_result<I, F> operator()(I first, S last, F f, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        compat::detail::invoke(f, compat::detail::invoke(proj, *first));
                    }
                    return {first, std::move(f)};
                }

                template <typename R, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr for_each_result<borrowed_iterator_t<R>, F> operator()(R&& r, F f, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f), std::move(proj));
                }
            };

            struct for_each_n_fn {
                template <typename I, typename Size, typename F, typename Proj = compat::identity>
                constexpr for_each_result<I, F> operator()(I first, Size n, F f, Proj proj = {}) const {
                    for (Size i = 0; i < n; ++i, ++first) {
                        compat::detail::invoke(f, compat::detail::invoke(proj, *first));
                    }
                    return {first, std::move(f)};
                }
            };

            struct count_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity>
                constexpr iter_difference_t<I> operator()(I first, S last, const T& value, Proj proj = {}) const {
                    iter_difference_t<I> counter = 0;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(proj, *first) == value) {
                            ++counter;
                        }
                    }
                    return counter;
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr range_difference_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct count_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr iter_difference_t<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    iter_difference_t<I> counter = 0;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            ++counter;
                        }
                    }
                    return counter;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr range_difference_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct mismatch_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr mismatch_result<I1, I2> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                             Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    while (first1 != last1 && first2 != last2 &&
                           compat::detail::invoke(pred, compat::detail::invoke(proj1, *first1),
                                                       compat::detail::invoke(proj2, *first2))) {
                        ++first1;
                        ++first2;
                    }
                    return {first1, first2};
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr mismatch_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>>
                operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct equal_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                          Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            return false;
                        }
                    }
                    return first1 == last1 && first2 == last2;
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct lexicographical_compare_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                          Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; (first1 != last1) && (first2 != last2); ++first1, ++first2) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            return true;
                        }
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                         compat::detail::invoke(proj1, *first1))) {
                            return false;
                        }
                    }
                    return (first1 == last1) && (first2 != last2);
                }

                template <typename R1, typename R2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr bool operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct find_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<!range<I>::value>::type>
                constexpr I operator()(I first, S last, const T& value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(proj, *first) == value) {
                            return first;
                        }
                    }
                    return first;
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct find_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return first;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct find_if_not_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return first;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct adjacent_find_fn {
                template <typename I, typename S, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Pred pred = {}, Proj proj = {}) const {
                    if (first == last) return first;
                    I next = first;
                    ++next;
                    for (; next != last; ++first, ++next) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first),
                                                         compat::detail::invoke(proj, *next))) {
                            return first;
                        }
                    }
                    return next;
                }

                template <typename R, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Pred pred = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct search_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr subrange<I1> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                  Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    if (first2 == last2) return {first1, first1};
                    for (; first1 != last1; ++first1) {
                        I1 it1 = first1;
                        I2 it2 = first2;
                        while (it1 != last1 && it2 != last2 &&
                               compat::detail::invoke(pred, compat::detail::invoke(proj1, *it1),
                                                           compat::detail::invoke(proj2, *it2))) {
                            ++it1;
                            ++it2;
                        }
                        if (it2 == last2) return {first1, it1};
                    }
                    return {first1, first1};
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr borrowed_subrange_t<R1> operator()(R1&& r1, R2&& r2,
                                                             Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct contains_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, const T& value, Proj proj = {}) const {
                    return find_fn{}(first, last, value, std::move(proj)) != last;
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct starts_with_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                          Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            return false;
                        }
                    }
                    return first2 == last2;
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct ends_with_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                          Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    // 若為雙向迭代器則從尾部往前回溯
                    I1 end1 = first1;
                    while (end1 != last1) ++end1;
                    I2 end2 = first2;
                    while (end2 != last2) ++end2;

                    auto d1 = std::distance(first1, end1);
                    auto d2 = std::distance(first2, end2);
                    if (d1 < d2) return false;

                    std::advance(first1, d1 - d2);
                    return equal_fn{}(first1, end1, first2, end2, std::move(pred), std::move(proj1), std::move(proj2));
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct fold_left_fn {
                template <typename I, typename S, typename T, typename F>
                constexpr T operator()(I first, S last, T init, F f) const {
                    for (; first != last; ++first) {
                        init = compat::detail::invoke(f, std::move(init), *first);
                    }
                    return init;
                }

                template <typename R, typename T, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr T operator()(R&& r, T init, F f) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(init), std::move(f));
                }
            };

            // --- Modifying Sequence Algorithms ---

            struct copy_fn {
                template <typename I, typename S, typename O>
                constexpr copy_result<I, O> operator()(I first, S last, O result) const {
                    for (; first != last; ++first, ++result) {
                        *result = *first;
                    }
                    return {first, result};
                }

                template <typename R, typename O,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct copy_if_fn {
                template <typename I, typename S, typename O, typename Pred, typename Proj = compat::identity>
                constexpr copy_result<I, O> operator()(I first, S last, O result, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            *result = *first;
                            ++result;
                        }
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(pred), std::move(proj));
                }
            };

            struct copy_n_fn {
                template <typename I, typename Size, typename O>
                constexpr copy_result<I, O> operator()(I first, Size n, O result) const {
                    for (Size i = 0; i < n; ++i, ++first, ++result) {
                        *result = *first;
                    }
                    return {first, result};
                }
            };

            struct copy_backward_fn {
                template <typename I1, typename S1, typename I2>
                constexpr copy_result<I1, I2> operator()(I1 first1, S1 last1, I2 last2) const {
                    I1 it = last1;
                    while (it != first1) {
                        --it;
                        --last2;
                        *last2 = *it;
                    }
                    return {last1, last2};
                }

                template <typename R, typename I2,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr copy_result<borrowed_iterator_t<R>, I2> operator()(R&& r, I2 last2) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(last2));
                }
            };

            struct move_fn {
                template <typename I, typename S, typename O>
                constexpr move_result<I, O> operator()(I first, S last, O result) const {
                    for (; first != last; ++first, ++result) {
                        *result = std::move(*first);
                    }
                    return {first, result};
                }

                template <typename R, typename O,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr move_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct move_backward_fn {
                template <typename I1, typename S1, typename I2>
                constexpr move_result<I1, I2> operator()(I1 first1, S1 last1, I2 last2) const {
                    I1 it = last1;
                    while (it != first1) {
                        --it;
                        --last2;
                        *last2 = std::move(*it);
                    }
                    return {last1, last2};
                }

                template <typename R, typename I2,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr move_result<borrowed_iterator_t<R>, I2> operator()(R&& r, I2 last2) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(last2));
                }
            };

            struct fill_fn {
                template <typename T, typename I, typename S>
                constexpr I operator()(I first, S last, const T& value) const {
                    for (; first != last; ++first) {
                        *first = value;
                    }
                    return first;
                }

                template <typename T, typename R,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, const T& value) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value);
                }
            };

            struct fill_n_fn {
                template <typename T, typename I, typename Size>
                constexpr I operator()(I first, Size n, const T& value) const {
                    for (Size i = 0; i < n; ++i, ++first) {
                        *first = value;
                    }
                    return first;
                }
            };

            struct transform_fn {
                // 單元變換 (Unary)
                template <typename I, typename S, typename O, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<!range<I>::value>::type>
                constexpr unary_transform_result<I, O> operator()(I first, S last, O result, F op, Proj proj = {}) const {
                    for (; first != last; ++first, ++result) {
                        *result = compat::detail::invoke(op, compat::detail::invoke(proj, *first));
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value && !range<O>::value>::type>
                constexpr unary_transform_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, F op, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(op), std::move(proj));
                }

                // 二元變換 (Binary)
                template <typename I1, typename S1, typename I2, typename S2, typename O, typename F,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity>
                constexpr binary_transform_result<I1, I2, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                                        O result, F op,
                                                                        Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; first1 != last1 && first2 != last2; ++first1, ++first2, ++result) {
                        *result = compat::detail::invoke(op, compat::detail::invoke(proj1, *first1),
                                                             compat::detail::invoke(proj2, *first2));
                    }
                    return {first1, first2, result};
                }

                template <typename R1, typename R2, typename O, typename F,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr binary_transform_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>, O>
                operator()(R1&& r1, R2&& r2, O result, F op, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(op), std::move(proj1), std::move(proj2));
                }
            };

            struct generate_fn {
                template <typename I, typename S, typename F>
                constexpr I operator()(I first, S last, F gen) const {
                    for (; first != last; ++first) {
                        *first = gen();
                    }
                    return first;
                }

                template <typename R, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, F gen) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(gen));
                }
            };

            struct generate_n_fn {
                template <typename I, typename Size, typename F>
                constexpr I operator()(I first, Size n, F gen) const {
                    for (Size i = 0; i < n; ++i, ++first) {
                        *first = gen();
                    }
                    return first;
                }
            };

            struct remove_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    first = find_if_fn{}(first, last, pred, proj);
                    if (first != last) {
                        I i = first;
                        for (++i; i != last; ++i) {
                            if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *i))) {
                                *first = std::move(*i);
                                ++first;
                            }
                        }
                    }
                    return {first, first};
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct remove_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity>
                constexpr subrange<I> operator()(I first, S last, const T& value, Proj proj = {}) const {
                    return remove_if_fn{}(first, last, [&value](const T& x) { return x == value; }, std::move(proj));
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_subrange_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct replace_if_fn {
                template <typename I, typename S, typename Pred, typename T, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Pred pred, const T& new_value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            *first = new_value;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Pred pred, const T& new_value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), new_value, std::move(proj));
                }
            };

            struct replace_fn {
                template <typename I, typename S, typename T1, typename T2, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, const T1& old_value, const T2& new_value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(proj, *first) == old_value) {
                            *first = new_value;
                        }
                    }
                    return first;
                }

                template <typename R, typename T1, typename T2, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, const T1& old_value, const T2& new_value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), old_value, new_value, std::move(proj));
                }
            };

            struct swap_ranges_fn {
                template <typename I1, typename S1, typename I2, typename S2>
                constexpr swap_ranges_result<I1, I2> operator()(I1 first1, S1 last1, I2 first2, S2 last2) const {
                    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
                        using std::swap;
                        swap(*first1, *first2);
                    }
                    return {first1, first2};
                }

                template <typename R1, typename R2,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                constexpr swap_ranges_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>>
                operator()(R1&& r1, R2&& r2) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2));
                }
            };

            struct reverse_fn {
                template <typename I, typename S>
                constexpr I operator()(I first, S last) const {
                    I end_it = last;
                    while (first != end_it && first != --end_it) {
                        using std::swap;
                        swap(*first, *end_it);
                        ++first;
                    }
                    return last;
                }

                template <typename R,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r));
                }
            };

            struct reverse_copy_fn {
                template <typename I, typename S, typename O>
                constexpr reverse_copy_result<I, O> operator()(I first, S last, O result) const {
                    I it = last;
                    while (it != first) {
                        --it;
                        *result = *it;
                        ++result;
                    }
                    return {last, result};
                }

                template <typename R, typename O,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr reverse_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct rotate_fn {
                template <typename I, typename S>
                constexpr subrange<I> operator()(I first, I middle, S last) const {
                    I next = middle;
                    while (first != next) {
                        using std::swap;
                        swap(*first++, *next++);
                        if (next == last) {
                            next = middle;
                        } else if (first == middle) {
                            middle = next;
                        }
                    }
                    return {first, middle};
                }

                template <typename R,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_subrange_t<R> operator()(R&& r, iterator_t<R> middle) const {
                    return (*this)(range_detail::begin_fn{}(r), std::move(middle), range_detail::end_fn{}(r));
                }
            };

            struct unique_fn {
                template <typename I, typename S, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity>
                constexpr subrange<I> operator()(I first, S last, Pred pred = {}, Proj proj = {}) const {
                    if (first == last) return {first, first};
                    I dest = first;
                    ++first;
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *dest),
                                                         compat::detail::invoke(proj, *first))) {
                            ++dest;
                            if (dest != first) {
                                *dest = std::move(*first);
                            }
                        }
                    }
                    ++dest;
                    return {dest, first};
                }

                template <typename R, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_subrange_t<R> operator()(R&& r, Pred pred = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            // --- Partitioning Operations ---

            struct is_partitioned_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) break;
                    }
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) return false;
                    }
                    return true;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct partition_point_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    auto len = std::distance(first, last);
                    while (len > 0) {
                        auto half = len / 2;
                        I mid = first;
                        std::advance(mid, half);
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *mid))) {
                            first = ++mid;
                            len -= half + 1;
                        } else {
                            len = half;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct partition_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity>
                constexpr subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    first = find_if_not_fn{}(first, last, pred, proj);
                    if (first == last) return {first, first};
                    for (I i = std::next(first); i != last; ++i) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *i))) {
                            using std::swap;
                            swap(*first, *i);
                            ++first;
                        }
                    }
                    return {first, first};
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            // --- Sorting & Binary Search Operations ---

            struct is_sorted_until_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    if (first == last) return first;
                    I next = first;
                    for (++next; next != last; first = next, ++next) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj, *next),
                                                         compat::detail::invoke(proj, *first))) {
                            return next;
                        }
                    }
                    return next;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct is_sorted_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    return is_sorted_until_fn{}(first, last, std::move(comp), std::move(proj)) == last;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct sort_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    std::sort(first, last, [&comp, &proj](const decltype(*first)& a, const decltype(*first)& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a),
                                                            compat::detail::invoke(proj, b));
                    });
                    return last;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct stable_sort_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    std::stable_sort(first, last, [&comp, &proj](const decltype(*first)& a, const decltype(*first)& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a),
                                                            compat::detail::invoke(proj, b));
                    });
                    return last;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct lower_bound_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
                    auto len = std::distance(first, last);
                    while (len > 0) {
                        auto half = len / 2;
                        I mid = first;
                        std::advance(mid, half);
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj, *mid), value)) {
                            first = ++mid;
                            len -= half + 1;
                        } else {
                            len = half;
                        }
                    }
                    return first;
                }

                template <typename R, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            struct upper_bound_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
                    auto len = std::distance(first, last);
                    while (len > 0) {
                        auto half = len / 2;
                        I mid = first;
                        std::advance(mid, half);
                        if (!compat::detail::invoke(comp, value, compat::detail::invoke(proj, *mid))) {
                            first = ++mid;
                            len -= half + 1;
                        } else {
                            len = half;
                        }
                    }
                    return first;
                }

                template <typename R, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            struct equal_range_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr subrange<I> operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return {lower_bound_fn{}(first, last, value, comp, proj),
                            upper_bound_fn{}(first, last, value, comp, proj)};
                }

                template <typename R, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_subrange_t<R> operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            struct binary_search_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr bool operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
                    I it = lower_bound_fn{}(first, last, value, comp, proj);
                    return it != last && !compat::detail::invoke(comp, value, compat::detail::invoke(proj, *it));
                }

                template <typename R, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr bool operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            // --- Min/Max Operations ---

            struct min_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    if (first == last) return first;
                    I smallest = first;
                    ++first;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj, *first),
                                                         compat::detail::invoke(proj, *smallest))) {
                            smallest = first;
                        }
                    }
                    return smallest;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct max_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    if (first == last) return first;
                    I largest = first;
                    ++first;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj, *largest),
                                                         compat::detail::invoke(proj, *first))) {
                            largest = first;
                        }
                    }
                    return largest;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct minmax_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr minmax_element_result<I> operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I min_it = first;
                    I max_it = first;
                    if (first == last) return {min_it, max_it};
                    ++first;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj, *first),
                                                         compat::detail::invoke(proj, *min_it))) {
                            min_it = first;
                        } else if (compat::detail::invoke(comp, compat::detail::invoke(proj, *max_it),
                                                               compat::detail::invoke(proj, *first))) {
                            max_it = first;
                        }
                    }
                    return {min_it, max_it};
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr minmax_element_result<borrowed_iterator_t<R>> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct min_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr const T& operator()(const T& a, const T& b, Comp comp = {}, Proj proj = {}) const {
                    return compat::detail::invoke(comp, compat::detail::invoke(proj, b),
                                                        compat::detail::invoke(proj, a)) ? b : a;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr range_value_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    auto it = min_element_fn{}(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                    return *it;
                }
            };

            struct max_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr const T& operator()(const T& a, const T& b, Comp comp = {}, Proj proj = {}) const {
                    return compat::detail::invoke(comp, compat::detail::invoke(proj, a),
                                                        compat::detail::invoke(proj, b)) ? b : a;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr range_value_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    auto it = max_element_fn{}(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                    return *it;
                }
            };

            struct minmax_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr minmax_result<const T&> operator()(const T& a, const T& b, Comp comp = {}, Proj proj = {}) const {
                    if (compat::detail::invoke(comp, compat::detail::invoke(proj, b),
                                                    compat::detail::invoke(proj, a))) {
                        return {b, a};
                    }
                    return {a, b};
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                constexpr minmax_result<range_value_t<R>> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    auto res = minmax_element_fn{}(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                    return {*res.min, *res.max};
                }
            };

            struct clamp_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                constexpr const T& operator()(const T& val, const T& lo, const T& hi, Comp comp = {}, Proj proj = {}) const {
                    return compat::detail::invoke(comp, compat::detail::invoke(proj, val),
                                                        compat::detail::invoke(proj, lo)) ? lo
                         : compat::detail::invoke(comp, compat::detail::invoke(proj, hi),
                                                        compat::detail::invoke(proj, val)) ? hi : val;
                }
            };

        } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr detail::all_of_fn all_of{};
        inline constexpr detail::any_of_fn any_of{};
        inline constexpr detail::none_of_fn none_of{};
        inline constexpr detail::for_each_fn for_each{};
        inline constexpr detail::for_each_n_fn for_each_n{};
        inline constexpr detail::count_fn count{};
        inline constexpr detail::count_if_fn count_if{};
        inline constexpr detail::mismatch_fn mismatch{};
        inline constexpr detail::equal_fn equal{};
        inline constexpr detail::lexicographical_compare_fn lexicographical_compare{};
        inline constexpr detail::find_fn find{};
        inline constexpr detail::find_if_fn find_if{};
        inline constexpr detail::find_if_not_fn find_if_not{};
        inline constexpr detail::adjacent_find_fn adjacent_find{};
        inline constexpr detail::search_fn search{};
        inline constexpr detail::contains_fn contains{};
        inline constexpr detail::starts_with_fn starts_with{};
        inline constexpr detail::ends_with_fn ends_with{};
        inline constexpr detail::fold_left_fn fold_left{};

        inline constexpr detail::copy_fn copy{};
        inline constexpr detail::copy_if_fn copy_if{};
        inline constexpr detail::copy_n_fn copy_n{};
        inline constexpr detail::copy_backward_fn copy_backward{};
        inline constexpr detail::move_fn move{};
        inline constexpr detail::move_backward_fn move_backward{};
        inline constexpr detail::fill_fn fill{};
        inline constexpr detail::fill_n_fn fill_n{};
        inline constexpr detail::transform_fn transform{};
        inline constexpr detail::generate_fn generate{};
        inline constexpr detail::generate_n_fn generate_n{};
        inline constexpr detail::remove_fn remove{};
        inline constexpr detail::remove_if_fn remove_if{};
        inline constexpr detail::replace_fn replace{};
        inline constexpr detail::replace_if_fn replace_if{};
        inline constexpr detail::swap_ranges_fn swap_ranges{};
        inline constexpr detail::reverse_fn reverse{};
        inline constexpr detail::reverse_copy_fn reverse_copy{};
        inline constexpr detail::rotate_fn rotate{};
        inline constexpr detail::unique_fn unique{};

        inline constexpr detail::is_partitioned_fn is_partitioned{};
        inline constexpr detail::partition_fn partition{};
        inline constexpr detail::partition_point_fn partition_point{};

        inline constexpr detail::is_sorted_fn is_sorted{};
        inline constexpr detail::is_sorted_until_fn is_sorted_until{};
        inline constexpr detail::sort_fn sort{};
        inline constexpr detail::stable_sort_fn stable_sort{};
        inline constexpr detail::lower_bound_fn lower_bound{};
        inline constexpr detail::upper_bound_fn upper_bound{};
        inline constexpr detail::equal_range_fn equal_range{};
        inline constexpr detail::binary_search_fn binary_search{};

        inline constexpr detail::min_element_fn min_element{};
        inline constexpr detail::max_element_fn max_element{};
        inline constexpr detail::minmax_element_fn minmax_element{};
        inline constexpr detail::min_fn min{};
        inline constexpr detail::max_fn max{};
        inline constexpr detail::minmax_fn minmax{};
        inline constexpr detail::clamp_fn clamp{};
#else
        namespace {
            constexpr const detail::all_of_fn& all_of = compat::detail::self_ranges::static_const<detail::all_of_fn>::value;
            constexpr const detail::any_of_fn& any_of = compat::detail::self_ranges::static_const<detail::any_of_fn>::value;
            constexpr const detail::none_of_fn& none_of = compat::detail::self_ranges::static_const<detail::none_of_fn>::value;
            constexpr const detail::for_each_fn& for_each = compat::detail::self_ranges::static_const<detail::for_each_fn>::value;
            constexpr const detail::for_each_n_fn& for_each_n = compat::detail::self_ranges::static_const<detail::for_each_n_fn>::value;
            constexpr const detail::count_fn& count = compat::detail::self_ranges::static_const<detail::count_fn>::value;
            constexpr const detail::count_if_fn& count_if = compat::detail::self_ranges::static_const<detail::count_if_fn>::value;
            constexpr const detail::mismatch_fn& mismatch = compat::detail::self_ranges::static_const<detail::mismatch_fn>::value;
            constexpr const detail::equal_fn& equal = compat::detail::self_ranges::static_const<detail::equal_fn>::value;
            constexpr const detail::lexicographical_compare_fn& lexicographical_compare = compat::detail::self_ranges::static_const<detail::lexicographical_compare_fn>::value;
            constexpr const detail::find_fn& find = compat::detail::self_ranges::static_const<detail::find_fn>::value;
            constexpr const detail::find_if_fn& find_if = compat::detail::self_ranges::static_const<detail::find_if_fn>::value;
            constexpr const detail::find_if_not_fn& find_if_not = compat::detail::self_ranges::static_const<detail::find_if_not_fn>::value;
            constexpr const detail::adjacent_find_fn& adjacent_find = compat::detail::self_ranges::static_const<detail::adjacent_find_fn>::value;
            constexpr const detail::search_fn& search = compat::detail::self_ranges::static_const<detail::search_fn>::value;
            constexpr const detail::contains_fn& contains = compat::detail::self_ranges::static_const<detail::contains_fn>::value;
            constexpr const detail::starts_with_fn& starts_with = compat::detail::self_ranges::static_const<detail::starts_with_fn>::value;
            constexpr const detail::ends_with_fn& ends_with = compat::detail::self_ranges::static_const<detail::ends_with_fn>::value;
            constexpr const detail::fold_left_fn& fold_left = compat::detail::self_ranges::static_const<detail::fold_left_fn>::value;

            constexpr const detail::copy_fn& copy = compat::detail::self_ranges::static_const<detail::copy_fn>::value;
            constexpr const detail::copy_if_fn& copy_if = compat::detail::self_ranges::static_const<detail::copy_if_fn>::value;
            constexpr const detail::copy_n_fn& copy_n = compat::detail::self_ranges::static_const<detail::copy_n_fn>::value;
            constexpr const detail::copy_backward_fn& copy_backward = compat::detail::self_ranges::static_const<detail::copy_backward_fn>::value;
            constexpr const detail::move_fn& move = compat::detail::self_ranges::static_const<detail::move_fn>::value;
            constexpr const detail::move_backward_fn& move_backward = compat::detail::self_ranges::static_const<detail::move_backward_fn>::value;
            constexpr const detail::fill_fn& fill = compat::detail::self_ranges::static_const<detail::fill_fn>::value;
            constexpr const detail::fill_n_fn& fill_n = compat::detail::self_ranges::static_const<detail::fill_n_fn>::value;
            constexpr const detail::transform_fn& transform = compat::detail::self_ranges::static_const<detail::transform_fn>::value;
            constexpr const detail::generate_fn& generate = compat::detail::self_ranges::static_const<detail::generate_fn>::value;
            constexpr const detail::generate_n_fn& generate_n = compat::detail::self_ranges::static_const<detail::generate_n_fn>::value;
            constexpr const detail::remove_fn& remove = compat::detail::self_ranges::static_const<detail::remove_fn>::value;
            constexpr const detail::remove_if_fn& remove_if = compat::detail::self_ranges::static_const<detail::remove_if_fn>::value;
            constexpr const detail::replace_fn& replace = compat::detail::self_ranges::static_const<detail::replace_fn>::value;
            constexpr const detail::replace_if_fn& replace_if = compat::detail::self_ranges::static_const<detail::replace_if_fn>::value;
            constexpr const detail::swap_ranges_fn& swap_ranges = compat::detail::self_ranges::static_const<detail::swap_ranges_fn>::value;
            constexpr const detail::reverse_fn& reverse = compat::detail::self_ranges::static_const<detail::reverse_fn>::value;
            constexpr const detail::reverse_copy_fn& reverse_copy = compat::detail::self_ranges::static_const<detail::reverse_copy_fn>::value;
            constexpr const detail::rotate_fn& rotate = compat::detail::self_ranges::static_const<detail::rotate_fn>::value;
            constexpr const detail::unique_fn& unique = compat::detail::self_ranges::static_const<detail::unique_fn>::value;

            constexpr const detail::is_partitioned_fn& is_partitioned = compat::detail::self_ranges::static_const<detail::is_partitioned_fn>::value;
            constexpr const detail::partition_fn& partition = compat::detail::self_ranges::static_const<detail::partition_fn>::value;
            constexpr const detail::partition_point_fn& partition_point = compat::detail::self_ranges::static_const<detail::partition_point_fn>::value;

            constexpr const detail::is_sorted_fn& is_sorted = compat::detail::self_ranges::static_const<detail::is_sorted_fn>::value;
            constexpr const detail::is_sorted_until_fn& is_sorted_until = compat::detail::self_ranges::static_const<detail::is_sorted_until_fn>::value;
            constexpr const detail::sort_fn& sort = compat::detail::self_ranges::static_const<detail::sort_fn>::value;
            constexpr const detail::stable_sort_fn& stable_sort = compat::detail::self_ranges::static_const<detail::stable_sort_fn>::value;
            constexpr const detail::lower_bound_fn& lower_bound = compat::detail::self_ranges::static_const<detail::lower_bound_fn>::value;
            constexpr const detail::upper_bound_fn& upper_bound = compat::detail::self_ranges::static_const<detail::upper_bound_fn>::value;
            constexpr const detail::equal_range_fn& equal_range = compat::detail::self_ranges::static_const<detail::equal_range_fn>::value;
            constexpr const detail::binary_search_fn& binary_search = compat::detail::self_ranges::static_const<detail::binary_search_fn>::value;

            constexpr const detail::min_element_fn& min_element = compat::detail::self_ranges::static_const<detail::min_element_fn>::value;
            constexpr const detail::max_element_fn& max_element = compat::detail::self_ranges::static_const<detail::max_element_fn>::value;
            constexpr const detail::minmax_element_fn& minmax_element = compat::detail::self_ranges::static_const<detail::minmax_element_fn>::value;
            constexpr const detail::min_fn& min = compat::detail::self_ranges::static_const<detail::min_fn>::value;
            constexpr const detail::max_fn& max = compat::detail::self_ranges::static_const<detail::max_fn>::value;
            constexpr const detail::minmax_fn& minmax = compat::detail::self_ranges::static_const<detail::minmax_fn>::value;
            constexpr const detail::clamp_fn& clamp = compat::detail::self_ranges::static_const<detail::clamp_fn>::value;
        }
#endif

    } // namespace ranges
    } // namespace self_algo
    } // namespace detail

} // namespace compat
