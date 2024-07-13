#include <iostream>

#include "common.hpp"

using namespace std;

// 调试类
class Test {
    public:
        void fun(){
            tcp::acceptor *ss;
            asio::io_context *context;

            cout << handle << typeid(handle).name() << endl;

            cout << 1 << endl;
        }
    
};

int main() {
	
	// 调试
    Test test;
    test.fun();

}
