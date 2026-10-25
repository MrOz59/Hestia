#pragma once

// Which Hestia build is newer, kept free of Qt and the network so it can be
// checked on its own.
//
// A release is a plain version, 0.1.0. Every other CI build appends the commit
// it was built from as SemVer build metadata, 0.1.0+abc1234, the way Hermes
// stamps its nightlies (see .github/workflows/build.yml). The two follow
// different channels: a release build is told about newer releases, and a
// nightly about a nightly built from another commit, which the rolling
// "nightly" release always carries in its name.

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace HestiaUpdate {

struct BuildVersion {
    std::vector<int> release;  ///< 0.1.0 -> {0, 1, 0}
    std::string commit;        ///< Lower-case build metadata; empty for a release.

    bool isNightly() const { return !commit.empty(); }
};

/**
 * Parse "0.1.0", "v0.1.0" or "0.1.0+abc1234". Returns nothing for anything
 * that does not start with a dotted number, such as an empty tag.
 */
inline std::optional<BuildVersion> parse(std::string_view text)
{
    if (!text.empty() && (text.front() == 'v' || text.front() == 'V')) {
        text.remove_prefix(1);
    }

    BuildVersion version;
    const auto plus = text.find('+');
    std::string_view release = text.substr(0, plus);
    if (plus != std::string_view::npos) {
        for (char ch : text.substr(plus + 1)) {
            if (!std::isxdigit(static_cast<unsigned char>(ch))) {
                return std::nullopt;
            }
            version.commit.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        if (version.commit.empty()) {
            return std::nullopt;
        }
    }

    while (true) {
        const auto dot = release.find('.');
        const std::string_view part = release.substr(0, dot);
        if (part.empty() || part.size() > 6) {
            return std::nullopt;
        }
        int value = 0;
        for (char ch : part) {
            if (!std::isdigit(static_cast<unsigned char>(ch))) {
                return std::nullopt;
            }
            value = value * 10 + (ch - '0');
        }
        version.release.push_back(value);
        if (dot == std::string_view::npos) {
            break;
        }
        release.remove_prefix(dot + 1);
    }
    return version;
}

/** Back to text: 0.1.0, or 0.1.0+abc1234 for a nightly. */
inline std::string format(const BuildVersion& version)
{
    std::string text;
    for (size_t i = 0; i < version.release.size(); i++) {
        if (i > 0) {
            text.push_back('.');
        }
        text += std::to_string(version.release[i]);
    }
    if (version.isNightly()) {
        text += '+' + version.commit;
    }
    return text;
}

/** -1, 0 or 1; missing components count as 0, so 0.1 == 0.1.0. */
inline int compareRelease(const std::vector<int>& a, const std::vector<int>& b)
{
    const size_t count = a.size() > b.size() ? a.size() : b.size();
    for (size_t i = 0; i < count; i++) {
        const int left = i < a.size() ? a[i] : 0;
        const int right = i < b.size() ? b[i] : 0;
        if (left != right) {
            return left < right ? -1 : 1;
        }
    }
    return 0;
}

/** Whether two abbreviated commits name the same one. */
inline bool sameCommit(const std::string& a, const std::string& b)
{
    if (a.empty() || b.empty()) {
        return false;
    }
    return a.size() <= b.size() ? b.compare(0, a.size(), a) == 0 : a.compare(0, b.size(), b) == 0;
}

/** A release build should move to `latest` when it is a newer release. */
inline bool releaseIsNewer(const BuildVersion& current, const BuildVersion& latest)
{
    return compareRelease(current.release, latest.release) < 0;
}

/**
 * A nightly build should move to `nightly` when that one is based on a newer
 * release, or on the same release from another commit. A nightly based on an
 * older release than the running build - a local build ahead of CI - is not.
 */
inline bool nightlyIsNewer(const BuildVersion& current, const BuildVersion& nightly)
{
    if (!nightly.isNightly()) {
        return false;
    }
    const int order = compareRelease(current.release, nightly.release);
    return order < 0 || (order == 0 && !sameCommit(current.commit, nightly.commit));
}

/**
 * The build version in the rolling nightly release's name,
 * "Hestia nightly (v0.1.0+abc1234)". Empty when the name carries none, as the
 * nightlies published before builds were stamped did not.
 */
inline std::optional<BuildVersion> versionInNightlyName(std::string_view name)
{
    for (size_t start = name.find('v'); start != std::string_view::npos; start = name.find('v', start + 1)) {
        size_t end = start + 1;
        while (end < name.size() &&
               (std::isalnum(static_cast<unsigned char>(name[end])) || name[end] == '.' || name[end] == '+')) {
            end++;
        }
        if (auto version = parse(name.substr(start, end - start)); version && version->isNightly()) {
            return version;
        }
    }
    return std::nullopt;
}

}  // namespace HestiaUpdate
