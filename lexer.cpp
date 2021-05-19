#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <cstring>

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

//读取文件，1读3型文法，2读令牌表，3读2型文法，4读测试代码
vector<string> readfile(int type) {
    if (type == 1 || type == 2 || type == 3 || type == 4) {
        ifstream inf;
        vector<string> v;
        string s;
        if (type == 1) {
            inf.open("lexgra.txt", ios::in);//只读
            string last;
            //读取注释行信息
            while (getline(inf, s)) {
                if (s[0] == '#') {
                    last = s[s.size() - 1];
                }
                if (s[0] != '#') {
                    s.insert(0, last);
                    v.push_back(s);
                }
            }
        } else if (type == 2 || type == 3) {
            string lo;
            if (type == 2) {
                lo = "token.txt";
            } else {
                lo = "syngra.txt";
            }
            inf.open(lo, ios::in);//只读
            while (getline(inf, s)) {
                v.push_back(s);
            }
        } else {
            inf.open("code.txt", ios::in);//只读
            while (getline(inf, s)) {
                int po = (int) s.find_first_of('/');
                //删除双斜杠注释
                if (po != -1) {
                    if (s[po + 1] == '/') {
                        s.erase(po);
                    }
                }
                v.push_back(s);
            }
        }
        if (v.empty()) {
            throw graexcep("文件为空！");
        }
        inf.close();
        return v;
    } else {
        throw graexcep("文件读取异常！");//抛出文件读异常
    }
}

//字符串到产生式
vector<prod> strtoprod(const vector<string> &prods) {
    vector<prod> pro;
    for (auto &x : prods) {
        int po1 = (int) x.find_first_of('-');
        prod p;
        p.left = x.substr(0, po1);//左部
        //C4018警告，x.size()为uint
        //从->开始，单或双引号后的内容取出
        for (unsigned int i = po1 + 3; i <= x.size(); i++) {
            unsigned int j = i;
            for (; j < x.size(); j++) {
                if (x[j] == '\'' || x[j] == '\"') {
                    break;
                }
            }
            p.right.push_back(x.substr(i - 1, (j - i + 1)));
            i = j;
        }
        pro.push_back(p);
    }
    return pro;
}

//处理关键字,界符
set<string> prodtokl(const vector<prod> &v) {
    set<string> k;
    for (auto x:v) {
        k.insert(x.right[0]);
    }
    return k;
}

//产生式到文法
gra prodtogra(const vector<prod> &p) {
    gra g;
    for (auto x:p) {
        string t = x.left;
        t.erase(0, 1);//去除产生式左部的类别代号
        x.left = t;
        g.pro.push_back(x);
        g.vn.insert(t);
        if (t[0] == '[') { g.start = t; }
        for (auto y:x.right) {
            if (y[0] == '\'') { g.vt.insert(y); }
        }
    }
    return g;
}

//正规文法到NFA
fa gratofa(const gra &g) {
    fa f;
    f.word = g.vt;
    f.state = g.vn;
    f.start = g.start;
    f.state.insert("\"Z");
    f.end.insert("\"Z");
    for (auto x:g.pro) {
        string s = x.left + x.right[0];
        if (x.right.size() == 1) {
            f.fun[s].insert("\"Z");
        }
        if (x.right.size() == 2) {
            f.fun[s].insert(x.right[1]);
        }
    }
    return f;
}

//sta的w闭包，w-closure(sta)
set<string> closure(const set<string> &sta, const string &w, const map<string, set<string>> &fun) {
    set<string> cl = sta;
    vector<string> v;
    v.reserve(sta.size());
    for (const auto &x:sta) {
        v.push_back(x);
    }
    //v.size在每次循环时都会重新计算，保证新插入的状态都会被计算
    for (int i = 0; i < v.size(); i++) {
        string s = v[i] + w;
        auto it = fun.find(s);
        if (it != fun.end()) {
            for (const auto &x:it->second) {
                v.push_back(x);
            }
        }
        cl.insert(v[i]);
    }
    return cl;
}

//sta的w弧转换，move(sta,w)
set<string> move(const set<string> &sta, const string &w, const map<string, set<string>> &fun) {
    set<string> m;
    for (const auto &x:sta) {
        string s = x + w;
        auto it = fun.find(s);
        if (it != fun.end()) {
            for (const auto &y:it->second) {
                m.insert(y);
            }
        }
    }
    return m;
}

