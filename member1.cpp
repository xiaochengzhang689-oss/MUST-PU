#include "member1.h"

PostReplyManager::PostReplyManager() 
{
    boardList.emplace_back(1,"Study");
    boardList.emplace_back(2,"Campus Life");
    boardList.emplace_back(3,"Sports & Entertainment");
}

bool PostReplyManager::isBoardExist(int boardId) const
{
    for(auto &b : boardList)
    {
        if(b.boardId == boardId) return true;
    }
    return false;
}

void PostReplyManager::refreshDowngradeAllPosts()
{
    time_t now = time(nullptr);
    for(auto &p : postList){
        time_t delta = now - p.lastReplyTime;
        if(delta > DOWNGRADE_THRESHOLD)
        {
            p.isDowngraded = true;
        }
    }
}

int PostReplyManager::createPost(int boardId, int authorUid, string title, string content){
    if(!isBoardExist(boardId))
    {
        cout << "[ERROR] Board ID " << boardId << " does not exist. Post creation rejected.\n";
        return -1;
    }
    if(title.empty() || content.empty())
    {
        cout << "[ERROR] Title and content cannot be empty. Post creation rejected.\n";
        return -1;
    }
    Post p(nextPostId, boardId, authorUid, title, content);
    postList.push_back(p);
    return nextPostId++;
}

int PostReplyManager::createReply(int postId, int authorUid, string replyContent)
{
    if(replyContent.empty())
    {
        cout << "[ERROR] Reply content cannot be empty. Reply rejected.\n";
        return -1;
    }

    bool found = false;
    for(auto &p : postList)
    {
        if(p.postId == postId)
        {
            found = true;
            p.lastReplyTime = time(nullptr);
            p.isDowngraded = false;
            break;
        }
    }
    if(!found)
    {
        cout << "[ERROR] Post ID " << postId << " does not exist. Reply rejected.\n";
        return -1;
    }

    Reply r(nextReplyId, postId, authorUid, replyContent);
    replyList.push_back(r);
    return nextReplyId++;
}

vector<Post> PostReplyManager::getBoardVisiblePosts(int boardId)
{
    refreshDowngradeAllPosts();
    vector<Post> res;
    for(auto &p : postList)
    {
        if(p.boardId == boardId && !p.isDowngraded)
        {
            res.push_back(p);
        }
    }
    return res;
}

Post* PostReplyManager::getPostById(int postId)
{
    refreshDowngradeAllPosts();
    for(auto &p : postList)
    {
        if(p.postId == postId)
        {
            return &p;
        }
    }
    return nullptr;
}

vector<Reply> PostReplyManager::getRepliesOfPost(int postId)
{
    vector<Reply> res;
    for(auto &r : replyList)
    {
        if(r.postId == postId)
        {
            res.push_back(r);
        }
    }
    return res;
}

void PostReplyManager::printBoards()
{
    cout << "\n=====Board List=====\n";
    for(auto &b : boardList)
    {
        cout << b.boardId << ". " << b.boardName << endl;
    }
}

void PostReplyManager::printBoardPosts(int boardId)
{
    auto posts = getBoardVisiblePosts(boardId);
    cout << "\n---Board Post List (Downgraded Posts Filtered)---\n";
    if(posts.empty())
    {
        cout << "No visible posts\n";
        return;
    }
    for(auto &p : posts)
    {
        cout << "postId:"<<p.postId<<"｜Title:"<<p.title
             <<"｜Downgraded:"<<(p.isDowngraded?"Yes":"No")<<endl;
    }
}

void PostReplyManager::printPostDetail(int postId)
{
    Post* p = getPostById(postId);
    if(p == nullptr)
    {
        cout << "Post does not exist\n";
        return;
    }
    cout << "\n========Post Detail postId:"<<p->postId<<"========\n";
    cout << "Title："<<p->title<<endl;
    cout << "Content："<<p->content<<endl;
    cout << "Downgraded："<< (p->isDowngraded?"YES":"NO") <<endl;
    cout << "---Reply List---\n";
    auto rep = getRepliesOfPost(postId);
    for(auto &r : rep)
    {
        cout << ">>Reply:" << r.content <<endl;
    }
}

void PostReplyManager::writeStr(ofstream &fout, const string &s)
{
    fout << s.size() << "\n" << s << "\n";
}

string PostReplyManager::readStr(ifstream &fin)
{
    size_t len;
    fin >> len;
    fin.ignore();
    string s(len, '\0');
    fin.read(&s[0], (streamsize)len);
    fin.ignore();
    return s;
}

