#include <iostream>
#include <fcntl.h>    // 파일 및 장치 제어 (O_RDWR 등)
#include <termios.h>  // 리눅스 시리얼 포트 제어 (속도, Raw 모드)
#include <unistd.h>   // read, close 등 OS API
#include <cstring>
#include "../include/packet.h" // 32B 패킷 구조체

using namespace std;

int main() {
    cout << "🚀 Edge-Telemetry 수신 엔진 가동 준비..." << endl;

    // 1. 시리얼 포트 경로 
    // (아두이노 연결 상태에 따라 /dev/ttyUSB0 일 수도 있습니다. 확인 필요!)
    const char* port_name = "/dev/ttyACM0"; 

    // 2. 포트 열기 (읽기/쓰기 모드, 다른 프로그램 간섭 방지)
    int serial_fd = open(port_name, O_RDWR | O_NOCTTY);
    if (serial_fd < 0) {
        cerr << "❌ 에러: 시리얼 포트를 열 수 없습니다! 포트 이름을 확인하세요." << endl;
        return -1;
    }
    cout << "✅ 시리얼 포트 개방 성공!" << endl;

    // 3. termios 통신 환경 설정 (운영체제 간섭 완벽 차단)
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(serial_fd, &tty) != 0) {
        cerr << "❌ 에러: 포트 설정을 읽어올 수 없습니다." << endl;
        return -1;
    }

    // 보레이트 921,600bps 설정
    cfsetospeed(&tty, B921600);
    cfsetispeed(&tty, B921600);

    // 8-N-1 (8 데이터 비트, 파리티 없음, 1 스톱 비트) 
    tty.c_cflag &= ~PARENB; // 패리티 비트 없음
    tty.c_cflag &= ~CSTOPB; // 1 스톱 비트
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8 데이터 비트
    tty.c_cflag &= ~CRTSCTS; // 하드웨어 흐름 제어 끄기
    tty.c_cflag |= CREAD | CLOCAL; // 읽기 활성화

    // 🌟 핵심: Raw 모드 (특수문자, 줄바꿈, 개행 무시하고 바이너리 그대로 수신)
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    tty.c_oflag &= ~OPOST;

    // 블로킹 모드 (최소 1바이트가 들어올 때까지 대기)
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    // 설정 최종 적용
    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        cerr << "❌ 에러: 포트 설정을 적용할 수 없습니다." << endl;
        return -1;
    }
    cout << "✅ 921,600bps Raw 모드 세팅 완료! 데이터 수신 대기 중..." << endl;

    // TODO: 3단계 (Sync 맞추기 및 32바이트 구조체 캐스팅) 위치
    // ... 앞부분 설정 코드와 이어집니다 ...
    cout << "✅ 921,600bps Raw 모드 세팅 완료! 데이터 수신 대기 중..." << endl;

    // 🌟 3 & 4단계: 동기화(Sync) 맞추기 및 구조체 덮어씌우기
    uint8_t buffer[32];
    uint8_t byte_in;
    int state = 0;        // 0: 머리(0xAA) 찾는 중, 1: 나머지 31바이트 수집 중
    int buffer_index = 0;

    while (true) {
        // 1바이트씩 읽기 (블로킹 모드라 데이터가 없으면 여기서 대기함)
        if (read(serial_fd, &byte_in, 1) > 0) {
            
            // 상태 0: Sync Head (0xAA) 찾기
            if (state == 0) {
                if (byte_in == 0xAA) {
                    buffer[0] = byte_in;
                    buffer_index = 1;
                    state = 1; // 머리 찾음! 이제부터 몸통 수집 시작
                }
            } 
            // 상태 1: 나머지 31바이트 긁어모으기
            else if (state == 1) {
                buffer[buffer_index++] = byte_in;
                
                // 32바이트를 다 모았다면?
                if (buffer_index == 32) {
                    // 꼬리(Sync Tail, 0x55)가 제대로 들어왔는지 확인
                    if (buffer[31] == 0x55) {
                        
                        // 🚀 마법의 순간 (Memory Casting)
                        // 바이트 배열(buffer)을 TelemetryPacket32B 포인터로 강제 변환!
                        TelemetryPacket32B* packet = reinterpret_cast<TelemetryPacket32B*>(buffer);
                        
                        // 100을 곱해서 보냈던 온도를 다시 실수로 원복 (예: 2550 -> 25.5)
                        float real_temp = packet->temperature / 100.0f;

                        // 화면에 예쁘게 출력
                        cout << "[수신 완료] "
                             << "Seq: " << (int)packet->seq_num << "\t"
                             << "Temp: " << real_temp << "°C\t"
                             << "Accel X: " << packet->accel[0] << "\t"
                             << "Time: " << packet->timestamp_us << "us" << endl;
                    } else {
                        // 꼬리가 0x55가 아니면 데이터가 밀린 것 -> 가차 없이 버림
                        cerr << "⚠️ 패킷 꼬리 불일치 (데이터 밀림 감지)! 드랍합니다." << endl;
                    }
                    
                    // 다음 패킷을 받기 위해 상태 초기화
                    state = 0; 
                }
            }
        }
    }

    close(serial_fd);
    return 0;
}