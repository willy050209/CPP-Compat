#pragma once

#include "../Config.hpp"
#include "../Optional.hpp"
#include "SelfRanges.hpp"

#include <utility>
#include <type_traits>
#include <iterator>
#include <algorithm>
#include <random>

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

        template <typename I, typename T>
        struct in_value_result {
            I in;
            T value;
        };

        template <typename O, typename T>
        struct out_value_result {
            O out;
            T value;
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
        using replace_copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using replace_copy_if_result = in_out_result<I, O>;

        template <typename I, typename O>
        using remove_copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using remove_copy_if_result = in_out_result<I, O>;

        template <typename I, typename O>
        using reverse_copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using rotate_copy_result = in_out_result<I, O>;

        template <typename I, typename O>
        using unique_copy_result = in_out_result<I, O>;

        template <typename I, typename O1, typename O2>
        using partition_copy_result = in_out_out_result<I, O1, O2>;

        template <typename I, typename O>
        using partial_sort_copy_result = in_out_result<I, O>;

        template <typename I1, typename I2, typename O>
        using merge_result = in_in_out_result<I1, I2, O>;

        template <typename I1, typename I2, typename O>
        using set_union_result = in_in_out_result<I1, I2, O>;

        template <typename I1, typename I2, typename O>
        using set_intersection_result = in_in_out_result<I1, I2, O>;

        template <typename I1, typename O>
        using set_difference_result = in_out_result<I1, O>;

        template <typename I1, typename I2, typename O>
        using set_symmetric_difference_result = in_in_out_result<I1, I2, O>;

        template <typename I, typename T>
        using fold_left_with_iter_result = in_value_result<I, T>;

        template <typename I, typename T>
        using fold_left_first_with_iter_result = in_value_result<I, T>;

        template <typename O, typename T>
        using iota_result = out_value_result<O, T>;

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

            template <typename I, typename S>
            struct is_iterator_sentinel_pair : std::integral_constant<bool,
                sentinel_for<S, I>::value && !range<typename std::decay<I>::type>::value
            > {};

            // --- Non-modifying Sequence Algorithms ---

            struct all_of_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return false;
                        }
                    }
                    return true;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct any_of_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return true;
                        }
                    }
                    return false;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct none_of_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return false;
                        }
                    }
                    return true;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct for_each_fn {
                template <typename I, typename S, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 for_each_result<I, F> operator()(I first, S last, F f, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        compat::detail::invoke(f, compat::detail::invoke(proj, *first));
                    }
                    return {first, std::move(f)};
                }

                template <typename R, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 for_each_result<borrowed_iterator_t<R>, F> operator()(R&& r, F f, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f), std::move(proj));
                }
            };

            struct for_each_n_fn {
                template <typename I, typename Size, typename F, typename Proj = compat::identity>
                COMPAT_CONSTEXPR_14 for_each_result<I, F> operator()(I first, Size n, F f, Proj proj = {}) const {
                    for (Size i = 0; i < n; ++i, ++first) {
                        compat::detail::invoke(f, compat::detail::invoke(proj, *first));
                    }
                    return {first, std::move(f)};
                }
            };

            struct count_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 iter_difference_t<I> operator()(I first, S last, const T& value, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 range_difference_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct count_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 iter_difference_t<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 range_difference_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct mismatch_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 mismatch_result<I1, I2> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 mismatch_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>>
                operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct equal_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct lexicographical_compare_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct find_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, const T& value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(proj, *first) == value) {
                            return first;
                        }
                    }
                    return first;
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct find_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return first;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct find_if_not_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            return first;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct adjacent_find_fn {
                template <typename I, typename S, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Pred pred = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Pred pred = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct search_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I1> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R1> operator()(R1&& r1, R2&& r2,
                                                             Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct contains_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, const T& value, Proj proj = {}) const {
                    return find_fn{}(first, last, value, std::move(proj)) != last;
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct starts_with_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct ends_with_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct contains_subrange_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                    Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    if (first2 == last2) return true;
                    auto sub = search_fn{}(first1, last1, first2, last2, std::move(pred), std::move(proj1), std::move(proj2));
                    return sub.begin() != last1;
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct find_last_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, const T& value, Proj proj = {}) const {
                    I found = first;
                    bool has_found = false;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(proj, *first) == value) {
                            found = first;
                            has_found = true;
                        }
                    }
                    if (has_found) {
                        return subrange<I>{found, first};
                    }
                    return subrange<I>{first, first};
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct find_last_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    I found = first;
                    bool has_found = false;
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            found = first;
                            has_found = true;
                        }
                    }
                    if (has_found) {
                        return subrange<I>{found, first};
                    }
                    return subrange<I>{first, first};
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct find_last_if_not_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    I found = first;
                    bool has_found = false;
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            found = first;
                            has_found = true;
                        }
                    }
                    if (has_found) {
                        return subrange<I>{found, first};
                    }
                    return subrange<I>{first, first};
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct find_end_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I1> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                            Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    if (first2 == last2) {
                        I1 it = first1;
                        while (it != last1) ++it;
                        return {it, it};
                    }
                    auto match = search_fn{}(first1, last1, first2, last2, pred, proj1, proj2);
                    if (match.begin() == last1) return match;
                    auto last_match = match;
                    while (true) {
                        I1 next_first = last_match.begin();
                        ++next_first;
                        auto next_match = search_fn{}(next_first, last1, first2, last2, pred, proj1, proj2);
                        if (next_match.begin() == last1) break;
                        last_match = next_match;
                    }
                    return last_match;
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R1> operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct find_first_of_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 I1 operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                  Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; first1 != last1; ++first1) {
                        for (I2 it2 = first2; it2 != last2; ++it2) {
                            if (compat::detail::invoke(pred, compat::detail::invoke(proj1, *first1),
                                                             compat::detail::invoke(proj2, *it2))) {
                                return first1;
                            }
                        }
                    }
                    return first1;
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R1> operator()(R1&& r1, R2&& r2,
                                                                       Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct search_n_fn {
                template <typename I, typename S, typename T,
                          typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, iter_difference_t<I> count, const T& value,
                                                           Pred pred = {}, Proj proj = {}) const {
                    if (count <= 0) return {first, first};
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first), value)) {
                            I match_begin = first;
                            iter_difference_t<I> n = 1;
                            while (n < count) {
                                ++first;
                                if (first == last) return {first, first};
                                if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first), value)) break;
                                ++n;
                            }
                            if (n == count) {
                                I match_end = first;
                                ++match_end;
                                return {match_begin, match_end};
                            }
                        }
                    }
                    return {first, first};
                }

                template <typename R, typename T,
                          typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, range_difference_t<R> count, const T& value,
                                                                      Pred pred = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), count, value,
                                   std::move(pred), std::move(proj));
                }
            };

            struct fold_left_fn {
                template <typename I, typename S, typename T, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 T operator()(I first, S last, T init, F f) const {
                    for (; first != last; ++first) {
                        init = compat::detail::invoke(f, std::move(init), *first);
                    }
                    return init;
                }

                template <typename R, typename T, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 T operator()(R&& r, T init, F f) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(init), std::move(f));
                }
            };

            struct fold_left_first_fn {
                template <typename I, typename S, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                auto operator()(I first, S last, F f) const
                    -> compat::optional<typename std::decay<decltype(compat::detail::invoke(f, *first, *first))>::type> {
                    using T = typename std::decay<decltype(compat::detail::invoke(f, *first, *first))>::type;
                    if (first == last) return compat::optional<T>{};
                    T acc = *first;
                    ++first;
                    for (; first != last; ++first) {
                        acc = compat::detail::invoke(f, std::move(acc), *first);
                    }
                    return compat::optional<T>{std::move(acc)};
                }

                template <typename R, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                auto operator()(R&& r, F f) const
                    -> decltype((*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f))) {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f));
                }
            };

            struct fold_right_fn {
                template <typename I, typename S, typename T, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                T operator()(I first, S last, T init, F f) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    while (end_it != first) {
                        --end_it;
                        init = compat::detail::invoke(f, *end_it, std::move(init));
                    }
                    return init;
                }

                template <typename R, typename T, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                T operator()(R&& r, T init, F f) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(init), std::move(f));
                }
            };

            struct fold_right_last_fn {
                template <typename I, typename S, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                auto operator()(I first, S last, F f) const
                    -> compat::optional<typename std::decay<decltype(compat::detail::invoke(f, *first, *first))>::type> {
                    using T = typename std::decay<decltype(compat::detail::invoke(f, *first, *first))>::type;
                    if (first == last) return compat::optional<T>{};
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    --end_it;
                    T acc = *end_it;
                    while (end_it != first) {
                        --end_it;
                        acc = compat::detail::invoke(f, *end_it, std::move(acc));
                    }
                    return compat::optional<T>{std::move(acc)};
                }

                template <typename R, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                auto operator()(R&& r, F f) const
                    -> decltype((*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f))) {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f));
                }
            };

            struct fold_left_with_iter_fn {
                template <typename I, typename S, typename T, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                fold_left_with_iter_result<I, T> operator()(I first, S last, T init, F f) const {
                    for (; first != last; ++first) {
                        init = compat::detail::invoke(f, std::move(init), *first);
                    }
                    return {first, std::move(init)};
                }

                template <typename R, typename T, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                fold_left_with_iter_result<borrowed_iterator_t<R>, T> operator()(R&& r, T init, F f) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(init), std::move(f));
                }
            };

            struct fold_left_first_with_iter_fn {
                template <typename I, typename S, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                auto operator()(I first, S last, F f) const
                    -> fold_left_first_with_iter_result<I, compat::optional<typename std::decay<decltype(compat::detail::invoke(f, *first, *first))>::type>> {
                    using T = typename std::decay<decltype(compat::detail::invoke(f, *first, *first))>::type;
                    if (first == last) return {first, compat::optional<T>{}};
                    T acc = *first;
                    ++first;
                    for (; first != last; ++first) {
                        acc = compat::detail::invoke(f, std::move(acc), *first);
                    }
                    return {first, compat::optional<T>{std::move(acc)}};
                }

                template <typename R, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                auto operator()(R&& r, F f) const
                    -> fold_left_first_with_iter_result<borrowed_iterator_t<R>, compat::optional<typename std::decay<decltype(compat::detail::invoke(f, *range_detail::begin_fn{}(r), *range_detail::begin_fn{}(r)))>::type>> {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(f));
                }
            };

            // --- Modifying Sequence Algorithms ---

            struct copy_fn {
                template <typename I, typename S, typename O,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 copy_result<I, O> operator()(I first, S last, O result) const {
                    for (; first != last; ++first, ++result) {
                        *result = *first;
                    }
                    return {first, result};
                }

                template <typename R, typename O,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct copy_if_fn {
                template <typename I, typename S, typename O, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 copy_result<I, O> operator()(I first, S last, O result, Pred pred, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(pred), std::move(proj));
                }
            };

            struct copy_n_fn {
                template <typename I, typename Size, typename O>
                COMPAT_CONSTEXPR_14 copy_result<I, O> operator()(I first, Size n, O result) const {
                    for (Size i = 0; i < n; ++i, ++first, ++result) {
                        *result = *first;
                    }
                    return {first, result};
                }
            };

            struct copy_backward_fn {
                template <typename I1, typename S1, typename I2,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value>::type>
                COMPAT_CONSTEXPR_14 copy_result<I1, I2> operator()(I1 first1, S1 last1, I2 last2) const {
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
                COMPAT_CONSTEXPR_14 copy_result<borrowed_iterator_t<R>, I2> operator()(R&& r, I2 last2) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(last2));
                }
            };

            struct move_fn {
                template <typename I, typename S, typename O,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 move_result<I, O> operator()(I first, S last, O result) const {
                    for (; first != last; ++first, ++result) {
                        *result = std::move(*first);
                    }
                    return {first, result};
                }

                template <typename R, typename O,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 move_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct move_backward_fn {
                template <typename I1, typename S1, typename I2,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value>::type>
                COMPAT_CONSTEXPR_14 move_result<I1, I2> operator()(I1 first1, S1 last1, I2 last2) const {
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
                COMPAT_CONSTEXPR_14 move_result<borrowed_iterator_t<R>, I2> operator()(R&& r, I2 last2) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(last2));
                }
            };

            struct fill_fn {
                template <typename T, typename I, typename S,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, const T& value) const {
                    for (; first != last; ++first) {
                        *first = value;
                    }
                    return first;
                }

                template <typename T, typename R,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, const T& value) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value);
                }
            };

            struct fill_n_fn {
                template <typename T, typename I, typename Size>
                COMPAT_CONSTEXPR_14 I operator()(I first, Size n, const T& value) const {
                    for (Size i = 0; i < n; ++i, ++first) {
                        *first = value;
                    }
                    return first;
                }
            };

            struct transform_fn {
                // 單元變換 (Unary)
                template <typename I, typename S, typename O, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 unary_transform_result<I, O> operator()(I first, S last, O result, F op, Proj proj = {}) const {
                    for (; first != last; ++first, ++result) {
                        *result = compat::detail::invoke(op, compat::detail::invoke(proj, *first));
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename F, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value && !range<O>::value>::type>
                COMPAT_CONSTEXPR_14 unary_transform_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, F op, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(op), std::move(proj));
                }

                // 二元變換 (Binary)
                template <typename I1, typename S1, typename I2, typename S2, typename O, typename F,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 binary_transform_result<I1, I2, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
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
                COMPAT_CONSTEXPR_14 binary_transform_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>, O>
                operator()(R1&& r1, R2&& r2, O result, F op, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(op), std::move(proj1), std::move(proj2));
                }
            };

            struct generate_fn {
                template <typename I, typename S, typename F,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, F gen) const {
                    for (; first != last; ++first) {
                        *first = gen();
                    }
                    return first;
                }

                template <typename R, typename F,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, F gen) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(gen));
                }
            };

            struct generate_n_fn {
                template <typename I, typename Size, typename F>
                COMPAT_CONSTEXPR_14 I operator()(I first, Size n, F gen) const {
                    for (Size i = 0; i < n; ++i, ++first) {
                        *first = gen();
                    }
                    return first;
                }
            };

            struct remove_if_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct remove_fn {
                template <typename I, typename S, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, const T& value, Proj proj = {}) const {
                    return remove_if_fn{}(first, last, [&value](const T& x) { return x == value; }, std::move(proj));
                }

                template <typename R, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(proj));
                }
            };

            struct replace_if_fn {
                template <typename I, typename S, typename Pred, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Pred pred, const T& new_value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            *first = new_value;
                        }
                    }
                    return first;
                }

                template <typename R, typename Pred, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Pred pred, const T& new_value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), new_value, std::move(proj));
                }
            };

            struct replace_fn {
                template <typename I, typename S, typename T1, typename T2, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, const T1& old_value, const T2& new_value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(proj, *first) == old_value) {
                            *first = new_value;
                        }
                    }
                    return first;
                }

                template <typename R, typename T1, typename T2, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, const T1& old_value, const T2& new_value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), old_value, new_value, std::move(proj));
                }
            };

            struct swap_ranges_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 swap_ranges_result<I1, I2> operator()(I1 first1, S1 last1, I2 first2, S2 last2) const {
                    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
                        using std::swap;
                        swap(*first1, *first2);
                    }
                    return {first1, first2};
                }

                template <typename R1, typename R2,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 swap_ranges_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>>
                operator()(R1&& r1, R2&& r2) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2));
                }
            };

            struct reverse_fn {
                template <typename I, typename S,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r));
                }
            };

            struct reverse_copy_fn {
                template <typename I, typename S, typename O,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 reverse_copy_result<I, O> operator()(I first, S last, O result) const {
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
                COMPAT_CONSTEXPR_14 reverse_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct rotate_fn {
                template <typename I, typename S,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, I middle, S last) const {
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
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, iterator_t<R> middle) const {
                    return (*this)(range_detail::begin_fn{}(r), std::move(middle), range_detail::end_fn{}(r));
                }
            };

            struct unique_fn {
                template <typename I, typename S, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, Pred pred = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, Pred pred = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct replace_copy_fn {
                template <typename I, typename S, typename O, typename T1, typename T2, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 replace_copy_result<I, O> operator()(I first, S last, O result, const T1& old_val, const T2& new_val, Proj proj = {}) const {
                    for (; first != last; ++first, ++result) {
                        if (compat::detail::invoke(proj, *first) == old_val) {
                            *result = new_val;
                        } else {
                            *result = *first;
                        }
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename T1, typename T2, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 replace_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, const T1& old_val, const T2& new_val, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), old_val, new_val, std::move(proj));
                }
            };

            struct replace_copy_if_fn {
                template <typename I, typename S, typename O, typename Pred, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 replace_copy_result<I, O> operator()(I first, S last, O result, Pred pred, const T& new_val, Proj proj = {}) const {
                    for (; first != last; ++first, ++result) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            *result = new_val;
                        } else {
                            *result = *first;
                        }
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename Pred, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 replace_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, Pred pred, const T& new_val, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(pred), new_val, std::move(proj));
                }
            };

            struct remove_copy_fn {
                template <typename I, typename S, typename O, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 remove_copy_result<I, O> operator()(I first, S last, O result, const T& value, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!(compat::detail::invoke(proj, *first) == value)) {
                            *result = *first;
                            ++result;
                        }
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename T, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 remove_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, const T& value, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), value, std::move(proj));
                }
            };

            struct remove_copy_if_fn {
                template <typename I, typename S, typename O, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 remove_copy_result<I, O> operator()(I first, S last, O result, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            *result = *first;
                            ++result;
                        }
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 remove_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(pred), std::move(proj));
                }
            };

            struct unique_copy_fn {
                template <typename I, typename S, typename O, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 unique_copy_result<I, O> operator()(I first, S last, O result, Pred pred = {}, Proj proj = {}) const {
                    if (first == last) return {first, result};
                    *result = *first;
                    I prev = first;
                    ++first;
                    ++result;
                    for (; first != last; ++first) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj, *prev),
                                                         compat::detail::invoke(proj, *first))) {
                            *result = *first;
                            prev = first;
                            ++result;
                        }
                    }
                    return {first, result};
                }

                template <typename R, typename O, typename Pred = compat::detail::equal_to_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 unique_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, O result, Pred pred = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(result), std::move(pred), std::move(proj));
                }
            };

            struct rotate_copy_fn {
                template <typename I, typename S, typename O,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 rotate_copy_result<I, O> operator()(I first, I middle, S last, O result) const {
                    auto r1 = copy_fn{}(middle, last, std::move(result));
                    auto r2 = copy_fn{}(first, middle, std::move(r1.out));
                    return {r1.in, r2.out};
                }

                template <typename R, typename O,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 rotate_copy_result<borrowed_iterator_t<R>, O> operator()(R&& r, iterator_t<R> middle, O result) const {
                    return (*this)(range_detail::begin_fn{}(r), std::move(middle), range_detail::end_fn{}(r), std::move(result));
                }
            };

            struct shift_left_fn {
                template <typename I, typename S,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                subrange<I> operator()(I first, S last, iter_difference_t<I> n) const {
                    if (n <= 0) return {first, first};
                    I mid = first;
                    for (iter_difference_t<I> i = 0; i < n; ++i) {
                        if (mid == last) return {first, first};
                        ++mid;
                    }
                    I dest = first;
                    for (; mid != last; ++dest, ++mid) {
                        *dest = std::move(*mid);
                    }
                    return {first, dest};
                }

                template <typename R,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_subrange_t<R> operator()(R&& r, range_difference_t<R> n) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), n);
                }
            };

            struct shift_right_fn {
                template <typename I, typename S,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                subrange<I> operator()(I first, S last, iter_difference_t<I> n) const {
                    if (n <= 0) return {first, first};
                    auto len = std::distance(first, last);
                    if (n >= len) return {first, first};
                    I end_it = first;
                    std::advance(end_it, len);
                    I src = end_it;
                    std::advance(src, -n);
                    I dest = end_it;
                    while (src != first) {
                        --dest;
                        --src;
                        *dest = std::move(*src);
                    }
                    I new_first = first;
                    std::advance(new_first, n);
                    return {new_first, end_it};
                }

                template <typename R,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_subrange_t<R> operator()(R&& r, range_difference_t<R> n) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), n);
                }
            };

            struct shuffle_fn {
                template <typename I, typename S, typename Gen,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Gen&& g) const {
                    auto n = std::distance(first, last);
                    if (n <= 1) return first + n;
                    for (auto i = n - 1; i > 0; --i) {
                        std::uniform_int_distribution<decltype(n)> dist(0, i);
                        auto j = dist(g);
                        using std::swap;
                        swap(first[i], first[j]);
                    }
                    return first + n;
                }

                template <typename R, typename Gen,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Gen&& g) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::forward<Gen>(g));
                }
            };

            struct sample_fn {
                template <typename I, typename S, typename O, typename Gen,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                O operator()(I first, S last, O out, iter_difference_t<I> n, Gen&& g) const {
                    if (n <= 0) return out;
                    iter_difference_t<I> k = 0;
                    for (; first != last && k < n; ++first, ++k) {
                        out[k] = *first;
                    }
                    iter_difference_t<I> count = k;
                    for (; first != last; ++first, ++count) {
                        std::uniform_int_distribution<iter_difference_t<I>> dist(0, count);
                        auto idx = dist(g);
                        if (idx < n) {
                            out[idx] = *first;
                        }
                    }
                    return out + k;
                }

                template <typename R, typename O, typename Gen,
                          typename = typename std::enable_if<range<R>::value>::type>
                O operator()(R&& r, O out, range_difference_t<R> n, Gen&& g) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(out), n, std::forward<Gen>(g));
                }
            };

            // --- Partitioning Operations ---

            struct is_partitioned_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, Pred pred, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct partition_point_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Pred pred, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct partition_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            // --- Sorting & Binary Search Operations ---

            struct is_sorted_until_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct is_sorted_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    return is_sorted_until_fn{}(first, last, std::move(comp), std::move(proj)) == last;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct sort_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::sort(first, last, [&comp, &proj](const val_t& a, const val_t& b) {
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
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::stable_sort(first, last, [&comp, &proj](const val_t& a, const val_t& b) {
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
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            struct upper_bound_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            struct equal_range_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 subrange<I> operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return {lower_bound_fn{}(first, last, value, comp, proj),
                            upper_bound_fn{}(first, last, value, comp, proj)};
                }

                template <typename R, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_subrange_t<R> operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            struct binary_search_fn {
                template <typename I, typename S, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) const {
                    I it = lower_bound_fn{}(first, last, value, comp, proj);
                    return it != last && !compat::detail::invoke(comp, value, compat::detail::invoke(proj, *it));
                }

                template <typename R, typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, const T& value, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value, std::move(comp), std::move(proj));
                }
            };

            // --- Min/Max Operations ---

            struct min_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct max_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct minmax_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 minmax_element_result<I> operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
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
                COMPAT_CONSTEXPR_14 minmax_element_result<borrowed_iterator_t<R>> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct min_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                COMPAT_CONSTEXPR_14 const T& operator()(const T& a, const T& b, Comp comp = {}, Proj proj = {}) const {
                    return compat::detail::invoke(comp, compat::detail::invoke(proj, b),
                                                        compat::detail::invoke(proj, a)) ? b : a;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 range_value_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    auto it = min_element_fn{}(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                    return *it;
                }
            };

            struct max_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                COMPAT_CONSTEXPR_14 const T& operator()(const T& a, const T& b, Comp comp = {}, Proj proj = {}) const {
                    return compat::detail::invoke(comp, compat::detail::invoke(proj, a),
                                                        compat::detail::invoke(proj, b)) ? b : a;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 range_value_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    auto it = max_element_fn{}(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                    return *it;
                }
            };

            struct minmax_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                COMPAT_CONSTEXPR_14 minmax_result<const T&> operator()(const T& a, const T& b, Comp comp = {}, Proj proj = {}) const {
                    if (compat::detail::invoke(comp, compat::detail::invoke(proj, b),
                                                    compat::detail::invoke(proj, a))) {
                        return {b, a};
                    }
                    return {a, b};
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 minmax_result<range_value_t<R>> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    auto res = minmax_element_fn{}(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                    return {*res.min, *res.max};
                }
            };

            struct clamp_fn {
                template <typename T, typename Comp = compat::detail::less_fn, typename Proj = compat::identity>
                COMPAT_CONSTEXPR_14 const T& operator()(const T& val, const T& lo, const T& hi, Comp comp = {}, Proj proj = {}) const {
                    return compat::detail::invoke(comp, compat::detail::invoke(proj, val),
                                                        compat::detail::invoke(proj, lo)) ? lo
                         : compat::detail::invoke(comp, compat::detail::invoke(proj, hi),
                                                        compat::detail::invoke(proj, val)) ? hi : val;
                }
            };

            // --- Additional Partitioning & Sorting Operations ---

            struct partition_copy_fn {
                template <typename I, typename S, typename O1, typename O2, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 partition_copy_result<I, O1, O2> operator()(I first, S last, O1 out_true, O2 out_false, Pred pred, Proj proj = {}) const {
                    for (; first != last; ++first) {
                        if (compat::detail::invoke(pred, compat::detail::invoke(proj, *first))) {
                            *out_true = *first;
                            ++out_true;
                        } else {
                            *out_false = *first;
                            ++out_false;
                        }
                    }
                    return {first, out_true, out_false};
                }

                template <typename R, typename O1, typename O2, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 partition_copy_result<borrowed_iterator_t<R>, O1, O2> operator()(R&& r, O1 out_true, O2 out_false, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(out_true), std::move(out_false), std::move(pred), std::move(proj));
                }
            };

            struct stable_partition_fn {
                template <typename I, typename S, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                subrange<I> operator()(I first, S last, Pred pred, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    auto it = std::stable_partition(first, end_it, [&pred, &proj](const val_t& x) {
                        return compat::detail::invoke(pred, compat::detail::invoke(proj, x));
                    });
                    return {it, end_it};
                }

                template <typename R, typename Pred, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_subrange_t<R> operator()(R&& r, Pred pred, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(pred), std::move(proj));
                }
            };

            struct partial_sort_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::partial_sort(first, middle, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, iterator_t<R> middle, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), std::move(middle), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct nth_element_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, I nth, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::nth_element(first, nth, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, iterator_t<R> nth, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), std::move(nth), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            // --- Heap Operations ---

            struct is_heap_until_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    auto n = std::distance(first, last);
                    for (decltype(n) child = 1; child < n; ++child) {
                        auto parent = (child - 1) / 2;
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj, first[parent]),
                                                         compat::detail::invoke(proj, first[child]))) {
                            return first + child;
                        }
                    }
                    return first + n;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct is_heap_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    return is_heap_until_fn{}(first, last, std::move(comp), std::move(proj)) == last;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct push_heap_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::push_heap(first, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct pop_heap_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::pop_heap(first, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct make_heap_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::make_heap(first, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct sort_heap_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::sort_heap(first, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct partial_sort_copy_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                partial_sort_copy_result<I1, I2> operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                            Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    if (first1 == last1 || first2 == last2) return {first1, first2};
                    I2 out = first2;
                    for (; first1 != last1 && out != last2; ++first1, ++out) {
                        *out = *first1;
                    }
                    make_heap_fn{}(first2, out, comp, proj2);
                    for (; first1 != last1; ++first1) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1), compat::detail::invoke(proj2, *first2))) {
                            *first2 = *first1;
                            pop_heap_fn{}(first2, out, comp, proj2);
                            push_heap_fn{}(first2, out, comp, proj2);
                        }
                    }
                    sort_heap_fn{}(first2, out, comp, proj2);
                    return {first1, out};
                }

                template <typename R1, typename R2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                partial_sort_copy_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>>
                operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            // --- Set Operations ---

            struct includes_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                    Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; first2 != last2; ++first1) {
                        if (first1 == last1 || compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                                            compat::detail::invoke(proj1, *first1))) {
                            return false;
                        }
                        if (!compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1),
                                                          compat::detail::invoke(proj2, *first2))) {
                            ++first2;
                        }
                    }
                    return true;
                }

                template <typename R1, typename R2,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct set_union_fn {
                template <typename I1, typename S1, typename I2, typename S2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 set_union_result<I1, I2, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2, O result,
                                                                          Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    while (first1 != last1 && first2 != last2) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            *result = *first1;
                            ++first1;
                        } else if (compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                                compat::detail::invoke(proj1, *first1))) {
                            *result = *first2;
                            ++first2;
                        } else {
                            *result = *first1;
                            ++first1;
                            ++first2;
                        }
                        ++result;
                    }
                    auto r1 = copy_fn{}(first1, last1, std::move(result));
                    auto r2 = copy_fn{}(first2, last2, std::move(r1.out));
                    return {r1.in, r2.in, r2.out};
                }

                template <typename R1, typename R2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 set_union_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>, O>
                operator()(R1&& r1, R2&& r2, O result, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct set_intersection_fn {
                template <typename I1, typename S1, typename I2, typename S2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 set_intersection_result<I1, I2, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2, O result,
                                                                                  Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    while (first1 != last1 && first2 != last2) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            ++first1;
                        } else if (compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                                compat::detail::invoke(proj1, *first1))) {
                            ++first2;
                        } else {
                            *result = *first1;
                            ++result;
                            ++first1;
                            ++first2;
                        }
                    }
                    while (first1 != last1) ++first1;
                    while (first2 != last2) ++first2;
                    return {first1, first2, result};
                }

                template <typename R1, typename R2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 set_intersection_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>, O>
                operator()(R1&& r1, R2&& r2, O result, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct set_difference_fn {
                template <typename I1, typename S1, typename I2, typename S2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 set_difference_result<I1, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2, O result,
                                                                            Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    while (first1 != last1 && first2 != last2) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            *result = *first1;
                            ++result;
                            ++first1;
                        } else if (compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                                compat::detail::invoke(proj1, *first1))) {
                            ++first2;
                        } else {
                            ++first1;
                            ++first2;
                        }
                    }
                    auto r = copy_fn{}(first1, last1, std::move(result));
                    return {r.in, r.out};
                }

                template <typename R1, typename R2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 set_difference_result<borrowed_iterator_t<R1>, O>
                operator()(R1&& r1, R2&& r2, O result, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct set_symmetric_difference_fn {
                template <typename I1, typename S1, typename I2, typename S2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 set_symmetric_difference_result<I1, I2, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2, O result,
                                                                                          Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    while (first1 != last1 && first2 != last2) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            *result = *first1;
                            ++result;
                            ++first1;
                        } else if (compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                                compat::detail::invoke(proj1, *first1))) {
                            *result = *first2;
                            ++result;
                            ++first2;
                        } else {
                            ++first1;
                            ++first2;
                        }
                    }
                    auto r1 = copy_fn{}(first1, last1, std::move(result));
                    auto r2 = copy_fn{}(first2, last2, std::move(r1.out));
                    return {r1.in, r2.in, r2.out};
                }

                template <typename R1, typename R2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 set_symmetric_difference_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>, O>
                operator()(R1&& r1, R2&& r2, O result, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            // --- Merge Operations ---

            struct merge_fn {
                template <typename I1, typename S1, typename I2, typename S2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 merge_result<I1, I2, O> operator()(I1 first1, S1 last1, I2 first2, S2 last2, O result,
                                                                       Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    while (first1 != last1 && first2 != last2) {
                        if (compat::detail::invoke(comp, compat::detail::invoke(proj2, *first2),
                                                         compat::detail::invoke(proj1, *first1))) {
                            *result = *first2;
                            ++first2;
                        } else {
                            *result = *first1;
                            ++first1;
                        }
                        ++result;
                    }
                    auto r1 = copy_fn{}(first1, last1, std::move(result));
                    auto r2 = copy_fn{}(first2, last2, std::move(r1.out));
                    return {r1.in, r2.in, r2.out};
                }

                template <typename R1, typename R2, typename O,
                          typename Comp = compat::detail::less_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 merge_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>, O>
                operator()(R1&& r1, R2&& r2, O result, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(result), std::move(comp), std::move(proj1), std::move(proj2));
                }
            };

            struct inplace_merge_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    std::inplace_merge(first, middle, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return end_it;
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, iterator_t<R> middle, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), std::move(middle), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            // --- Permutation Operations ---

            struct is_permutation_fn {
                template <typename I1, typename S1, typename I2, typename S2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I1, S1>::value && is_iterator_sentinel_pair<I2, S2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(I1 first1, S1 last1, I2 first2, S2 last2,
                                                    Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
                        if (!compat::detail::invoke(pred, compat::detail::invoke(proj1, *first1),
                                                         compat::detail::invoke(proj2, *first2))) {
                            break;
                        }
                    }
                    if (first1 == last1 && first2 == last2) return true;
                    auto d1 = std::distance(first1, last1);
                    auto d2 = std::distance(first2, last2);
                    if (d1 != d2) return false;
                    typedef typename std::iterator_traits<I1>::value_type val1_t;
                    typedef typename std::iterator_traits<I2>::value_type val2_t;
                    for (I1 i = first1; i != last1; ++i) {
                        bool already_seen = false;
                        for (I1 k = first1; k != i; ++k) {
                            if (compat::detail::invoke(pred, compat::detail::invoke(proj1, *k),
                                                             compat::detail::invoke(proj1, *i))) {
                                already_seen = true;
                                break;
                            }
                        }
                        if (already_seen) continue;
                        auto count1 = count_if_fn{}(first1, last1, [&pred, &proj1, &i](const val1_t& x) {
                            return compat::detail::invoke(pred, compat::detail::invoke(proj1, x), compat::detail::invoke(proj1, *i));
                        });
                        auto count2 = count_if_fn{}(first2, last2, [&pred, &proj2, &proj1, &i](const val2_t& x) {
                            return compat::detail::invoke(pred, compat::detail::invoke(proj2, x), compat::detail::invoke(proj1, *i));
                        });
                        if (count1 != count2) return false;
                    }
                    return true;
                }

                template <typename R1, typename R2,
                          typename Pred = compat::detail::equal_to_fn,
                          typename Proj1 = compat::identity, typename Proj2 = compat::identity,
                          typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
                COMPAT_CONSTEXPR_14 bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
                    return (*this)(range_detail::begin_fn{}(r1), range_detail::end_fn{}(r1),
                                   range_detail::begin_fn{}(r2), range_detail::end_fn{}(r2),
                                   std::move(pred), std::move(proj1), std::move(proj2));
                }
            };

            struct next_permutation_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                next_permutation_result<I> operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    bool res = std::next_permutation(first, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return {end_it, res};
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                next_permutation_result<borrowed_iterator_t<R>> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            struct prev_permutation_fn {
                template <typename I, typename S, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                prev_permutation_result<I> operator()(I first, S last, Comp comp = {}, Proj proj = {}) const {
                    I end_it = first;
                    std::advance(end_it, std::distance(first, last));
                    typedef typename std::iterator_traits<I>::value_type val_t;
                    bool res = std::prev_permutation(first, end_it, [&comp, &proj](const val_t& a, const val_t& b) {
                        return compat::detail::invoke(comp, compat::detail::invoke(proj, a), compat::detail::invoke(proj, b));
                    });
                    return {end_it, res};
                }

                template <typename R, typename Comp = compat::detail::less_fn, typename Proj = compat::identity,
                          typename = typename std::enable_if<range<R>::value>::type>
                prev_permutation_result<borrowed_iterator_t<R>> operator()(R&& r, Comp comp = {}, Proj proj = {}) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(comp), std::move(proj));
                }
            };

            // --- Numeric & Random Operations ---

            struct iota_fn {
                template <typename O, typename S, typename T,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<O, S>::value>::type>
                COMPAT_CONSTEXPR_14 out_value_result<O, T> operator()(O first, S last, T value) const {
                    for (; first != last; ++first, ++value) {
                        *first = value;
                    }
                    return {first, value};
                }

                template <typename R, typename T,
                          typename = typename std::enable_if<range<R>::value>::type>
                COMPAT_CONSTEXPR_14 out_value_result<borrowed_iterator_t<R>, T> operator()(R&& r, T value) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::move(value));
                }
            };

            struct generate_random_fn {
                template <typename I, typename S, typename Gen,
                          typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
                I operator()(I first, S last, Gen&& g) const {
                    for (; first != last; ++first) {
                        *first = g();
                    }
                    return first;
                }

                template <typename R, typename Gen,
                          typename = typename std::enable_if<range<R>::value>::type>
                borrowed_iterator_t<R> operator()(R&& r, Gen&& g) const {
                    return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), std::forward<Gen>(g));
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
        inline constexpr detail::contains_subrange_fn contains_subrange{};
        inline constexpr detail::find_last_fn find_last{};
        inline constexpr detail::find_last_if_fn find_last_if{};
        inline constexpr detail::find_last_if_not_fn find_last_if_not{};
        inline constexpr detail::find_end_fn find_end{};
        inline constexpr detail::find_first_of_fn find_first_of{};
        inline constexpr detail::search_n_fn search_n{};

        inline constexpr detail::fold_left_fn fold_left{};
        inline constexpr detail::fold_left_first_fn fold_left_first{};
        inline constexpr detail::fold_right_fn fold_right{};
        inline constexpr detail::fold_right_last_fn fold_right_last{};
        inline constexpr detail::fold_left_with_iter_fn fold_left_with_iter{};
        inline constexpr detail::fold_left_first_with_iter_fn fold_left_first_with_iter{};

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
        inline constexpr detail::replace_copy_fn replace_copy{};
        inline constexpr detail::replace_copy_if_fn replace_copy_if{};
        inline constexpr detail::remove_copy_fn remove_copy{};
        inline constexpr detail::remove_copy_if_fn remove_copy_if{};
        inline constexpr detail::unique_copy_fn unique_copy{};
        inline constexpr detail::rotate_copy_fn rotate_copy{};
        inline constexpr detail::shift_left_fn shift_left{};
        inline constexpr detail::shift_right_fn shift_right{};
        inline constexpr detail::shuffle_fn shuffle{};
        inline constexpr detail::sample_fn sample{};

        inline constexpr detail::is_partitioned_fn is_partitioned{};
        inline constexpr detail::partition_fn partition{};
        inline constexpr detail::partition_point_fn partition_point{};
        inline constexpr detail::partition_copy_fn partition_copy{};
        inline constexpr detail::stable_partition_fn stable_partition{};

        inline constexpr detail::is_sorted_fn is_sorted{};
        inline constexpr detail::is_sorted_until_fn is_sorted_until{};
        inline constexpr detail::sort_fn sort{};
        inline constexpr detail::stable_sort_fn stable_sort{};
        inline constexpr detail::partial_sort_fn partial_sort{};
        inline constexpr detail::partial_sort_copy_fn partial_sort_copy{};
        inline constexpr detail::nth_element_fn nth_element{};
        inline constexpr detail::lower_bound_fn lower_bound{};
        inline constexpr detail::upper_bound_fn upper_bound{};
        inline constexpr detail::equal_range_fn equal_range{};
        inline constexpr detail::binary_search_fn binary_search{};

        inline constexpr detail::is_heap_fn is_heap{};
        inline constexpr detail::is_heap_until_fn is_heap_until{};
        inline constexpr detail::push_heap_fn push_heap{};
        inline constexpr detail::pop_heap_fn pop_heap{};
        inline constexpr detail::make_heap_fn make_heap{};
        inline constexpr detail::sort_heap_fn sort_heap{};

        inline constexpr detail::includes_fn includes{};
        inline constexpr detail::set_union_fn set_union{};
        inline constexpr detail::set_intersection_fn set_intersection{};
        inline constexpr detail::set_difference_fn set_difference{};
        inline constexpr detail::set_symmetric_difference_fn set_symmetric_difference{};

        inline constexpr detail::merge_fn merge{};
        inline constexpr detail::inplace_merge_fn inplace_merge{};

        inline constexpr detail::is_permutation_fn is_permutation{};
        inline constexpr detail::next_permutation_fn next_permutation{};
        inline constexpr detail::prev_permutation_fn prev_permutation{};

        inline constexpr detail::iota_fn iota{};
        inline constexpr detail::generate_random_fn generate_random{};

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
            constexpr const detail::contains_subrange_fn& contains_subrange = compat::detail::self_ranges::static_const<detail::contains_subrange_fn>::value;
            constexpr const detail::find_last_fn& find_last = compat::detail::self_ranges::static_const<detail::find_last_fn>::value;
            constexpr const detail::find_last_if_fn& find_last_if = compat::detail::self_ranges::static_const<detail::find_last_if_fn>::value;
            constexpr const detail::find_last_if_not_fn& find_last_if_not = compat::detail::self_ranges::static_const<detail::find_last_if_not_fn>::value;
            constexpr const detail::find_end_fn& find_end = compat::detail::self_ranges::static_const<detail::find_end_fn>::value;
            constexpr const detail::find_first_of_fn& find_first_of = compat::detail::self_ranges::static_const<detail::find_first_of_fn>::value;
            constexpr const detail::search_n_fn& search_n = compat::detail::self_ranges::static_const<detail::search_n_fn>::value;

            constexpr const detail::fold_left_fn& fold_left = compat::detail::self_ranges::static_const<detail::fold_left_fn>::value;
            constexpr const detail::fold_left_first_fn& fold_left_first = compat::detail::self_ranges::static_const<detail::fold_left_first_fn>::value;
            constexpr const detail::fold_right_fn& fold_right = compat::detail::self_ranges::static_const<detail::fold_right_fn>::value;
            constexpr const detail::fold_right_last_fn& fold_right_last = compat::detail::self_ranges::static_const<detail::fold_right_last_fn>::value;
            constexpr const detail::fold_left_with_iter_fn& fold_left_with_iter = compat::detail::self_ranges::static_const<detail::fold_left_with_iter_fn>::value;
            constexpr const detail::fold_left_first_with_iter_fn& fold_left_first_with_iter = compat::detail::self_ranges::static_const<detail::fold_left_first_with_iter_fn>::value;

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
            constexpr const detail::replace_copy_fn& replace_copy = compat::detail::self_ranges::static_const<detail::replace_copy_fn>::value;
            constexpr const detail::replace_copy_if_fn& replace_copy_if = compat::detail::self_ranges::static_const<detail::replace_copy_if_fn>::value;
            constexpr const detail::remove_copy_fn& remove_copy = compat::detail::self_ranges::static_const<detail::remove_copy_fn>::value;
            constexpr const detail::remove_copy_if_fn& remove_copy_if = compat::detail::self_ranges::static_const<detail::remove_copy_if_fn>::value;
            constexpr const detail::unique_copy_fn& unique_copy = compat::detail::self_ranges::static_const<detail::unique_copy_fn>::value;
            constexpr const detail::rotate_copy_fn& rotate_copy = compat::detail::self_ranges::static_const<detail::rotate_copy_fn>::value;
            constexpr const detail::shift_left_fn& shift_left = compat::detail::self_ranges::static_const<detail::shift_left_fn>::value;
            constexpr const detail::shift_right_fn& shift_right = compat::detail::self_ranges::static_const<detail::shift_right_fn>::value;
            constexpr const detail::shuffle_fn& shuffle = compat::detail::self_ranges::static_const<detail::shuffle_fn>::value;
            constexpr const detail::sample_fn& sample = compat::detail::self_ranges::static_const<detail::sample_fn>::value;

            constexpr const detail::is_partitioned_fn& is_partitioned = compat::detail::self_ranges::static_const<detail::is_partitioned_fn>::value;
            constexpr const detail::partition_fn& partition = compat::detail::self_ranges::static_const<detail::partition_fn>::value;
            constexpr const detail::partition_point_fn& partition_point = compat::detail::self_ranges::static_const<detail::partition_point_fn>::value;
            constexpr const detail::partition_copy_fn& partition_copy = compat::detail::self_ranges::static_const<detail::partition_copy_fn>::value;
            constexpr const detail::stable_partition_fn& stable_partition = compat::detail::self_ranges::static_const<detail::stable_partition_fn>::value;

            constexpr const detail::is_sorted_fn& is_sorted = compat::detail::self_ranges::static_const<detail::is_sorted_fn>::value;
            constexpr const detail::is_sorted_until_fn& is_sorted_until = compat::detail::self_ranges::static_const<detail::is_sorted_until_fn>::value;
            constexpr const detail::sort_fn& sort = compat::detail::self_ranges::static_const<detail::sort_fn>::value;
            constexpr const detail::stable_sort_fn& stable_sort = compat::detail::self_ranges::static_const<detail::stable_sort_fn>::value;
            constexpr const detail::partial_sort_fn& partial_sort = compat::detail::self_ranges::static_const<detail::partial_sort_fn>::value;
            constexpr const detail::partial_sort_copy_fn& partial_sort_copy = compat::detail::self_ranges::static_const<detail::partial_sort_copy_fn>::value;
            constexpr const detail::nth_element_fn& nth_element = compat::detail::self_ranges::static_const<detail::nth_element_fn>::value;
            constexpr const detail::lower_bound_fn& lower_bound = compat::detail::self_ranges::static_const<detail::lower_bound_fn>::value;
            constexpr const detail::upper_bound_fn& upper_bound = compat::detail::self_ranges::static_const<detail::upper_bound_fn>::value;
            constexpr const detail::equal_range_fn& equal_range = compat::detail::self_ranges::static_const<detail::equal_range_fn>::value;
            constexpr const detail::binary_search_fn& binary_search = compat::detail::self_ranges::static_const<detail::binary_search_fn>::value;

            constexpr const detail::is_heap_fn& is_heap = compat::detail::self_ranges::static_const<detail::is_heap_fn>::value;
            constexpr const detail::is_heap_until_fn& is_heap_until = compat::detail::self_ranges::static_const<detail::is_heap_until_fn>::value;
            constexpr const detail::push_heap_fn& push_heap = compat::detail::self_ranges::static_const<detail::push_heap_fn>::value;
            constexpr const detail::pop_heap_fn& pop_heap = compat::detail::self_ranges::static_const<detail::pop_heap_fn>::value;
            constexpr const detail::make_heap_fn& make_heap = compat::detail::self_ranges::static_const<detail::make_heap_fn>::value;
            constexpr const detail::sort_heap_fn& sort_heap = compat::detail::self_ranges::static_const<detail::sort_heap_fn>::value;

            constexpr const detail::includes_fn& includes = compat::detail::self_ranges::static_const<detail::includes_fn>::value;
            constexpr const detail::set_union_fn& set_union = compat::detail::self_ranges::static_const<detail::set_union_fn>::value;
            constexpr const detail::set_intersection_fn& set_intersection = compat::detail::self_ranges::static_const<detail::set_intersection_fn>::value;
            constexpr const detail::set_difference_fn& set_difference = compat::detail::self_ranges::static_const<detail::set_difference_fn>::value;
            constexpr const detail::set_symmetric_difference_fn& set_symmetric_difference = compat::detail::self_ranges::static_const<detail::set_symmetric_difference_fn>::value;

            constexpr const detail::merge_fn& merge = compat::detail::self_ranges::static_const<detail::merge_fn>::value;
            constexpr const detail::inplace_merge_fn& inplace_merge = compat::detail::self_ranges::static_const<detail::inplace_merge_fn>::value;

            constexpr const detail::is_permutation_fn& is_permutation = compat::detail::self_ranges::static_const<detail::is_permutation_fn>::value;
            constexpr const detail::next_permutation_fn& next_permutation = compat::detail::self_ranges::static_const<detail::next_permutation_fn>::value;
            constexpr const detail::prev_permutation_fn& prev_permutation = compat::detail::self_ranges::static_const<detail::prev_permutation_fn>::value;

            constexpr const detail::iota_fn& iota = compat::detail::self_ranges::static_const<detail::iota_fn>::value;
            constexpr const detail::generate_random_fn& generate_random = compat::detail::self_ranges::static_const<detail::generate_random_fn>::value;

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
