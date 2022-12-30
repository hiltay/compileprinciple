#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
typedef struct NFA {
    // abcd ε
    vector<int> table[200][5];
} NFA;

typedef struct EdgeNode {
    int adjvex;
    char sign;
    EdgeNode *next;
} EdgeNode;

typedef struct VertexNode {
    int state; // 状态号
    EdgeNode *next_edge;
} VertexNode;

class NFATools {
private:
    // 自增标号
    int counter = 0;
    NFA nfa;

    bool is_character(char c) {
        // 数字字母下划线被认为是普通字符
        return isalnum(c) || c == '_';
    }

    bool is_special_character(char c) {
        // . * ? + |被认为是特殊字符
        return c == '.' || c == '*' || c == '?' || c == '+' || c == '|';
    }

    bool is_open_parenthesis(char c) {
        // 左括号
        return c == '(';
    }

    bool is_close_parenthesis(char c) {
        // 右括号
        return c == ')';
    }

    int priority(char c) {
        // 优先级
        int pri = 0;
        switch (c) {
            case '|': {
                pri = 1;
                break;
            }
            case '&': {
                pri = 2;
                break;
            }
            case '*':
            case '.':
            case '+':
            case '?': {
                pri = 3;
                break;
            }
            default:
                break;
        }
        return pri;
    }

    void clear_NFA() {
        for (int i = 0; i < 200; i++) {
            for (int j = 0; j < 5; j++)
                nfa.table[i][j].clear();

        }
    }


    string convert_postfix_exp(string infix_exp) {
        // see:https://zh.m.wikipedia.org/zh-hans/%E8%B0%83%E5%BA%A6%E5%9C%BA%E7%AE%97%E6%B3%95
        string postfix_exp;
        stack<char> operators;
        char cur_elem;
        for (int i = 0; i < infix_exp.size(); i++) {
            cur_elem = infix_exp[i];
            // 字符数字下划线
            if (is_character(cur_elem)) {
                if (i != 0) {
                    // 当前为普通字符，且非首个字符，查看上一个字符是否为普通字符或者除了左括号以外的特殊字符
                    // 例如：aa )a *a +a .a ?a
                    char last_char = infix_exp[i - 1];
                    if (is_character(last_char) ||
                        (is_special_character(last_char) && priority(last_char) >= priority('&')) ||
                        is_close_parenthesis(last_char)) {
                        // 在加入连接符之前，判断&与操作符栈顶的优先级，如果&的优先级不大于栈顶操作符的优先级
                        // 则需要将栈顶的操作符输出，直到&的优先级更大为止
                        while (!operators.empty() && priority('&') <= priority(operators.top())) {
                            // 比较当前字符和运算符栈顶字符的优先级，如果当前字符的优先级小
                            // 则取出栈顶的运算符，添加到输出中
                            char opt = operators.top();
                            postfix_exp += opt;
                            operators.pop();
                        }
                        // 加入连接符 &
                        operators.push('&');
                    }
                }
                // 将其添加到输出中
                postfix_exp += cur_elem;
            } else if (is_open_parenthesis(cur_elem)) {
                // 在入栈之前检查上一个字符是否为普通字符或者除了左括号以外的特殊字符
                // 例如： a( *( )(
                if (i != 0) {
                    char last_char = infix_exp[i - 1];
                    if (is_character(last_char) || is_special_character(last_char) || is_close_parenthesis(last_char)) {
                        // 加入连接符 &
                        while (!operators.empty() && priority('&') <= priority(operators.top())) {
                            // 比较当前字符和运算符栈顶字符的优先级，如果当前字符的优先级小
                            // 则取出栈顶的运算符，添加到输出中
                            char opt = operators.top();
                            postfix_exp += opt;
                            operators.pop();
                        }
                        operators.push('&');
                    }
                }
                // 左括号直接入栈
                operators.push(cur_elem);
            } else if (is_close_parenthesis(cur_elem)) {
                // 右括号
                while (!is_open_parenthesis(operators.top())) {
                    // 一直出栈，直到遇到左括号
                    char c = operators.top();
                    // 将栈中字符输出
                    postfix_exp += c;
                    operators.pop();
                }
                // 删掉栈中的(
                operators.pop();
            } else if (is_special_character(cur_elem)) {
                // 特殊运算符*.?+|
                while (!operators.empty() && priority(cur_elem) <= priority(operators.top())) {
                    // 比较当前字符和运算符栈顶字符的优先级，如果当前字符的优先级小
                    // 则取出栈顶的运算符，添加到输出中
                    char opt = operators.top();
                    postfix_exp += opt;
                    operators.pop();
                }
                // 将当前运算符压入栈中
                operators.push(cur_elem);
            }

        }
        // 字符串扫描完毕
        // 取出栈顶的运算符，逐个放入输出中。重复执行上述操作，直至运算符栈为空
        while (!operators.empty()) {
            char opt = operators.top();
            operators.pop();
            postfix_exp += opt;
        }
        // 至此得到后缀表达式
        cout << "输入的中缀表达式：" << infix_exp << "\n对应的后缀表达式：" << postfix_exp << endl;
        return postfix_exp;
    }

    void basic_char_node(){

    }

public:

    NFA construct(string re) {
        // 清空NFA
        clear_NFA();
        // 得到后缀表达式
        string postfix_exp = convert_postfix_exp(re);
        // 确定最大状态数，以分配邻接表内存
        auto max_state_num = postfix_exp.size() * 2;
        VertexNode NFA[max_state_num];
        // 辅助栈

        // todo see:https://segmentfault.com/a/1190000018258326
        for (int i = 0; i < postfix_exp.size(); i++) {
            char current_char = postfix_exp[i];
            cout << "current char:" << current_char << endl;
            if (is_character(current_char)) {
                // 如果是字符，构建子NFA
                EdgeNode edge = {.adjvex=counter+1,.sign=current_char,.next=nullptr,};
                VertexNode basic_node_start={.state=counter++,.next_edge=&edge};
                VertexNode basic_node_end={.state=counter++,.next_edge=nullptr};

            }
        }
        return nfa;
    }

};

int main() {
    // 正则表达式支持：字母、数字、下划线，特殊字符. + ? *，小括号()
    // b&b+
    string re = "(a|b)*abb";


    NFATools tools = NFATools();
    tools.construct(re);
    return 0;

    // leetcode:https://leetcode.cn/problems/Valid-Number/
}