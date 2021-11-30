#include "parser.h"

//string到令牌表
vector<tok> strtotoken(const vector<string> &v) {
    vector<tok> t;
    for (const auto &x:v) {
        tok to;
        unsigned int po1 = x.find_first_of('\t');
        unsigned int po2 = x.find_last_of('\t');
        to.linen = stoi(x.substr(0, po1));
        to.type = stoi(x.substr(po1 + 1, 1));
        to.cont = x.substr(po2 + 1);
        t.push_back(to);
    }
    return t;
}

//产生式到2文法
gra prodtogra2(const vector<prod> &p) {
    gra g;
    for (const auto &x:p) {
        g.pro.push_back(x);
        string t = x.left;
        g.vn.insert(t);
        if (t[0] == '[') {
            g.start = t;
        }
        for (auto y:x.right) {
            if (y[0] == '\'') {
                g.vt.insert(y);
            }
        }
    }
    return g;
}

//所有符号的first集
map<string, set<string>> getfirstnt(const gra &g) {
    map<string, set<string>> f;
    //终结符的first集中只有终结符本身
    for (const auto &x:g.vt) {
        set<string> s;
        s.insert(x);
        f[x] = s;
    }
    bool flag = true;//设置标志，在vn的first集变化时继续循环
    while (flag) {
        flag = false;
        for (auto x:g.pro) {//每一个产生式
            //左部第一个是终结符或是空
            if (x.right[0] == "'@" || *(x.right[0].begin()) == '\'') {
                if (!f[x.left].empty()) {
                    if (f[x.left].find(x.right[0]) == f[x.left].end()) {
                        flag = true;
                        f[x.left].insert(x.right[0]);
                    }
                } else {
                    flag = true;
                    f[x.left].insert(x.right[0]);
                }

            } else {
                bool next = true;
                unsigned int i = 0;
                while (next && i < x.right.size()) {
                    next = false;
                    string t = x.right[i];
                    if (g.vn.find(t) != g.vn.end()) {
                        if (!f[t].empty()) {
                            for (const auto &y:f[t]) {
                                //当前符号的非空部分加入first集
                                if (y != "'@") {
                                    if (f[x.left].find(y) == f[x.left].end()) {
                                        flag = true;
                                        f[x.left].insert(y);
                                    }
                                }
                            }
                        }
                        //当前该非终结符能推出空，继续判断下一符号
                        if (f[t].find("'@") != f[t].end()) {
                            next = true;
                            i++;
                        }
                    } else {
                        if (f[x.left].find(t) == f[x.left].end()) {
                            flag = true;
                            f[x.left].insert(t);
                        }
                    }
                }
            }
        }
    }
    return f;
}

//串str的first集
set<string> getfirststr(const vector<string> &str, const set<string> &vt, map<string, set<string>> &fir) {
    set<string> s;
    bool next = true;
    unsigned int i = 0;
    while (next && i < str.size()) {
        next = false;
        //终结符或空
        if ((vt.find(str[i]) != vt.end()) || str[i] == "'@") {
            if (str[i] != "'@") {
                if (s.find(str[i]) == s.end()) {
                    s.insert(str[i]);
                }
            } else {
                next = true;
            }
        } else {
            string st = str[i];
            for (const auto &x:fir[st]) {
                //先不添加空，并判断str的下一个符号
                if (x == "'@") {
                    next = true;
                    continue;
                }
                s.insert(x);
            }
        }
        i++;
    }
    //str能推出空
    if (next) {
        s.insert("'@");
    }
    //cout << "str:";for (auto x:str) { cout << x; }for (auto x:s) { cout << x; }cout << endl;
    return s;
}

