#ifndef MEMBER3_H
#define MEMBER3_H

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include "member1.h"

using namespace std;

// 全局登录会话变量：在 member3.cpp 中唯一定义，其他模块通过 extern 引用
extern int currentLoginUserId;

// 用户结构体
struct User
{
    int userId;
    string username;
    string password;
    time_t createTime;
    User(int uid, string un, string pw);
};

// 收藏结构体：多用户版本，userId+postId唯一约束
struct Collection
{
    int collectionId;
    int userId;
    int postId;
    time_t collectTime;
    Collection(int cid, int uid, int pid);
};

class UserManager
{
private:
    PostReplyManager& postMgr;  // 引用成员1的帖子管理器，访问公共数据
    vector<User> userList;
    vector<Collection> collectionList;
    int nextUserId;
    int nextCollectionId;

    void writeStr(ofstream &fout, const string &s);
    string readStr(ifstream &fin);

public:
    UserManager(PostReplyManager& mgr);

    // 注册：成功返回userId，失败返回-1
    int registerUser(string username, string password);

    // 登录：成功设置全局currentLoginUserId并返回userId，失败返回-1
    int login(string username, string password);

    // 退出登录
    void logout();

    // 切换收藏状态：已收藏则取消，未收藏则添加；成功返回1，失败返回-1
    int toggleCollection(int postId);

    // 获取当前用户的所有帖子
    vector<Post> getMyPosts();

    // 获取当前用户的所有回复
    vector<Reply> getMyReplies();

    // 获取当前用户收藏的所有帖子ID
    vector<int> getMyCollectionPostIds();

    void printMyPosts();
    void printMyReplies();
    void printMyCollections();

    // 持久化：格式完全对齐成员1的长度前缀序列化
    void saveToFile(const string &filename);
    void loadFromFile(const string &filename);

    // 模块单元测试
    void runUnitTest();
};

#endif
