#include <compat/Compat.hpp>
#include <cstdint>
#include <iostream>
#include <string>

/// <summary>
/// 安全整數除法純函數，除數為零時快速失敗並回傳錯誤描述。
/// </summary>
/// <param name="numerator">被除數</param>
/// <param name="denominator">除數</param>
/// <returns>若成功回傳商數，若除數為 0 則回傳 unexpected 錯誤描述</returns>
[[nodiscard]] compat::expected<int32_t, compat::string_view> SafeDivide(int32_t numerator, int32_t denominator) noexcept {
    if (denominator == 0) {
        return compat::unexpected<compat::string_view>("Error: Division by zero");
    }
    return numerator / denominator;
}

/// <summary>
/// 應用程式進入點，展示相容層各項特性與業務邏輯。
/// </summary>
/// <returns>執行狀態碼</returns>
int main() {
    compat::println("==================================================");
    compat::println("    CPP-Compat Cross-Standard Demonstration App   ");
    compat::println("==================================================");

    // 1. 展示業務邏輯 SafeDivide 與 expected 機制
    int32_t num = 100;
    int32_t den = 5;
    auto div_ok = SafeDivide(num, den);
    if (div_ok) {
        compat::println("[SafeDivide OK] {} / {} = {}", num, den, div_ok.value());
    }

    auto div_err = SafeDivide(num, 0);
    if (!div_err) {
        compat::println("[SafeDivide Error] {} / 0 -> {}", num, div_err.error());
    }

    // 2. 展示型態轉換 parse<int32_t> 與 Fail-fast 機制
    compat::string_view valid_num_str = "2026";
    auto parsed_ok = compat::parse<int32_t>(valid_num_str);
    if (parsed_ok) {
        compat::println("[Parse OK] Parsed '{}' -> int32_t: {}", valid_num_str, parsed_ok.value());
    }

    compat::string_view invalid_num_str = "99999999999999999";
    auto parsed_overflow = compat::parse<int32_t>(invalid_num_str);
    if (!parsed_overflow) {
        compat::println("[Parse Fail-Fast] Parsing '{}' failed: {}", invalid_num_str, parsed_overflow.error());
    }

    // 3. 展示 compat::format 字串格式化
    std::string formatted_message = compat::format("Formatted report: Target year = {}, Status = {}", 2026, "Operational");
    compat::println("[Format] {}", formatted_message);

    compat::println("==================================================");
    compat::println("Demonstration completed successfully!");
    return 0;
}
