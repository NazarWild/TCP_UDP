#include "Connection.h"
#include "MessageContainerSingleton.h"
#include <thread>

int main(int argc, const char *argv[]) {
    std::thread TcpTransmitterThread(TcpTransmitter, "127.0.0.1", 5002);
    std::thread TcpReceiverThread(TcpReceiver, 5002);
    std::thread UdpReceiverThread1(UdpReceiver, 5000);
    std::thread UdpReceiverThread2(UdpReceiver, 5001);


    Message msg;
    msg.id = 25;
    msg.data = 25;
    msg.type = 4;
    msg.size = 20;

    UdpTransmitter("127.0.0.1", 5000, msg);
    UdpTransmitter("127.0.0.1", 5001, msg);
    msg.id = 26;
    msg.data = 10;
    UdpTransmitter("127.0.0.1", 5000, msg);
    msg.id = 24;
    UdpTransmitter("127.0.0.1", 5001, msg);
    msg.id = 27;
    msg.data = 300;
    UdpTransmitter("127.0.0.1", 5000, msg);
    UdpTransmitter("127.0.0.1", 5001, msg);
    msg.id = 28;
    msg.data = 400;
    UdpTransmitter("127.0.0.1", 5000, msg);
    UdpTransmitter("127.0.0.1", 5001, msg);

    std::this_thread::sleep_for(std::chrono::seconds(20));

    MessageMapSingleton::GetInstance().StopThreads();

    TcpTransmitterThread.join();
    UdpReceiverThread1.join();
    UdpReceiverThread2.join();
    TcpReceiverThread.join();

    return 0;
}
