#include "member3.h"

// 全局登录会话变量：本文件唯一定义，其他模块通过extern引用
int currentLoginUserId = -1;

// ==================== 结构体构造函数 ====================
User::User(int uid, string un, string pw) : userId(uid), username(un), password(pw)
{
    createTime = time(nullptr);
}

Collection::Collection(int cid, int uid, int pid) : collectionId(cid), userId(uid), postId(pid)
{
    collectTime = time(nullptr);
}

// ==================== 内部工具函数：和成员1写法完全一致 ====================
void UserManager::writeStr(ofstream &fout, const string &s)
{
    fout << s.size() << "\n" << s << "\n";
}

string UserManager::readStr(ifstream &fin)
{
    size_t len;
    fin >> len;
    fin.ignore();
    string s(len, '\0');
    fin.read(&s[0], (streamsize)len);
    fin.ignore();
    return s;
}

// ==================== 类实现 ====================
UserManager::UserManager(PostReplyManager& mgr) : postMgr(mgr), nextUserId(1), nextCollectionId(1)
{
}

int UserManager::registerUser(string username, string password)
{
    if (username.empty() || password.empty())
    {
        cout << "[ERROR] Username and password cannot be empty.\n";
        return -1;
    }

    for (int i = 0; i < userList.size(); i++)
    {
        if (userList[i].username == username)
        {
            cout << "[ERROR] Username already exists.\n";
            return -1;
        }
    }

    User newUser(nextUserId, username, password);
    userList.push_back(newUser);
    return nextUserId++;
}

int UserManager::login(string username, string password)
{
    for (int i = 0; i < userList.size(); i++)
    {
        if (userList[i].username == username && userList[i].password == password)
        {
            currentLoginUserId = userList[i].userId;
            return userList[i].userId;
        }
    }
    cout << "[ERROR] Invalid username or password.\n";
    return -1;
}

void UserManager::logout()
{
    currentLoginUserId = -1;
}

int UserManager::toggleCollection(int postId)
{
    if (currentLoginUserId == -1)
    {
        cout << "[ERROR] Please login first.\n";
        return -1;
    }

    Post* targetPost = postMgr.getPostById(postId);
    if (targetPost == nullptr)
    {
        cout << "[ERROR] Post not found.\n";
        return -1;
    }

    for (int i = 0; i < collectionList.size(); i++)
    {
        if (collectionList[i].userId == currentLoginUserId && collectionList[i].postId == postId)
        {
            collectionList.erase(collectionList.begin() + i);
            return 1;
        }
    }

    Collection newColl(nextCollectionId, currentLoginUserId, postId);
    collectionList.push_back(newColl);
    nextCollectionId++;
    return 1;
}

vector<Post> UserManager::getMyPosts()
{
    vector<Post> res;
    if (currentLoginUserId == -1)
        return res;

    // 遍历前强制刷新降级状态，严格遵守约定
    postMgr.refreshDowngradeAllPosts();

    // 直接访问成员1的public容器postList
    for (int i = 0; i < postMgr.postList.size(); i++)
    {
        if (postMgr.postList[i].authorUserId == currentLoginUserId)
        {
            res.push_back(postMgr.postList[i]);
        }
    }
    return res;
}

vector<Reply> UserManager::getMyReplies()
{
    vector<Reply> res;
    if (currentLoginUserId == -1)
        return res;

    // 直接访问成员1的public容器replyList
    for (int i = 0; i < postMgr.replyList.size(); i++)
    {
        if (postMgr.replyList[i].authorUserId == currentLoginUserId)
        {
            res.push_back(postMgr.replyList[i]);
        }
    }
    return res;
}

vector<int> UserManager::getMyCollectionPostIds()
{
    vector<int> res;
    if (currentLoginUserId == -1)
        return res;

    for (int i = 0; i < collectionList.size(); i++)
    {
        if (collectionList[i].userId == currentLoginUserId)
        {
            res.push_back(collectionList[i].postId);
        }
    }
    return res;
}

void UserManager::printMyPosts()
{
    auto posts = getMyPosts();
    cout << "\n--- My Posts ---\n";
    if (posts.empty())
    {
        cout << "No visible posts\n";
        return;
    }
    for (auto& p : posts)
    {
        cout << "postId:" << p.postId << "｜Title:" << p.title << endl;
    }
}

