#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
// see:https://blog.csdn.net/Ederick/article/details/8558672
/*********************正则转NFA**********************************************/
unsigned N, M;					//分别表示NFA和DFA的状态数
string s;						//正则表达式
string sub( "abcde)*+?" );		//正则表达式中需要构造子NFA的字符
stack<char> sign;				//存储操作符
stack< pair<int, int> > nfa;	//存储子NFA
vector<int> NFA[ 160 ][ 5 ];	//NFA的状态转移表

/*对于是字母（a、b、c、d、e）的NFA构造方法*/
void alpNFA( int i ) {
    pair<int, int> r;
    r.first = N++;
    r.second = N++;
    NFA[ r.first ][ i ].push_back( r.second );
    nfa.push( r );
}

/*对于是|的NFA构造方法*/
void orNFA() {
    sign.pop();
    pair<int, int> r;
    r.first = N++;
    r.second = N++;
    pair<int, int> s = nfa.top();
    nfa.pop();
    pair<int, int> t = nfa.top();
    nfa.pop();
    NFA[ r.first ][ 4 ].push_back( s.first );
    NFA[ r.first ][ 4 ].push_back( t.first );
    NFA[ s.second ][ 4 ].push_back( r.second );
    NFA[ t.second ][ 4 ].push_back( r.second );
    nfa.push( r );
}

/*对于是？的NFA构造方法*/
void queNFA() {
    pair<int, int> r;
    r.first = N++;
    r.second = N++;
    pair<int, int> s = nfa.top();
    nfa.pop();
    NFA[ r.first ][ 4 ].push_back( s.first );
    NFA[ r.first ][ 4 ].push_back( r.second );
    NFA[ s.second ][ 4 ].push_back( r.second );
    nfa.push( r );
}

/*对于是&的NFA构造方法*/
void catNFA() {
    sign.pop();
    pair<int, int> t = nfa.top();
    nfa.pop();
    pair<int, int> s = nfa.top();
    nfa.pop();
    pair<int, int> r( s.first, t.second );
    NFA[ s.second ][ 4 ].push_back( t.first );
    nfa.push( r );
}

/*对于是*的NFA构造方法*/
void starNFA() {
    pair<int, int> r;
    r.first = N++;
    r.second = N++;
    pair<int, int> s = nfa.top();
    nfa.pop();
    NFA[ r.first ][ 4 ].push_back( s.first );
    NFA[ r.first ][ 4 ].push_back( r.second );
    NFA[ s.second ][ 4 ].push_back( s.first );
    NFA[ s.second ][ 4 ].push_back( r.second );
    nfa.push( r );
}

/*对于是+的NFA构造方法*/
void plusNFA() {
    pair<int, int> r;
    r.first = N++;
    r.second = N++;
    pair<int, int> s = nfa.top();
    nfa.pop();
    NFA[ r.first ][ 4 ].push_back( s.first );
    NFA[ s.second ][ 4 ].push_back( s.first );
    NFA[ s.second ][ 4 ].push_back( r.second );
    nfa.push( r );
}

/*初始化NFA相关状态*/
void resetNFA() {
    N = 0;
    for ( int i = 0; i < 160; i++ ) {
        for ( int j = 0; j < 5; j++ )
            NFA[ i ][ j ].clear();
    }
    while ( !sign.empty() )
        sign.pop();
    while ( !nfa.empty() )
        nfa.pop();
}

/*正则表达式转NFA*/
void RE2NFA() {
    resetNFA();
    /*遍历字符串，对相关字符进行对应操作*/
    for ( unsigned i = 0; i < s.size(); i++ ) {
        if ( isalpha( s[ i ] ) ) {
            /*判断是否需要加上操作符&*/
            if ( i != 0 && sub.find( s[ i - 1 ] ) != string::npos ) {
                if ( !sign.empty() && sign.top() == '&' )
                    catNFA();
                sign.push( '&' );
            }
            alpNFA( s[ i ] - 'a' );
        }
        else if ( s[ i ] == '|' ) {
            while ( !sign.empty() && sign.top() != '(' ) {
                if ( sign.top() == '|' )
                    orNFA();
                else if ( sign.top() == '&' )
                    catNFA();
            }
            sign.push( '|' );
        }
        else if ( s[ i ] == '?' )
            queNFA();
        else if ( s[ i ] == '*' )
            starNFA();
        else if ( s[ i ] == '+' )
            plusNFA();
        else if ( s[ i ] == '(' ) {
            /*判断是否需要加上操作符&*/
            if ( i != 0 && sub.find( s[ i - 1 ] ) != string::npos ) {
                if ( !sign.empty() && sign.top() == '&' )
                    catNFA();
                sign.push( '&' );
            }
            sign.push( '(' );
        }
        else if ( s[ i ] == ')' ) {
            while ( sign.top() != '(' ) {
                if ( sign.top() == '|' )
                    orNFA();
                else if ( sign.top() == '&' )
                    catNFA();
            }
            sign.pop();
        }
    }

    /*清空操作符栈，获取最终NFA*/
    while ( !sign.empty() ) {
        if ( sign.top() == '|' )
            orNFA();
        else if ( sign.top() == '&' )
            catNFA();
    }
}

/*输出NFA的各项数据*/
void showNFA() {
    cout << "**********第一步：NFA各项数据如下***************\n";
    pair<int, int> r = nfa.top();
    cout << "状态数：" << N << "\t";
    cout << "开始状态：" << r.first << "\t" << "接受状态：" << r.second << endl;
    cout << "\ta\tb\tc\td\tε\n";
    for ( unsigned i = 0; i < N; i++ ) {
        cout << i << ":\t";
        for ( unsigned j = 0; j < 5; j++ ) {
            sort( NFA[ i ][ j ].begin(), NFA[ i ][ j ].end() );
            cout << "{";
            for ( unsigned k = 0; k < NFA[ i ][ j ].size(); k++ ) {
                cout << NFA[ i ][ j ][ k ];
                if ( k + 1 != NFA[ i ][ j ].size() )
                    cout << ",";
            }
            cout << "}\t";
        }
        cout << endl;
    }
}

int main(){
    s = "a*b+";
    RE2NFA();
    showNFA();
}