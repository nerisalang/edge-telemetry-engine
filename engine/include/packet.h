#ifndef PACKET_H
#define PACKET_H

#include <cstdint>
#include <cstddef>

// 컴파일러가 임의로 메모리 패딩(빈 공간)을 넣는 것을 방지하여 정확히 32바이트로 압착합니다.
#pragma pack(push, 1)

struct TelemetryPacket32B {
    // --------------------------------------------------
    // [1. 헤더 구역: 8 Bytes]
    // --------------------------------------------------
    uint8_t  sync_head;      // 1B: 패킷의 시작을 알리는 고정 바이트 (0xAA) 
    uint8_t  seq_num;        // 1B: 0~255 순환 번호, 패킷 유실 검증용 
    uint8_t  packet_type;    // 1B: 패킷 종류 식별 (텔레메트리 0x01 고정)
    uint8_t  status_flags;   // 1B: 아두이노 상태 및 센서 에러 플래그 
    uint32_t timestamp_us;   // 4B: 아두이노 내부 micros() 타임스탬프 (지연시간 측정용)

    // --------------------------------------------------
    // [2. 페이로드 구역: 20 Bytes - 센서 데이터] 
    // --------------------------------------------------
    int16_t  accel[3];       // 6B: MPU6050 X, Y, Z 가속도 데이터
    int16_t  gyro[3];        // 6B: MPU6050 X, Y, Z 자이로 데이터
    int16_t  temperature;    // 2B: BME280 온도 데이터 (실수에 100을 곱한 정수)
    uint16_t humidity;       // 2B: BME280 습도 데이터 (실수에 100을 곱한 정수)
    uint16_t voltage_mv;     // 2B: INA219 전압 데이터 (mV 단위)
    int16_t  current_ma;     // 2B: INA219 전류 데이터 (mA 단위, 부호 있음)

    // --------------------------------------------------
    // [3. 푸터 구역: 4 Bytes] 
    // --------------------------------------------------
    uint16_t checksum;       // 2B: 헤더~페이로드 데이터 무결성 검증용 (CRC16) 
    uint8_t  reserved;       // 1B: 향후 확장을 위해 남겨둔 예약 공간 
    uint8_t  sync_tail;      // 1B: 패킷의 종료를 알리는 고정 바이트 (0x55) 
};

#pragma pack(pop)

// 사이즈가 정확히 32바이트인지 컴파일러가 강제로 확인합니다 (1ms 성능 최적화의 핵심 방어선).
static_assert(sizeof(TelemetryPacket32B) == 32, "CRITICAL: Packet size must be exactly 32 bytes!");

#endif // PACKET_H