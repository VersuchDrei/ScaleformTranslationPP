#include "LocaleManager.h"

#include <Windows.h>

std::wstring LocaleManager::ConvertStringToWString(const std::string& a_str) {
    if (a_str.empty()) {
        return std::wstring();
    }

    auto size =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, a_str.c_str(), static_cast<int>(a_str.length()), nullptr, 0);
    bool err = size == 0;
    if (!err) {
        std::wstring strTo;
        strTo.resize(size);
        err = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, a_str.c_str(), static_cast<int>(a_str.length()),
                                  strTo.data(), size) == 0;
        if (!err) {
            return strTo;
        }
    }

    if (err) {
        logger::error("MultiByteToWideChar failed with error code ({})", GetLastError());
    }

    return std::wstring();
}

std::string LocaleManager::ConvertWStringToString(const std::wstring& a_str) {
    if (a_str.empty()) {
        return std::string();
    }

    auto size =
        WideCharToMultiByte(CP_UTF8, 0, a_str.c_str(), static_cast<int>(a_str.length()), nullptr, 0, nullptr, nullptr);
    bool err = size == 0;
    if (!err) {
        std::string strTo;
        strTo.resize(size);
        err = WideCharToMultiByte(CP_UTF8, 0, a_str.c_str(), static_cast<int>(a_str.length()), strTo.data(), size,
                                  nullptr, nullptr) == 0;
        if (!err) {
            return strTo;
        }
    }

    if (err) {
        logger::error("WideCharToMultiByte failed with error code ({})", GetLastError());
    }

    return std::string();
}

LocaleManager::LocaleManager(const RE::BSTranslator& a_translator) : _localizations_ENG(), _localizations_LOC() {
    LoadLocalizationMap(a_translator);
    LoadLocalizationStrings();
}

void LocaleManager::Translate(TranslateInfo* a_translateInfo) {
    auto key = a_translateInfo->GetKey();
    if (key && key[0] == L'$') {
        auto localization = GetLocalization(key);
        a_translateInfo->SetResult(localization.c_str(), localization.length());
    }
}

void LocaleManager::Dump() {
    std::string key;
    std::string value;
    for (auto& pair : GetLocalizationMap()) {
        key = ConvertWStringToString(pair.first);
        value = ConvertWStringToString(pair.second);
        logger::debug("{}: {}", key.c_str(), value.c_str());
    }
}

std::wstring LocaleManager::GetLocalization(std::wstring a_key) { return GetLocalizationInternal(a_key); }

std::string LocaleManager::GetLocalization(std::string a_key) {
    auto str = ConvertStringToWString(a_key);
    str = GetLocalization(str);
    return ConvertWStringToString(str);
}

void LocaleManager::LoadLocalizationStrings() {
    constexpr wchar_t REGEX_PREFIX[] = L".*_";
    constexpr wchar_t ENGLISH[] = L"ENGLISH";
    constexpr wchar_t REGEX_POSTFIX[] = L"\\.txt$";
    constexpr auto REGEX_FLAGS = std::regex_constants::grep | std::regex_constants::icase;

    std::filesystem::path path("data/interface/translations");

    std::wstring pattern(REGEX_PREFIX);
    std::wstring wLanguage(ENGLISH);
    auto setting = RE::GetINISetting("sLanguage:General");
    if (setting) {
        auto u8Language = setting->GetString();
        wLanguage = ConvertStringToWString(u8Language);
    }
    pattern += wLanguage;
    pattern += REGEX_POSTFIX;
    std::wregex regex(pattern, REGEX_FLAGS);

    bool english = _wcsicmp(ENGLISH, wLanguage.c_str()) == 0;

    FindFiles(path, regex, english);
    if (!english) {
        pattern = REGEX_PREFIX;
        pattern += ENGLISH;
        pattern += REGEX_POSTFIX;
        regex.assign(pattern, REGEX_FLAGS);
        FindFiles(path, regex, true);
    }
}

void LocaleManager::LoadLocalizationMap(const RE::BSTranslator& a_translator) {
    auto& localizations = GetLocalizationMap();
    localizations.reserve(a_translator.translationMap.size());
    std::wstring key;
    std::wstring value;
    std::optional<std::wstring> sanitizedKey;
    for (auto& entry : a_translator.translationMap) {
        key = entry.first;
        sanitizedKey = SanitizeKey(key);
        if (sanitizedKey) {
            key = std::move(*sanitizedKey);
        }
        value = entry.second;
        localizations.insert({std::move(key), std::move(value)});
    }
}

void LocaleManager::FindFiles(const std::filesystem::path& a_path, const std::wregex& a_pattern, bool a_english) {
    std::error_code err;
    if (!std::filesystem::exists(a_path, err)) {
        return;
    }

    std::filesystem::path fileName;
    for (auto& dirEntry : std::filesystem::directory_iterator(a_path)) {
        fileName = dirEntry.path().filename();
        if (std::regex_match(fileName.native(), a_pattern)) {
            ReadFromFile(dirEntry.path(), a_english);
        }
    }
}

