#include "../ytdlp-interface/gui.hpp"
#include "../ytdlp-interface/settings_io.hpp"
#include <iostream>

namespace
{
    unsigned checks = 0;
    void require(bool condition, const char *message)
    {
        ++checks;
        if(!condition) throw std::runtime_error(message);
    }

    void configuration_tests()
    {
        using nlohmann::json;
        settings_t defaults;
        json full;
        defaults.to_json(full);
        settings_t roundtrip;
        roundtrip.from_json(full);
        require(roundtrip.max_concurrent_downloads == 1, "default config roundtrip");

        settings_t partial;
        partial.from_json(json{{"outpath", "下载/测试😀"}, {"cbtheme", 1}, {"window", {{"x", 42}}}, {"cb_zeropadding", true}});
        require(partial.outpath.lexically_normal() == fs::u8path("下载/测试😀").lexically_normal(), "partial config keeps Unicode path");
        require(partial.cbtheme == 1 && partial.winrect.x == 42, "partial nested config keeps values");
        require(partial.max_concurrent_downloads == 1, "missing fields keep defaults");

        settings_t legacy;
        legacy.from_json(json{{"pref_res", 2}});
        require(legacy.pref_res == 4, "legacy resolution migration survives default completion");
        settings_t modern;
        modern.from_json(json{{"pref_res", 2}, {"json_hide_null", false}});
        require(modern.pref_res == 2, "modern resolution is not migrated");

        for(const auto &bad : std::vector<json>{
            json::array(), json{{"ratelim", "fast"}}, json{{"window", "invalid"}, {"cb_zeropadding", true}},
            json{{"cbtheme", 200}}, json{{"pref_res", 999}},
            json{{"presets", json::array({{{"name", "broken"}, {"data", 12}}})}}
        })
        {
            bool rejected = false;
            try { settings_t candidate; candidate.from_json(bad); }
            catch(const std::exception &) { rejected = true; }
            require(rejected, "malformed config rejected with a catchable exception");
        }

        auto valid_presets = full;
        valid_presets["presets"] = json::array({{{"name", "partial"}, {"data", {{"outpath", "test"}}}}});
        settings_t with_preset;
        with_preset.from_json(valid_presets);
        require(GUI::conf_presets.at("partial").outpath == fs::path("test"), "partial preset completed safely");
        GUI::conf_presets.clear();

        const auto fixture = fs::current_path() / "invalid-config-fixture.json";
        { std::ofstream out(fixture); out << "{ broken"; }
        const auto first = settings_io::backup_invalid(fixture);
        const auto second = settings_io::backup_invalid(fixture);
        require(first != second && fs::exists(first) && fs::exists(second), "backups do not overwrite previous backups");
        std::ifstream in(first);
        const std::string contents{std::istreambuf_iterator<char>(in), {}};
        require(contents == "{ broken", "invalid config bytes preserved");
        in.close();
        fs::remove(fixture); fs::remove(first); fs::remove(second);
    }

    void process_tests(const std::wstring &self)
    {
        auto command = [&](DWORD code) { return L"\"" + self + L"\" --child " + std::to_wstring(code); };
        util::process_result result;
        auto output = util::run_piped_process(command(0), nullptr, nullptr, nullptr, nullptr, "", &result);
        require(result.succeeded() && output.find("fixture output") != std::string::npos, "successful process captures output");
        std::atomic_bool working {true};
        auto progress = [](ULONGLONG, ULONGLONG, std::string, int, int) {};
        for(DWORD code : {1UL, 2UL, 3UL, 259UL, 0xC0000409UL})
        {
            output = util::run_piped_process(command(code), &working, nullptr, progress, nullptr, "", &result);
            require(!result.succeeded() && result.exit_code_available && result.exit_code == code, "nonzero exit remains failure");
            require(output == "failed", "legacy progress callers also receive failure");
        }
        util::run_piped_process(L"\"Z:\\not-a-real-program-for-regression.exe\"", nullptr, nullptr, nullptr, nullptr, "", &result);
        require(!result.started && !result.succeeded() && result.error_code != 0, "launch failure is reported");
        std::thread cancel([&] { Sleep(150); working = false; });
        util::run_piped_process(L"\"" + self + L"\" --sleep", &working, nullptr, nullptr, nullptr, "", &result);
        cancel.join();
        require(result.cancelled && !result.succeeded(), "cancelled process never succeeds");
    }
}

int wmain(int argc, wchar_t **argv)
{
    if(argc == 3 && std::wstring_view(argv[1]) == L"--child")
    {
        std::cout << "fixture output\n" << std::flush;
        ExitProcess(static_cast<DWORD>(std::stoull(argv[2])));
    }
    if(argc == 2 && std::wstring_view(argv[1]) == L"--sleep")
    { Sleep(INFINITE); return 0; }
    try
    {
        configuration_tests();
        process_tests(util::app_path().wstring());
        std::cout << checks << " runtime regression checks passed\n";
        return 0;
    }
    catch(const std::exception &error)
    { std::cerr << "Regression failed: " << error.what() << '\n'; return 1; }
}
