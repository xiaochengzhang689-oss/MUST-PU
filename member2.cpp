#include "member2.h"
#include "member1.h"
// #include "whc.h"//后面whc有.h文件再做修改

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;

bool PopularityModule::isValidUser(int userId) const
{
    return userId != -1;
}

bool PopularityModule::isPostExist(int postId) const
{
    for(const auto &post : manager.postList)
    {
        if(post.postId == postId)
        {
            return true;
        }
    }

    return false;
}

bool PopularityModule::isReplyExist(
    int replyId,
    int postId
) const
{
    for(const auto &reply : manager.replyList)
    {
        if(reply.replyId == replyId &&
           reply.postId == postId)
        {
            return true;
        }
    }

    return false;
}

VoteSummary PopularityModule::countVotes(
    const map<int, map<int, int>> &voteRecords,
    int targetId
) const
{
    VoteSummary result;

    auto targetIt = voteRecords.find(targetId);

    if(targetIt == voteRecords.end())
    {
        return result;
    }

    for(const auto &userVote : targetIt->second)
    {
        if(userVote.second == UP_VOTE)
        {
            result.likes++;
        }
        else if(userVote.second == DOWN_VOTE)
        {
            result.dislikes++;
        }
    }

    return result;
}

double PopularityModule::getInactiveDays(
    time_t lastReplyTime
) const
{
    time_t now = time(nullptr);

    double seconds = difftime(now, lastReplyTime);

    if(seconds < 0)
    {
        seconds = 0;
    }

    return seconds / (60.0 * 60.0 * 24.0);
}

int PopularityModule::getReplyCount(int postId) const
{
    int count = 0;

    for(const auto &reply : manager.replyList)
    {
        if(reply.postId == postId)
        {
            count++;
        }
    }

    return count;
}

double PopularityModule::calculateScoreByData(
    int likes,
    int dislikes,
    int replyCount,
    time_t lastReplyTime
) const
{
    double rawScore =
        likes * likeWeight
        - dislikes * dislikeWeight
        + replyCount * replyWeight;

    double inactiveDays =
        getInactiveDays(lastReplyTime);

    double decayBase =
        1.0 + inactiveDays / decayDays;

    double finalScore =
        rawScore / pow(decayBase, decayPower);

    return finalScore;
}

HotPostResult PopularityModule::makeResult(
    const Post &post
) const
{
    VoteSummary voteSummary =
        countVotes(postVoteRecords, post.postId);

    int replyCount =
        getReplyCount(post.postId);

    double hotScore =
        calculateScoreByData(
            voteSummary.likes,
            voteSummary.dislikes,
            replyCount,
            post.lastReplyTime
        );

    HotPostResult result;

    result.postId = post.postId;
    result.boardId = post.boardId;
    result.authorUserId = post.authorUserId;
    result.title = post.title;

    result.likes = voteSummary.likes;
    result.dislikes = voteSummary.dislikes;
    result.replyCount = replyCount;

    result.hotScore = hotScore;
    result.isDowngraded = post.isDowngraded;
    result.lastReplyTime = post.lastReplyTime;

    return result;
}

PopularityModule::PopularityModule(
    PostReplyManager &mgr
)
    : manager(mgr)
{
}

int PopularityModule::votePost(
    int postId,
    int userId,
    VoteType voteType
)
{
    if(!isValidUser(userId))
    {
        cout << "[ERROR] User is not logged in.\n";
        return -1;
    }

    if(!isPostExist(postId))
    {
        cout << "[ERROR] Post does not exist.\n";
        return -1;
    }

    if(voteType != UP_VOTE &&
       voteType != DOWN_VOTE)
    {
        cout << "[ERROR] Invalid vote type.\n";
        return -1;
    }

    int newVote =
        static_cast<int>(voteType);

    int oldVote = NO_VOTE;

    auto postIt =
        postVoteRecords.find(postId);

    if(postIt != postVoteRecords.end())
    {
        auto userIt =
            postIt->second.find(userId);

        if(userIt != postIt->second.end())
        {
            oldVote = userIt->second;
        }
    }

    // Clicking the same vote again cancels the vote.
    if(oldVote == newVote)
    {
        postVoteRecords[postId].erase(userId);

        cout << "[INFO] Post vote cancelled.\n";
        return 0;
    }

    // Add a new vote or change the existing vote.
    postVoteRecords[postId][userId] = newVote;

    if(newVote == UP_VOTE)
    {
        cout << "[INFO] Post liked successfully.\n";
    }
    else
    {
        cout << "[INFO] Post disliked successfully.\n";
    }

    return 1;
}