void LocaleManager::ReadFromFile(const std::filesystem::path& a_path, bool a_english) {
    constexpr auto CVT_MODE = std::codecvt_mode(std::little_endian | std::consume_header);
    constexpr auto NPOS = std::wstring::npos;

    auto& localizations = a_english ? _localizations_ENG : _localizations_LOC;
    std::wifstream inFile(a_path);
    inFile.imbue(std::locale(inFile.getloc(), new std::codecvt_utf16<wchar_t, 0x10FFFF, CVT_MODE>));  // UCS-2 LE w/ BOM
    std::wstring line;
    std::wstring key;
    std::wstring value;
    if (!inFile.is_open()) {
        logger::error("Failed to open file \"{}\"!", a_path.string().c_str());
        return;
    }

    while (std::getline(inFile, line)) {
        if (!line.empty() && line.back() == L'\r') {
            line.pop_back();  // discard carriage return
        }
        if (line.empty() || line.front() != L'$') {
            continue;
        }
        key.clear();
        value.clear();

        auto pos = line.find_first_of(L'\t');
        if (pos != NPOS) {
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
        }

        if (!key.empty() && !value.empty()) {
            auto sanitizedKey = SanitizeKey(key);
            if (sanitizedKey) {
                key = std::move(*sanitizedKey);
            }
            localizations.insert(std::make_pair(std::move(key), std::move(value)));
        }
    }
}

LocaleManager::LocalizationMap& LocaleManager::GetLocalizationMap() {
    return _localizations_LOC.empty() ? _localizations_ENG : _localizations_LOC;
}

std::wstring LocaleManager::GetLocalizationInternal(const std::wstring& a_key) {
    if (a_key.empty() || a_key[0] != L'$') {
        return a_key;
    }

    auto sanitizedKey = SanitizeKey(a_key);
    if (!sanitizedKey) {
        return a_key;
    }

    auto localization = FindLocalization(*sanitizedKey);
    if (!localization) {
        return a_key;
    }

    std::stack<size_type> stack;
    std::queue<std::wstring> queue;
    if (!GetNestedLocalizations(a_key, stack, queue)) {
        return *localization;
    }

    while (!stack.empty()) {
        stack.pop();
    }
    InsertLocalizations(*localization, stack, queue);
    return *localization;
}

std::optional<std::wstring> LocaleManager::SanitizeKey(std::wstring a_key) {
    std::stack<size_type> stack;
    for (size_type pos = 0; pos < a_key.size(); ++pos) {
        switch (a_key[pos]) {
            case L'{':
                stack.push(pos);
                break;
            case L'}': {
                switch (stack.size()) {
                    case 0:
                        return std::nullopt;
                    case 1: {
                        size_type last = stack.top();
                        stack.pop();
                        auto off = last + 1;
                        auto count = pos - last - 1;
                        if (count > 0) {
                            a_key.replace(off, count, L"");
                        }
                        pos = off;
                    } break;
                    default:
                        stack.pop();
                }
                break;
            }
        }
    }

    if (!a_key.empty() && a_key.back() == L'\r') {
        a_key.pop_back();
    }

    return std::make_optional(a_key);
}

bool LocaleManager::GetNestedLocalizations(const std::wstring& a_key, std::stack<size_type>& a_stack,
                                           std::queue<std::wstring>& a_queue) {
    for (size_type pos = 0; pos < a_key.size(); ++pos) {
        switch (a_key[pos]) {
            case L'{':
                a_stack.push(pos);
                break;
            case L'}': {
                switch (a_stack.size()) {
                    case 0:
                        return false;
                    case 1: {
                        size_type last = a_stack.top();
                        a_stack.pop();
                        auto off = last + 1;
                        auto count = pos - last - 1;
                        if (count == 0) {
                            return false;  // nothing to replace {} with
                        }
                        auto subStr = a_key.substr(off, count);
                        a_queue.push(GetLocalizationInternal(subStr));
                    } break;
                    default:
                        a_stack.pop();
                }
                break;
            }
        }
    }

    return true;
}

std::optional<std::wstring> LocaleManager::FindLocalization(const std::wstring& a_key) {
    auto& localizations = GetLocalizationMap();
    auto it = localizations.find(a_key);
    if (it == localizations.end()) {
        if (&localizations != &_localizations_ENG) {
            it = _localizations_ENG.find(a_key);
            if (it == _localizations_ENG.end()) {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }

    return std::make_optional(it->second);
}

bool LocaleManager::InsertLocalizations(std::wstring& a_localization, std::stack<size_type>& a_stack,
                                        std::queue<std::wstring>& a_queue) {
    for (size_type pos = 0; pos < a_localization.size(); ++pos) {
        switch (a_localization[pos]) {
            case L'{':
                a_stack.push(pos);
                break;
            case L'}': {
                if (a_stack.empty() || a_queue.empty()) {
                    return false;
                }

                size_type beg = a_stack.top();
                a_stack.pop();
                auto subStr = a_queue.front();
                a_queue.pop();

                a_localization.replace(beg, pos - beg + 1, subStr);
                pos = beg;
            } break;
        }
    }

    return true;
}