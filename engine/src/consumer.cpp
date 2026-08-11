#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/packet.h" // 어제 만든 32바이트 구조체 재활용!

using namespace std;

int main() {
    cout << " 공유 메모리..." << endl;

    // 1. 어제 수신 엔진이 만들어둔 공유 메모리 
    const char* shm_name = "/telemetry_shm";
    
    // 2. 이번엔 쓸 필요 없이 '읽기 전용(O_RDONLY)'으로만 엽니다.
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "❌ 에러: 공유 메모리가 없습니다! receiver 엔진이 켜져 있는지 확인하세요." << endl;
        return -1;
    }
    
    // 3. 메모리 포인터 연결 (마법의 캐스팅)
    TelemetryPacket32B* shared_packet = (TelemetryPacket32B*)mmap(0, sizeof(TelemetryPacket32B), PROT_READ, MAP_SHARED, shm_fd, 0);

    cout << "✅ 파이프라인 연결 성공! 데이터를 읽어옵니다.\n" << endl;

    // 4. 1초에 한 번씩 데이터를 읽어와서 화면에 출력
    while (true) {
        float real_temp = shared_packet->temperature / 100.0f;
        
        cout << "[Consumer] " 
             << "온도: " << real_temp << "°C \t" 
             << "가속도X: " << shared_packet->accel[0] << " \t"
             << "시간: " << shared_packet->timestamp_us << "us" << endl;
             
        sleep(1); // 1초(1000ms) 대기
    }
    
    return 0;
}