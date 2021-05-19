#include "parser.cpp"

int main() {
    try {
        int opt=0;//1打印所有，2只打印令牌表，3只打印项目集规范族，4只打印预测分析表，5只打印分析过程
        vector<tok> t = codetotoken(opt);
        print(opt, t);
        return 0;
    } catch (graexcep ge) {
        cout << ge.what();
    } catch (logic_error le) {
        cout << le.what()<<" 逻辑错误";
    }
}