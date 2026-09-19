#pragma once

#include "../Config.hpp"
#include "SelfRanges.hpp"
#include "SelfAlgorithm.hpp"

#include <memory>
#include <utility>
#include <type_traits>
#include <new>

namespace compat {
namespace detail {
namespace self_memory {
namespace ranges {

    using compat::detail::self_algo::ranges::in_out_result;

    template <typename I, typename O>
    using uninitialized_copy_result = in_out_result<I, O>;

    template <typename I, typename O>
    using uninitialized_copy_n_result = in_out_result<I, O>;

    template <typename I, typename O>
    using uninitialized_move_result = in_out_result<I, O>;

    template <typename I, typename O>
    using uninitialized_move_n_result = in_out_result<I, O>;

    namespace detail {

        using namespace compat::detail::self_ranges::ranges;
        namespace range_detail = compat::detail::self_ranges::ranges::detail;
        using compat::detail::self_algo::ranges::detail::is_iterator_sentinel_pair;

        template <typename T>
        constexpr void destroy_at_impl(T* p) noexcept {
            p->~T();
        }

        // --- Exception Rollback Guard ---
        template <typename O>
        struct rollback_guard {
            O first;
            O current;
            bool active;

            constexpr rollback_guard(O f) : first(f), current(f), active(true) {}
            ~rollback_guard() {
                if (active) {
                    for (; first != current; ++first) {
                        destroy_at_impl(std::addressof(*first));
                    }
                }
            }
            void release() noexcept { active = false; }
        };

        // --- construct_at & destroy_at ---

        struct construct_at_fn {
            template <typename T, typename... Args>
            constexpr auto operator()(T* p, Args&&... args) const
                -> decltype(::new (static_cast<void*>(p)) T(std::forward<Args>(args)...)) {
                return ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
            }
        };

        struct destroy_at_fn {
            template <typename T>
            constexpr void operator()(T* p) const noexcept {
                destroy_at_impl(p);
            }
        };

        struct destroy_fn {
            template <typename I, typename S,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
            constexpr I operator()(I first, S last) const noexcept {
                for (; first != last; ++first) {
                    destroy_at_fn{}(std::addressof(*first));
                }
                return first;
            }

            template <typename R,
                      typename = typename std::enable_if<range<R>::value>::type>
            constexpr borrowed_iterator_t<R> operator()(R&& r) const noexcept {
                return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r));
            }
        };

        struct destroy_n_fn {
            template <typename I>
            constexpr I operator()(I first, std::size_t n) const noexcept {
                for (; n > 0; --n, ++first) {
                    destroy_at_fn{}(std::addressof(*first));
                }
                return first;
            }
        };

        // --- uninitialized algorithms ---

        struct uninitialized_copy_fn {
            template <typename I, typename S1, typename O, typename S2,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<I, S1>::value && is_iterator_sentinel_pair<O, S2>::value>::type>
            uninitialized_copy_result<I, O> operator()(I first, S1 last, O result, S2 result_last) const {
                rollback_guard<O> guard(result);
                for (; first != last && guard.current != result_last; ++first, ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current), *first);
                }
                guard.release();
                return {first, guard.current};
            }

            template <typename R1, typename R2,
                      typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
            uninitialized_copy_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>> operator()(R1&& in_range, R2&& out_range) const {
                return (*this)(range_detail::begin_fn{}(in_range), range_detail::end_fn{}(in_range),
                               range_detail::begin_fn{}(out_range), range_detail::end_fn{}(out_range));
            }
        };

