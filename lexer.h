#ifndef CCOMPLIER_LEXER_H
#define CCOMPLIER_LEXER_H

#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <queue>
#include <stack>
using namespace std;
//产生式
typedef struct prod {
    string left;//产生式左部
    vector<string> right;//产生式右部，含多个符号

    //重载!=和==
    bool operator==(const prod &p) const {
        if (left != p.left) { return false; }
        for (int i = 0; i < right.size(); i++) {
            if (i >= p.right.size()) { return false; }
            if (right[i] != p.right[i]) { return false; }
        }
        return true;
    }

    bool operator!=(const prod &p) const {
        if (left != p.left) { return true; }
        for (int i = 0; i < right.size(); i++) {
            if (i >= p.right.size()) { return true; }
            if (right[i] != p.right[i]) { return true; }
        }
        return false;
    }
} prod;

//文法
typedef struct gra {
    set<string> vn;//非终结符集
    set<string> vt;//终结符集合
    string start;//开始符号
    vector<prod> pro;//产生式序列
} gra;

//自动机
typedef struct fa {
    set<string> state;//状态集
    set<string> word;//字符集
    string start;//开始符号
    set<string> end;//终态集
    map<string, set<string>> fun;//状态转换，当前状态和输入字符
} fa;

//子集法
typedef struct clo {
    set<string> s;//子集法产生的子集
    string sign = "n";//自己是否被标记
    string n;//重命名时用到

    //<重载，不然set无法对数据结构建立红黑树
    bool operator<(const clo &x) const { return s < x.s; }
} clo;

//token令牌表，行号、类型、内容
typedef struct tok {
    int linen{};//行号
    int type{};//类别
    string cont;//内容
} tok;

//通用异常类
class graexcep : public exception {
private:
    string msg;//错误内容
public:
    explicit graexcep(string s) {
        msg = std::move(s);
    }

    const char *what() const throw() {
        return msg.c_str();
    }
};


vector<string> readfile(int type);
vector<prod> strtoprod(const vector<string> &prods);
set<string> prodtokl(const vector<prod> &v);
gra prodtogra(const vector<prod> &p);
fa gratofa(const gra &g);
set<string> closure(const set<string> &sta, const string &w, const map<string, set<string>> &fun);
set<string> move(const set<string> &sta, const string &w, const map<string, set<string>> &fun);
fa nfatodfa(const fa &n);
vector<tok> codetotoken(int opt);


#endif //CCOMPLIER_LEXER_H
