#include <iostream>
#include <string>
#include <asio.hpp>
#include <thread>

#include <exception>

#include <map>
#include <memory>


using namespace std;

using namespace asio;

using asio::ip::tcp;

// typedef struct {
//     int type;
   
//     string userName;
//     string passwd;

//     string receiver; // 接收者
//     string msg; // 发送的数据
// } SendData;

typedef struct {
    int type;
   
    char userName[20];
    char passwd[20];

    char receiver[20]; // 接收者
    char msg[100]; // 发送的数据

} ClientData;

// 接收的数据
typedef struct {
    int type;
    
    int successFlag; //成功,还是,失败
    
    char msg[100]; // 发送的数据

} ServerData;

// 功能
enum {
    FUNC_USER_LOGIN,
    FUNC_USER_LIST,
    FUNC_USER_CHAT,

    FUNC_USER_EXIT,

};
// 端口号
#define SERVER_PORT 6666


// 用户的账号密码
map<string,string> userPasswdArr1 = {
    {"h1","p1"},
    {"h2","p2"},
    {"h3","p3"},

};


// 调试的开关
int debug_flag=1;

// // 打印,debug
// template<typename T> void debug(T a) {
//     if (! debug_flag) return;
//     cout << a << endl;
// }


// 辅助函数,用于递归,展开参数包并打印
void debug_helper() {
    std::cout << std::endl;  // 打印换行符作为终止条件
}
template<typename T, typename... Args>
void debug_helper(T firstArg, Args... args) {
    std::cout << firstArg << " ";
    debug_helper(args...);  // 递归调用自身，继续处理剩余的参数
}

// 主 debug 函数，调用辅助函数开始展开参数包
template<typename... Args>
void debug(Args... args) {
    debug_helper(args...);
}


// 是否数字
bool isNumeric(const string& str) {
    for (char c : str) {
        if (!isdigit(c)) {
            return false;  // 如果有任何一个字符不是数字字符，返回 false
        }
    }
    return true;  // 所有字符都是数字字符，返回 true
}
