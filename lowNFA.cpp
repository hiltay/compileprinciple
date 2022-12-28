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

typedef struct Node {
    char character;
    Node *left;
    Node *right;

    explicit Node(char c) : character(c), left(nullptr), right(nullptr) {};
} Node;

class NFATools {
private:
    // 自增标号
    int counter = 0;
    NFA nfa;

    void clear_NFA() {
        for (int i = 0; i < 200; i++) {
            for (int j = 0; j < 5; j++)
                nfa.table[i][j].clear();

        }
    }

    string convert_postfix_exp(string infix_exp) {
        stack<char> operators;
        stack<Node*> tree_node;
        char cur_elem;
        for (int i = 0; i < infix_exp.size(); i++) {
            cur_elem = infix_exp[i];
            // 字符数字下划线
            if (isalnum(cur_elem) || cur_elem == '_') {
                Node* root = new Node(cur_elem);
                tree_node.push(root);
            } else if (cur_elem == '(' || cur_elem == ')') {
                // 左右小括号
                if (cur_elem == '(' || operators.empty()) {
                    operators.push(cur_elem);
                } else {
                    while (!operators.empty() && operators.top() != '(') {
                        char c = operators.top();
                        operators.pop();
                    }
                    // 删掉栈中的(
                    operators.pop();
                }
            } else if (cur_elem) {
                // 特殊运算符
            }

        }
        return infix_exp;
    }

public:

    NFA construct(string re) {
        // 清空NFA
        clear_NFA();
        convert_postfix_exp(re);
        for (int i = 0; i < re.size(); i++) {
            char current_char = re[i];
            cout << "current char:" << re[i] << endl;
            if (isalpha(re[i])) {
                // 如果是字符，构建子NFA

            }
        }
        return nfa;
    }

};

int main() {
    // 正则表达式支持：字母、数字、下划线，特殊字符. + ? *，小括号()
    string re = "aaab1";

    NFATools tools = NFATools();
    tools.construct(re);
    return 0;
}