int PopularityModule::voteReply(
    int postId,
    int replyId,
    int userId,
    VoteType voteType
)
{
    if(!isValidUser(userId))
    {
        cout << "[ERROR] User is not logged in.\n";
        return -1;
    }

    if(!isPostExist(postId))
    {
        cout << "[ERROR] Post does not exist.\n";
        return -1;
    }

    if(!isReplyExist(replyId, postId))
    {
        cout << "[ERROR] Reply does not exist.\n";
        return -1;
    }

    if(voteType != UP_VOTE &&
       voteType != DOWN_VOTE)
    {
        cout << "[ERROR] Invalid vote type.\n";
        return -1;
    }

    int newVote =
        static_cast<int>(voteType);

    int oldVote = NO_VOTE;

    auto replyIt =
        replyVoteRecords.find(replyId);

    if(replyIt != replyVoteRecords.end())
    {
        auto userIt =
            replyIt->second.find(userId);

        if(userIt != replyIt->second.end())
        {
            oldVote = userIt->second;
        }
    }

    // Clicking the same vote again cancels the vote.
    if(oldVote == newVote)
    {
        replyVoteRecords[replyId].erase(userId);

        cout << "[INFO] Reply vote cancelled.\n";
        return 0;
    }

    // Add a new vote or change the existing vote.
    replyVoteRecords[replyId][userId] = newVote;

    if(newVote == UP_VOTE)
    {
        cout << "[INFO] Reply liked successfully.\n";
    }
    else
    {
        cout << "[INFO] Reply disliked successfully.\n";
    }

    return 1;
}

VoteSummary PopularityModule::getPostVoteSummary(
    int postId
) const
{
    return countVotes(
        postVoteRecords,
        postId
    );
}

VoteSummary PopularityModule::getReplyVoteSummary(
    int replyId
) const
{
    return countVotes(
        replyVoteRecords,
        replyId
    );
}

double PopularityModule::calculatePostScore(
    int postId
)
{
    manager.refreshDowngradeAllPosts();

    Post *post =
        manager.getPostById(postId);

    if(post == nullptr)
    {
        return -1.0;
    }

    VoteSummary voteSummary =
        getPostVoteSummary(postId);

    int replyCount =
        getReplyCount(postId);

    return calculateScoreByData(
        voteSummary.likes,
        voteSummary.dislikes,
        replyCount,
        post->lastReplyTime
    );
}

vector<HotPostResult> PopularityModule::getHotPosts(
    int limit,
    int boardId
)
{
    vector<HotPostResult> result;

    if(limit <= 0)
    {
        return result;
    }

    manager.refreshDowngradeAllPosts();

    for(const auto &post : manager.postList)
    {
        if(boardId != -1 &&
           post.boardId != boardId)
        {
            continue;
        }

        // Downgraded posts are excluded from hot ranking.
        if(post.isDowngraded)
        {
            continue;
        }

        HotPostResult item =
            makeResult(post);

        // Posts with non-positive scores are excluded.
        if(item.hotScore <= 0)
        {
            continue;
        }

        result.push_back(item);
    }

    sort(
        result.begin(),
        result.end(),
        [](const HotPostResult &a,
           const HotPostResult &b)
        {
            if(a.hotScore != b.hotScore)
            {
                return a.hotScore > b.hotScore;
            }

            if(a.lastReplyTime != b.lastReplyTime)
            {
                return a.lastReplyTime > b.lastReplyTime;
            }

            return a.postId < b.postId;
        }
    );

    if(static_cast<int>(result.size()) > limit)
    {
        result.resize(limit);
    }

    return result;
}

