#include <iostream>
#include <fcntl.h>    // O_RDWR, O_NOCTTY 등 파일(장치) 제어용
#include <termios.h>  // POSIX 터미널(시리얼 속도, Raw 모드) 제어용
#include <unistd.h>   // read, close 등 POSIX OS API
#include "../include/packet.h" 

using namespace std;

int main() {
    cout << "🚀 Edge-Telemetry 수신 엔진 가동 준비 완료..." << endl;

    // 1. 시리얼 포트 경로 설정
    // 아두이노가 라즈베리파이에 연결된 실제 포트 이름으로 맞춰야 합니다.
    const char* port_name = "/dev/ttyACM0"; 
    
    // TODO: 여기에 open() 시스템 콜과 termios 세팅(921,600bps, Raw Mode)이 들어갑니다.

    return 0;
}