        struct uninitialized_copy_n_fn {
            template <typename I, typename O, typename S2,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<O, S2>::value>::type>
            uninitialized_copy_n_result<I, O> operator()(I first, std::size_t n, O result, S2 result_last) const {
                rollback_guard<O> guard(result);
                for (; n > 0 && guard.current != result_last; --n, ++first, ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current), *first);
                }
                guard.release();
                return {first, guard.current};
            }
        };

        struct uninitialized_fill_fn {
            template <typename I, typename S, typename T,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
            I operator()(I first, S last, const T& value) const {
                rollback_guard<I> guard(first);
                for (; guard.current != last; ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current), value);
                }
                guard.release();
                return guard.current;
            }

            template <typename R, typename T,
                      typename = typename std::enable_if<range<R>::value>::type>
            borrowed_iterator_t<R> operator()(R&& r, const T& value) const {
                return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r), value);
            }
        };

        struct uninitialized_fill_n_fn {
            template <typename I, typename T>
            I operator()(I first, std::size_t n, const T& value) const {
                rollback_guard<I> guard(first);
                for (; n > 0; --n, ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current), value);
                }
                guard.release();
                return guard.current;
            }
        };

        struct uninitialized_move_fn {
            template <typename I, typename S1, typename O, typename S2,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<I, S1>::value && is_iterator_sentinel_pair<O, S2>::value>::type>
            uninitialized_move_result<I, O> operator()(I first, S1 last, O result, S2 result_last) const {
                rollback_guard<O> guard(result);
                for (; first != last && guard.current != result_last; ++first, ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current), std::move(*first));
                }
                guard.release();
                return {first, guard.current};
            }

            template <typename R1, typename R2,
                      typename = typename std::enable_if<range<R1>::value && range<R2>::value>::type>
            uninitialized_move_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>> operator()(R1&& in_range, R2&& out_range) const {
                return (*this)(range_detail::begin_fn{}(in_range), range_detail::end_fn{}(in_range),
                               range_detail::begin_fn{}(out_range), range_detail::end_fn{}(out_range));
            }
        };

        struct uninitialized_move_n_fn {
            template <typename I, typename O, typename S2,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<O, S2>::value>::type>
            uninitialized_move_n_result<I, O> operator()(I first, std::size_t n, O result, S2 result_last) const {
                rollback_guard<O> guard(result);
                for (; n > 0 && guard.current != result_last; --n, ++first, ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current), std::move(*first));
                }
                guard.release();
                return {first, guard.current};
            }
        };

        struct uninitialized_default_construct_fn {
            template <typename I, typename S,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
            I operator()(I first, S last) const {
                rollback_guard<I> guard(first);
                for (; guard.current != last; ++guard.current) {
                    ::new (static_cast<void*>(std::addressof(*guard.current)))
                        typename std::remove_reference<decltype(*guard.current)>::type;
                }
                guard.release();
                return guard.current;
            }

            template <typename R,
                      typename = typename std::enable_if<range<R>::value>::type>
            borrowed_iterator_t<R> operator()(R&& r) const {
                return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r));
            }
        };

        struct uninitialized_default_construct_n_fn {
            template <typename I>
            I operator()(I first, std::size_t n) const {
                rollback_guard<I> guard(first);
                for (; n > 0; --n, ++guard.current) {
                    ::new (static_cast<void*>(std::addressof(*guard.current)))
                        typename std::remove_reference<decltype(*guard.current)>::type;
                }
                guard.release();
                return guard.current;
            }
        };

        struct uninitialized_value_construct_fn {
            template <typename I, typename S,
                      typename = typename std::enable_if<is_iterator_sentinel_pair<I, S>::value>::type>
            I operator()(I first, S last) const {
                rollback_guard<I> guard(first);
                for (; guard.current != last; ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current));
                }
                guard.release();
                return guard.current;
            }

            template <typename R,
                      typename = typename std::enable_if<range<R>::value>::type>
            borrowed_iterator_t<R> operator()(R&& r) const {
                return (*this)(range_detail::begin_fn{}(r), range_detail::end_fn{}(r));
            }
        };

        struct uninitialized_value_construct_n_fn {
            template <typename I>
            I operator()(I first, std::size_t n) const {
                rollback_guard<I> guard(first);
                for (; n > 0; --n, ++guard.current) {
                    construct_at_fn{}(std::addressof(*guard.current));
                }
                guard.release();
                return guard.current;
            }
        };

    } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
    inline constexpr detail::construct_at_fn construct_at{};
    inline constexpr detail::destroy_at_fn destroy_at{};
    inline constexpr detail::destroy_fn destroy{};
    inline constexpr detail::destroy_n_fn destroy_n{};
    inline constexpr detail::uninitialized_copy_fn uninitialized_copy{};
    inline constexpr detail::uninitialized_copy_n_fn uninitialized_copy_n{};
    inline constexpr detail::uninitialized_fill_fn uninitialized_fill{};
    inline constexpr detail::uninitialized_fill_n_fn uninitialized_fill_n{};
    inline constexpr detail::uninitialized_move_fn uninitialized_move{};
    inline constexpr detail::uninitialized_move_n_fn uninitialized_move_n{};
    inline constexpr detail::uninitialized_default_construct_fn uninitialized_default_construct{};
    inline constexpr detail::uninitialized_default_construct_n_fn uninitialized_default_construct_n{};
    inline constexpr detail::uninitialized_value_construct_fn uninitialized_value_construct{};
    inline constexpr detail::uninitialized_value_construct_n_fn uninitialized_value_construct_n{};
#else
    namespace {
        constexpr const detail::construct_at_fn& construct_at = compat::detail::self_ranges::static_const<detail::construct_at_fn>::value;
        constexpr const detail::destroy_at_fn& destroy_at = compat::detail::self_ranges::static_const<detail::destroy_at_fn>::value;
        constexpr const detail::destroy_fn& destroy = compat::detail::self_ranges::static_const<detail::destroy_fn>::value;
        constexpr const detail::destroy_n_fn& destroy_n = compat::detail::self_ranges::static_const<detail::destroy_n_fn>::value;
        constexpr const detail::uninitialized_copy_fn& uninitialized_copy = compat::detail::self_ranges::static_const<detail::uninitialized_copy_fn>::value;
        constexpr const detail::uninitialized_copy_n_fn& uninitialized_copy_n = compat::detail::self_ranges::static_const<detail::uninitialized_copy_n_fn>::value;
        constexpr const detail::uninitialized_fill_fn& uninitialized_fill = compat::detail::self_ranges::static_const<detail::uninitialized_fill_fn>::value;
        constexpr const detail::uninitialized_fill_n_fn& uninitialized_fill_n = compat::detail::self_ranges::static_const<detail::uninitialized_fill_n_fn>::value;
        constexpr const detail::uninitialized_move_fn& uninitialized_move = compat::detail::self_ranges::static_const<detail::uninitialized_move_fn>::value;
        constexpr const detail::uninitialized_move_n_fn& uninitialized_move_n = compat::detail::self_ranges::static_const<detail::uninitialized_move_n_fn>::value;
        constexpr const detail::uninitialized_default_construct_fn& uninitialized_default_construct = compat::detail::self_ranges::static_const<detail::uninitialized_default_construct_fn>::value;
        constexpr const detail::uninitialized_default_construct_n_fn& uninitialized_default_construct_n = compat::detail::self_ranges::static_const<detail::uninitialized_default_construct_n_fn>::value;
        constexpr const detail::uninitialized_value_construct_fn& uninitialized_value_construct = compat::detail::self_ranges::static_const<detail::uninitialized_value_construct_fn>::value;
        constexpr const detail::uninitialized_value_construct_n_fn& uninitialized_value_construct_n = compat::detail::self_ranges::static_const<detail::uninitialized_value_construct_n_fn>::value;
    }
#endif

} // namespace ranges
} // namespace self_memory
} // namespace detail
} // namespace compat
