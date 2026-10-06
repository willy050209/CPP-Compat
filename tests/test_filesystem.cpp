#include "test_helpers.hpp"
#include <compat/Filesystem.hpp>

#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <system_error>

namespace {

namespace fs = compat::filesystem;

/// <summary>
/// 測試 path 的字法解析、組合與分解運算。
/// </summary>
void test_fs_path_syntax_and_decomposition() {
    std::cout << "  [Subtest] test_fs_path_syntax_and_decomposition..." << std::endl;

    // 1. 基本建構與字串轉換
    fs::path p1("foo/bar/baz.txt");
    TEST_ASSERT(!p1.empty());
    TEST_ASSERT(p1.filename() == "baz.txt");
    TEST_ASSERT(p1.stem() == "baz");
    TEST_ASSERT(p1.extension() == ".txt");
    TEST_ASSERT(p1.parent_path() == "foo/bar" || p1.parent_path() == "foo\\bar");

    // 2. 串接運算子 /= 與 /
    fs::path p2 = "dir";
    p2 /= "sub";
    p2 /= "file.dat";
    TEST_ASSERT(p2.filename() == "file.dat");
    TEST_ASSERT(p2.parent_path().filename() == "sub");

    fs::path p3 = fs::path("alpha") / "beta" / "gamma.cpp";
    TEST_ASSERT(p3.filename() == "gamma.cpp");
    TEST_ASSERT(p3.extension() == ".cpp");

    // 3. 字串附加 += 與 concat
    fs::path p4 = "archive";
    p4 += ".tar";
    p4.concat(".gz");
    TEST_ASSERT(p4.filename() == "archive.tar.gz");
    TEST_ASSERT(p4.extension() == ".gz");

    // 4. 修改器: replace_filename, replace_extension, remove_filename
    fs::path p5 = "folder/document.old";
    p5.replace_extension(".new");
    TEST_ASSERT(p5.extension() == ".new");
    TEST_ASSERT(p5.filename() == "document.new");

    p5.replace_filename("readme.md");
    TEST_ASSERT(p5.filename() == "readme.md");

    p5.remove_filename();
    TEST_ASSERT(p5.filename().empty());

    // 5. 查詢函式 (Queries)
    fs::path p_query = "dir/file.txt";
    TEST_ASSERT(p_query.has_filename());
    TEST_ASSERT(p_query.has_parent_path());
    TEST_ASSERT(p_query.has_stem());
    TEST_ASSERT(p_query.has_extension());
    TEST_ASSERT(p_query.is_relative());

    fs::path p_empty;
    TEST_ASSERT(p_empty.empty());
    TEST_ASSERT(!p_empty.has_filename());

    // 6. 正規化 (lexically_normal)
    fs::path p_norm = fs::path("foo/./bar/../baz").lexically_normal();
    TEST_ASSERT(p_norm.filename() == "baz");
    fs::path p_norm_dot = fs::path("foo/./bar/../baz/.").lexically_normal();
    TEST_ASSERT(p_norm_dot.filename() == "" || p_norm_dot.filename() == ".");

    // 7. 疊代器 (Iterators)
    fs::path p_iter = "a/b/c";
    std::vector<std::string> parts;
    for (const auto& elem : p_iter) {
        parts.push_back(elem.string());
    }
    TEST_ASSERT(parts.size() == 3);
    TEST_ASSERT(parts[0] == "a");
    TEST_ASSERT(parts[1] == "b");
    TEST_ASSERT(parts[2] == "c");

    // 8. 比較運算子
    fs::path pa = "foo/bar";
    fs::path pb = "foo/bar";
    fs::path pc = "foo/baz";
    TEST_ASSERT(pa == pb);
    TEST_ASSERT(pa != pc);
}

/// <summary>
/// 測試 列舉值、位元遮罩運算及 file_status。
/// </summary>
void test_fs_enums_and_bitmasks() {
    std::cout << "  [Subtest] test_fs_enums_and_bitmasks..." << std::endl;

    // perms 位元運算
    fs::perms p1 = fs::perms::owner_read | fs::perms::owner_write;
    TEST_ASSERT((p1 & fs::perms::owner_read) != fs::perms::none);
    TEST_ASSERT((p1 & fs::perms::owner_exec) == fs::perms::none);

    p1 |= fs::perms::owner_exec;
    TEST_ASSERT((p1 & fs::perms::owner_all) == (fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec));

    fs::perms p2 = ~fs::perms::none;
    TEST_ASSERT(p2 != fs::perms::none);

    // copy_options 位元運算
    fs::copy_options c_opt = fs::copy_options::overwrite_existing | fs::copy_options::recursive;
    TEST_ASSERT((c_opt & fs::copy_options::overwrite_existing) != fs::copy_options::none);
    TEST_ASSERT((c_opt & fs::copy_options::skip_existing) == fs::copy_options::none);

    // directory_options 位元運算
    fs::directory_options d_opt = fs::directory_options::follow_directory_symlink;
    TEST_ASSERT((d_opt & fs::directory_options::follow_directory_symlink) != fs::directory_options::none);

    // file_status
    fs::file_status st(fs::file_type::regular, fs::perms::owner_read);
    TEST_ASSERT(st.type() == fs::file_type::regular);
    TEST_ASSERT(st.permissions() == fs::perms::owner_read);
    TEST_ASSERT(fs::is_regular_file(st));
    TEST_ASSERT(!fs::is_directory(st));
}

/// <summary>
/// 測試 檔案與目錄實體操作 API。
/// </summary>
void test_fs_operations_and_queries() {
    std::cout << "  [Subtest] test_fs_operations_and_queries..." << std::endl;

    fs::path temp_base = fs::temp_directory_path();
    TEST_ASSERT(!temp_base.empty());
    TEST_ASSERT(fs::exists(temp_base));
    TEST_ASSERT(fs::is_directory(temp_base));

    fs::path test_dir = temp_base / "cpp_compat_test_fs_dir";
    std::error_code ec;

    // 清理殘留舊目錄
    if (fs::exists(test_dir)) {
        fs::remove_all(test_dir, ec);
    }

    // 1. 建立目錄
    bool created = fs::create_directory(test_dir);
    TEST_ASSERT(created);
    TEST_ASSERT(fs::exists(test_dir));
    TEST_ASSERT(fs::is_directory(test_dir));
    TEST_ASSERT(fs::is_empty(test_dir));

    // 2. 建立巢狀目錄
    fs::path nested_dir = test_dir / "nested" / "inner";
    bool created_nested = fs::create_directories(nested_dir);
    TEST_ASSERT(created_nested);
    TEST_ASSERT(fs::exists(nested_dir));
    TEST_ASSERT(fs::is_directory(nested_dir));

    // 3. 檔案建立與讀寫
    fs::path file1 = test_dir / "test1.txt";
    {
        std::ofstream ofs(file1.string());
        ofs << "Hello CPP-Compat Filesystem!";
    }
    TEST_ASSERT(fs::exists(file1));
    TEST_ASSERT(fs::is_regular_file(file1));
    TEST_ASSERT(!fs::is_directory(file1));
    TEST_ASSERT(!fs::is_empty(file1));

    uintmax_t sz = fs::file_size(file1);
    TEST_ASSERT(sz == 28);

    // 4. 複製檔案
    fs::path file2 = test_dir / "test2.txt";
    bool copied = fs::copy_file(file1, file2);
    TEST_ASSERT(copied);
    TEST_ASSERT(fs::exists(file2));
    TEST_ASSERT(fs::file_size(file2) == sz);

    // 5. 重新命名檔案
    fs::path file3 = test_dir / "test3.txt";
    fs::rename(file2, file3);
    TEST_ASSERT(!fs::exists(file2));
    TEST_ASSERT(fs::exists(file3));

    // 6. 狀態查詢與 current_path
    fs::file_status status1 = fs::status(file1);
    TEST_ASSERT(fs::status_known(status1));
    TEST_ASSERT(status1.type() == fs::file_type::regular);

    fs::path cur = fs::current_path();
    TEST_ASSERT(!cur.empty());
    TEST_ASSERT(fs::exists(cur));

    // 7. space 空間查詢
    fs::space_info sp = fs::space(temp_base);
    TEST_ASSERT(sp.capacity > 0);
    TEST_ASSERT(sp.free > 0);

    // 8. 錯誤處理: 嘗試讀取不存在的檔案
    fs::path non_existent = test_dir / "no_such_file.xyz";
    TEST_ASSERT(!fs::exists(non_existent));
    ec.clear();
    uintmax_t bad_sz = fs::file_size(non_existent, ec);
    TEST_ASSERT(ec);
    (void)bad_sz;

    TEST_ASSERT_THROWS(fs::file_size(non_existent), fs::filesystem_error);

    // 9. 移除檔案與目錄樹
    bool rem1 = fs::remove(file3);
    TEST_ASSERT(rem1);
    TEST_ASSERT(!fs::exists(file3));

    uintmax_t count_removed = fs::remove_all(test_dir);
    TEST_ASSERT(count_removed > 0);
    TEST_ASSERT(!fs::exists(test_dir));
}

/// <summary>
/// 測試 directory_iterator 與 recursive_directory_iterator。
/// </summary>
void test_fs_directory_iterators() {
    std::cout << "  [Subtest] test_fs_directory_iterators..." << std::endl;

    fs::path temp_base = fs::temp_directory_path();
    fs::path root = temp_base / "cpp_compat_test_iter_dir";
    std::error_code ec;

    if (fs::exists(root)) {
        fs::remove_all(root, ec);
    }

    fs::create_directories(root / "sub1");
    fs::create_directories(root / "sub2");

    {
        std::ofstream( (root / "file1.txt").string() ) << "1";
        std::ofstream( (root / "file2.txt").string() ) << "22";
        std::ofstream( (root / "sub1" / "nested.txt").string() ) << "333";
    }

    // 1. 單層目錄走訪: directory_iterator
    std::vector<std::string> top_entries;
    for (const auto& entry : fs::directory_iterator(root)) {
        top_entries.push_back(entry.path().filename().string());
    }
    std::sort(top_entries.begin(), top_entries.end());
    TEST_ASSERT(top_entries.size() == 4);
    TEST_ASSERT(std::find(top_entries.begin(), top_entries.end(), "file1.txt") != top_entries.end());
    TEST_ASSERT(std::find(top_entries.begin(), top_entries.end(), "file2.txt") != top_entries.end());
    TEST_ASSERT(std::find(top_entries.begin(), top_entries.end(), "sub1") != top_entries.end());
    TEST_ASSERT(std::find(top_entries.begin(), top_entries.end(), "sub2") != top_entries.end());

    // 2. 遞迴走訪: recursive_directory_iterator
    std::vector<std::string> all_files;
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (entry.is_regular_file()) {
            all_files.push_back(entry.path().filename().string());
        }
    }
    std::sort(all_files.begin(), all_files.end());
    TEST_ASSERT(all_files.size() == 3);
    TEST_ASSERT(all_files[0] == "file1.txt");
    TEST_ASSERT(all_files[1] == "file2.txt");
    TEST_ASSERT(all_files[2] == "nested.txt");

    // 清理
    fs::remove_all(root, ec);
}

} // namespace

/// <summary>
/// compat::filesystem 單元測試進入點。
/// </summary>
void run_test_filesystem() {
    std::cout << "[TEST] Running test_filesystem..." << std::endl;
    test_fs_path_syntax_and_decomposition();
    test_fs_enums_and_bitmasks();
    test_fs_operations_and_queries();
    test_fs_directory_iterators();
    std::cout << "[PASS] test_filesystem passed." << std::endl;
}
