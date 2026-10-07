#ifndef POST_REPLY_MANAGER_H
#define POST_REPLY_MANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>

using namespace std;

struct Board 
{
    int boardId;
    string boardName;
    Board(int bid, string name):boardId(bid),boardName(name){}
};

struct Reply 
{
    int replyId;
    int postId;
    int authorUserId;
    string content;
    time_t createTime;
    Reply(int rid, int pid, int uid, string c):replyId(rid),postId(pid),authorUserId(uid),content(c),createTime(time(nullptr)){}
};

struct Post
{
    int postId;
    int boardId;
    int authorUserId;
    string title;
    string content;
    time_t createTime;
    time_t lastReplyTime;
    bool isDowngraded;

    Post(int pid, int bid, int uid, string t, string c)
        :postId(pid),boardId(bid),authorUserId(uid),title(t),content(c),isDowngraded(false)
    {
        createTime = time(nullptr);
        lastReplyTime = createTime;
    }
};

class PostReplyManager
{
public:
    vector<Board> boardList;
    vector<Post> postList;
    vector<Reply> replyList;

    int nextPostId = 1;
    int nextReplyId = 1;

    const time_t DOWNGRADE_THRESHOLD = 15;

    PostReplyManager();

    bool isBoardExist(int boardId) const;

    void refreshDowngradeAllPosts();

    int createPost(int boardId, int authorUid, string title, string content);

    int createReply(int postId, int authorUid, string replyContent);

    vector<Post> getBoardVisiblePosts(int boardId);

    Post* getPostById(int postId);

    vector<Reply> getRepliesOfPost(int postId);

    void printBoards();

    void printBoardPosts(int boardId);

    void printPostDetail(int postId);

    void saveToFile(const string &filename);

    void loadFromFile(const string &filename);

    void runUnitTest();

private:
    void writeStr(ofstream &fout, const string &s);
    string readStr(ifstream &fin);
};

#endif