vector<HotPostResult> PopularityModule::getLatestPosts(
    int limit,
    int boardId
)
{
    vector<HotPostResult> result;

    if(limit <= 0)
    {
        return result;
    }

    manager.refreshDowngradeAllPosts();

    for(const auto &post : manager.postList)
    {
        if(boardId != -1 &&
           post.boardId != boardId)
        {
            continue;
        }

        // The latest-post page only shows visible posts.
        if(post.isDowngraded)
        {
            continue;
        }

        result.push_back(makeResult(post));
    }

    sort(
        result.begin(),
        result.end(),
        [](const HotPostResult &a,
           const HotPostResult &b)
        {
            if(a.lastReplyTime != b.lastReplyTime)
            {
                return a.lastReplyTime > b.lastReplyTime;
            }

            return a.postId < b.postId;
        }
    );

    if(static_cast<int>(result.size()) > limit)
    {
        result.resize(limit);
    }

    return result;
}

void PopularityModule::printHotPosts(
    int limit,
    int boardId
)
{
    vector<HotPostResult> posts =
        getHotPosts(limit, boardId);

    cout << "\n========== Hot Post Ranking ==========\n";

    if(posts.empty())
    {
        cout << "No hot posts found.\n";
        return;
    }

    cout << fixed << setprecision(2);

    for(size_t i = 0; i < posts.size(); i++)
    {
        const HotPostResult &post =
            posts[i];

        cout << i + 1 << ". "
             << "Post ID: " << post.postId
             << " | Board ID: " << post.boardId
             << " | Title: " << post.title
             << " | Score: " << post.hotScore
             << " | Likes: " << post.likes
             << " | Dislikes: " << post.dislikes
             << " | Replies: " << post.replyCount
             << "\n";
    }
}

void PopularityModule::printLatestPosts(
    int limit,
    int boardId
)
{
    vector<HotPostResult> posts =
        getLatestPosts(limit, boardId);

    cout << "\n========== Latest Posts ==========\n";

    if(posts.empty())
    {
        cout << "No latest posts found.\n";
        return;
    }

    cout << fixed << setprecision(2);

    for(size_t i = 0; i < posts.size(); i++)
    {
        const HotPostResult &post =
            posts[i];

        cout << i + 1 << ". "
             << "Post ID: " << post.postId
             << " | Board ID: " << post.boardId
             << " | Title: " << post.title
             << " | Likes: " << post.likes
             << " | Dislikes: " << post.dislikes
             << " | Replies: " << post.replyCount
             << "\n";
    }
}

void PopularityModule::printPostPopularity(
    int postId
)
{
    manager.refreshDowngradeAllPosts();

    Post *post =
        manager.getPostById(postId);

    if(post == nullptr)
    {
        cout << "[ERROR] Post does not exist.\n";
        return;
    }

    VoteSummary summary =
        getPostVoteSummary(postId);

    int replyCount =
        getReplyCount(postId);

    double score =
        calculatePostScore(postId);

    cout << "\n========== Post Popularity ==========\n";
    cout << "Post ID: " << postId << "\n";
    cout << "Title: " << post->title << "\n";
    cout << "Likes: " << summary.likes << "\n";
    cout << "Dislikes: " << summary.dislikes << "\n";
    cout << "Replies: " << replyCount << "\n";

    cout << fixed << setprecision(2);
    cout << "Hot Score: " << score << "\n";

    cout << "Downgraded: "
         << (post->isDowngraded ? "Yes" : "No")
         << "\n";
}

const map<int, map<int, int>> &
PopularityModule::getPostVoteRecords() const
{
    return postVoteRecords;
}

const map<int, map<int, int>> &
PopularityModule::getReplyVoteRecords() const
{
    return replyVoteRecords;
}

void PopularityModule::loadPostVote(
    int postId,
    int userId,
    int vote
)
{
    if(vote == UP_VOTE ||
       vote == DOWN_VOTE)
    {
        postVoteRecords[postId][userId] = vote;
    }
}

void PopularityModule::loadReplyVote(
    int replyId,
    int userId,
    int vote
)
{
    if(vote == UP_VOTE ||
       vote == DOWN_VOTE)
    {
        replyVoteRecords[replyId][userId] = vote;
    }
}