//项目集p的闭包
vector<proj> closure(const vector<proj> &p, const gra &g, map<string, set<string>> &m) {
    vector<proj> pro;//闭包后的项目集
    pro.assign(p.begin(), p.end());//J=I
    bool flag = true;
    //对所有项目循环，再对所有产生式循环
    while (flag) {
        flag = false;
        vector<proj> pnew;//存储项目集
        pnew.assign(pro.begin(), pro.end());//Jnew=J
        for (auto &x:pnew) {
            if (x.lo < x.p.right.size()) {
                string b = x.p.right[x.lo];//点后第一个符号B
                //跳过终结符
                if (g.vn.find(b) != g.vn.end()) {
                    vector<string> v;//用以表示βa
                    v.assign((x.p.right.begin() + x.lo + 1), x.p.right.end());
                    //for (auto r:v){cout<<r<<" ";}cout<<endl;
                    v.push_back(x.next);
                    set<string> baf = getfirststr(v, g.vt, m);//βa的first集
                    //for (auto r:baf){cout<<r<<" ";}cout<<endl;
                    for (auto y:g.pro) {
                        if (y.left == b) {
                            for (const auto &z:baf) {
                                proj po;//y,b
                                if (y.right[0] != "'@") {
                                    po.lo = 0;
                                } else {
                                    po.lo = 1;
                                }
                                po.next = z;
                                po.p.left = y.left;
                                po.p.right.assign(y.right.begin(), y.right.end());
                                bool ini = false;
                                for (const auto &h:pro) {
                                    if (po == h) {
                                        ini = true;
                                        break;
                                    }
                                }
                                //找不到则插入并继续循环
                                if (!ini) {
                                    flag = true;
                                    pro.push_back(po);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //for(auto x:pro){cout<<x.p.left;for (auto y:x.p.right) { cout<< " "<< y;   }cout<<x.lo<<" "<<x.next<<endl;}
    return pro;
}

//GO(I,X)=CLOSURE(J),/I中形如A->α·Xβ则A->αX·β在J的核中，然后对核求闭包
vector<proj> go(const vector<proj> &pi, const string &sx, const gra &g, map<string, set<string>> &m) {
    vector<proj> pj;
    vector<proj> pjt;
    for (auto x:pi) {
        if (x.lo < x.p.right.size()) {
            string xb = x.p.right[x.lo];
            if (xb == sx) {
                proj p;
                p.lo = x.lo + 1;
                p.next = x.next;
                p.p.left = x.p.left;
                p.p.right.assign(x.p.right.begin(), x.p.right.end());
                pjt.push_back(p);
            }
        }
    }
    if (!pjt.empty()) {
        pj = closure(pjt, g, m);
    }
    return pj;
}

//找到某项目集在规范族中的位置
int isincanclet(vector<proj> &p, canclet &c) {
    for (int i = 0; i < c.pjset.size(); ++i) {
        if (c.pjset[i].p.size() != p.size()) {
            continue;
        }
        //对某项目集中的每个项目
        bool flag = true;
        for (const auto &x:p) {
            bool ini = false;
            for (const auto &h:c.pjset[i].p) {
                if (x == h) {//某个项目在项目集中
                    ini = true;
                    break;
                }
            }
            //x没找到
            if (!ini) {
                flag = false;
                break;
            }
        }
        if (flag) {
            return i;
        }
    }
    return -1;
}

//构建项目集规范族
canclet gratocc(gra &g, map<string, set<string>> &m) {
    canclet cc;
    proj pt;
    pt.lo = 0;
    pt.next = "#";
    pt.p.left = g.pro[0].left;
    pt.p.right.assign(g.pro[0].right.begin(), g.pro[0].right.end());

    vector<proj> ttt;
    ttt.push_back(pt);
    vector<proj> pj = closure(ttt, g, m);//C={closure({S'->·S})}
    projset pjst;//初始项目集
    pjst.p.assign(pj.begin(), pj.end());
    pjst.n = 0;//转态别名0
    cc.pjset.push_back(pjst);
    queue<projset> q;//存储待扩展项目集
    q.push(cc.pjset[0]);
    while (!q.empty()) {
        pjst = q.front();
        for (const auto &x:g.vt) {
            projset pjs;
            vector<proj> pttt = go(pjst.p, x, g, m);
            pjs.p.assign(pttt.begin(), pttt.end());
            if (!pjs.p.empty()) {
                int lo = isincanclet(pjs.p, cc);//项目集是否已存在
                //不存在
                if (lo == -1) {
                    pjs.n = (int) cc.pjset.size();//unsigned int到int,可表示的范围变小
                    cc.pjset.push_back(pjs);
                    q.push(pjs);
                } else {
                    pjs.n = lo;//！！！
                }
                cc.f[pjst.n][x] = pjs.n;//某个项目集pjst.n对某个输入x转移到下一个项目集别名pjs.n
                //cout << pjst.n << " " << x << " " << pjs.n << endl;
            }
        }
        for (const auto &x:g.vn) {
            projset pjs;
            vector<proj> pttt = go(pjst.p, x, g, m);
            pjs.p.assign(pttt.begin(), pttt.end());
            if (!pjs.p.empty()) {
                int lo = isincanclet(pjs.p, cc);
                //项目集不存在
                if (lo == -1) {
                    pjs.n = (int) cc.pjset.size();//unsigned int到int,可表示的范围变小
                    cc.pjset.push_back(pjs);
                    q.push(pjs);
                } else {
                    pjs.n = lo;//！！！
                }
                cc.f[pjst.n][x] = pjs.n;//某个项目集pjst.n对某个输入x转移到下一个项目集别名pjs.n
                //cout << pjst.n << " " << x << " " << pjs.n << endl;
            }
        }
        q.pop();
    }
    return cc;
}

//构建预测分析表
void crapretb(canclet &c, const gra &g) {
    for (const auto &x:c.pjset) {//每个项目集
        for (auto y:x.p) {//每个项目
            //移进项目
            if (y.lo < y.p.right.size()) {
                string s = y.p.right[y.lo];//点后第一个
                if (s != "'@") {
                    if (g.vt.find(s) != g.vt.end()) {
                        auto z = c.f.find(x.n);//看x.n是否有状态转换
                        if (z != c.f.end()) {
                            auto r = z->second.find(s);//查询是否有s弧
                            if (r != z->second.end()) {
                                c.ag[to_string(x.n) + s] = "S" + to_string(r->second);//将符号s和状态j移入栈
                            }
                        }
                    }
                }
            } else {//规约项目
                if (y.p.left == g.start) {
                    if (y.next == "#") {
                        c.ag[to_string(x.n) + "#"] = "acc";
                    }
                } else {
                    for (int k = 0; k < g.pro.size(); k++) {
                        if (y.p == g.pro[k]) {
                            c.ag[to_string(x.n) + y.next] = "r" + to_string(k);
                            break;
                        }
                    }
                }
            }
        }
    }
    //GO(k,A)=j
    for (const auto &y:c.f) {
        for (const auto &z:y.second) {
            if (g.vn.find(z.first) != g.vn.end()) {
                c.ag[to_string(y.first) + z.first] = to_string(z.second);
                //cout<<y.first<<" "<<z.first<<" "<<z.second<<endl;
            }
        }
    }

}

//总控程序
void gctrl(const canclet &c, const vector<tok> &vt, const gra &g, int opt) {
    stack<wst> st;
    wst wtt;
    wtt.sta = 0;

    wtt.wrd = "#";
    st.push(wtt);//初始状态和符号
    int i = 0;
    while (true) {
        int sta = st.top().sta;
        string s;
        auto x = vt[i];
        if (x.type == 1 || x.type == 4) {//关键词和界符
            s = "\'" + x.cont;
        } else if (x.type == 2) {
            s = "'ident";
        } else if (x.type == 3) {
            s = "'cons";
        } else if (x.type == 5) {
            s = "'opert";
        } else {
            s = x.cont;
        }
        //cout<<sta<<s;
        string sw = to_string(sta) + s;
        if (opt == 1 || opt == 5) { cout << "分析内容：" << x.cont << '\t' << "动作："; }
        //<< sw << "\t"
        auto it = c.ag.find(sw);
        if (it != c.ag.end()) {
            string nex = it->second;
            char t = nex[0];
            if (opt == 1 || opt == 5) { cout << nex << endl; }
            //移入
            if (t == 'S') {
                nex.erase(0, 1);
                wtt.sta = stoi(nex);
                wtt.wrd = s;
                st.push(wtt);
                i++;
            } else if (t == 'r') {//规约
                nex.erase(0, 1);
                prod p = g.pro[stoi(nex)];
                if (opt == 1 || opt == 5) { cout << '\t' << p.left << "->"; }
                for (const auto &j:p.right) {
                    if (j != "'@") {
                        //cout << "%%pop:" << st.top().sta << " " << st.top().wrd << "%%";
                        st.pop();//弹栈
                    }
                    if (opt == 1 || opt == 5) { cout << j; }
                }
                //cout << "%%pop:" << st.top().sta << " " << st.top().wrd << "%%";
                if (opt == 1 || opt == 5) { cout << endl; }
                int stat;
                stat = st.top().sta;
                string lo = to_string(stat) + p.left;
                auto tem = c.ag.find(lo);
                if (tem != c.ag.end()) {
                    //cout<<stat<<p.left<<tem->second;
                    wtt.sta = stoi(tem->second);
                    wtt.wrd = p.left;
                    st.push(wtt);
                }
            } else if (nex == "acc") {
                cout << "YES";
                break;
            }
        } else {
            cout << "NO";
            break;
        }
    }
}

void print(int t, vector<tok> &vt) {
    tok tokk;
    tokk.type = 0;
    tokk.cont = "#";
    vt.push_back(tokk);//添加输入串的#

    vector<string> v = readfile(3);
    vector<prod> p = strtoprod(v);
    gra g = prodtogra2(p);//2型文法

    //cout<<g.start;for (auto x:g.vn){cout<<x<<" ";}cout<<" vn:";for (auto x:g.vt){cout<<x<<" ";    }
    //for (auto x:g.vn) {cout << x << ": ";for (auto y:m[x]) {cout << y << " ";}cout << endl;}
    //for (auto x:g.vt) {cout << x << ": ";for (auto y:m[x]) {cout << y << " ";}cout << endl;}cout << endl;
    //system("mode con cols=列数lines=行数");

    g.vt.insert("#");//！！！
    map<string, set<string>> m = getfirstnt(g);//构建first集
    canclet c = gratocc(g, m);//项目集规范族
    crapretb(c, g);//预测分析表

    //打印项目集
    if (t == 1 || t == 3) {
        for (const auto &x:c.pjset) {
            cout << "状态" << x.n << ":";
            for (const auto &y:x.p) {
                if (y != x.p[0]) {
                    cout << "     ";
                }
                cout << y.p.left << "->";
                int i = 0;
                int k = (int) y.p.right.size();
                for (const auto &z:y.p.right) {
                    if (y.lo == i) {
                        cout << "·";
                    }
                    cout << z;
                    i++;
                }
                if (y.lo == k) {
                    cout << "·";
                }
                cout << "," << y.next << endl;
            }
        }
    }

    //打印预测分析表
    //cout << "\033[4m";//添加下划线
    ofstream outt;
    outt.open("pretb.txt", ios::out | ios::trunc);
    for (const auto &x:g.vt) {
        if (t == 1 || t == 4) { cout << '\t' << x; }
        outt << '\t' << x;
    }
    for (const auto &x:g.vn) {
        if (x != g.start) {
            if (t == 1 || t == 4) { cout << '\t' << '\t' << x; }
            outt << '\t' << '\t' << x;
        }
    }
    if (t == 1 || t == 4) { cout << endl; }
    outt << endl;
    for (const auto &x:c.pjset) {
        if (t == 1 || t == 4) { cout << x.n; }
        outt << x.n;
        string s = to_string(x.n);
        for (const auto &y:g.vt) {
            if (t == 1 || t == 4) { cout << '\t'; }
            outt << '\t';
            if (c.ag.find(s + y) != c.ag.end()) {
                if (t == 1 || t == 4) { cout << c.ag[s + y]; }
                outt << c.ag[s + y];
            }
        }
        for (const auto &y:g.vn) {
            if (y != g.start) {
                if (t == 1 || t == 4) { cout << '\t' << '\t'; }
                outt << '\t' << '\t';
                if (c.ag.find(s + y) != c.ag.end()) {
                    if (t == 1 || t == 4) { cout << c.ag[s + y]; }
                    outt << c.ag[s + y];
                }
            }
        }
        if (t == 1 || t == 4) { cout << endl; }
        outt << endl;
    }
    outt.close();

    gctrl(c, vt, g, t);//总控程序

}