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

    std::cout << "[PASS] test_optional passed." << std::endl;
}
