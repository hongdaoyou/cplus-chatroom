#include <iostream>
#include "common.hpp"

using namespace std;


typedef struct {
    string name;
    int successFlag;

    int  exitFlag; // 线程,是否退出 0: 不退出, 1:退出
    tcp::socket *sock; // 用户的套接字

    std::thread *th; // 用户的线程对象

} UserInfo ;

// 服务器
class Server {
private:
    // 存储,用户登录的信息 : 用户名, 用户是否登录, 客户端的套接字
    //  userArr[100];
    map<int,UserInfo> userArr;

    map<string,string> *userPasswdArr;

    // 服务器的套接字
    // tcp::acceptor *ss;
    shared_ptr<tcp::acceptor> ss;

    // asio::io_context *context;
    shared_ptr<asio::io_context> context;

public:
    void start_service(){
        // 创建,服务器
        create_server();

        // 创建线程
        // std::thread t1(&Server::create_server, this);
    }

    //创建,服务器的函数
        // 创建套接字, 建立服务器, 监听,客户端的连接. 用户来了,就分配,一个线程
    void create_server() {
        debug("create_server");

        userPasswdArr = &userPasswdArr1;

        // 创建,io上下文
        // context = new asio::io_context();
        context = make_shared<asio::io_context>();

        // 创建,端口
        tcp::endpoint ep(ip::address::from_string("127.0.0.1"), SERVER_PORT);
        
        // 创建,服务器套接字
        // ss = new tcp::acceptor(context, ep); // 我传入了,指针,应该传入实例的
        // ss = new tcp::acceptor(*context, ep);
        ss = make_shared<tcp::acceptor>(*context, ep);

        int fd ;
        tcp::socket *cs;
        std::thread *th;
        while (true) {
            // 创建,客户端的套接字
            cs =  new tcp::socket(*context);
            
            debug("accept before");
            // 接收,客户端的连接
            ss->accept(*cs );
            debug("accept after");
            
            // 套接字的文件描述符
            fd = getFd(cs);
            
            // 保存,客户端的套接字
            userArr[fd].sock = cs;

            // 创建,一个线程函数,处理
            // std::thread *th(&Server::handle_request, this, cs);
            th = new std::thread(&Server::handle_request, this, cs);

            // 将用户的线程,也存储下来
            userArr[fd].th = th;
        
        } // end of while

    }

    // 获取,文件描述符fd
    int getFd(tcp::socket *cs) {
        return cs->native_handle();
    }
    
    // 获取,客户端的请求的函数  监听,客户端,发送信息. 处理完. 再发送过去
    void handle_request(tcp::socket *cs) {
        ClientData data;
        int fd = getFd(cs);

        // while(true
        while(! userArr[fd].exitFlag ) {
            try {
                read(*cs, asio::buffer(&data, sizeof(ClientData)));
    
            } catch (exception &e) {
                debug("一个用户,ctrl+c关闭");
                data.type = FUNC_USER_EXIT;
                
                // 处理,退出的逻辑
                handle_user_exit(cs, &data); 
                
                break; // 退出线程函数
            }
            debug("I recv:", data.type);

            switch(data.type) {
                case FUNC_USER_LOGIN: // 登录
                    handle_login(cs, &data);

                break;

                case FUNC_USER_LIST: // 用户列表
                    handle_user_list(cs, &data);

                break;

                case FUNC_USER_CHAT: // 聊天
                    handle_user_chat(cs, &data);

                break;

                case FUNC_USER_EXIT: // 退出
                    handle_user_exit(cs, &data);

                break;

                default:
                    cout << "错误的参数" << endl;
                break;
            }
        }

        // 线程退出后, 清理,它的存储信息
        userArr.erase(fd);
        debug("一个线程,结束了");
    }
    