//NFA到DFA
fa nfatodfa(const fa &n) {
    fa d;
    set<clo> c;
    clo cl;
    set<string> tem;
    tem.insert(n.start);
    cl.s = closure(tem, "'@", n.fun);//计算c中初始子集
    tem.clear();
    cl.n = "0";//K0被标记为0
    c.insert(cl);
    int k = 1;//给产生的不同子集标记
    set<clo>::iterator i;//指向set<clo>的指针i
    while (true) {
        //判断子集是否被标记
        for (i = c.begin(); i != c.end(); i++) {
            if (i->sign == "n") {
                break;
            }
        }
        if (i != c.end()) {
            clo t = *i;
            t.sign = "y";
            //修改set中某一元素，先删除再插入，效率很低
            c.erase(i);
            c.insert(t);
            for (const auto &a:n.word) {
                if (a != "\'@") {
                    clo tu;
                    tu.s = closure(move(t.s, a, n.fun), "'@", n.fun);
                    if (tu.s.empty()) {
                        continue;
                    }//防止算出的子集为空
                    //set<string> stt;stt= move(t.s,a,n.fun);
                    //cout<<"集合:";for(const auto& tt:t.s){cout<<tt<<" ";}
                    //cout<<"  move:";for(const auto& tt:stt){cout<<tt<<" ";}
                    //cout << "  closure:";for (const auto &tt:tu.s) { cout << tt << " "; }cout << endl;
                    tu.sign = "y";
                    auto it2 = c.find(tu);
                    if (it2 == c.end()) {
                        //cout<<"未标记 ";for(const auto& z:tu.s){cout<<z<<" ";}cout<<"  被标记为"<<k<<endl;
                        tu.sign = "n";
                        tu.n = to_string(k);//标记子集
                        c.insert(tu);
                        k++;
                    }
                }
            }
        } else {
            break;
        }
    }
    d.word = n.word;
    for (auto x:c) {
        d.state.insert(x.n);
        if (x.s.find("\"Z") != x.s.end()) { d.end.insert(x.n); }
        if (x.s == cl.s) { d.start = x.n; }
        set<string> kej;
        //用以确定当前状态可接收的字符
        for (const auto &y:d.word) {
            for (const auto &z:x.s) {
                string wor = z + y;
                auto it = n.fun.find(wor);
                if (it != n.fun.end()) {
                    kej.insert(y);
                }
            }
        }
        for (const auto &y:kej) {
            if (y != "'@") {
                string s = x.n + y;
                set<string> se;
                se = closure(move(x.s, y, n.fun), "'@", n.fun);
                for (const auto &z:c) {
                    //找到下一转态的别名并退出循环
                    if (z.s == se) {
                        d.fun[s].insert(z.n);
                        break;
                    }
                }
            }
        }
    }
    return d;
}

