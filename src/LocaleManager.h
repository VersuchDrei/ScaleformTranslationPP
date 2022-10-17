#pragma once

class LocaleManager : public RE::GFxTranslator {
public:
    static std::wstring ConvertStringToWString(const std::string& a_str);
    static std::string ConvertWStringToString(const std::wstring& a_str);

    LocaleManager() = delete;
    LocaleManager(const RE::BSTranslator& a_translator);
    LocaleManager(const LocaleManager&) = default;
    LocaleManager(LocaleManager&&) = default;
    virtual ~LocaleManager() = default;

    LocaleManager& operator=(const LocaleManager&) = default;
    LocaleManager& operator=(LocaleManager&&) = default;

    virtual void Translate(TranslateInfo* a_translateInfo) override;

    void Dump();
    std::wstring GetLocalization(std::wstring a_key);
    std::string GetLocalization(std::string a_key);

private:
    using LocalizationMap = std::unordered_map<std::wstring, std::wstring>;
    using size_type = std::wstring::size_type;

    void LoadLocalizationStrings();
    void LoadLocalizationMap(const RE::BSTranslator& a_translator);
    void FindFiles(const std::filesystem::path& a_path, const std::wregex& a_pattern, bool a_english);
    void ReadFromFile(const std::filesystem::path& a_path, bool a_english);
    LocalizationMap& GetLocalizationMap();
    std::wstring GetLocalizationInternal(const std::wstring& a_key);
    std::optional<std::wstring> SanitizeKey(std::wstring a_key);
    bool GetNestedLocalizations(const std::wstring& a_key, std::stack<size_type>& a_stack,
                                std::queue<std::wstring>& a_queue);
    std::optional<std::wstring> FindLocalization(const std::wstring& a_key);
    bool InsertLocalizations(std::wstring& a_localization, std::stack<size_type>& a_stack,
                             std::queue<std::wstring>& a_queue);

    LocalizationMap _localizations_ENG;
    LocalizationMap _localizations_LOC;
};