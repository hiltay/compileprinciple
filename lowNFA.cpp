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

typedef struct FA {
    int start, end;
    VertexNode Graph[];
} FA;

class NFATools {
private:
    // nfa、dfa图的自增标号
    int nfa_counter = 0;
    int dfa_counter = 0;
    // 动态分配内存？todo
    string seen_char;

    bool is_character(char c) {
        // 数字字母下划线被认为是普通字符
        return isalnum(c) || c == '_' || c == '.';
    }

    bool is_special_character(char c) {
        // . * ? + |被认为是特殊字符
        return c == '*' || c == '?' || c == '+' || c == '|';
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


    string convert_postfix_exp(const string &infix) {
        // see:https://zh.m.wikipedia.org/zh-hans/%E8%B0%83%E5%BA%A6%E5%9C%BA%E7%AE%97%E6%B3%95
        string infix_exp = infix;
        // 输出结果为后缀表达式
        string postfix_exp;
        // 辅助栈，存放操作符
        stack<char> operators;
        // 辅助栈 存放左括号的索引位置
        stack<int> open_parenthesis_index;
        // 存放key（右括号索引）:value（左括号索引）
        unordered_map<int, int> parenthesis_index;
        // 当前的字符
        char cur_elem;
        // 保存这个正则表达式输入中出现的所有字符
        set<char> seen_character;
        // 记录上一个字符，初始为-1
        char last_char = -1;
        for (int i = 0; i < infix_exp.size(); i++) {
            cur_elem = infix_exp[i];
            // 字符数字下划线
            if (is_character(cur_elem)) {
                // 添加到set，用于后续展示nfa使用，和构造后缀表达式的逻辑无关
                seen_character.insert(cur_elem);
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
                if (cur_elem == '+') {
                    // 对于+，有两种情况：
                    if (is_character(last_char)) {
                        // 如果上一个字符是普通字符
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
                        i = parenthesis_index[i - 1] - 1;
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
        // 获取正则出现的所有字符
        for (auto it = seen_character.begin(); it != seen_character.end(); it++) {
            seen_char += *it;
        }
        // 末尾添加空串
        seen_char += '^';

        return postfix_exp;
    }


    set<int> closure(FA *nfa, int status, char c) {
        // 计算从nfa的status状态（索引）开始，只经过标号为epsilon（^）的路径到达的所有状态的索引
        set<int> result;
        // 已遍历的节点保存，防止重复遍历
        set<int> dup_check;
        // 辅助栈记录需要遍历的起始索引
        stack<int> iter_start_status;
        iter_start_status.push(status);
        while (!iter_start_status.empty()) {
            int current_index = iter_start_status.top();
            iter_start_status.pop();
            dup_check.insert(current_index);
            if (c == '^') {
                // 本身肯定是可以通过空串到达的状态，直接加入
                result.insert(nfa->Graph[current_index].index);
            }
            EdgeNode *next_e = nfa->Graph[current_index].next_edge;
            if (next_e == nullptr) continue;
            if (next_e->data == c) {
                result.insert(next_e->adjvex);
                if (dup_check.find(next_e->adjvex) == dup_check.end())
                    // 如果之前没有遍历过，则将节点入栈
                    iter_start_status.push(next_e->adjvex);
            }
            while (next_e->next != nullptr) {
                next_e = next_e->next;
                if (next_e->data == c) {
                    result.insert(next_e->adjvex);
                    if (dup_check.find(next_e->adjvex) == dup_check.end())
                        // 如果之前没有遍历过，则将节点入栈
                        iter_start_status.push(next_e->adjvex);
                }

            }

        }

        return result;

    }

    set<int> epsilon_closure(FA *nfa, set<int> status) {
        set<int> result;
        // 计算从nfa的status状态（索引）开始，只经过标号为epsilon（^）的路径到达的所有状态的索引
        for (auto it = status.begin(); it != status.end(); it++) {
            set<int> r = closure(nfa, *it, '^');
            result.insert(r.begin(), r.end());
        }
        return result;

    }


    set<int> move(FA *nfa, set<int> T, char c) {
        set<int> result;
        for (auto it = T.begin(); it != T.end(); it++) {
            set<int> r = closure(nfa, *it, c);
            result.insert(r.begin(), r.end());
        }

        return result;
    }

    bool set_is_equal(set<int> &s1, set<int> &s2) {
        if (s1.size() != s2.size()) return false;
        for (auto it1 = s1.begin(), it2 = s2.begin(); it1 != s1.end(); it1++, it2++) {
            if (*it1 != *it2) return false;
        }
        return true;

    }

    int get_new_status_index(vector<set<int>> &Dtran, set<int> &s) {
        for (int i = 0; i < Dtran.size(); i++) {
            // 遍历当前Dtran中的每个状态
            if (set_is_equal(Dtran[i], s))
                // 如果新状态与Dtran其中任何一个状态相等，则说明重复
                // 返回重复的Dtran索引
                return i;
        }
        // 没有重复，说明是新状态，返回-1
        return -1;

    }

public:
    FA *construct(const string &re) {
        // 得到后缀表达式
        string postfix_exp = convert_postfix_exp(re);
        // 确定最大状态数，以分配邻接表内存
        auto max_state_num = postfix_exp.size() * 2;
        // 构建能容纳字符长度的NFA
        FA *nfa = (FA *) malloc(sizeof(struct FA) + max_state_num * sizeof(VertexNode));
        // 辅助栈，保存两个节点的索引和边的方向
        stack<pair<int, int>> assist;

        // todo see:https://segmentfault.com/a/1190000018258326
        for (int i = 0; i < postfix_exp.size(); i++) {
            char current_char = postfix_exp[i];
            cout << "current char:" << current_char << endl;
            if (is_character(current_char) || current_char == '.') {
                // 如果是字符，构建子NFA，并将其NFA的节点索引入栈
                EdgeNode *edge = new EdgeNode(nfa_counter + 1, current_char);
                VertexNode basic_node_start = {.index=nfa_counter++, .next_edge=edge};
                VertexNode basic_node_end = {.index=nfa_counter++, .next_edge=nullptr};

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
                VertexNode basic_node_start = {.index=nfa_counter++, .next_edge=edge2};
                // 创建新节点
                VertexNode basic_node_end = {.index=nfa_counter++, .next_edge=nullptr};
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
                VertexNode basic_node_start = {.index=nfa_counter++, .next_edge=nullptr};
                VertexNode basic_node_end = {.index=nfa_counter++, .next_edge=nullptr};
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

        return nfa;
    }

    void show_nfa(FA *nfa) {
        // 输出矩阵
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

    void show_dfa(FA *dfa) {
        // 输出矩阵
        vector<int> result_matrix[dfa->end + 1][seen_char.size()-1];
        // 列字符索引映射
        unordered_map<char, int> column_index_map;
        for (int i = 0; i < seen_char.size()-1; i++) {
            column_index_map[seen_char[i]] = i;
        }
        // 已遍历的节点保存，防止重复遍历
        set<int> dup_check;
        // 辅助栈记录需要遍历的起始索引
        stack<int> iter_start_status;
        iter_start_status.push(dfa->start);
        while (!iter_start_status.empty()) {
            int current_index = iter_start_status.top();
            iter_start_status.pop();
            if(dup_check.find(current_index) != dup_check.end()) continue;

            if (dfa->Graph[current_index].next_edge != nullptr) {
                EdgeNode *next_e = dfa->Graph[current_index].next_edge;
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
        cout << "状态数：" << dfa->end + 1 << "\t";
        cout << "开始状态：" << dfa->start << "\t" << "接受状态：" << dfa->end << endl;
        cout << setw(4);
        for (int i = 0; i < seen_char.size()-1; i++) {
            cout << seen_char[i] << setw(8);
        }
        cout << endl;
        for (int i = 0; i < dfa->end + 1; i++) {
            cout << left << setw(4) << i;
            for (int j = 0; j < seen_char.size()-1; j++) {
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
        cout<<"done"<<endl;
    }


    FA *n2d(FA *nfa) {
        // nfa to dfa
        // 将T的所有状态压入stack中;
        // todo nfa结构体有问题，不用可变长结构体，改用指针+长度的形式？
        stack<pair<set<int>, int>> st;
        //将ε-closure(T)初始化为T;
        //while(stack非空){
        //    将栈顶元素t弹出栈;
        //    for(每个满足如下条件的u：从t出发有一个标号为ε的转换到达状态u)
        //        if(u不在ε-closure(T)中){ todo
        //            将u加入到ε-closure(T)中;
        //            将u压入栈stack中;
        //        }
        //}.
        vector<set<int>> Dtran;

        // 开始状态
        set<int> result = closure(nfa, nfa->start, '^');
        Dtran.push_back(result);
        st.push(make_pair(result, dfa_counter));
        // 构建能容纳字符长度的NFA，dfa的状态数可能是对应nfa的状态数的指数（实践中一般不会）
        // 最坏情况，假设nfa状态数为n，dfa状态数可能为2^n
//        FA *dfa = new FA();
        FA *dfa = (FA *) malloc(sizeof(struct FA) + 100*nfa->end * sizeof(VertexNode));
        VertexNode new_node = {.index=dfa_counter, .next_edge=nullptr};
        dfa->Graph[dfa_counter++] = new_node;

        while (!st.empty()) {
            // 将栈顶元素初始状态集合T弹出栈;
            set<int> T = st.top().first;
            int edge_out_index = st.top().second;
            st.pop();
            for (int i = 0; i < seen_char.size() - 1; i++) {
                // for(每个满足如下条件的u：从index出发有一个标号为seen_char[i]的转换到达状态u)
                // 计算 move(index,seen_char[i])
                set<int> move_result = move(nfa, T, seen_char[i]);
                move_result = epsilon_closure(nfa, move_result);
                // ε-closure(move(T,a))的结果查看是否是新状态
                int new_status_index = get_new_status_index(Dtran, move_result);
                if (new_status_index == -1) {
                    // 添加到栈、Dtran
                    Dtran.push_back(move_result);
                    st.push(make_pair(move_result, dfa_counter));
                    // 新状态，构造图
                    EdgeNode *edge = new EdgeNode(dfa_counter, seen_char[i]);
                    VertexNode node = {.index=dfa_counter, .next_edge=nullptr};
                    // 添加一条边，插入到邻接表链表末尾
                    EdgeNode *tmp_edge = dfa->Graph[edge_out_index].next_edge;
                    if (tmp_edge == nullptr) dfa->Graph[edge_out_index].next_edge = edge;
                    else {
                        while (tmp_edge->next != nullptr) {
                            tmp_edge = tmp_edge->next;
                        }
                        tmp_edge->next = edge;
                    }
                    // 添加新状态
                    dfa->Graph[dfa_counter++] = node;
                } else {
                    // 旧状态，构造图
                    EdgeNode *edge = new EdgeNode(new_status_index, seen_char[i]);
                    // 添加一条边，插入到邻接表链表末尾
                    EdgeNode *tmp_edge = dfa->Graph[edge_out_index].next_edge;
                    if (tmp_edge == nullptr) dfa->Graph[edge_out_index].next_edge = edge;
                    else {
                        while (tmp_edge->next != nullptr) {
                            tmp_edge = tmp_edge->next;
                        }
                        tmp_edge->next = edge;
                    }
                }
            }
        }
        dfa->start = 0;
        dfa->end = dfa_counter - 1;
        return dfa;
    }
};

int main() {
    // 正则表达式支持：字母、数字、下划线，特殊字符. + ? * | ，小括号()
    // 定义 ^ 代表空串 & 代表连接
    // . 在构建nfa状态转换图时，直接视作普通字符
    string re = "(a|b)*abb";

    NFATools tools = NFATools();
    FA *result = tools.construct(re);
    tools.show_nfa(result);
    FA *dfa = tools.n2d(result);
    tools.show_dfa(dfa);
    return 0;

    // leetcode:https://leetcode.cn/problems/Valid-Number/
}