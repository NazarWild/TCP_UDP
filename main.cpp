#include <iostream>
#include "Connection.h"
#include "MessageContainerSingleton.h"

int main(int argc, const char *argv[]) {
    std::thread UdpReceiverThread1(UdpReceiver, 5000);
    std::thread UdpReceiverThread2(UdpReceiver, 5001);
    std::thread TcpReceiverThread(TcpReceiver, 5002);
    std::thread TcpTransmitterThread(TcpTransmitter, "127.0.0.1", 5002);


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

    std::this_thread::sleep_for(std::chrono::seconds(30));

    MessageMapSingleton::GetInstance().StopThreads();

    UdpReceiverThread1.join();
    UdpReceiverThread2.join();
    TcpReceiverThread.join();
    TcpTransmitterThread.join();

    std::cout << "END";

    return 0;
}
