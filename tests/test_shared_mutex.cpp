#include "test_helpers.hpp"
#include "compat/SharedMutex.hpp"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>

static void test_shared_mutex_basic() {
    compat::shared_mutex sm;

    // Exclusive lock / unlock
    sm.lock();
    sm.unlock();

    // Shared lock / unlock
    sm.lock_shared();
    sm.unlock_shared();

    // try_lock
    TEST_ASSERT(sm.try_lock());
    // While exclusive lock is held, try_lock_shared should fail
    TEST_ASSERT(!sm.try_lock_shared());
    sm.unlock();

    // try_lock_shared
    TEST_ASSERT(sm.try_lock_shared());
    // Multiple shared locks are permitted
    TEST_ASSERT(sm.try_lock_shared());
    // Exclusive lock should fail while shared lock is held
    TEST_ASSERT(!sm.try_lock());
    sm.unlock_shared();
    sm.unlock_shared();

    // Now exclusive lock should succeed again
    TEST_ASSERT(sm.try_lock());
    sm.unlock();
}

static void test_shared_lock_raii() {
    compat::shared_mutex sm;

    // Scope-based acquisition and automatic release
    {
        compat::shared_lock<compat::shared_mutex> lk(sm);
        TEST_ASSERT(lk.owns_lock());
        TEST_ASSERT(static_cast<bool>(lk));
        TEST_ASSERT(lk.mutex() == &sm);

        // Exclusive lock should fail while shared_lock is active
        TEST_ASSERT(!sm.try_lock());
    }

    // Now exclusive lock must succeed
    TEST_ASSERT(sm.try_lock());
    sm.unlock();

    // Defer lock
    {
        compat::shared_lock<compat::shared_mutex> lk(sm, std::defer_lock);
        TEST_ASSERT(!lk.owns_lock());
        TEST_ASSERT(lk.mutex() == &sm);
        lk.lock();
        TEST_ASSERT(lk.owns_lock());
        lk.unlock();
        TEST_ASSERT(!lk.owns_lock());
    }

    // Try to lock
    {
        compat::shared_lock<compat::shared_mutex> lk(sm, std::try_to_lock);
        TEST_ASSERT(lk.owns_lock());
    }

    // Adopt lock
    {
        sm.lock_shared();
        compat::shared_lock<compat::shared_mutex> lk(sm, std::adopt_lock);
        TEST_ASSERT(lk.owns_lock());
    }
    // After scope exit, lock was released. Verify exclusive lock works:
    TEST_ASSERT(sm.try_lock());
    sm.unlock();

    // Move construction and assignment
    {
        compat::shared_lock<compat::shared_mutex> lk1(sm);
        TEST_ASSERT(lk1.owns_lock());

        compat::shared_lock<compat::shared_mutex> lk2(std::move(lk1));
        TEST_ASSERT(!lk1.owns_lock());
        TEST_ASSERT(lk1.mutex() == nullptr);
        TEST_ASSERT(lk2.owns_lock());
        TEST_ASSERT(lk2.mutex() == &sm);

        compat::shared_lock<compat::shared_mutex> lk3;
        lk3 = std::move(lk2);
        TEST_ASSERT(!lk2.owns_lock());
        TEST_ASSERT(lk3.owns_lock());
        TEST_ASSERT(lk3.mutex() == &sm);
    }

    // Release
    {
        compat::shared_lock<compat::shared_mutex> lk(sm);
        compat::shared_mutex* pm = lk.release();
        TEST_ASSERT(pm == &sm);
        TEST_ASSERT(!lk.owns_lock());
        pm->unlock_shared();
    }

    // Swap
    {
        compat::shared_mutex sm2;
        compat::shared_lock<compat::shared_mutex> lk1(sm);
        compat::shared_lock<compat::shared_mutex> lk2(sm2, std::defer_lock);
        lk1.swap(lk2);
        TEST_ASSERT(!lk1.owns_lock());
        TEST_ASSERT(lk1.mutex() == &sm2);
        TEST_ASSERT(lk2.owns_lock());
        TEST_ASSERT(lk2.mutex() == &sm);
    }
}

static void test_shared_mutex_multithreaded() {
    compat::shared_mutex sm;
    std::atomic<bool> reader1_holding(false);
    std::atomic<bool> reader2_holding(false);
    std::atomic<bool> release_readers(false);

    // Thread 1 acquires shared lock
    std::thread reader1([&]() {
        compat::shared_lock<compat::shared_mutex> lk(sm);
        reader1_holding.store(true);
        while (!release_readers.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Wait until reader1 holds lock
    while (!reader1_holding.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Thread 2 acquires shared lock concurrently
    std::thread reader2([&]() {
        compat::shared_lock<compat::shared_mutex> lk(sm);
        reader2_holding.store(true);
        while (!release_readers.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Wait until reader2 also holds lock
    while (!reader2_holding.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Both readers are actively holding the shared lock simultaneously
    TEST_ASSERT(reader1_holding.load());
    TEST_ASSERT(reader2_holding.load());

    // Exclusive lock MUST fail right now
    TEST_ASSERT(!sm.try_lock());

    // Another shared lock MUST succeed right now
    TEST_ASSERT(sm.try_lock_shared());
    sm.unlock_shared();

    // Release readers
    release_readers.store(true);
    reader1.join();
    reader2.join();

    // Now exclusive lock MUST succeed
    TEST_ASSERT(sm.try_lock());
    sm.unlock();
}

void run_test_shared_mutex() {
    test_shared_mutex_basic();
    test_shared_lock_raii();
    test_shared_mutex_multithreaded();
}
