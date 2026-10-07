#include "member4.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>

using namespace std;

static const unsigned INVALID_UTF8 = 0xFFFFFFFFu;

static unsigned decodeUtf8(const string& s, size_t i, size_t& length) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    size_t n;
    unsigned cp;
    if (c < 0x80) { length = 1; return c; }
    if (c >= 0xC2 && c <= 0xDF) { n = 2; cp = c & 0x1F; }
    else if (c >= 0xE0 && c <= 0xEF) { n = 3; cp = c & 0x0F; }
    else if (c >= 0xF0 && c <= 0xF4) { n = 4; cp = c & 0x07; }
    else { length = 1; return INVALID_UTF8; }
    if (i + n > s.size()) { length = 1; return INVALID_UTF8; }
    for (size_t k = 1; k < n; ++k) {
        unsigned char d = static_cast<unsigned char>(s[i + k]);
        if ((d & 0xC0) != 0x80) { length = 1; return INVALID_UTF8; }
        cp = (cp << 6) | (d & 0x3F);
    }
    length = n;
    return cp;
}

static bool isSeparator(unsigned cp) {
    if (cp == 0x3005 || cp == 0x3006 || cp == 0x3007) return false;
    return (cp >= 0xA0 && cp <= 0xBF) || cp == 0xD7 || cp == 0xF7 ||
        (cp >= 0x2000 && cp <= 0x206F) ||
        (cp >= 0x2190 && cp <= 0x2BFF) ||
        (cp >= 0x3000 && cp <= 0x303F) ||
        (cp >= 0xFE00 && cp <= 0xFE0F) || (cp >= 0xFE30 && cp <= 0xFE4F) || cp == 0xFEFF ||
        (cp >= 0xFF01 && cp <= 0xFF0F) || (cp >= 0xFF1A && cp <= 0xFF20) ||
        (cp >= 0xFF3B && cp <= 0xFF40) || (cp >= 0xFF5B && cp <= 0xFF65) ||
        (cp >= 0x1F000 && cp <= 0x1FAFF);
}

static NormalizeResult normalizeText(const string& s) {
    NormalizeResult r;
    size_t i = 0;
    while (i < s.size()) {
        size_t len;
        unsigned cp = decodeUtf8(s, i, len);
        string output;
        if (cp == INVALID_UTF8) {
            output = s.substr(i, len);
        } else {
            char a = 0;
            if (cp < 128) a = static_cast<char>(cp);
            else if (cp >= 0xFF10 && cp <= 0xFF19) a = static_cast<char>('0' + (cp - 0xFF10));
            else if (cp >= 0xFF21 && cp <= 0xFF3A) a = static_cast<char>('a' + (cp - 0xFF21));
            else if (cp >= 0xFF41 && cp <= 0xFF5A) a = static_cast<char>('a' + (cp - 0xFF41));
            if (a) {
                if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
                if ((a >= '0' && a <= '9') || (a >= 'a' && a <= 'z')) output = string(1, a);
            } else if (!isSeparator(cp)) {
                output = s.substr(i, len);
            }
        }
        for (char b : output) { r.text += b; r.startPos.push_back(i); r.endPos.push_back(i + len); }
        i += len;
    }
    return r;
}

static string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\n' || s[a] == '\r')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\n' || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

static bool isBlank(const string& s) {
    for (size_t i = 0; i < s.size();) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++i;
        else if (s.compare(i, 3, "\xE3\x80\x80") == 0) i += 3;
        else return false;
    }
    return true;
}

static bool parsePositiveInt(const string& s, int& result) {
    string t = trim(s);
    if (t.empty() || t.size() > 9) return false;
    for (char c : t) if (c < '0' || c > '9') return false;
    result = stoi(t);
    return true;
}

static string csvEscape(const string& s) {
    string r = "\"";
    for (char c : s) { if (c == '"') r += '"'; r += c; }
    return r + "\"";
}

