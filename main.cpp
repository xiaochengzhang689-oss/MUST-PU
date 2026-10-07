#include <iostream>
#include <string>
#include <sstream>
#include "member1.h"
#include "member2.h"
#include "member3.h"
#include "member4.h"
using namespace std;

int main()
{
    system("chcp 65001 > nul");  
    
    PostReplyManager mgr;
    mgr.loadFromFile("member1_data.txt");
    PopularityModule popularity(mgr);
    UserManager um(mgr);
    um.loadFromFile("member3_data.txt");
    
    SensitiveWordFilter filter;
    filter.addWord("idiot");
    filter.addWord("badschool");
    filter.addWord("cheat");
    filter.addWord("ghostwrite");
    filter.addWord("gambling");
    filter.addWord("cheathack");
    filter.addWord("fuck");
    filter.addWord("shit");
    filter.addWord("wechattransfer");
    filter.loadFromFile("sensitive_words.txt");
    
    ReportManager reportMgr(3);
    reportMgr.loadFromFile("reports.csv");
    
    ContentSecurity security(filter, reportMgr);
    
    int opt;
    while(true)
    {
        cout << "\n=====【Main Menu｜Integrated System】=====\n";
        cout << "1. View Board List\n";
        cout << "2. Create Post\n";
        cout << "3. View Visible Posts In Board\n";
        cout << "4. Open Post Detail (Include Downgraded)\n";
        cout << "5. Add Reply\n";
        cout << "6. Run Module1 Unit Test\n";
        cout << "7. Save Post Data To File\n";
        cout << "8. Load Post Data From File\n";
        cout << "9. Like or Dislike Post\n";
        cout << "10. Like or Dislike Reply\n";
        cout << "11. View Hot Post Ranking\n";
        cout << "12. View Latest Posts\n";
        cout << "13. View Post Popularity\n";
        cout << "14. Register User\n";
        cout << "15. Login\n";
        cout << "16. Logout\n";
        cout << "17. Collect / Uncollect a Post\n";
        cout << "18. View My Posts\n";
        cout << "19. View My Replies\n";
        cout << "20. View My Collections\n";
        cout << "21. Save User Data To File\n";
        cout << "22. Load User Data From File\n";
        cout << "23. Run Member3 Unit Test\n";
        cout << "24. Check Sensitive Words (Post)\n";
        cout << "25. Check Sensitive Words (Reply)\n";
        cout << "26. Report a Post\n";
        cout << "27. View Marked Posts\n";
        cout << "28. Save Report Data To File\n";
        cout << "29. Load Report Data From File\n";
        cout << "30. Run Member4 Unit Test\n";
        cout << "0. Exit\n";
        cout << "Please input option：";
        
        string optionLine;
        getline(cin, optionLine);
        stringstream optionStream(optionLine);
        char extra;
        if(!(optionStream >> opt) ||
            (optionStream >> extra))
        {
            cout << "Invalid input. Please enter only a number.\n";
            continue;
        }
        if(opt == 0) break;
        else if(opt ==1)
        {
            mgr.printBoards();
        }
        else if(opt ==2)
        {
            int bid,uid;
            string title,content;
            cout << "Input Board ID："; 
            cin>>bid;
            cout << "Input Simulated User ID (Passed from login module)："; 
            cin>>uid;
            cin.ignore(10000, '\n');
            cout << "Post Title："; 
            getline(cin,title);
            cout << "Post Content："; 
            getline(cin,content);
            
            CheckResult cr = security.checkPost(title, content);
            if(!cr.allowed) {
                cout << "[BLOCKED] " << cr.message << "\n";
                cout << "Matched words: ";
                for(auto& w : cr.matchedWords) cout << w << " ";
                cout << "\n";
                continue;
            }
            
            int newPid = mgr.createPost(bid,uid,title,content);
            if(newPid > 0)
                cout<<"Post created successfully, postId="<<newPid<<endl;
        }
        else if(opt ==3)
        {
            int bid;
            cout << "Input Board ID："; 
            cin>>bid;
            mgr.printBoardPosts(bid);
        }
        else if(opt ==4)
        {
            int pid;
            cout << "Input Post ID："; 
            cin>>pid;
            mgr.printPostDetail(pid);
            if(reportMgr.isMarked(pid)) {
                cout << "[WARNING] This post has been marked by reports.\n";
            }
        }
        else if(opt ==5)
        {
            int pid, ruid; string c;
            cout << "Reply to Post ID："; 
            cin>>pid; 
            cin.ignore(10000, '\n');
            cout << "Input Reply Author User ID (Passed from login module)："; 
            cin>>ruid; 
            cin.ignore(10000, '\n');
            cout << "Reply Content："; 
            getline(cin,c);
            
            CheckResult cr = security.checkReply(c);
            if(!cr.allowed) {
                cout << "[BLOCKED] " << cr.message << "\n";
                cout << "Matched words: ";
                for(auto& w : cr.matchedWords) cout << w << " ";
                cout << "\n";
                continue;
            }
            
            int rid = mgr.createReply(pid, ruid, c);
            if(rid > 0)
                cout<<"Reply created successfully replyId="<<rid<<endl;
        }
        else if(opt ==6)
        {
            mgr.runUnitTest();
        }
        else if(opt ==7)
        {
            mgr.saveToFile("member1_data.txt");
        }
        else if(opt ==8)
        {
            mgr.loadFromFile("member1_data.txt");
        }
        else if(opt == 9)
        {
            int postId;
            int voteOption;
            
            if(currentLoginUserId == -1)
            {
                cout << "Please login first.\n";
                continue;
            }
            int userId = currentLoginUserId;
            cout << "Input Post ID: ";
            cin >> postId;
            cout << "1. Like\n";
            cout << "2. Dislike\n";
            cout << "Input vote option: ";
            cin >> voteOption;
            VoteType voteType;
            if(voteOption == 1)
            {
                voteType = UP_VOTE;
            }
            else if(voteOption == 2)
            {
                voteType = DOWN_VOTE;
            }
            else
            {
                cout << "Invalid vote option.\n";
                continue;
            }
            popularity.votePost(
                postId,
                userId,
                voteType
            );
        }
        else if(opt == 10)
        {
            int postId;
            int replyId;
            int voteOption;
            if(currentLoginUserId == -1)
            {
                cout << "Please login first.\n";
                continue;
            }
            int userId = currentLoginUserId;
            cout << "Input Post ID: ";
            cin >> postId;
            cout << "Input Reply ID: ";
            cin >> replyId;
            cout << "1. Like\n";
            cout << "2. Dislike\n";
            cout << "Input vote option: ";
            cin >> voteOption;
            VoteType voteType;
            if(voteOption == 1)
            {
                voteType = UP_VOTE;
            }
            else if(voteOption == 2)
            {
                voteType = DOWN_VOTE;
            }
            else
            {
                cout << "Invalid vote option.\n";
                continue;
            }
            popularity.voteReply(
                postId,
                replyId,
                userId,
                voteType
            );
        }  
        else if(opt == 11)
        {
            int limit;
            int boardId;
            cout << "Input ranking limit: ";
            cin >> limit;
            cout << "Input Board ID (-1 for all boards): ";
            cin >> boardId;
            popularity.printHotPosts(
                limit,
                boardId
            );
        }
        else if(opt == 12)
        {
            int limit;
            int boardId;
            cout << "Input post limit: ";
            cin >> limit;
            cout << "Input Board ID (-1 for all boards): ";
            cin >> boardId;
            popularity.printLatestPosts(
                limit,
                boardId
            );
        }
        else if(opt == 13)
        {
            int postId;
            cout << "Input Post ID: ";
            cin >> postId;
            popularity.printPostPopularity(postId);
        }
        else if(opt == 14)
        {
            string uname, pwd;
            cout << "Input Username: "; cin >> uname;
            cout << "Input Password: "; cin >> pwd;
            cin.ignore(10000, '\n');
            int uid = um.registerUser(uname, pwd);
            if(uid > 0) cout << "Register success, userId = " << uid << "\n";
        }
        else if(opt == 15)
        {
            string uname, pwd;
            cout << "Input Username: "; cin >> uname;
            cout << "Input Password: "; cin >> pwd;
            cin.ignore(10000, '\n');
            int uid = um.login(uname, pwd);
            if(uid > 0) cout << "Login success, userId = " << uid << "\n";
        }
        else if(opt == 16)
        {
            um.logout();
            cout << "Logged out.\n";
        }
        else if(opt == 17)
        {
            int postId;
            cout << "Input Post ID: "; cin >> postId;
            cin.ignore(10000, '\n');
            int r = um.toggleCollection(postId);
            if(r == 1) cout << "Collection toggled.\n";
        }
        else if(opt == 18)
        {
            um.printMyPosts();
        }
        else if(opt == 19)
        {
            um.printMyReplies();
        }
        else if(opt == 20)
        {
            um.printMyCollections();
        }
        else if(opt == 21)
        {
            um.saveToFile("member3_data.txt");
        }
        else if(opt == 22)
        {
            um.loadFromFile("member3_data.txt");
        }
        else if(opt == 23)
        {
            um.runUnitTest();
        }
        else if(opt == 24)
        {
            string title, content;
            cout << "Post Title: ";
            getline(cin, title);
            cout << "Post Content: ";
            getline(cin, content);
            CheckResult cr = security.checkPost(title, content);
            cout << (cr.allowed ? "PASS" : "BLOCKED") << ": " << cr.message << "\n";
            if(!cr.matchedWords.empty()) {
                cout << "Matched words: ";
                for(auto& w : cr.matchedWords) cout << w << " ";
                cout << "\n";
            }
        }
        else if(opt == 25)
        {
            string content;
            cout << "Reply Content: ";
            getline(cin, content);
            CheckResult cr = security.checkReply(content);
            cout << (cr.allowed ? "PASS" : "BLOCKED") << ": " << cr.message << "\n";
            if(!cr.matchedWords.empty()) {
                cout << "Matched words: ";
                for(auto& w : cr.matchedWords) cout << w << " ";
                cout << "\n";
            }
        }
        else if(opt == 26)
        {
            int pid;
            string uname, reason;
            cout << "Input Post ID: ";
            cin >> pid;
            cin.ignore(10000, '\n');
            cout << "Input Username: ";
            getline(cin, uname);
            cout << "Input Report Reason: ";
            getline(cin, reason);
            ReportStatus s = security.reportPost(pid, uname, reason);
            if(s == ReportStatus::SUCCESS) {
                cout << "Report submitted successfully.\n";
                if(security.isPostMarked(pid)) {
                    cout << "This post is now marked (threshold reached).\n";
                }
            } else if(s == ReportStatus::DUPLICATE) {
                cout << "You have already reported this post.\n";
            } else {
                cout << "Invalid input.\n";
            }
        }
        else if(opt == 27)
        {
            vector<int> marked = reportMgr.getMarkedPosts();
            cout << "\n--- Marked Posts ---\n";
            if(marked.empty()) {
                cout << "No marked posts.\n";
            } else {
                for(int id : marked) {
                    cout << "Post ID: " << id 
                         << " (Reports: " << reportMgr.getReportCount(id) << ")\n";
                }
            }
        }
        else if(opt == 28)
        {
            if(reportMgr.saveToFile("reports.csv")) {
                cout << "Report data saved to reports.csv\n";
            } else {
                cout << "Failed to save report data.\n";
            }
        }
        else if(opt == 29)
        {
            reportMgr.loadFromFile("reports.csv");
            cout << "Report data loaded from reports.csv\n";
        }
        else if(opt == 30)
        {
            runMember4UnitTest();
        }
        else
        {
            cout << "Invalid option.\n";
        }
    }      
    
    mgr.saveToFile("member1_data.txt");
    um.saveToFile("member3_data.txt");
    reportMgr.saveToFile("reports.csv");
    
    cout << "Program Terminated\n";
    return 0;
}