void PostReplyManager::saveToFile(const string &filename)
{
    ofstream fout(filename);
    if(!fout){ cout << "[WARN] Cannot open file to save: " << filename << "\n"; return; }
    fout << boardList.size() << "\n";
    for(auto &b : boardList) fout << b.boardId << "\n" << b.boardName << "\n";
    fout << postList.size() << "\n";
    for(auto &p : postList)
    {
        fout << p.postId << "\n" << p.boardId << "\n" << p.authorUserId << "\n"
             << (p.isDowngraded?1:0) << "\n"
             << (long long)p.createTime << "\n" << (long long)p.lastReplyTime << "\n";
        writeStr(fout, p.title);
        writeStr(fout, p.content);
    }
    fout << replyList.size() << "\n";
    for(auto &r : replyList)
    {
        fout << r.replyId << "\n" << r.postId << "\n" << r.authorUserId << "\n" << (long long)r.createTime << "\n";
        writeStr(fout, r.content);
    }
    fout << nextPostId << "\n" << nextReplyId << "\n";
    fout.close();
    cout << "[INFO] Data saved to " << filename << "\n";
}

void PostReplyManager::loadFromFile(const string &filename)
{
    ifstream fin(filename);
    if(!fin){ cout << "[INFO] No save file found, starting fresh.\n"; return; }
    boardList.clear(); postList.clear(); replyList.clear();
    size_t nb; fin >> nb; fin.ignore();
    for(size_t i=0;i<nb;i++){ int id; string name; fin>>id; fin.ignore(); getline(fin,name); boardList.emplace_back(id,name); }
    size_t np; fin >> np; fin.ignore();
    for(size_t i=0;i<np;i++)
    {
        int pid,bid,uid,dg; long long ct,lrt;
        fin>>pid>>bid>>uid>>dg>>ct>>lrt; fin.ignore();
        Post p(pid,bid,uid,readStr(fin),readStr(fin));
        p.createTime = (time_t)ct; p.lastReplyTime = (time_t)lrt; p.isDowngraded = (dg==1);
        postList.push_back(p);
    }
    size_t nr; fin >> nr; fin.ignore();
    for(size_t i=0;i<nr;i++)
    {
        int rid,pid,ruid; long long ct;
        fin>>rid>>pid>>ruid>>ct; fin.ignore();
        Reply r(rid,pid,ruid,readStr(fin));
        r.createTime = (time_t)ct;
        replyList.push_back(r);
    }
    fin>>nextPostId>>nextReplyId; fin.ignore();
    fin.close();
    cout << "[INFO] Data loaded from " << filename << "\n";
}

void PostReplyManager::runUnitTest()
{
    cout << "\n======【Unit Test Start】Member1 Module ======\n";
    int passed = 0, failed = 0;

    int pid = createPost(2, 1001, "Test Post", "Wait for auto downgrade");
    if(pid > 0){ cout << "[PASS] T1 create post in valid board\n"; passed++; }
    else { cout << "[FAIL] T1 create post in valid board\n"; failed++; }

    int bad = createPost(999, 1001, "x", "y");
    if(bad == -1){ cout << "[PASS] T2 invalid board ID rejected\n"; passed++; }
    else { cout << "[FAIL] T2 invalid board ID rejected\n"; failed++; }

    int badRep = createReply(99999, 1001, "hello");
    if(badRep == -1){ cout << "[PASS] T3 reply to non-existent post rejected\n"; passed++; }
    else { cout << "[FAIL] T3 reply to non-existent post rejected\n"; failed++; }

    refreshDowngradeAllPosts();
    Post* p = getPostById(pid);
    if(p && !p->isDowngraded){ cout << "[PASS] T4 new post not downgraded initially\n"; passed++; }
    else { cout << "[FAIL] T4 new post not downgraded initially\n"; failed++; }

    cout << "Waiting " << (DOWNGRADE_THRESHOLD+1) << " seconds to verify auto-downgrade...\n";
    time_t waitStart = time(nullptr);
    while(time(nullptr) - waitStart <= DOWNGRADE_THRESHOLD) { }
    refreshDowngradeAllPosts();
    p = getPostById(pid);
    if(p && p->isDowngraded){ cout << "[PASS] T5 post auto-downgraded after threshold\n"; passed++; }
    else { cout << "[FAIL] T5 post auto-downgraded after threshold\n"; failed++; }

    int rid = createReply(pid, 1001, "new reply restores post");
    p = getPostById(pid);
    if(rid > 0 && p && !p->isDowngraded){ cout << "[PASS] T6 reply restores post from downgrade\n"; passed++; }
    else { cout << "[FAIL] T6 reply restores post from downgrade\n"; failed++; }

    cout << "\n======Unit Test Result: " << passed << " passed, " << failed << " failed ======\n";
}
