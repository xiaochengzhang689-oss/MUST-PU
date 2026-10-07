#ifndef POPULARITY_MODULE_H
#define POPULARITY_MODULE_H

#include <ctime>
#include <map>
#include <string>
#include <vector>

// PostReplyManager 和 Post 应该在成员1的头文件中定义。
// 这里使用前向声明，避免重复定义。
struct Post;
class PostReplyManager;

// 投票类型
enum VoteType
{
    NO_VOTE = 0,
    UP_VOTE = 1,
    DOWN_VOTE = -1
};

// 投票统计结果
struct VoteSummary
{
    int likes = 0;
    int dislikes = 0;
};

// 热帖或最新帖子查询结果
struct HotPostResult
{
    int postId;
    int boardId;
    int authorUserId;

    std::string title;

    int likes;
    int dislikes;
    int replyCount;

    double hotScore;
    bool isDowngraded;
    std::time_t lastReplyTime;
};

class PopularityModule
{
private:
    // 对成员1的帖子管理器进行引用
    PostReplyManager &manager;

    // 帖子投票记录
    //
    // 第一层 key：postId
    // 第二层 key：userId
    // value：1 表示点赞，-1 表示点踩
    std::map<int, std::map<int, int>> postVoteRecords;

    // 回复投票记录
    //
    // 第一层 key：replyId
    // 第二层 key：userId
    // value：1 表示点赞，-1 表示点踩
    std::map<int, std::map<int, int>> replyVoteRecords;

    // 热度计算参数
    double likeWeight = 3.0;
    double dislikeWeight = 2.0;
    double replyWeight = 2.0;

    // 时间衰减参数
    double decayDays = 2.0;
    double decayPower = 0.8;

    // 检查用户是否登录
    bool isValidUser(int userId) const;

    // 检查帖子是否存在
    bool isPostExist(int postId) const;

    // 检查回复是否存在，并确认回复属于指定帖子
    bool isReplyExist(int replyId, int postId) const;

    // 统计指定帖子或回复的点赞和点踩数量
    VoteSummary countVotes(
        const std::map<int, std::map<int, int>> &voteRecords,
        int targetId
    ) const;

    // 计算距离最后回复的天数
    double getInactiveDays(std::time_t lastReplyTime) const;

    // 获取指定帖子的回复数量
    int getReplyCount(int postId) const;

    // 根据投票、回复和时间计算热度
    double calculateScoreByData(
        int likes,
        int dislikes,
        int replyCount,
        std::time_t lastReplyTime
    ) const;

    // 将 Post 转换为排行榜结果
    HotPostResult makeResult(const Post &post) const;

public:
    explicit PopularityModule(PostReplyManager &mgr);

    // 帖子投票
    //
    // 返回值：
    // -1：操作失败
    //  0：取消原有投票
    //  1：新增投票或修改投票
    int votePost(
        int postId,
        int userId,
        VoteType voteType
    );

    // 回复投票
    //
    // 返回值：
    // -1：操作失败
    //  0：取消原有投票
    //  1：新增投票或修改投票
    int voteReply(
        int postId,
        int replyId,
        int userId,
        VoteType voteType
    );

    // 查询帖子投票统计
    VoteSummary getPostVoteSummary(int postId) const;

    // 查询回复投票统计
    VoteSummary getReplyVoteSummary(int replyId) const;

    // 计算单个帖子的热度
    //
    // 帖子不存在时返回 -1.0
    double calculatePostScore(int postId);

    // 获取热帖排行榜
    //
    // boardId == -1 表示查询全部板块
    std::vector<HotPostResult> getHotPosts(
        int limit,
        int boardId = -1
    );

    // 获取最新帖子排行榜
    //
    // boardId == -1 表示查询全部板块
    std::vector<HotPostResult> getLatestPosts(
        int limit,
        int boardId = -1
    );

    // 打印热帖排行榜
    void printHotPosts(
        int limit,
        int boardId = -1
    );

    // 打印最新帖子排行榜
    void printLatestPosts(
        int limit,
        int boardId = -1
    );

    // 打印单个帖子的热度信息
    void printPostPopularity(int postId);

    // 获取帖子投票记录，供成员4保存数据
    const std::map<int, std::map<int, int>> &
    getPostVoteRecords() const;

    // 获取回复投票记录，供成员4保存数据
    const std::map<int, std::map<int, int>> &
    getReplyVoteRecords() const;

    // 从文件加载帖子投票记录
    void loadPostVote(
        int postId,
        int userId,
        int vote
    );

    // 从文件加载回复投票记录
    void loadReplyVote(
        int replyId,
        int userId,
        int vote
    );
};

#endif
