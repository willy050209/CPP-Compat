#include "test_helpers.hpp"
#include <compat/Memory.hpp>
#include <compat/Ranges.hpp>

#include <vector>
#include <string>
#include <utility>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace {

/// <summary>
/// 用於生命週期追蹤的物件結構體。
/// </summary>
struct MemTracker {
    static int32_t live_count;
    static int32_t ctor_count;
    static int32_t dtor_count;

    static void reset() noexcept {
        live_count = 0;
        ctor_count = 0;
        dtor_count = 0;
    }

    int32_t id{0};

    MemTracker() noexcept : id(0) {
        ++live_count;
        ++ctor_count;
    }

    explicit MemTracker(int32_t i) noexcept : id(i) {
        ++live_count;
        ++ctor_count;
    }

    MemTracker(const MemTracker& o) noexcept : id(o.id) {
        ++live_count;
        ++ctor_count;
    }

    MemTracker(MemTracker&& o) noexcept : id(o.id) {
        o.id = -1;
        ++live_count;
        ++ctor_count;
    }

    MemTracker& operator=(const MemTracker& o) noexcept {
        id = o.id;
        return *this;
    }

    MemTracker& operator=(MemTracker&& o) noexcept {
        id = o.id;
        o.id = -1;
        return *this;
    }

    ~MemTracker() noexcept {
        --live_count;
        ++dtor_count;
    }
};

int32_t MemTracker::live_count = 0;
int32_t MemTracker::ctor_count = 0;
int32_t MemTracker::dtor_count = 0;

/// <summary>
/// 僅支援搬移之物件結構體。
/// </summary>
struct MemMoveOnly {
    int32_t val{0};

    MemMoveOnly() = default;
    explicit MemMoveOnly(int32_t v) noexcept : val(v) {}
    MemMoveOnly(const MemMoveOnly&) = delete;
    MemMoveOnly& operator=(const MemMoveOnly&) = delete;
    MemMoveOnly(MemMoveOnly&& o) noexcept : val(o.val) { o.val = -1; }
    MemMoveOnly& operator=(MemMoveOnly&& o) noexcept {
        if (this != &o) {
            val = o.val;
            o.val = -1;
        }
        return *this;
    }
};

/// <summary>
/// 拷貝時依旗標拋出例外之測試結構體，用於檢驗回滾安全性。
/// </summary>
struct RollbackTester {
    static int32_t live_count;
    static bool trigger_throw;
    static int32_t throw_at_index;
    static int32_t current_copy_count;

    static void reset() noexcept {
        live_count = 0;
        trigger_throw = false;
        throw_at_index = -1;
        current_copy_count = 0;
    }

    int32_t val{0};

    RollbackTester() noexcept : val(0) {
        ++live_count;
    }

    explicit RollbackTester(int32_t v) noexcept : val(v) {
        ++live_count;
    }

    RollbackTester(const RollbackTester& o) : val(o.val) {
        if (trigger_throw && current_copy_count == throw_at_index) {
            throw std::runtime_error("RollbackTester intentional copy exception");
        }
        ++current_copy_count;
        ++live_count;
    }

    RollbackTester(RollbackTester&& o) noexcept : val(o.val) {
        o.val = -1;
        ++live_count;
    }

    ~RollbackTester() noexcept {
        --live_count;
    }
};

int32_t RollbackTester::live_count = 0;
bool RollbackTester::trigger_throw = false;
int32_t RollbackTester::throw_at_index = -1;
int32_t RollbackTester::current_copy_count = 0;

/// <summary>
/// 測試 construct_at, destroy_at, destroy, destroy_n。
/// </summary>
void test_construct_destroy() {
    MemTracker::reset();

    alignas(alignof(MemTracker)) char raw[sizeof(MemTracker) * 4];
    MemTracker* ptr = reinterpret_cast<MemTracker*>(raw);

    // construct_at
    MemTracker* p0 = compat::ranges::construct_at(ptr, 100);
    TEST_ASSERT(p0 == ptr);
    TEST_ASSERT(p0->id == 100);
    TEST_ASSERT(MemTracker::live_count == 1);

    // destroy_at
    compat::ranges::destroy_at(p0);
    TEST_ASSERT(MemTracker::live_count == 0);

    // destroy iterator pair
    compat::ranges::construct_at(ptr, 1);
    compat::ranges::construct_at(ptr + 1, 2);
    compat::ranges::construct_at(ptr + 2, 3);
    TEST_ASSERT(MemTracker::live_count == 3);

    auto end_it = compat::ranges::destroy(ptr, ptr + 3);
    TEST_ASSERT(end_it == ptr + 3);
    TEST_ASSERT(MemTracker::live_count == 0);

    // destroy range overload
    compat::ranges::construct_at(ptr, 10);
    compat::ranges::construct_at(ptr + 1, 20);
    TEST_ASSERT(MemTracker::live_count == 2);

    auto sub = compat::ranges::subrange<MemTracker*>(ptr, ptr + 2);
    auto end_r = compat::ranges::destroy(sub);
    TEST_ASSERT(end_r == ptr + 2);
    TEST_ASSERT(MemTracker::live_count == 0);

    // destroy_n
    compat::ranges::construct_at(ptr, 100);
    compat::ranges::construct_at(ptr + 1, 200);
    TEST_ASSERT(MemTracker::live_count == 2);

    auto end_n = compat::ranges::destroy_n(ptr, 2);
    TEST_ASSERT(end_n == ptr + 2);
    TEST_ASSERT(MemTracker::live_count == 0);
}

