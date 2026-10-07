# Campus Forum System 

## Project Overview

This is a comprehensive C++ training project for a software engineering course, implementing a complete **console-based campus forum system** using only the C++ standard library. The system follows a "Board-Post-Reply" core business model and covers content publishing, user interaction, popularity ranking, and content security management throughout the full stack — with zero third-party dependencies and cross-platform compilation support.

## Development Environment

- **Programming Language**: Standard C++ (compatible with C++11 and above)
- **Compiler**: GCC / MinGW-w64 / MSVC / Clang
- **Development Tools**: VS Code / Dev-C++ / CLion / Visual Studio
- **Supported Platforms**: Windows / Linux / macOS (full cross-platform compatibility)

---

## Core Features

### 1. Multi-Board Content Management
- Built-in three boards: Study, Campus Life, Sports & Entertainment
- Post creation, reply management, and board-based browsing
- Automatic post downgrading (sinking old posts) and reply-driven revival mechanism

### 2. Complete User System
- User registration and login with global session management
- Post bookmarking/collection
- Personal center (My Posts / My Replies / My Collections)

### 3. Interaction & Popularity Algorithm
- Double-dimension upvote/downvote for posts and replies
- Vote cancellation and vote change support
- Popularity model based on interaction-weighted scoring and time decay
- Hot posts ranking and latest posts ranking

### 4. Intelligent Content Security
- Multi-form sensitive word detection resistant to full-width characters, separator insertion, case confusion, and special symbol embedding
- User reporting system with threshold-based post marking mechanism

### 5. Reliable Data Persistence
- Two storage formats: length-prefix serialization + CSV
- Automatic and manual save/load modes with strong compatibility and fault tolerance

### 6. Comprehensive Testing System
- Three core modules with built-in unit tests covering normal flows, edge cases, and dirty data handling

---

## System Architecture

The system adopts a layered modular architecture with clear responsibilities and low coupling:

| Layer | Module | Description |
|---|---|---|
| **Data Layer** | `member1` | `PostReplyManager` maintains core data for boards, posts, and replies; provides CRUD and state refresh capabilities |
| **Business Layer** | `member2` | Popularity & voting module — maintains voting data independently, implements popularity calculation and ranking |
|  | `member3` | User & collection module — handles account system, login state, and collection management |
|  | `member4` | Content security module — implements sensitive word filtering and reporting, decoupled from business logic |
| **Entry Layer** | `main.cpp` | Module initialization, menu interaction, request dispatch, and global data persistence |

**Design Principles**: Business modules share data layer instances via reference for data consistency; security module is independently encapsulated for standalone replacement and testing.

---

## Directory Structure

```
Campus-BBS/
├── main.cpp              # Main program entry: menu and control logic
├── member1.h             # Post & reply management module header
├── member1.cpp           # Post & reply management implementation
├── member2.h             # Popularity & voting module header
├── member2.cpp           # Popularity & voting implementation
├── member3.h             # User account module header
├── member3.cpp           # User account implementation
├── member4.h             # Content security module header
├── member4.cpp           # Content security implementation
├── sensitive_words.txt   # Optional: sensitive word dictionary file
└── README.md             # Project documentation
```

Data files auto-generated on first run: `member1_data.txt` (posts), `member3_data.txt` (users), `reports.csv` (reports).

---

## Build & Run Instructions

### Build Requirements
- Compiler must support C++11 or later
- All source files must be compiled together; compiling `main.cpp` alone is not allowed

### Command Line Build (Recommended)
```bash
# Windows (MinGW)
g++ -std=c++11 -O2 main.cpp member1.cpp member2.cpp member3.cpp member4.cpp -o MUST-PU.exe

# Linux / macOS
g++ -std=c++11 -O2 main.cpp member1.cpp member2.cpp member3.cpp member4.cpp -o MUST-PU
```

### IDE Build
- **Dev-C++ / Code::Blocks**: Create a console project, add all source files, set compiler standard to C++11, then compile and run.
- **Visual Studio**: Create an empty project, add all source files, set character set to UTF-8 and language standard to C++11 in project properties.

### Running
- Launch the program in the same directory as the executable.
- Chinese display is handled automatically on Windows via `chcp 65001`.
- Program auto-loads historical data on startup; starts fresh if no data files exist.
- Voting, bookmarking, and reporting require login first.

---

## Operation Guide

### Main Menu Overview

The program provides 30 functions in six categories:

| Category | Menu Items | Description |
|---|---|---|
| Forum Browsing | 1–5 | View boards, create post, view post, reply to post |
| Voting & Popularity | 9–13 | Upvote/downvote, hot posts ranking, latest posts, post popularity |
| User Center | 14–20 | Register, login, bookmark, view personal content |
| Content Security | 24–27 | Sensitive word check, report post, view marked posts |
| Data Management | 7/8, 21/22, 28/29 | Save/load three types of data |
| Unit Tests | 6, 23, 30 | Test suites for three modules |

### Recommended Operation Flow
1. Input `14` to register an account, then input `15` to login
2. Input `1` to view boards, select a board ID, then input `2` to create a post
3. Input `3` to browse posts in the board, input `4` to view post details and replies
4. Input `9` to upvote, input `11` to view hot posts ranking
5. Input `17` to bookmark posts, input `20` to view collections
6. Input `0` to exit — data will be saved automatically

---

## Core Mechanisms

### 1. Automatic Post Downgrading (Sinking)
Posts that haven't received a reply for 15 seconds are automatically marked as downgraded and hidden from board lists and rankings. A new reply resets the timer and restores the post to visible.

### 2. Popularity Scoring Algorithm
```
hotScore = (upvotes×3 - downvotes×2 + replies×2) / (1 + inactiveDays/2)^0.8
```
- Interaction weighting reflects content quality
- Time decay factor ensures fresh content exposure

### 3. Sensitive Word Anti-Evasion Detection
Text is normalized first: full-width to half-width, uniform lowercase, separators and special symbols removed, then substring matching. Recognizes various morphological variants. Mask replacement is also supported.

### 4. Reporting & Marking Mechanism
One user can report a post only once. After reaching the threshold (3 reports), the post is automatically marked. A warning is shown when browsing marked posts.

---

## Data Persistence

- **Posts/User Data**: Length-prefix serialization — strings are preceded by their length, preserving newlines, spaces, and special characters without truncation.
- **Report Data**: Standard CSV format with proper escaping for commas, quotes, and newlines. Compatible with Excel.
- **Sensitive Word Dictionary**: Plain text line-by-line format. Comment lines (starting with `#`) and empty lines are ignored.

---

## Unit Tests

Three independent test suites cover core functionality and edge cases:

| Test Suite | Coverage |
|---|---|
| **member1** (Menu 6) | Post validation, downgrade mechanism, reply revival — 6 test cases including time-waiting tests |
| **member3** (Menu 23) | Registration, duplicate username check, login, password validation — account core flows |
| **member4** (Menu 30) | Table-driven tests covering sensitive word variant detection, masking, reporting logic, CSV read/write, dirty data tolerance — 50+ cases with pass/fail output |

---

## Team Contributions

| Module | Responsible | Description |
|---|---|---|
| member1 | Member 1 | Core data model, post/reply management, downgrade mechanism, data serialization |
| member2 | Member 2 | Voting logic, popularity algorithm, ranking implementation |
| member3 | Member 3 | User system, login state, bookmarking, personal center |
| member4 | Member 4 | Sensitive word filtering, reporting system, CSV read/write, unit tests |
| Integration & Documentation | All Members | `main.cpp` integration, joint debugging, README writing |

---


