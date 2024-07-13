#include "common.hpp"

// #include <stdlib.h>
// #include <unistd.h>

// 客户端
class Client {
private:
    asio::io_context context;
    // tcp::socket *ss;
    shared_ptr<tcp::socket> ss;

    // std::thread *t1;
    // std::thread *t2;

    shared_ptr<std::thread> t1;
    shared_ptr<std::thread> t2;

    int successFlag = 0;

public:
    // 构造函数,初始化
    Client() {

    }

    void start_service() {
        debug("start_service");

        // 连接, 服务器线程
        int ret = connect_server();
        if (!ret) {
            cout << "服务器,连接失败" << endl;
            return ;
        }

        // 开启线程, 启动ui
        // t2 = new std::thread( &Client::first_ui, this );
        t2 = make_shared<std::thread>( &Client::first_ui, this );

        // 主线程,等待结束
        t1->join();
        t2->join();

    }
    
    // ui界面
    void first_ui(){
        debug("first_ui");

        int ch; 
        string inputStr;
        while (true) {
            // debug("1");
            system("clear");
            // debug("2");

            cout << "        chat-room" << endl;
            cout << "    1) 登录" << endl;
            cout << "    2) 退出" << endl;
            
            cout << "请输入,你的选择" << endl;
            // cin >> ch;
            getline(cin, inputStr);

            if (! isNumeric(inputStr)) {
                cout  << "请输入数字" << endl;
            }
            ch = stoi(inputStr);
            // getchar();

            switch (ch) {
                case 1:
                    ui_login();
                break;

                case 2:
                    exit(1);
                break;
                
                default:
                    cout << "输入错误" << endl;
                break;
            }
            getchar();
        }
        cout << "退出" << endl;
        // getchar();
    }

    // 登录的ui
    void ui_login() {
        system("clear");

        debug("ui_login");

        // 客户端,发送的数据
        ClientData data;
        data.type = FUNC_USER_LOGIN;

        cout << "        chat-room" << endl << endl;
        cout << "请输入,你的用户名" << endl;
        // cin >> data.userName;
        cin.getline(data.userName, sizeof(data.userName) );

        // getchar();

        cout << "请输入,你的密码" << endl;
        // cin >> data.passwd;
        // getline(cin, data.passwd);
        cin.getline(data.passwd, sizeof(data.passwd) );
        
        // getchar();

        sendMsg(&data);

        // 等待1秒
        // sleep(1);
        getchar();

        if (successFlag == 1) {
            // cout << "登录成功" << endl;
            ui_chat();

        }

    }
    
    // 连接服务器
    int connect_server() {
        debug("connect_server");
        // 创建,客户端,套接字
        // ss(context);
        // ss = new tcp::socket(context);
        ss =  make_shared<tcp::socket>(context);

        // 创建端点
        tcp::endpoint ep(ip::address::from_string("127.0.0.1"), SERVER_PORT);

        try {
            debug("connect before");

            // 客户端,连接服务器 
            ss->connect(ep);

            debug("connect after");

            // 创建,一个线程,去处理它
            // t1 = std::thread(  &Client::thread_read, this );
            // t1 = new std::thread(  &Client::thread_read, this );

            t1 = make_shared<std::thread>( &Client::thread_read, this );

        } catch(exception &e) {
            // cout << "连接失败" << endl;
            debug("连接失败");
            return 0;
        }
        return 1;
    }

    // 聊天的页面
    void ui_chat () {
        // 输入的字符串
        string inputStr;

        while (1) {
            system("clear");
            debug("ui_chat");

            cout << "        chat-room" << endl << endl;
            cout << "    1) 列出,当前的用户" << endl;
            cout << "    2) 聊天,当前的用户" << endl;
            cout << "    3) 退出" << endl << endl;
            
            int ch;
            cout << "请输入,编号" << endl;
            // cin >> ch;

            getline(cin, inputStr);

            if (! isNumeric(inputStr)) {
                cout  << "请输入数字" << endl;
            }
            ch = stoi(inputStr);

            switch (ch) {
                case 1: // 查看,用户列表
                    get_list_user();
                break;

                case 2: //聊天
                    send_chat();

                break;
                case 3: //聊天
                    send_exit();
                break;

                default:
                    cout << "输入错误" << endl;
                    getchar();
                break;
            }

            getchar();
        }

    }


    // 退出
    void send_exit() {
        debug("send_exit");

        ClientData data;

        data.type= FUNC_USER_EXIT;
        sendMsg(&data);

        // 退出
        exit(0);
    }

    // 获取,用户列表
    void get_list_user() {
        debug("get_list_user");

        ClientData data;

        data.type= FUNC_USER_LIST;

        sendMsg(&data);
        getchar();

    }

    // 向指定的节点,发送消息
    void sendMsg(ClientData *data) {
        debug("sendMsg");

        cout << "I send " <<  data->type << endl;

        try {
            write( *ss, asio::buffer(data, sizeof(ClientData) ) );
        } catch(exception &e) {
            cout << "发送失败" << endl;
        }
        // cout << "111" << endl;

    }

    // 发送,聊天
    void send_chat() {
        debug("send_chat");

        ClientData data;
        data.type = FUNC_USER_CHAT;

        cout << "你想发送给谁" << endl;
        // cin >> data.receiver;
        // getline(cin, data.receiver);
        cin.getline( data.receiver, sizeof(data.receiver));

        // getchar();

        cout << "你想,发送的信息" << endl;
        // cin >> data.msg;
        // getline(cin, data.msg);
        cin.getline( data.msg, sizeof(data.msg));
        
        // getchar();

        sendMsg(&data);
        getchar();
    }

    // 读取的线程函数
    void thread_read() {
        debug("thread_read");

        // return;
        ServerData data;
        while (1) {
            try {
                // 读取信息
                read(*ss, asio::buffer(&data, sizeof(ServerData)) );
            } catch (exception &e) {
                cout << "服务器,出现问题了. 退出了" << endl;
                exit(1);
                break;
            }
 
            debug("I recv", data.type);

            // 对读取到的,数据,进行解析
            switch (data.type) {
                case FUNC_USER_LOGIN: // 登录
                    // debug(data.msg);

                    successFlag = data.successFlag;
                    cout << string(data.msg) << endl;
                    
                break;

                case FUNC_USER_LIST: // 用户列表
                    cout << "用户列表: " <<  data.msg << endl;

                break;

                case FUNC_USER_CHAT: // 聊天
                    cout << "聊天: " <<  data.msg << endl;

                break;

            }
        }
        
    }



};

int main() {
	
	// 调试
    Client client;
    client.start_service();

}