/// <summary>
/// 測試 uninitialized_copy 與 uninitialized_copy_n。
/// </summary>
void test_uninitialized_copy() {
    MemTracker::reset();

    std::vector<MemTracker> src;
    src.reserve(3);
    src.emplace_back(1);
    src.emplace_back(2);
    src.emplace_back(3);

    alignas(alignof(MemTracker)) char raw[sizeof(MemTracker) * 3];
    MemTracker* dst = reinterpret_cast<MemTracker*>(raw);

    // uninitialized_copy (iterator pair)
    auto res1 = compat::ranges::uninitialized_copy(src.begin(), src.end(), dst, dst + 3);
    TEST_ASSERT(res1.in == src.end());
    TEST_ASSERT(res1.out == dst + 3);
    TEST_ASSERT(dst[0].id == 1 && dst[1].id == 2 && dst[2].id == 3);

    compat::ranges::destroy(dst, dst + 3);

    // uninitialized_copy (range overload)
    auto res2 = compat::ranges::uninitialized_copy(src, compat::ranges::subrange<MemTracker*>(dst, dst + 3));
    TEST_ASSERT(res2.out == dst + 3);
    TEST_ASSERT(dst[0].id == 1 && dst[1].id == 2 && dst[2].id == 3);

    compat::ranges::destroy(dst, dst + 3);

    // uninitialized_copy_n
    auto res3 = compat::ranges::uninitialized_copy_n(src.begin(), 2, dst, dst + 3);
    TEST_ASSERT(res3.in == src.begin() + 2);
    TEST_ASSERT(res3.out == dst + 2);
    TEST_ASSERT(dst[0].id == 1 && dst[1].id == 2);

    compat::ranges::destroy(dst, dst + 2);
}

/// <summary>
/// 測試 uninitialized_move 與 uninitialized_move_n。
/// </summary>
void test_uninitialized_move() {
    std::vector<MemMoveOnly> src;
    src.emplace_back(10);
    src.emplace_back(20);
    src.emplace_back(30);

    alignas(alignof(MemMoveOnly)) char raw[sizeof(MemMoveOnly) * 3];
    MemMoveOnly* dst = reinterpret_cast<MemMoveOnly*>(raw);

    // uninitialized_move (iterator pair)
    auto res1 = compat::ranges::uninitialized_move(src.begin(), src.end(), dst, dst + 3);
    TEST_ASSERT(res1.in == src.end());
    TEST_ASSERT(res1.out == dst + 3);
    TEST_ASSERT(dst[0].val == 10 && dst[1].val == 20 && dst[2].val == 30);
    TEST_ASSERT(src[0].val == -1 && src[1].val == -1 && src[2].val == -1);

    compat::ranges::destroy(dst, dst + 3);

    // Reset src
    src.clear();
    src.emplace_back(100);
    src.emplace_back(200);

    // uninitialized_move (range overload)
    auto res2 = compat::ranges::uninitialized_move(src, compat::ranges::subrange<MemMoveOnly*>(dst, dst + 2));
    TEST_ASSERT(res2.out == dst + 2);
    TEST_ASSERT(dst[0].val == 100 && dst[1].val == 200);
    TEST_ASSERT(src[0].val == -1 && src[1].val == -1);

    compat::ranges::destroy(dst, dst + 2);

    // uninitialized_move_n
    src.clear();
    src.emplace_back(7);
    src.emplace_back(8);
    auto res3 = compat::ranges::uninitialized_move_n(src.begin(), 2, dst, dst + 2);
    TEST_ASSERT(res3.out == dst + 2);
    TEST_ASSERT(dst[0].val == 7 && dst[1].val == 8);

    compat::ranges::destroy(dst, dst + 2);
}

