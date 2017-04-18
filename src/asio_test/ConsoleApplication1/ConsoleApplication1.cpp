// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include "generic_server.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <random>

using namespace boost;
using namespace boost::asio;

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream,
    const std::chrono::time_point<Clock, Duration> &time_point) {
    const time_t time = Clock::to_time_t(time_point);
#if __GNUC__ > 4 || \
    ((__GNUC__ == 4) && __GNUC_MINOR__ > 8 && __GNUC_REVISION__ > 1)
    // Maybe the put_time will be implemented later?
    struct tm tm;
    localtime_r(&time, &tm);
    return stream << std::put_time(&tm, "%c"); // Print standard date&time
#else
    char ch[26] = { 0 };
    ctime_s(ch, sizeof(ch), &time);
    return stream << ch;
#endif
}

// buffer
void test_buffer()
{


    {
        char data[] = { "fuck you" };
        std::size_t size = sizeof(data);

        buffer(data, size);

        std::string str = "from std::sting";
        buffer(str);
    }

    {
        std::array<uint8_t, 4> head = { 0xba, 0xbe, 0xfa, 0xce };
        std::string msg("CppCon Rocks!");
        std::vector<uint8_t> data(256);
        std::vector<const_buffer> bufs{ asio::buffer(head)
            , asio::buffer(msg)
            , asio::buffer(data)
        };
    }

}


void timer_expired(std::string id)
{
    std::cout << std::chrono::system_clock::now() << id << " enter.\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << std::chrono::system_clock::now() << id << " leave.\n";
}
class Test1
{

};

class Test: public boost::enable_shared_from_this<Test>
{
public:
    //析构函数
    ~Test() { std::cout << "Test Destructor." << std::endl; }
    //获取指向当前对象的指针
    boost::shared_ptr<Test> GetObject()
    {
        //std::shared_ptr<Test> pTest(this);
        return shared_from_this();
    }
};

int main()
{
    {
        //std::cout << "max random" << std::random_device::max();
        std::mt19937 mt(1000);
        std::uniform_int_distribution<int> dist(0, 99);
        for (int i=0; i < 100; ++i)
        {
            std::cout << dist(mt) << " ";
            if (i % 10 == 9) { std::cout << std::endl; }
        }

        std::mt19937 mt1(1001);
        for (int i=0; i < 100; ++i)
        {
            std::cout << dist(mt1) << " ";
            if (i % 10 == 9) { std::cout << std::endl; }
        }
    }

    {
        boost::shared_ptr<Test1> p1(new Test1());
        boost::shared_ptr<Test> p(new Test());
        boost::shared_ptr<Test> q = p->GetObject();

        std::cout<<"test shared_ptr"<<std::endl;
    }

    {
        io_service ios;
        strand strand(ios);

        deadline_timer t1(ios, boost::posix_time::seconds(5));        deadline_timer t2(ios, boost::posix_time::seconds(5));
        t1.async_wait(strand.wrap([](auto ... vn) { timer_expired("t1"); }));
        t2.async_wait(strand.wrap([](auto ... vn) { timer_expired("t2"); }));

        std::thread butler1([&]() {ios.run(); });
        std::thread butler2([&]() {ios.run(); });    
        butler1.join();
        butler2.join();



    }

    {
        io_service service;
        service.post([]() { std::cout << "eat\n"; });
        service.post([]() { std::cout << "drink\n"; });
        service.post([]() { std::cout << "and be merry!\n"; });

        std::thread butler([&]() {service.run(); });
        butler.join();

    }

    std::cout << "done" << std::endl;
    std::system("pause");
    return 0;
}