//令牌表初始化
vector<tok> codetotoken(int opt) {
    vector<string> prods = readfile(1);
    //先获取全部的产生式，再分成不同类别的产生式
    vector<prod> pro = strtoprod(prods);
    vector<prod> keyw, ident, cons, limt, opert;
    for (auto x:pro) {
        if (x.left[0] == '1') { keyw.push_back(x); }
        if (x.left[0] == '2') { ident.push_back(x); }
        if (x.left[0] == '3') { cons.push_back(x); }
        if (x.left[0] == '4') { limt.push_back(x); }
        if (x.left[0] == '5') { opert.push_back(x); }
    }
    pro.clear();
    /*
        gra g = prodtogra(idten);
        fa nfa = gratofa(g);
        fa dfa = nfatodfa(nfa);
        cout << "初态：" << dfa.start << endl << "状态：";
        for (const auto &x:dfa.state) { cout << x << " "; }
        cout << endl << "字母表：";
        for (const auto &x:dfa.word) { cout << x << " "; }
        cout << endl << "终态：";
        for (const auto &x:dfa.end) { cout << x << " "; }
        cout << endl;
        for (const auto &x:dfa.fun) {
            cout << x.first << "\t";
            for (auto y:x.second) { cout << y << "  "; }
            cout << endl;
        }
    }
    */
    set<string> skeyw = prodtokl(keyw);
    set<string> slimt = prodtokl(limt);
    gra gidten = prodtogra(ident);
    gra gcons = prodtogra(cons);
    gra gopert = prodtogra(opert);

    fa nfaidten = gratofa(gidten);
    fa nfacons = gratofa(gcons);
    fa nfaopert = gratofa(gopert);

    fa dfaidten = nfatodfa(nfaidten);
    fa dfacons = nfatodfa(nfacons);
    fa dfaopert = nfatodfa(nfaopert);

    vector<tok> to;//存储令牌表
    vector<string> codes = readfile(4);
    //字母和数字的集合，在识别时会被用'let和'num代替
    set<string> letter;
    set<string> num;
    for (int i = 0; i < 26; ++i) {
        char ct = (char) ('a' + i);
        char ctl = (char) ('A' + i);
        string s(1, ct);
        string sl(1, ctl);
        letter.insert(s);
        letter.insert(sl);
    }
    for (int i = 0; i < 10; ++i) {
        char c = (char) ('0' + i);
        string s(1, c);
        num.insert(s);
    }
    set<string> ope = nfaopert.word;
    //从上到下，从前到后，一个个字符处理
    for (int i = 1; i <= codes.size(); i++) {
        string line = codes[i - 1];//每一行的数据
        int linelen = (int) line.size();
        for (int j = 1; j <= linelen; ++j) {
            string ch = line.substr(j - 1, 1);
            tok t;
            if (ch == " ") {
                continue;//跳过空格
            } else if (slimt.find("\'" + ch) != slimt.end()) {//处理界符
                t.linen = i;
                t.type = 4;
                t.cont = ch;
                to.push_back(t);
            } else if (letter.find(ch) != letter.end()) {//处理标识符
                set<string> end = dfaidten.end;
                string sta = dfaidten.start;
                string cha;
                int k;
                //标识符不是一个字符，需要向后读取字符
                for (k = j; k <= linelen; k++) {
                    cha = line.substr(k - 1, 1);
                    //cout<<k<<" "<<cha<<endl;
                    string c;
                    //dfa中用let和num来代指字母和数字
                    if (letter.find(cha) != letter.end()) {
                        c = sta + "\'let";
                    } else if (num.find(cha) != num.end()) {
                        c = sta + "\'num";
                    } else {
                        break; //标识符只有字母数字，其它则出错
                    }
                    auto it = dfaidten.fun.find(c);
                    //查找下一状态，没有则退出循环
                    if (it != dfaidten.fun.end()) {
                        sta = *(it->second.begin());
                    } else {
                        break;
                    }
                }
                //退出循环后判断当前是否是终态，不是就抛出异常
                if (end.find(sta) != end.end()) {
                    t.cont = line.substr(j - 1, k - j);
                    t.linen = i;
                    if (skeyw.find("\'" + t.cont) != skeyw.end()) {
                        t.type = 1;//关键词
                    } else {
                        t.type = 2;//标识符
                    }
                    to.push_back(t);
                    j = k - 1;
                } else {
                    string stt = "第" + to_string(i) + "行的 " + line.substr(j - 1, k - j) + "识别失败，该标识符无法被识别";
                    throw graexcep(stt);
                }
            } else if (num.find(ch) != num.end()) {//处理常量
                set<string> end = dfacons.end;
                string sta = dfacons.start;
                string cha;
                int k;
                for (k = j; k <= linelen; k++) {
                    cha = line.substr(k - 1, 1);
                    string ct = sta + "\'";
                    string c = ct + cha;
                    //数字用num代指
                    if (num.find(cha) != num.end()) {
                        c.clear();
                        c = sta + "\'num";
                    }
                    auto it = dfacons.fun.find(c);
                    if (it != dfacons.fun.end()) {
                        sta = *(it->second.begin());
                    } else {
                        break;
                    }
                }
                if (end.find(sta) != end.end()) {
                    t.cont = line.substr(j - 1, k - j);
                    t.linen = i;
                    t.type = 3;
                    to.push_back(t);
                    j = k - 1;
                } else {
                    string stt = "第" + to_string(i) + "行的 " + line.substr(j - 1, k - j) + "识别失败，该常量无法被识别";
                    throw graexcep(stt);
                }
            } else if (ope.find("\'" + ch) != ope.end()) {//操作符
                set<string> end = dfaopert.end;
                string sta = dfaopert.start;//初始状态
                string cha;
                int k;
                for (k = j; k <= linelen; k++) {
                    cha = line.substr(k - 1, 1);
                    string ct = sta + "\'";
                    string c = ct + cha;
                    auto it = dfaopert.fun.find(c);
                    if (it != dfaopert.fun.end()) {
                        sta = *(it->second.begin());
                    } else {
                        break;
                    }
                }
                if (end.find(sta) != end.end()) {
                    t.cont = line.substr(j - 1, k - j);
                    t.linen = i;
                    t.type = 5;
                    to.push_back(t);
                    j = k - 1;
                } else {
                    string stt = "第" + to_string(i) + "行的 " + line.substr(j - 1, k - j) + "识别失败，该操作符无法被识别";
                    throw graexcep(stt);
                }
            } else {//若所有的dfa都不能识别，则令牌表创建失败
                string stt = "第" + to_string(i) + "识别失败，该符号无法被识别";
                throw graexcep(stt);
            }
        }
    }
    //打印token令牌表
    if (opt == 1 || opt == 2) {
        int i = 0;
        for (const auto &x:to) {
            cout << x.linen << "\t";
            if (x.type == 1) {
                cout << "关键字";
            } else if (x.type == 2) {
                cout << "标识符";
            } else if (x.type == 3) {
                cout << "常量";
            } else if (x.type == 4) {
                cout << "界符";
            } else if (x.type == 5) {
                cout << "运算符";
            }
            if (i != (to.size() - 1)) {
                cout << "\t" << x.cont << endl;
            } else {
                cout << "\t" << x.cont;
            }
            i++;
        }
    }
    ofstream out;
    out.open("token.txt", ios::out | ios::trunc);//删除并创建token.txt
    int i = 0;
    //写入token.txt
    for (const auto &x:to) {
        if (i != (to.size() - 1)) {
            out << x.linen << "\t" << x.type << "\t" << x.cont << endl;
        } else {
            out << x.linen << "\t" << x.type << "\t" << x.cont;
        }
        i++;
    }
    out.close();
    return to;
}