/// <summary>
/// 測試 uninitialized_fill 與 uninitialized_fill_n。
/// </summary>
void test_uninitialized_fill() {
    alignas(alignof(std::string)) char raw[sizeof(std::string) * 4];
    std::string* dst = reinterpret_cast<std::string*>(raw);

    // uninitialized_fill (iterator pair)
    auto out1 = compat::ranges::uninitialized_fill(dst, dst + 2, std::string("compat"));
    TEST_ASSERT(out1 == dst + 2);
    TEST_ASSERT(dst[0] == "compat" && dst[1] == "compat");
    compat::ranges::destroy(dst, dst + 2);

    // uninitialized_fill (range overload)
    auto sub = compat::ranges::subrange<std::string*>(dst, dst + 3);
    auto out2 = compat::ranges::uninitialized_fill(sub, std::string("fill_rng"));
    TEST_ASSERT(out2 == dst + 3);
    TEST_ASSERT(dst[0] == "fill_rng" && dst[1] == "fill_rng" && dst[2] == "fill_rng");
    compat::ranges::destroy(dst, dst + 3);

    // uninitialized_fill_n
    auto out3 = compat::ranges::uninitialized_fill_n(dst, 4, std::string("n_fill"));
    TEST_ASSERT(out3 == dst + 4);
    TEST_ASSERT(dst[0] == "n_fill" && dst[3] == "n_fill");
    compat::ranges::destroy(dst, dst + 4);
}

/// <summary>
/// 測試 uninitialized_default_construct 與 uninitialized_value_construct。
/// </summary>
void test_uninitialized_default_and_value_construct() {
    MemTracker::reset();

    alignas(alignof(MemTracker)) char raw[sizeof(MemTracker) * 3];
    MemTracker* dst = reinterpret_cast<MemTracker*>(raw);

    // default_construct (iterator pair)
    auto out1 = compat::ranges::uninitialized_default_construct(dst, dst + 3);
    TEST_ASSERT(out1 == dst + 3);
    TEST_ASSERT(MemTracker::live_count == 3);
    compat::ranges::destroy(dst, dst + 3);
    TEST_ASSERT(MemTracker::live_count == 0);

    // default_construct (range overload)
    auto sub1 = compat::ranges::subrange<MemTracker*>(dst, dst + 2);
    auto out2 = compat::ranges::uninitialized_default_construct(sub1);
    TEST_ASSERT(out2 == dst + 2);
    TEST_ASSERT(MemTracker::live_count == 2);
    compat::ranges::destroy(dst, dst + 2);

    // default_construct_n
    auto out3 = compat::ranges::uninitialized_default_construct_n(dst, 3);
    TEST_ASSERT(out3 == dst + 3);
    TEST_ASSERT(MemTracker::live_count == 3);
    compat::ranges::destroy(dst, dst + 3);

    // value_construct (iterator pair)
    auto out4 = compat::ranges::uninitialized_value_construct(dst, dst + 2);
    TEST_ASSERT(out4 == dst + 2);
    TEST_ASSERT(MemTracker::live_count == 2);
    compat::ranges::destroy(dst, dst + 2);

    // value_construct (range overload)
    auto sub2 = compat::ranges::subrange<MemTracker*>(dst, dst + 3);
    auto out5 = compat::ranges::uninitialized_value_construct(sub2);
    TEST_ASSERT(out5 == dst + 3);
    TEST_ASSERT(MemTracker::live_count == 3);
    compat::ranges::destroy(dst, dst + 3);

    // value_construct_n
    auto out6 = compat::ranges::uninitialized_value_construct_n(dst, 2);
    TEST_ASSERT(out6 == dst + 2);
    TEST_ASSERT(MemTracker::live_count == 2);
    compat::ranges::destroy(dst, dst + 2);
}

/// <summary>
/// 測試例外回滾保護機制 (Rollback Guard)。
/// </summary>
void test_rollback_guard() {
#if COMPAT_HAS_EXCEPTIONS
    RollbackTester::reset();

    std::vector<RollbackTester> src;
    src.emplace_back(1);
    src.emplace_back(2);
    src.emplace_back(3);
    src.emplace_back(4);

    alignas(alignof(RollbackTester)) char raw[sizeof(RollbackTester) * 4];
    RollbackTester* dst = reinterpret_cast<RollbackTester*>(raw);

    // 設定在複製第 2 個元素 (index 2) 時拋出例外
    RollbackTester::trigger_throw = true;
    RollbackTester::throw_at_index = 2;
    RollbackTester::current_copy_count = 0;

    bool threw = false;
    try {
        compat::ranges::uninitialized_copy(src.begin(), src.end(), dst, dst + 4);
    } catch (const std::runtime_error&) {
        threw = true;
    }

    TEST_ASSERT(threw);
    // 檢查已建構之元素 (index 0, 1) 是否已全數被回滾釋放，未產生洩漏
    TEST_ASSERT(RollbackTester::live_count == 4); // 僅剩 src 中的 4 個實例

    RollbackTester::reset();
#endif
}

} // namespace

/// <summary>
/// 記憶體模組單元測試進入點。
/// </summary>
void run_test_memory() {
    std::cout << "[TEST] Running test_memory (compat::ranges memory algorithms)..." << std::endl;
    test_construct_destroy();
    test_uninitialized_copy();
    test_uninitialized_move();
    test_uninitialized_fill();
    test_uninitialized_default_and_value_construct();
    test_rollback_guard();
    std::cout << "[PASS] test_memory passed." << std::endl;
}