    // 处理,登录请求的函数
    void handle_login(tcp::socket *cs, ClientData *data) {
        debug("handle_login");

        ServerData data2;
        data2.type = data->type;
    
        int successFlag = 0;

        debug("用户名,密码:", data->userName, data->passwd);

        // 查找,该用户,是否存在
        auto ptr = userPasswdArr->find(string(data->userName) );
        if (ptr != userPasswdArr->end() ) {
            string s1 = string(data->passwd);

            if (s1.compare( ptr->second) == 0) {
                successFlag = 1 ; 

                strcpy(data2.msg, "登录成功");
            } else {
                strcpy(data2.msg, "密码不正确");
            }
        } else {
            strcpy(data2.msg, "该用户,不存在");
        }
        data2.successFlag=successFlag;
        debug("登录状态:", successFlag);

        // 在服务器,设置,用户登录成功
        int fd = getFd(cs);
        userArr[fd].successFlag = successFlag; 
        userArr[fd].name = string(data->userName); 

        sendMsg(cs, &data2);
    }

    // 获取,用户列表的函数
    void handle_user_list(tcp::socket *cs, ClientData *data) {
        debug("handle_user_list");

        ServerData data2;
        // 重置
        memset(&data2, 0, sizeof(ServerData));

        data2.type = data->type;

        UserInfo *userInfo = NULL;
        
        // cout << "用户列表的长度: " << userArr.size() << endl;

        // 遍历, 用户列表
        for (auto it=userArr.begin(); it != userArr.end(); it++) {
            userInfo = &(it->second);
            
            // cout << "succesFlag:"  << userInfo->successFlag << endl;

            if (userInfo->successFlag != 1) {
                continue;
            }  

            // cout << "name:" << userInfo->name.c_str() << endl;
            strcat(data2.msg, userInfo->name.c_str());
            strcat(data2.msg, " ");
            // cout << "str:" << data2.msg << endl;
        }

        sendMsg(cs, &data2);
    }


    // 处理,用户发送信息的函数
    void handle_user_chat(tcp::socket *cs, ClientData *data) {
        debug("handle_user_chat");

        ServerData data2;
        data2.type = data->type;

        string receiver = string(data->receiver);

        // 接收者
        UserInfo *receiverInfo = NULL;

        int existFlag = 0;
        for (auto it = userArr.begin(); it != userArr.end(); it++) {
            // 当前用户在线
            cout << it->second.name << " - " << receiver << endl;
            if (it->second.name == receiver) {
                existFlag = 1;
                receiverInfo = &(it->second);
                break;
            } 
        }
        
        debug("接收者,状态 :",existFlag );
        // 存在
        if (existFlag == 0) {
            strcpy(data2.msg, "接收者,不在线");

        } else {
            strcpy(data2.msg, "接收者,在线, 已发送");

            // 发送给,接收者的数据
            ServerData data3;
            memset(&data3, 0, sizeof(ServerData));

            data3.type = data->type;
 
            // sprintf(data3.msg, "来自%s的消息: ", data->msg);
            // sprintf(data3.msg, "来自%s的消息: %s", data->userName, data->msg);

            char buf[60];
            sprintf(buf, "来自%s的消息 ", (userArr[getFd(cs)].name).c_str());

            strcat(data3.msg, buf);
            strcat(data3.msg, data->msg);

            sendMsg(receiverInfo->sock, &data3);
        }

        sendMsg(cs, &data2);
    }
    // 处理,用户退出的函数
    void handle_user_exit(tcp::socket *sock, ClientData *data) {
        debug("handle_user_exit");

        // 用户退出:  关闭资源-套接字,线程 
        int fd = getFd(sock);

        // 关闭,套接字
        close(fd);

        // 设置,线程,退出的标识
        userArr[fd].exitFlag = 1;

        // 设置,用户退出
        userArr[fd].successFlag = 0;    
    }

    // 发送的封装函数
    void sendMsg(tcp::socket *sock, ServerData *data) {
        debug("sendMsg");

        try {
            write( *sock, asio::buffer(data, sizeof(ServerData) ) );
        } catch(exception &e) {
            cout << "发送失败" << endl;
        }
    }

};


int main() {
	
    Server server;
    server.start_service();
}
