#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/packet.h"

using namespace std;

int main() {
    cout << "🍽️ [Lock-free 모드] 식사 준비 완료! 공유 레일에 접근 중..." << endl;

    const char* shm_name = "/telemetry_shm";
    
    // 식탁에 접근 (읽고 쓰기 권한 필요: tail을 업데이트해야 하므로 O_RDWR로 변경)
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "❌ 에러: 식탁이 없습니다! receiver 엔진이 켜져 있는지 확인하세요." << endl;
        return -1;
    }
    
    // 🌟 변경점: 단일 패킷에서 256개짜리 링 버퍼 전체로 캐스팅!
    TelemetryRingBuffer* shared_ring = (TelemetryRingBuffer*)mmap(0, sizeof(TelemetryRingBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    cout << "✅ 파이프라인 연결 성공! 데이터 먹방을 시작합니다.\n" << endl;

    // 내가 마지막으로 먹은 접시 번호(로컬 카운터)
    uint32_t my_tail = shared_ring->tail.load(std::memory_order_relaxed);

    while (true) {
        // 1. 주방장이 어디까지 요리했는지(head) 확인 (memory_order_acquire)
        // acquire: "주방장이 종(release)을 치기 전에 미리 읽지 마라!" 방어막
        uint32_t current_head = shared_ring->head.load(std::memory_order_acquire);

        // 2. 내가 먹을 접시(my_tail)가 주방장이 만든 접시(current_head)보다 뒤처져 있다면? (먹을 게 있다면!)
        if (my_tail < current_head) {
            
            // 밀린 데이터를 한 번에 다 퍼먹어도 되지만, 우리는 항상 "가장 최신" 데이터 1개만 원함.
            // 주방장이 너무 빨라서 100그릇을 쌓아뒀다면, 앞의 99그릇은 스킵하고 가장 마지막 그릇으로 점프!
            my_tail = current_head - 1; 
            uint32_t index = my_tail % RING_BUFFER_SIZE;

            // 3. 접시 획득! 찢어짐(Tearing) 걱정 없이 완벽하게 안전한 읽기
            TelemetryPacket32B latest_packet = shared_ring->buffer[index];

            // 4. 내가 여기까지 먹었다고 식탁에 기록 (다른 Consumer들을 위해)
            shared_ring->tail.store(current_head, std::memory_order_relaxed);
            my_tail = current_head; // 내 로컬 카운터도 업데이트

            // 5. 맛있게 냠냠 (화면 출력)
            float real_temp = latest_packet.temperature / 100.0f;
            cout << "[Consumer 냠냠] " 
                 << "온도: " << real_temp << "°C \t" 
                 << "가속도X: " << latest_packet.accel[0] << " \t"
                 << "시간: " << latest_packet.timestamp_us << "us" << endl;
        }

        // 아주 짧게 숨 고르기 (1초 대기)
        sleep(1); 
    }
    
    return 0;
}