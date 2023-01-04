#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <iomanip>

using namespace std;

typedef struct EdgeNode {
    int adjvex;
    char data;
    EdgeNode *next;

    EdgeNode(char data) : adjvex(0), data(data), next(nullptr) {};

    EdgeNode(int adjvex, char data) : adjvex(adjvex), data(data), next(nullptr) {};

    EdgeNode(int adjvex, char data, EdgeNode *next) : adjvex(adjvex), data(data), next(next) {};
} EdgeNode;

typedef struct VertexNode {
    int index; // 索引号
    EdgeNode *next_edge;
} VertexNode;

typedef struct NFA {
    int start, end;
    string *seen_char;
    VertexNode Graph[];
} NFA;

class NFATools {
private:
    // 自增标号
    int counter = 0;

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


    pair<string, set<char>> convert_postfix_exp(const string &infix) {
        // see:https://zh.m.wikipedia.org/zh-hans/%E8%B0%83%E5%BA%A6%E5%9C%BA%E7%AE%97%E6%B3%95
        string infix_exp = infix;
        // 输出结果为后缀表达式
        string postfix_exp;
        // 辅助栈，存放操作符
        stack<char> operators;
        // 辅助栈 存放左括号的索引位置
        stack<int> open_parenthesis_index;
        // 存放key（右括号索引）:value（左括号索引）
        unordered_map<int,int> parenthesis_index;
        // 当前的字符
        char cur_elem;
        // 保存这个正则表达式输入中出现的所有字符
        set<char> seen_char;
        // 记录上一个字符，初始为-1
        char last_char = -1;
        for (int i = 0; i < infix_exp.size(); i++) {
            cur_elem = infix_exp[i];
            // 字符数字下划线
            if (is_character(cur_elem)) {
                // 添加到set，用于后续展示nfa使用，和构造后缀表达式的逻辑无关
                seen_char.insert(cur_elem);
                if (last_char != -1) {
                    // 当前为普通字符，且非首个字符，查看上一个字符是否为普通字符或者除了左括号以外的特殊字符
                    // 例如：aa )a *a +a .a ?a
//                    char last_char = infix_exp[i - 1];
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
                if (last_char != -1) {
//                    char last_char = infix_exp[i - 1];
                    if (is_character(last_char) || is_close_parenthesis(last_char)) {
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
                // 当前的左括号索引为最近一次遇见左括号
                open_parenthesis_index.push(i);
            } else if (is_close_parenthesis(cur_elem)) {
                // 右括号
                while (!is_open_parenthesis(operators.top())) {
                    // 一直出栈，直到遇到左括号
                    char c = operators.top();
                    // 将栈中字符输出
                    postfix_exp += c;
                    operators.pop();
                }
                // 记录括号匹配的索引
                parenthesis_index[i] = open_parenthesis_index.top();
                open_parenthesis_index.pop();
                // 删掉栈中的(
                operators.pop();
            } else if (is_special_character(cur_elem)) {
                // 通配符.在展示时视作普通字符，所以需要添加到set，用于后续展示nfa使用，和构造后缀表达式的逻辑无关
                if (cur_elem == '.')
                    seen_char.insert(cur_elem);
                if (cur_elem == '+') {
                    // 对于+，有两种情况：
                    if (is_character(last_char) || last_char == '.') {
                        // 如果上一个字符是普通字符或者通配符.
                        // 即：.+ a+ b+ c+ 这种情况
                        // 我们将其看做..* aa* bb* cc*，因此，我们的做法是
                        // 1、将当前的字符+替换为*
                        infix_exp[i] = '*';
                        // 2、将当前索引回退到上一个字符之前
                        // 上一个字符此时不变即可
                        i -= 2;
                        continue;
                    } else if (last_char == ')') {
                        // 如果上一个字符是右括号，则我们需要将指针回退到这个右括号对应匹配的左括号之前
                        // 由于)后面紧接着就是+，所以可以确定刚才从operators出栈的就是最近的括号
                        // 将当前的字符+替换为*
                        infix_exp[i] = '*';
                        // 因此可以直接将索引回退到最近的左括号之前
                        // 获取左括号的索引位置减1
                        i = parenthesis_index[i-1] - 1;
                        // 同时将上一个字符重新设置
                        last_char = ')';
                        continue;
                    }

                }

                // 特殊运算符.*?|
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
            last_char = cur_elem;

        }
        // 字符串扫描完毕
        // 取出栈顶的运算符，逐个放入输出中。重复执行上述操作，直至运算符栈为空
        while (!operators.empty()) {
            char opt = operators.top();
            operators.pop();
            postfix_exp += opt;
        }
        // 至此得到后缀表达式
        cout << "输入的中缀表达式：" << infix << "\n对应的后缀表达式：" << postfix_exp << endl;
        return make_pair(postfix_exp, seen_char);
    }

public:

    NFA *construct(const string &re) {
        // 得到后缀表达式
        pair<string, set<char>> result = convert_postfix_exp(re);
        string postfix_exp = result.first;
        // 确定最大状态数，以分配邻接表内存
        auto max_state_num = postfix_exp.size() * 2;
        // 构建能容纳字符长度的NFA
        NFA *nfa = (NFA *) malloc(sizeof(struct NFA) + max_state_num * sizeof(VertexNode));
        // 辅助栈，保存两个节点的索引和边的方向
        stack<pair<int, int>> assist;

        // todo see:https://segmentfault.com/a/1190000018258326
        for (int i = 0; i < postfix_exp.size(); i++) {
            char current_char = postfix_exp[i];
            cout << "current char:" << current_char << endl;
            if (is_character(current_char) || current_char == '.') {
                // 如果是字符，构建子NFA，并将其NFA的节点索引入栈
                EdgeNode *edge = new EdgeNode(counter + 1, current_char);
                VertexNode basic_node_start = {.index=counter++, .next_edge=edge};
                VertexNode basic_node_end = {.index=counter++, .next_edge=nullptr};

                // 添加到图中
                nfa->Graph[basic_node_start.index] = basic_node_start;
                nfa->Graph[basic_node_end.index] = basic_node_end;
                assist.push(make_pair(basic_node_start.index, basic_node_end.index));
            } else if (current_char == '|') {
                // | 的构建规则：如果为 |，弹出栈内两个元素 N(s)、N(t)，构建 N(r) 将其入栈（r = s|t）
                auto right_opt = assist.top();
                assist.pop();
                auto left_opt = assist.top();
                assist.pop();
                // 新节点连接两个旧节点的头
                EdgeNode *edge1 = new EdgeNode(left_opt.first, '^');
                EdgeNode *edge2 = new EdgeNode(right_opt.first, '^', edge1);
                VertexNode basic_node_start = {.index=counter++, .next_edge=edge2};
                // 创建新节点
                VertexNode basic_node_end = {.index=counter++, .next_edge=nullptr};
                // 两个旧节点尾部连接新节点
                nfa->Graph[left_opt.second].next_edge = new EdgeNode(basic_node_end.index, '^');
                nfa->Graph[right_opt.second].next_edge = new EdgeNode(basic_node_end.index, '^');

                // 添加到图中
                nfa->Graph[basic_node_start.index] = basic_node_start;
                nfa->Graph[basic_node_end.index] = basic_node_end;
                assist.push(make_pair(basic_node_start.index, basic_node_end.index));
            } else if (current_char == '*') {
                // * 的构建规则：如果为 *，弹出栈内一个元素 N(s)，构建 N(r) 将其入栈（r = s*）
                auto opt = assist.top();
                assist.pop();
                EdgeNode *edge1 = new EdgeNode(opt.first, '^');
                VertexNode basic_node_start = {.index=counter++, .next_edge=nullptr};
                VertexNode basic_node_end = {.index=counter++, .next_edge=nullptr};
                EdgeNode *edge2 = new EdgeNode(basic_node_end.index, '^', edge1);
                basic_node_start.next_edge = edge2;
                // 修改图
                nfa->Graph[opt.second].next_edge = new EdgeNode(basic_node_end.index, '^',
                                                                new EdgeNode(basic_node_start.index, '^'));

                // 添加到图中
                nfa->Graph[basic_node_start.index] = basic_node_start;
                nfa->Graph[basic_node_end.index] = basic_node_end;
                assist.push(make_pair(basic_node_start.index, basic_node_end.index));
            } else if (current_char == '&') {
                // & 的构建规则：如果为 &，弹出栈内两个元素 N(s)、N(t)，构建 N(r) 将其入栈（r = st）
                auto right_opt = assist.top();
                assist.pop();
                auto left_opt = assist.top();
                assist.pop();
                // 两个旧节点尾部连接新节点
                // 添加到图中
                nfa->Graph[left_opt.second].next_edge = new EdgeNode(right_opt.first, '^');
                assist.push(make_pair(left_opt.first, right_opt.second));
            } else if (current_char == '?') {
                // ? 的构建规则：如果为 ?，弹出栈内一个元素 N(s)，构建 N(r) 将其入栈（r = s?）
                // ? 为0次或1次，只需要添加一个空串的转换即可
                // 注意对于扩展的正则操作符，可能不会符合每次都会构建出两个状态的规律（可能不构建状态）
                auto opt = assist.top();
                // 这里不需要出栈，因为仅仅添加一条边
                EdgeNode *edge = new EdgeNode(opt.second, '^');
                // 添加一条边，插入到邻接表链表末尾
                EdgeNode *current_edge = nfa->Graph[opt.first].next_edge;
                while (current_edge->next != nullptr) {
                    current_edge = current_edge->next;
                }
                current_edge->next = edge;
            }
        }
        // 栈中应该只有一个元素，这就是nfa的起始状态和终止状态
        auto status = assist.top();
        nfa->start = status.first;
        nfa->end = status.second;
        string characters;
        // 获取正则出现的所有字符
        set<char> seen_char = result.second;
        for (auto it = seen_char.begin(); it != seen_char.end(); it++) {
            characters += *it;
        }
        // 末尾添加空串
        characters += '^';
        nfa->seen_char = &characters;
        return nfa;
    }

    void show_nfa(NFA *nfa) {
        // 输出矩阵
        string seen_char = *nfa->seen_char;
        vector<int> result_matrix[nfa->end + 1][seen_char.size()];
        // 列字符索引映射
        unordered_map<char, int> column_index_map;
        for (int i = 0; i < seen_char.size(); i++) {
            column_index_map[seen_char[i]] = i;
        }
        // 已遍历的节点保存，防止重复遍历
        set<int> dup_check;
        // 辅助栈记录需要遍历的起始索引
        stack<int> iter_start_status;
        iter_start_status.push(nfa->start);
        while (!iter_start_status.empty()) {
            int current_index = iter_start_status.top();
            iter_start_status.pop();

            if (nfa->Graph[current_index].next_edge != nullptr) {
                EdgeNode *next_e = nfa->Graph[current_index].next_edge;
                result_matrix[current_index][column_index_map[next_e->data]].push_back(next_e->adjvex);

                if (dup_check.find(next_e->adjvex) == dup_check.end())
                    // 如果之前没有遍历过，则将节点入栈
                    iter_start_status.push(next_e->adjvex);
                while (next_e->next != nullptr) {
                    next_e = next_e->next;
                    result_matrix[current_index][column_index_map[next_e->data]].push_back(next_e->adjvex);
                    if (dup_check.find(next_e->adjvex) == dup_check.end())
                        // 如果之前没有遍历过，则将节点入栈
                        iter_start_status.push(next_e->adjvex);
                }
            }
            dup_check.insert(current_index);
        }
        cout << "状态数：" << nfa->end + 1 << "\t";
        cout << "开始状态：" << nfa->start << "\t" << "接受状态：" << nfa->end << endl;
        cout << setw(4);
        for (int i = 0; i < seen_char.size(); i++) {
            cout << seen_char[i] << setw(8);
        }
        cout << endl;
        for (int i = 0; i < nfa->end + 1; i++) {
            cout << left << setw(4) << i;
            for (int j = 0; j < seen_char.size(); j++) {
                cout << '{';
                for (auto it = result_matrix[i][j].begin(); it != result_matrix[i][j].end(); it++) {
                    cout << *it;
                    if (it + 1 != result_matrix[i][j].end())
                        cout << ',';
                }
                cout << '}';
                cout << right << setw(4);
            }
            cout << endl;
        }
    }

};

int main() {
    // 正则表达式支持：字母、数字、下划线，特殊字符. + ? * | ，小括号()
    // 定义 ^ 代表空串 & 代表连接
    // . 在构建nfa状态转换图时，直接视作普通字符
    // + `a+` 可以转换为 `aa*`
    // ? `a?` 可以转换为 `(a|^)`
    // (a|b)? --> ((a|b)|^)
    // todo `.`处理？
    string re = "(a|(b|c))+";


    NFATools tools = NFATools();
    NFA *result = tools.construct(re);
    tools.show_nfa(result);

    return 0;

    // leetcode:https://leetcode.cn/problems/Valid-Number/
}