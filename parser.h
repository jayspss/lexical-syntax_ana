#ifndef CCOMPLIER_PARSER_H
#define CCOMPLIER_PARSER_H

#include "lexer.h"

//项目
typedef struct proj {
    prod p;//项目
    unsigned int lo{};//点的位置
    string next;//向前搜索符

    //重载==
    bool operator==(const proj &pr) const {
        if (p != pr.p) { return false; }
        if (lo != pr.lo) { return false; }
        if (next != pr.next) { return false; }
        return true;
    }

    bool operator!=(const proj &pr) const {
        if (p != pr.p) { return true; }
        if (lo != pr.lo) { return true; }
        if (next != pr.next) { return true; }
        return false;
    }
} proj;

//项目集
typedef struct projset {
    vector<proj> p;//项目集
    int n{};//别名
} projset;

//项目集规范族
typedef struct canclet {
    vector<projset> pjset;//项目集规范族
    map<int, map<string, int>> f;//状态输入某字符的下一状态
    map<string, string> ag;//action表和goto表，状态和输入符号确定动作
} canclet;

//状态栈和符号栈的数据结构
typedef struct wst {
    int sta{};//状态
    string wrd;//符号
} wst;

vector<tok> strtotoken(const vector<string> &v);
gra prodtogra2(const vector<prod> &p);
map<string, set<string>> getfirstnt(const gra &g);
set<string> getfirststr(const vector<string> &str, const set<string> &vt, map<string, set<string>> &fir);
vector<proj> closure(const vector<proj> &p, const gra &g, map<string, set<string>> &m);
vector<proj> go(const vector<proj> &pi, const string &sx, const gra &g, map<string, set<string>> &m);
int isincanclet(vector<proj> &p, canclet &c);
canclet gratocc(gra &g, map<string, set<string>> &m);
void crapretb(canclet &c, const gra &g);
void gctrl(const canclet &c, const vector<tok> &vt, const gra &g, int opt);
void print(int t, vector<tok> &vt);

#endif //CCOMPLIER_PARSER_H
