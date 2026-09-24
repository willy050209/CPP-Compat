#include "test_helpers.hpp"
#include <compat/Expected.hpp>
#include <string>
#include <cstdint>
#include <type_traits>
#include <utility>

/// <summary>
/// Helper function tracking move-only invocation.
/// </summary>
/// <param name="val">Integer value.</param>
/// <returns>Expected incremented integer.</returns>
compat::expected<int32_t, std::string> increment_val(int32_t val) {
    return compat::expected<int32_t, std::string>(val + 1);
}

namespace {

struct LifetimeCounter {
    static int construct_count;
    static int copy_count;
    static int move_count;
    static int destroy_count;

    static void reset() {
        construct_count = 0;
        copy_count = 0;
        move_count = 0;
        destroy_count = 0;
    }

    static int live_count() {
        return construct_count + copy_count + move_count - destroy_count;
    }

    int id{0};
    LifetimeCounter() : id(0) { construct_count++; }
    explicit LifetimeCounter(int i) : id(i) { construct_count++; }
    LifetimeCounter(const LifetimeCounter& o) : id(o.id) { copy_count++; }
    LifetimeCounter(LifetimeCounter&& o) noexcept : id(o.id) { move_count++; o.id = -1; }
    LifetimeCounter& operator=(const LifetimeCounter& o) {
        id = o.id;
        return *this;
    }
    LifetimeCounter& operator=(LifetimeCounter&& o) noexcept {
        id = o.id;
        o.id = -1;
        return *this;
    }
    ~LifetimeCounter() { destroy_count++; }
};

int LifetimeCounter::construct_count = 0;
int LifetimeCounter::copy_count = 0;
int LifetimeCounter::move_count = 0;
int LifetimeCounter::destroy_count = 0;

struct ThrowOnCopy {
    int val{0};
    static bool should_throw;

    ThrowOnCopy() = default;
    explicit ThrowOnCopy(int v) : val(v) {}
    ThrowOnCopy(const ThrowOnCopy& o) : val(o.val) {
        if (should_throw) throw std::runtime_error("ThrowOnCopy copy ctor");
    }
    ThrowOnCopy(ThrowOnCopy&& o) noexcept : val(o.val) { o.val = -1; }
    ThrowOnCopy& operator=(const ThrowOnCopy& o) {
        if (should_throw) throw std::runtime_error("ThrowOnCopy copy assign");
        val = o.val;
        return *this;
    }
    ThrowOnCopy& operator=(ThrowOnCopy&& o) noexcept {
        val = o.val;
        o.val = -1;
        return *this;
    }
};
bool ThrowOnCopy::should_throw = false;

struct ThrowOnMove {
    int val{0};
    static bool should_throw;

    ThrowOnMove() = default;
    explicit ThrowOnMove(int v) : val(v) {}
    ThrowOnMove(const ThrowOnMove& o) : val(o.val) {}
    ThrowOnMove(ThrowOnMove&& o) {
        if (should_throw) throw std::runtime_error("ThrowOnMove move ctor");
        val = o.val;
        o.val = -1;
    }
    ThrowOnMove& operator=(const ThrowOnMove& o) {
        val = o.val;
        return *this;
    }
    ThrowOnMove& operator=(ThrowOnMove&& o) {
        if (should_throw) throw std::runtime_error("ThrowOnMove move assign");
        val = o.val;
        o.val = -1;
        return *this;
    }
};
bool ThrowOnMove::should_throw = false;

} // namespace