void UserManager::printMyReplies()
{
    auto replies = getMyReplies();
    cout << "\n--- My Replies ---\n";
    if (replies.empty())
    {
        cout << "No replies found\n";
        return;
    }
    for (auto& r : replies)
    {
        cout << "replyId:" << r.replyId << "｜postId:" << r.postId << "｜Content:" << r.content << endl;
    }
}

void UserManager::printMyCollections()
{
    auto ids = getMyCollectionPostIds();
    cout << "\n--- My Collections ---\n";
    if (ids.empty())
    {
        cout << "No collections found\n";
        return;
    }
    for (int pid : ids)
    {
        Post* p = postMgr.getPostById(pid);
        if (p != nullptr)
        {
            cout << "postId:" << p->postId << "｜Title:" << p->title << endl;
        }
    }
}

// ==================== 持久化：完全对齐成员1格式 ====================
void UserManager::saveToFile(const string &filename)
{
    ofstream fout(filename);
    if (!fout)
    {
        cout << "[WARN] Cannot open file to save: " << filename << "\n";
        return;
    }

    fout << userList.size() << "\n";
    for (auto& u : userList)
    {
        fout << u.userId << "\n";
        writeStr(fout, u.username);
        writeStr(fout, u.password);
        fout << (long long)u.createTime << "\n";
    }

    fout << collectionList.size() << "\n";
    for (auto& c : collectionList)
    {
        fout << c.collectionId << "\n" << c.userId << "\n" << c.postId << "\n";
        fout << (long long)c.collectTime << "\n";
    }

    fout << nextUserId << "\n" << nextCollectionId << "\n";

    fout.close();
    cout << "[INFO] Data saved to " << filename << "\n";
}

void UserManager::loadFromFile(const string &filename)
{
    ifstream fin(filename);
    if (!fin)
    {
        cout << "[INFO] No save file found, starting fresh.\n";
        return;
    }

    userList.clear();
    collectionList.clear();

    size_t userCount;
    fin >> userCount;
    fin.ignore();
    for (size_t i = 0; i < userCount; i++)
    {
        int uid;
        fin >> uid;
        fin.ignore();
        string username = readStr(fin);
        string password = readStr(fin);
        long long ct;
        fin >> ct;
        fin.ignore();

        User u(uid, username, password);
        u.createTime = (time_t)ct;
        userList.push_back(u);
    }

    size_t collCount;
    fin >> collCount;
    fin.ignore();
    for (size_t i = 0; i < collCount; i++)
    {
        int cid, uid, pid;
        long long ct;
        fin >> cid >> uid >> pid >> ct;
        fin.ignore();

        Collection c(cid, uid, pid);
        c.collectTime = (time_t)ct;
        collectionList.push_back(c);
    }

    fin >> nextUserId >> nextCollectionId;
    fin.ignore();

    fin.close();
    cout << "[INFO] Data loaded from " << filename << "\n";
}

// ==================== 单元测试 ====================
void UserManager::runUnitTest()
{
    cout << "\n======【Unit Test Start】Member3 Module ======\n";

    int uid1 = registerUser("testuser1", "123456");
    if (uid1 > 0) cout << "[PASS] Register normal user success\n";
    else cout << "[FAIL] Register normal user failed\n";

    int uid2 = registerUser("testuser1", "123456");
    if (uid2 == -1) cout << "[PASS] Duplicate username rejected\n";
    else cout << "[FAIL] Duplicate username not rejected\n";

    int loginRes = login("testuser1", "123456");
    if (loginRes == uid1 && currentLoginUserId == uid1)
        cout << "[PASS] Login success, userId set correctly\n";
    else
        cout << "[FAIL] Login failed\n";

    logout();
    int badLogin = login("testuser1", "wrongpass");
    if (badLogin == -1 && currentLoginUserId == -1)
        cout << "[PASS] Wrong password login rejected\n";
    else
        cout << "[FAIL] Wrong password login not rejected\n";

    login("testuser1", "123456");
    cout << "\n====== Unit Test Finished ======\n";
}
