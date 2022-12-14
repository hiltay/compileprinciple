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

public:

    NFA construct(string re) {
        // 清空NFA
        clear_NFA();

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
    string re = "aaab";

    NFATools tools = NFATools();
    tools.construct(re);
    return 0;
}