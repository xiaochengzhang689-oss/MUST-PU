#ifndef MEMBER4_H
#define MEMBER4_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>

using namespace std;

enum class ReportStatus {
    SUCCESS,
    DUPLICATE,
    INVALID
};

struct NormalizeResult {
    string text;
    vector<size_t> startPos;
    vector<size_t> endPos;
};

struct ReportRecord {
    int postId;
    string userId;
    string reason;
};

struct CheckResult {
    bool allowed;
    string message;
    vector<string> matchedWords;
};

class SensitiveWordFilter {
public:
    bool loadFromFile(const string& path);
    void addWord(const string& word);
    size_t wordCount() const;
    vector<string> findMatches(const string& text) const;
    bool isClean(const string& text) const;
    string mask(const string& text) const;
private:
    vector<string> wordList;
};

class ReportManager {
public:
    explicit ReportManager(int threshold = 3);
    ReportStatus report(int postId, const string& userId, const string& reason);
    int getReportCount(int postId) const;
    bool isMarked(int postId) const;
    vector<int> getMarkedPosts() const;
    const vector<ReportRecord>& getAllRecords() const;
    bool saveToFile(const string& path) const;
    bool loadFromFile(const string& path);
private:
    int threshold;
    vector<ReportRecord> records;
    map<int, set<string>> reporters;
};

class ContentSecurity {
public:
    ContentSecurity(SensitiveWordFilter& filter, ReportManager& reporter);
    CheckResult checkPost(const string& title, const string& content) const;
    CheckResult checkReply(const string& content) const;
    ReportStatus reportPost(int postId, const string& userId, const string& reason);
    bool isPostMarked(int postId) const;
private:
    SensitiveWordFilter& filter;
    ReportManager& reporter;
    CheckResult checkInternal(const string& text) const;
};

void runMember4UnitTest();

#endif