static vector<vector<string>> parseCSV(string content) {
    if (content.compare(0, 3, "\xEF\xBB\xBF") == 0) content.erase(0, 3);
    vector<vector<string>> rows;
    vector<string> row;
    string cell;
    bool inQuote = false;
    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        if (inQuote) {
            if (c == '"') {
                if (i + 1 < content.size() && content[i + 1] == '"') { cell += '"'; ++i; }
                else inQuote = false;
            } else {
                cell += c;
            }
        } else if (c == '"' && cell.empty()) {
            inQuote = true;
        } else if (c == ',') {
            row.push_back(cell); cell.clear();
        } else if (c == '\n' || c == '\r') {
            if (c == '\r' && i + 1 < content.size() && content[i + 1] == '\n') ++i;
            row.push_back(cell); cell.clear();
            rows.push_back(row); row.clear();
        } else {
            cell += c;
        }
    }
    if (!cell.empty() || !row.empty()) { row.push_back(cell); rows.push_back(row); }
    return rows;
}

bool SensitiveWordFilter::loadFromFile(const string& path) {
    ifstream in(path, ios::binary);
    if (!in) return false;
    string line;
    bool firstLine = true;
    while (getline(in, line)) {
        if (firstLine && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
        firstLine = false;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        addWord(line);
    }
    return true;
}

void SensitiveWordFilter::addWord(const string& word) {
    string n = normalizeText(word).text;
    if (!n.empty() && find(wordList.begin(), wordList.end(), n) == wordList.end())
        wordList.push_back(n);
}

size_t SensitiveWordFilter::wordCount() const {
    return wordList.size();
}

vector<string> SensitiveWordFilter::findMatches(const string& text) const {
    vector<string> matches;
    string n = normalizeText(text).text;
    for (const auto& w : wordList)
        if (n.find(w) != string::npos) matches.push_back(w);
    return matches;
}

bool SensitiveWordFilter::isClean(const string& text) const {
    return findMatches(text).empty();
}

string SensitiveWordFilter::mask(const string& text) const {
    NormalizeResult g = normalizeText(text);
    vector<char> flagged(text.size(), 0);
    for (const auto& w : wordList) {
        size_t pos = 0;
        while ((pos = g.text.find(w, pos)) != string::npos) {
            size_t a = g.startPos[pos], b = g.endPos[pos + w.size() - 1];
            for (size_t k = a; k < b && k < flagged.size(); ++k) flagged[k] = 1;
            ++pos;
        }
    }
    string output;
    for (size_t i = 0; i < text.size();) {
        size_t len;
        decodeUtf8(text, i, len);
        if (flagged[i]) output += '*';
        else output += text.substr(i, len);
        i += len;
    }
    return output;
}

ReportManager::ReportManager(int threshold) : threshold(threshold) {}

ReportStatus ReportManager::report(int postId, const string& userId, const string& reason) {
    string uid = trim(userId);
    if (postId <= 0 || uid.empty()) return ReportStatus::INVALID;
    if (reporters[postId].count(uid)) return ReportStatus::DUPLICATE;
    reporters[postId].insert(uid);
    records.push_back({ postId, uid, reason });
    return ReportStatus::SUCCESS;
}

int ReportManager::getReportCount(int postId) const {
    auto it = reporters.find(postId);
    return it == reporters.end() ? 0 : static_cast<int>(it->second.size());
}

bool ReportManager::isMarked(int postId) const {
    return getReportCount(postId) >= threshold;
}

vector<int> ReportManager::getMarkedPosts() const {
    vector<int> r;
    for (const auto& kv : reporters)
        if (static_cast<int>(kv.second.size()) >= threshold) r.push_back(kv.first);
    return r;
}

const vector<ReportRecord>& ReportManager::getAllRecords() const {
    return records;
}

bool ReportManager::saveToFile(const string& path) const {
    ofstream out(path, ios::binary);
    if (!out) return false;
    for (const auto& r : records)
        out << r.postId << ',' << csvEscape(r.userId) << ',' << csvEscape(r.reason) << '\n';
    return static_cast<bool>(out);
}

bool ReportManager::loadFromFile(const string& path) {
    ifstream in(path, ios::binary);
    if (!in) return false;
    stringstream ss;
    ss << in.rdbuf();
    for (const auto& row : parseCSV(ss.str())) {
        int id;
        if (row.size() < 2 || !parsePositiveInt(row[0], id)) continue;
        string reason;
        for (size_t k = 2; k < row.size(); ++k) { if (k > 2) reason += ','; reason += row[k]; }
        report(id, row[1], reason);
    }
    return true;
}

ContentSecurity::ContentSecurity(SensitiveWordFilter& f, ReportManager& r)
    : filter(f), reporter(r) {}

CheckResult ContentSecurity::checkPost(const string& title, const string& content) const {
    if (isBlank(title) || isBlank(content))
        return { false, "Title and content cannot be empty", {} };
    return checkInternal(title + "\n" + content);
}

CheckResult ContentSecurity::checkReply(const string& content) const {
    if (isBlank(content))
        return { false, "Reply content cannot be empty", {} };
    return checkInternal(content);
}

ReportStatus ContentSecurity::reportPost(int postId, const string& userId, const string& reason) {
    return reporter.report(postId, userId, reason);
}

bool ContentSecurity::isPostMarked(int postId) const {
    return reporter.isMarked(postId);
}

CheckResult ContentSecurity::checkInternal(const string& text) const {
    auto matches = filter.findMatches(text);
    if (matches.empty()) return { true, "Content check passed", {} };
    return { false, "Content contains forbidden words, submission rejected", matches };
}

static int totalTests = 0, failedTests = 0;

static string showVisible(const string& s) {
    string r;
    for (unsigned char c : s) {
        if (c == '\n') r += "\\n";
        else if (c == '\t') r += "\\t";
        else if (c == '\r') r += "\\r";
        else if (c == 0xFF || c == 0xFE) { char b[8]; snprintf(b, sizeof b, "\\x%02X", c); r += b; }
        else r += static_cast<char>(c);
    }
    return r.empty() ? "(empty)" : r;
}

static void recordTest(const string& category, const string& input,
                      const string& expected, const string& actual) {
    ++totalTests;
    bool pass = (expected == actual);
    if (!pass) ++failedTests;
    cout << (pass ? "PASS" : "FAIL") << " | " << totalTests << " | " << category
         << " | input: " << showVisible(input)
         << " | expected: " << showVisible(expected)
         << " | actual: " << showVisible(actual) << "\n";
}

static string boolStr(bool v) { return v ? "PASS" : "FAIL"; }
static string statusStr(ReportStatus s) {
    return s == ReportStatus::SUCCESS ? "SUCCESS" :
           s == ReportStatus::DUPLICATE ? "DUPLICATE" : "INVALID";
}
static string yesNo(bool b) { return b ? "YES" : "NO"; }

static void testFilter() {
    SensitiveWordFilter f;
    for (auto w : { "cheat", "fuck", "idiot", "badschool" }) f.addWord(w);
    struct Case { const char* desc; string input; bool shouldBlock; };
    vector<Case> cases = {
        {"normal text", "nice weather today", false},
        {"direct match", "anyone cheat here", true},
        {"halfwidth space", "che at", true},
        {"hyphen", "che-at", true},
        {"halfwidth dot", "che.at", true},
        {"cn comma", "che,at", true},
        {"cn period", "che.at", true},
        {"fullwidth space", "che　at", true},
        {"tab", "che\tat", true},
        {"newline", "che\nat", true},
        {"cn quotes", "che\"at\"", true},
        {"ellipsis", "che…at", true},
        {"interval dot", "che·at", true},
        {"emoji separator", "che😀at", true},
        {"zero width space", "che​at", true},
        {"upper case with dots", "F.U.C.K", true},
        {"spaced english", "f u c k", true},
        {"fullwidth english", "ＦＵＣＫ", true},
        {"phrase with space", "bad school", true},
        {"reverse order", "atch", false},
        {"similar but different", "factory exam", false},
        {"empty string", "", false},
        {"only whitespace", "   ", false},
        {"only punctuation", ",.?!...", false},
        {"invalid utf8 with word", "\xFFcheat", true},
        {"invalid utf8 no word", "\xFFhello", false},
        {"long text match at end", "this is a long normal text for testing, last says idiot", true},
        {"multiple words", "cheat fuck", true},
    };
    for (auto& x : cases)
        recordTest(string("filter/") + x.desc, x.input,
                   boolStr(!x.shouldBlock), boolStr(f.isClean(x.input)));
    recordTest("filter/multi-match count", "cheat fuck", "2", to_string(f.findMatches("cheat fuck").size()));
    recordTest("filter/duplicate add", "dedupe", "4", [&] { f.addWord("cheat"); f.addWord("che at"); return to_string(f.wordCount()); }());
}

static void testMask() {
    SensitiveWordFilter f;
    for (auto w : { "cheat", "fuck", "badschool", "school" }) f.addWord(w);
    struct Case { string input, expected; };
    vector<Case> cases = {
        {"no cheat", "no**"},
        {"F.u.c.k", "*******"},
        {"che,at", "***"},
        {"che at hello", "*** hello"},
        {"hello", "hello"},
        {"", ""},
        {"cheatcheat", "****"},
        {"badschool", "****"},
        {"ＦＵＣＫ", "****"},
        {"acheatbcheatc", "a**b**c"},
        {"\xFFcheat", "\xFF**"},
    };
    for (auto& x : cases)
        recordTest("mask", x.input, x.expected, f.mask(x.input));
    for (string s : {"F.u.c.k", "che,at", "ＦＵＣＫ", "che😀at", "bad school", "xxche.atyy"})
        recordTest("mask/consistency", s, "PASS", boolStr(f.isClean(f.mask(s))));
}

static void testReport() {
    ReportManager m(3);
    recordTest("report/first report", "post1 a", "SUCCESS", statusStr(m.report(1, "a", "x")));
    recordTest("report/duplicate", "post1 a again", "DUPLICATE", statusStr(m.report(1, "a", "x")));
    recordTest("report/trim username", "post1 ' a '", "DUPLICATE", statusStr(m.report(1, " a ", "x")));
    recordTest("report/id zero", "post0", "INVALID", statusStr(m.report(0, "a", "x")));
    recordTest("report/id negative", "post-5", "INVALID", statusStr(m.report(-5, "a", "x")));
    recordTest("report/empty user", "post1 ''", "INVALID", statusStr(m.report(1, "", "x")));
    recordTest("report/blank user", "post1 '   '", "INVALID", statusStr(m.report(1, "   ", "x")));
    recordTest("report/empty reason ok", "post1 d empty", "SUCCESS", statusStr(m.report(1, "d", "")));
    recordTest("report/below threshold", "post1 count=2", "NO", yesNo(m.isMarked(1)));
    m.report(1, "b", "x");
    recordTest("report/reach threshold", "post1 count=3", "YES", yesNo(m.isMarked(1)));
    recordTest("report/count", "post1", "3", to_string(m.getReportCount(1)));
    recordTest("report/other post unaffected", "post2", "NO", yesNo(m.isMarked(2)));
    recordTest("report/marked count", "-", "1", to_string(m.getMarkedPosts().size()));
    recordTest("report/never reported", "post999", "0", to_string(m.getReportCount(999)));
}

static void testCSV() {
    const string fn = "_test_report.csv";
    {
        ReportManager m(3);
        m.report(7, "u,1", "reason,with\"quotes\"\nsecond line");
        m.report(7, "bob", "normal reason");
        m.report(8, "a b", "");
        m.report(9, "cn user", "has comma, cn and ,en");
        recordTest("CSV/save", fn, "SUCCESS", m.saveToFile(fn) ? "SUCCESS" : "FAIL");
        ReportManager n(3);
        recordTest("CSV/load", fn, "SUCCESS", n.loadFromFile(fn) ? "SUCCESS" : "FAIL");
        recordTest("CSV/record count", "4", "4", to_string(n.getAllRecords().size()));
        bool same = n.getAllRecords().size() == m.getAllRecords().size();
        for (size_t i = 0; same && i < m.getAllRecords().size(); ++i) {
            const auto& a = m.getAllRecords()[i], & b = n.getAllRecords()[i];
            same = a.postId == b.postId && a.userId == b.userId && a.reason == b.reason;
        }
        recordTest("CSV/roundtrip special chars", "4", "CONSISTENT", same ? "CONSISTENT" : "INCONSISTENT");
        recordTest("CSV/duplicate rule after load", "post7 bob", "DUPLICATE", statusStr(n.report(7, "bob", "")));
    }
    {
        ofstream(fn, ios::binary) << "101,alice,ad,violation\r\n101,bob,x\r\n";
        ReportManager n(3);
        n.loadFromFile(fn);
        recordTest("CSV/old format count", "old 2 rows", "2", to_string(n.getAllRecords().size()));
        recordTest("CSV/old format reason comma", "old format", "ad,violation",
                   n.getAllRecords().empty() ? "" : n.getAllRecords()[0].reason);
    }
    {
        ofstream(fn, ios::binary) << "abc,u,r\n\n5,,x\n-1,u,r\n12abc,u,r\n0,u,r\n7\n";
        ReportManager n(3);
        recordTest("CSV/dirty data no crash", "garbage", "SUCCESS", n.loadFromFile(fn) ? "SUCCESS" : "FAIL");
        recordTest("CSV/dirty data all skipped", "6 invalid", "0", to_string(n.getAllRecords().size()));
    }
    {
        ofstream(fn, ios::binary) << "\xEF\xBB\xBF" << "3,\"u\",\"with BOM\"\n";
        ReportManager n(3);
        n.loadFromFile(fn);
        recordTest("CSV/BOM file", "BOM", "1", to_string(n.getAllRecords().size()));
    }
    remove(fn.c_str());
    ReportManager k;
    recordTest("CSV/file not found", "no_such.csv", "FAIL", k.loadFromFile("no_such_file_xyz.csv") ? "SUCCESS" : "FAIL");
}

static void testInterface() {
    SensitiveWordFilter f;
    for (auto w : { "cheat", "idiot" }) f.addWord(w);
    ReportManager m(2);
    ContentSecurity g(f, m);
    recordTest("iface/normal post", "title/content", "PASS", boolStr(g.checkPost("title", "normal content").allowed));
    recordTest("iface/content violation", "title/cheat", "FAIL", boolStr(g.checkPost("title", "cheat").allowed));
    recordTest("iface/title violation", "foul/content", "FAIL", boolStr(g.checkPost("idi ot", "content").allowed));
    recordTest("iface/empty title", "''/content", "FAIL", boolStr(g.checkPost("", "content").allowed));
    recordTest("iface/content blank", "title/'  '", "FAIL", boolStr(g.checkPost("title", "  \t").allowed));
    recordTest("iface/content fullwidth blank", "title/full space", "FAIL", boolStr(g.checkPost("title", "\xE3\x80\x80").allowed));
    recordTest("iface/punctuation is content", "title/???", "PASS", boolStr(g.checkPost("title", "???").allowed));
    recordTest("iface/normal reply", "thanks", "PASS", boolStr(g.checkReply("thanks").allowed));
    recordTest("iface/violation reply", "cheat", "FAIL", boolStr(g.checkReply("che,at").allowed));
    recordTest("iface/empty reply", "''", "FAIL", boolStr(g.checkReply("").allowed));
    recordTest("iface/matched word return", "cheat", "cheat",
               g.checkReply("someone cheat").matchedWords.empty() ? "" : g.checkReply("someone cheat").matchedWords[0]);
    g.reportPost(5, "a", "x");
    g.reportPost(5, "b", "x");
    recordTest("iface/post marked after reports", "post5", "YES", yesNo(g.isPostMarked(5)));
    recordTest("iface/not reported not marked", "post6", "NO", yesNo(g.isPostMarked(6)));
}

void runMember4UnitTest() {
    cout << "\n====== Unit Test Start: Member4 Module ======\n";
    totalTests = 0;
    failedTests = 0;
    cout << "Status | ID | Category | Input | Expected | Actual\n";
    testFilter();
    testMask();
    testReport();
    testCSV();
    testInterface();
    cout << "\n====== Unit Test Result: " << (totalTests - failedTests) << " passed, "
         << failedTests << " failed ======\n";
}