/// <summary>
/// Comprehensive test suite for compat::expected, unexpected, and bad_expected_access.
/// </summary>
void run_test_expected() {
    std::cout << "[TEST] Running test_expected..." << std::endl;

    // 1. Trivial Destructibility Assertions
    static_assert(std::is_trivially_destructible<compat::expected<int32_t, int32_t>>::value,
                  "expected<int32_t, int32_t> must be trivially destructible");
    static_assert(std::is_trivially_destructible<compat::expected<void, int32_t>>::value,
                  "expected<void, int32_t> must be trivially destructible");
    static_assert(!std::is_trivially_destructible<compat::expected<int32_t, std::string>>::value,
                  "expected<int32_t, std::string> must not be trivially destructible");
    static_assert(!std::is_trivially_destructible<compat::expected<void, std::string>>::value,
                  "expected<void, std::string> must not be trivially destructible");

    // 2. Test expected<int32_t, std::string> basic functionality
    compat::expected<int32_t, std::string> val_exp(12345);
    TEST_ASSERT(val_exp.has_value());
    TEST_ASSERT(static_cast<bool>(val_exp));
    TEST_ASSERT(val_exp.value() == 12345);
    TEST_ASSERT(*val_exp == 12345);

    compat::expected<int32_t, std::string> err_exp(compat::unexpected<std::string>("NetworkTimeout"));
    TEST_ASSERT(!err_exp.has_value());
    TEST_ASSERT(!static_cast<bool>(err_exp));
    TEST_ASSERT(err_exp.error() == "NetworkTimeout");

    // Test bad_expected_access exception throwing on value() access when in error state
    try {
        (void)err_exp.value();
        TEST_ASSERT(false); // Should not reach here
    } catch (const compat::bad_expected_access<std::string>& ex) {
        TEST_ASSERT(ex.error() == "NetworkTimeout");
        TEST_ASSERT(std::string(ex.what()).length() > 0);
    } catch (...) {
        TEST_ASSERT(false);
    }

#if !COMPAT_HAS_STD_EXPECTED
    // Error access on value state throws in fallback
    TEST_ASSERT_THROWS(val_exp.error(), compat::bad_expected_access<std::string>);
#endif

    // 3. Test expected<std::string, int32_t> and operator->()
    compat::expected<std::string, int32_t> str_exp("Modern C++ Compatibility");
    TEST_ASSERT(str_exp.has_value());
    TEST_ASSERT(str_exp->length() == 24);
    TEST_ASSERT(str_exp.value() == "Modern C++ Compatibility");

    // 4. Test expected<void, std::string> and expected<void, int32_t>
    {
        compat::expected<void, std::string> void_val;
        TEST_ASSERT(void_val.has_value());
        TEST_ASSERT(static_cast<bool>(void_val));
        void_val.value(); // Should not throw
        *void_val;        // operator* should compile and do nothing

        compat::expected<void, std::string> void_err(compat::unexpected<std::string>("VoidError"));
        TEST_ASSERT(!void_err.has_value());
        TEST_ASSERT(!static_cast<bool>(void_err));
        TEST_ASSERT(void_err.error() == "VoidError");

        try {
            void_err.value();
            TEST_ASSERT(false);
        } catch (const compat::bad_expected_access<std::string>& ex) {
            TEST_ASSERT(ex.error() == "VoidError");
        }
    }

    {
        compat::expected<void, int32_t> void_int_val;
        TEST_ASSERT(void_int_val.has_value());
        void_int_val.value();
        *void_int_val;

        compat::expected<void, int32_t> void_int_err(compat::unexpected<int32_t>(404));
        TEST_ASSERT(!void_int_err.has_value());
        TEST_ASSERT(void_int_err.error() == 404);

        try {
            void_int_err.value();
            TEST_ASSERT(false);
        } catch (const compat::bad_expected_access<int32_t>& ex) {
            TEST_ASSERT(ex.error() == 404);
        }
    }

    // 5. Monadic operations on expected<int32_t, std::string>
    // 5.1 and_then
    {
        // lvalue
        compat::expected<int32_t, std::string> e_val(10);
        auto res1 = e_val.and_then([](int32_t x) {
            return compat::expected<int32_t, std::string>(x * 2);
        });
        TEST_ASSERT(res1.has_value() && res1.value() == 20);

        // const lvalue
        const compat::expected<int32_t, std::string> c_val(10);
        auto res2 = c_val.and_then([](int32_t x) {
            return compat::expected<int32_t, std::string>(x + 5);
        });
        TEST_ASSERT(res2.has_value() && res2.value() == 15);

        // rvalue move
        auto res3 = compat::expected<int32_t, std::string>(10).and_then([](int32_t x) {
            return compat::expected<std::string, std::string>(std::to_string(x));
        });
        TEST_ASSERT(res3.has_value() && res3.value() == "10");

        // error case
        compat::expected<int32_t, std::string> e_err(compat::unexpected<std::string>("Fail"));
        auto res4 = e_err.and_then([](int32_t x) {
            return compat::expected<int32_t, std::string>(x * 2);
        });
        TEST_ASSERT(!res4.has_value() && res4.error() == "Fail");
    }

    // 5.2 or_else
    {
        // lvalue value
        compat::expected<int32_t, std::string> e_val(10);
        auto res1 = e_val.or_else([](const std::string& err) {
            return compat::expected<int32_t, std::string>(compat::unexpected<std::string>(err + "_recovered"));
        });
        TEST_ASSERT(res1.has_value() && res1.value() == 10);

        // lvalue error
        compat::expected<int32_t, std::string> e_err(compat::unexpected<std::string>("Err"));
        auto res2 = e_err.or_else([](const std::string& err) {
            return compat::expected<int32_t, std::string>(compat::unexpected<std::string>(err + "_recovered"));
        });
        TEST_ASSERT(!res2.has_value() && res2.error() == "Err_recovered");

        // const lvalue
        const compat::expected<int32_t, std::string> c_err(compat::unexpected<std::string>("Err"));
        auto res3 = c_err.or_else([](const std::string& err) {
            return compat::expected<int32_t, int32_t>(compat::unexpected<int32_t>(static_cast<int32_t>(err.length())));
        });
        TEST_ASSERT(!res3.has_value() && res3.error() == 3);

        // rvalue
        auto res4 = compat::expected<int32_t, std::string>(compat::unexpected<std::string>("Err")).or_else([](std::string&& err) {
            return compat::expected<int32_t, std::string>(compat::unexpected<std::string>(std::move(err) + "!"));
        });
        TEST_ASSERT(!res4.has_value() && res4.error() == "Err!");
    }

    // 5.3 transform (non-void and void)
    {
        compat::expected<int32_t, std::string> e_val(10);
        // non-void lvalue
        auto res1 = e_val.transform([](int32_t x) { return x * 3; });
        TEST_ASSERT(res1.has_value() && res1.value() == 30);

        // non-void const lvalue
        const auto& c_val = e_val;
        auto res2 = c_val.transform([](int32_t x) { return std::to_string(x); });
        TEST_ASSERT(res2.has_value() && res2.value() == "10");

        // non-void rvalue
        auto res3 = compat::expected<int32_t, std::string>(7).transform([](int32_t x) { return x + 1; });
        TEST_ASSERT(res3.has_value() && res3.value() == 8);

        // void return lvalue
        int32_t side_effect = 0;
        auto res4 = e_val.transform([&side_effect](int32_t x) { side_effect = x; });
        TEST_ASSERT(res4.has_value() && side_effect == 10);
        static_assert(std::is_same<decltype(res4), compat::expected<void, std::string>>::value, "transform to void");

        // void return rvalue
        auto res5 = compat::expected<int32_t, std::string>(5).transform([&side_effect](int32_t x) { side_effect += x; });
        TEST_ASSERT(res5.has_value() && side_effect == 15);

        // error propagation
        compat::expected<int32_t, std::string> e_err(compat::unexpected<std::string>("Err"));
        auto res6 = e_err.transform([](int32_t x) { return x * 2; });
        TEST_ASSERT(!res6.has_value() && res6.error() == "Err");

        auto res7 = e_err.transform([](int32_t) {});
        TEST_ASSERT(!res7.has_value() && res7.error() == "Err");
    }

    // 5.4 transform_error
    {
        compat::expected<int32_t, std::string> e_val(10);
        auto res1 = e_val.transform_error([](const std::string& err) { return err.length(); });
        TEST_ASSERT(res1.has_value() && res1.value() == 10);

        compat::expected<int32_t, std::string> e_err(compat::unexpected<std::string>("Hello"));
        // lvalue
        auto res2 = e_err.transform_error([](const std::string& err) { return static_cast<int32_t>(err.length()); });
        TEST_ASSERT(!res2.has_value() && res2.error() == 5);

        // const lvalue
        const auto& c_err = e_err;
        auto res3 = c_err.transform_error([](const std::string& err) { return err + " World"; });
        TEST_ASSERT(!res3.has_value() && res3.error() == "Hello World");

        // rvalue move
        auto res4 = compat::expected<int32_t, std::string>(compat::unexpected<std::string>("Temp")).transform_error([](std::string&& err) {
            return err + "!";
        });
        TEST_ASSERT(!res4.has_value() && res4.error() == "Temp!");
    }

    // 6. Monadic operations on expected<void, std::string>
    // 6.1 and_then
    {
        compat::expected<void, std::string> v_val;
        // lvalue
        auto res1 = v_val.and_then([]() {
            return compat::expected<int32_t, std::string>(42);
        });
        TEST_ASSERT(res1.has_value() && res1.value() == 42);

        // const lvalue
        const auto& cv_val = v_val;
        auto res2 = cv_val.and_then([]() {
            return compat::expected<void, std::string>();
        });
        TEST_ASSERT(res2.has_value());

        // rvalue
        auto res3 = compat::expected<void, std::string>().and_then([]() {
            return compat::expected<std::string, std::string>("VoidAndThen");
        });
        TEST_ASSERT(res3.has_value() && res3.value() == "VoidAndThen");

        // error case
        compat::expected<void, std::string> v_err(compat::unexpected<std::string>("VoidErr"));
        auto res4 = v_err.and_then([]() {
            return compat::expected<int32_t, std::string>(99);
        });
        TEST_ASSERT(!res4.has_value() && res4.error() == "VoidErr");
    }

    // 6.2 or_else
    {
        compat::expected<void, std::string> v_val;
        auto res1 = v_val.or_else([](const std::string& err) {
            return compat::expected<void, std::string>(compat::unexpected<std::string>(err));
        });
        TEST_ASSERT(res1.has_value());

        compat::expected<void, std::string> v_err(compat::unexpected<std::string>("VoidErr"));
        auto res2 = v_err.or_else([](const std::string& err) {
            return compat::expected<void, std::string>(compat::unexpected<std::string>(err + "_recovered"));
        });
        TEST_ASSERT(!res2.has_value() && res2.error() == "VoidErr_recovered");

        const auto& cv_err = v_err;
        auto res3 = cv_err.or_else([](const std::string& err) {
            return compat::expected<void, int32_t>(compat::unexpected<int32_t>(static_cast<int32_t>(err.length())));
        });
        TEST_ASSERT(!res3.has_value() && res3.error() == 7);

        auto res4 = compat::expected<void, std::string>(compat::unexpected<std::string>("RvalErr")).or_else([](std::string&& err) {
            return compat::expected<void, std::string>(compat::unexpected<std::string>(std::move(err) + "!"));
        });
        TEST_ASSERT(!res4.has_value() && res4.error() == "RvalErr!");
    }

    // 6.3 transform (non-void and void)
    {
        compat::expected<void, std::string> v_val;
        // non-void lvalue
        auto res1 = v_val.transform([]() { return 100; });
        TEST_ASSERT(res1.has_value() && res1.value() == 100);

        // const lvalue
        const auto& cv_val = v_val;
        auto res2 = cv_val.transform([]() { return std::string("Done"); });
        TEST_ASSERT(res2.has_value() && res2.value() == "Done");

        // void return lvalue
        int32_t counter = 0;
        auto res3 = v_val.transform([&counter]() { counter += 1; });
        TEST_ASSERT(res3.has_value() && counter == 1);

        // rvalue non-void & void
        auto res4 = compat::expected<void, std::string>().transform([]() { return 200; });
        TEST_ASSERT(res4.has_value() && res4.value() == 200);

        auto res5 = compat::expected<void, std::string>().transform([&counter]() { counter += 2; });
        TEST_ASSERT(res5.has_value() && counter == 3);

        // error case
        compat::expected<void, std::string> v_err(compat::unexpected<std::string>("Err"));
        auto res6 = v_err.transform([]() { return 50; });
        TEST_ASSERT(!res6.has_value() && res6.error() == "Err");

        auto res7 = v_err.transform([]() {});
        TEST_ASSERT(!res7.has_value() && res7.error() == "Err");
    }

    // 6.4 transform_error
    {
        compat::expected<void, std::string> v_val;
        auto res1 = v_val.transform_error([](const std::string& err) { return err.length(); });
        TEST_ASSERT(res1.has_value());

        compat::expected<void, std::string> v_err(compat::unexpected<std::string>("Failure"));
        auto res2 = v_err.transform_error([](const std::string& err) { return static_cast<int32_t>(err.length()); });
        TEST_ASSERT(!res2.has_value() && res2.error() == 7);

        const auto& cv_err = v_err;
        auto res3 = cv_err.transform_error([](const std::string& err) { return err + " Handled"; });
        TEST_ASSERT(!res3.has_value() && res3.error() == "Failure Handled");

        auto res4 = compat::expected<void, std::string>(compat::unexpected<std::string>("ErrMove")).transform_error([](std::string&& err) {
            return err + "!";
        });
        TEST_ASSERT(!res4.has_value() && res4.error() == "ErrMove!");
    }

    // 7. Test copy and assignment
    compat::expected<int32_t, std::string> copied = val_exp;
    TEST_ASSERT(copied.has_value());
    TEST_ASSERT(copied.value() == 12345);

    compat::expected<int32_t, std::string> assigned(0);
    assigned = err_exp;
    TEST_ASSERT(!assigned.has_value());
    TEST_ASSERT(assigned.error() == "NetworkTimeout");

    // TEST-EXP-005: conditional noexcept assertions
    {
        using NothrowType = int;
        using ThrowingMoveType = ThrowOnMove;
        static_assert(std::is_nothrow_move_constructible<compat::expected<NothrowType, int>>::value,
                      "expected with nothrow types must have nothrow move ctor");
        static_assert(!std::is_nothrow_move_constructible<compat::expected<ThrowingMoveType, int>>::value,
                      "expected with throwing move type must have throwing move ctor");
    }

    // TEST-EXP-001 & TEST-EXP-002: throwing copy/move construction
#if COMPAT_HAS_EXCEPTIONS
    {
        compat::expected<ThrowOnCopy, int> src(ThrowOnCopy{10});
        ThrowOnCopy::should_throw = true;
        try {
            compat::expected<ThrowOnCopy, int> dst(src);
            TEST_ASSERT(false); // Should throw
        } catch (const std::runtime_error& ex) {
            TEST_ASSERT(std::string(ex.what()) == "ThrowOnCopy copy ctor");
        }
        ThrowOnCopy::should_throw = false;
    }
    {
        compat::expected<ThrowOnMove, int> src(ThrowOnMove{20});
        ThrowOnMove::should_throw = true;
        try {
            compat::expected<ThrowOnMove, int> dst(std::move(src));
            TEST_ASSERT(false); // Should throw
        } catch (const std::runtime_error& ex) {
            TEST_ASSERT(std::string(ex.what()) == "ThrowOnMove move ctor");
        }
        ThrowOnMove::should_throw = false;
    }

    // TEST-EXP-003 & TEST-EXP-004: value -> error and error -> value transitions under exception
    {
        // Transition: Value -> Error when Error copy constructor throws
        ThrowOnCopy::should_throw = false;
        compat::expected<int, ThrowOnCopy> e_val(42);
        TEST_ASSERT(e_val.has_value());

        ThrowOnCopy err_obj(99);
        ThrowOnCopy::should_throw = true;
        try {
            e_val = compat::unexpected<ThrowOnCopy>(err_obj);
            TEST_ASSERT(false);
        } catch (const std::runtime_error&) {
            // Invariant: after exception during transition, state must remain valid!
            // Case B/C ensures e_val still holds valid value 42
            TEST_ASSERT(e_val.has_value());
            TEST_ASSERT(*e_val == 42);
        }
        ThrowOnCopy::should_throw = false;

        // Transition: Error -> Value when Value copy constructor throws
        compat::expected<ThrowOnCopy, int> e_err(compat::unexpected<int>(500));
        TEST_ASSERT(!e_err.has_value());

        ThrowOnCopy val_obj(77);
        ThrowOnCopy::should_throw = true;
        try {
            e_err = val_obj;
            TEST_ASSERT(false);
        } catch (const std::runtime_error&) {
            // Invariant: after exception, state remains valid error!
            TEST_ASSERT(!e_err.has_value());
            TEST_ASSERT(e_err.error() == 500);
        }
        ThrowOnCopy::should_throw = false;
    }
#endif // COMPAT_HAS_EXCEPTIONS

    // TEST-EXP-006: Object lifetime counters (verify no leaks or double destroys)
    {
        LifetimeCounter::reset();
        {
            compat::expected<LifetimeCounter, int> exp1(LifetimeCounter{1});
            TEST_ASSERT(exp1.has_value());
            TEST_ASSERT(LifetimeCounter::live_count() == 1);

            // emplace
            exp1.emplace(2);
            TEST_ASSERT(exp1.has_value());
            TEST_ASSERT(LifetimeCounter::live_count() == 1);

            // Transition: Value -> Error
            exp1 = compat::unexpected<int>(404);
            TEST_ASSERT(!exp1.has_value());
            TEST_ASSERT(LifetimeCounter::live_count() == 0);

            // Transition: Error -> Value
            exp1.emplace(3);
            TEST_ASSERT(exp1.has_value());
            TEST_ASSERT(LifetimeCounter::live_count() == 1);
        }
        // After exp1 is destroyed
        TEST_ASSERT(LifetimeCounter::live_count() == 0);
        TEST_ASSERT(LifetimeCounter::construct_count + LifetimeCounter::copy_count + LifetimeCounter::move_count == LifetimeCounter::destroy_count);
    }

    // emplace for expected<void, E>
    {
        compat::expected<void, std::string> ev(compat::unexpected<std::string>("err"));
        TEST_ASSERT(!ev.has_value());
        ev.emplace();
        TEST_ASSERT(ev.has_value());
    }

    std::cout << "[PASS] test_expected passed." << std::endl;
}
