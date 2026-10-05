#include "test_helpers.hpp"
#include <compat/Optional.hpp>
#include <memory>
#include <string>
#include <type_traits>

namespace {

struct MoveOnlyObj {
    int val{0};
    MoveOnlyObj() = default;
    explicit MoveOnlyObj(int v) : val(v) {}
    MoveOnlyObj(const MoveOnlyObj&) = delete;
    MoveOnlyObj& operator=(const MoveOnlyObj&) = delete;
    MoveOnlyObj(MoveOnlyObj&& other) noexcept : val(other.val) { other.val = -1; }
    MoveOnlyObj& operator=(MoveOnlyObj&& other) noexcept {
        if (this != &other) {
            val = other.val;
            other.val = -1;
        }
        return *this;
    }
};

} // namespace

void run_test_optional() {
    std::cout << "[TEST] Running test_optional..." << std::endl;

    // TEST-OPT-004 & TEST-OPT-005: Traits & SFINAE
    static_assert(!std::is_copy_constructible<compat::optional<MoveOnlyObj>>::value,
                  "optional<MoveOnlyObj> must not be copy constructible");
    static_assert(std::is_move_constructible<compat::optional<MoveOnlyObj>>::value,
                  "optional<MoveOnlyObj> must be move constructible");
    static_assert(!std::is_copy_assignable<compat::optional<MoveOnlyObj>>::value,
                  "optional<MoveOnlyObj> must not be copy assignable");
    static_assert(std::is_move_assignable<compat::optional<MoveOnlyObj>>::value,
                  "optional<MoveOnlyObj> must be move assignable");

    static_assert(std::is_trivially_destructible<compat::optional<int>>::value,
                  "optional<int> must be trivially destructible");

    // TEST-OPT-001: reset()
    compat::optional<int> opt1(42);
    TEST_ASSERT(opt1.has_value());
    TEST_ASSERT(*opt1 == 42);
    opt1.reset();
    TEST_ASSERT(!opt1.has_value());
    TEST_ASSERT(!opt1);

    // TEST-OPT-002: emplace()
    auto& ref = opt1.emplace(100);
    TEST_ASSERT(opt1.has_value());
    TEST_ASSERT(ref == 100);
    TEST_ASSERT(*opt1 == 100);

    // TEST-OPT-003: Move-only T
    compat::optional<MoveOnlyObj> opt_mo;
    TEST_ASSERT(!opt_mo.has_value());
    opt_mo.emplace(777);
    TEST_ASSERT(opt_mo.has_value() && opt_mo->val == 777);

    compat::optional<MoveOnlyObj> opt_mo2 = std::move(opt_mo);
    TEST_ASSERT(opt_mo2.has_value() && opt_mo2->val == 777);

    compat::optional<MoveOnlyObj> opt_mo3;
    opt_mo3 = std::move(opt_mo2);
    TEST_ASSERT(opt_mo3.has_value() && opt_mo3->val == 777);

    // bad_optional_access exception test
#if COMPAT_HAS_EXCEPTIONS
    compat::optional<int> empty_opt;
    TEST_ASSERT_THROWS(empty_opt.value(), compat::bad_optional_access);
#endif

    // value_or tests
    compat::optional<std::string> str_opt;
    TEST_ASSERT(str_opt.value_or("default") == "default");
    str_opt = "hello";
    TEST_ASSERT(str_opt.value_or("default") == "hello");

    // make_optional
    auto opt_made = compat::make_optional<int>(123);
    TEST_ASSERT(opt_made.has_value() && *opt_made == 123);

    // TEST-OPT-006: Copy constructor & Copy assignment
    compat::optional<std::string> opt_copy_src("copy_test");
    compat::optional<std::string> opt_copy_dst = opt_copy_src;
    TEST_ASSERT(opt_copy_dst.has_value() && *opt_copy_dst == "copy_test");
    compat::optional<std::string> opt_copy_assigned;
    opt_copy_assigned = opt_copy_src;
    TEST_ASSERT(opt_copy_assigned.has_value() && *opt_copy_assigned == "copy_test");

    // TEST-OPT-007: nullopt construction & assignment
    compat::optional<int> opt_null(compat::nullopt);
    TEST_ASSERT(!opt_null.has_value());
    compat::optional<int> opt_to_clear(999);
    TEST_ASSERT(opt_to_clear.has_value());
    opt_to_clear = compat::nullopt;
    TEST_ASSERT(!opt_to_clear.has_value());

    // TEST-OPT-008: value() on engaged optional
    compat::optional<int> opt_val(888);
    TEST_ASSERT(opt_val.value() == 888);
    opt_val.value() = 999;
    TEST_ASSERT(*opt_val == 999);

    // TEST-OPT-009: Rvalue value_or & Rvalue dereference
    compat::optional<std::string> opt_rval("moved_string");
    std::string extracted = std::move(opt_rval).value_or("fallback");
    TEST_ASSERT(extracted == "moved_string");

    compat::optional<std::string> opt_rval_empty;
    std::string fallback_res = std::move(opt_rval_empty).value_or("fallback");
    TEST_ASSERT(fallback_res == "fallback");

    // TEST-OPT-010: multi-arg make_optional
    auto opt_pair = compat::make_optional<std::pair<int, std::string>>(42, std::string("piecewise"));
    TEST_ASSERT(opt_pair.has_value());
    TEST_ASSERT(opt_pair->first == 42 && opt_pair->second == "piecewise");

    auto opt_pair_made = compat::make_optional<std::pair<int, int>>(10, 20);
    TEST_ASSERT(opt_pair_made.has_value());
    TEST_ASSERT(opt_pair_made->first == 10 && opt_pair_made->second == 20);

    // TEST-OPT-011: Full comparison operator matrix
    compat::optional<int> o_none1;
    compat::optional<int> o_none2(compat::nullopt);
    compat::optional<int> o_10(10);
    compat::optional<int> o_10_dup(10);
    compat::optional<int> o_20(20);

    // With value
    TEST_ASSERT(o_10 == 10);
    TEST_ASSERT(10 == o_10);
    TEST_ASSERT(o_10 != 20);
    TEST_ASSERT(20 != o_10);
    TEST_ASSERT(!(o_none1 == 10));
    TEST_ASSERT(!(10 == o_none1));

    // With nullopt
    TEST_ASSERT(o_none1 == compat::nullopt);
    TEST_ASSERT(compat::nullopt == o_none1);
    TEST_ASSERT(!(o_10 == compat::nullopt));
    TEST_ASSERT(!(compat::nullopt == o_10));
    TEST_ASSERT(o_10 != compat::nullopt);
    TEST_ASSERT(compat::nullopt != o_10);
    TEST_ASSERT(!(o_none1 != compat::nullopt));
    TEST_ASSERT(!(compat::nullopt != o_none1));

    // Between optionals
    TEST_ASSERT(o_none1 == o_none2);
    TEST_ASSERT(o_10 == o_10_dup);
    TEST_ASSERT(o_10 != o_20);
    TEST_ASSERT(o_10 != o_none1);
    TEST_ASSERT(o_none1 != o_10);

    std::cout << "[PASS] test_optional passed." << std::endl;
}
