#include <Arduino.h>
#include <Wire.h>
#include "packet.h"

TelemetryPacket32B packet;
uint32_t last_loop_time = 0;
uint8_t sequence_counter = 0;

// 간단한 체크섬 함수 (모든 바이트를 더함)
uint16_t calculateChecksum(uint8_t* data, size_t length) {
    uint16_t sum = 0;
    // 헤더부터 체크섬 이전 공간까지 합산 
    for(size_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum;
}

void setup() {
    // 1. 극한의 시리얼 속도 개방 (Arduino Uno 16MHz에서 오차율 0%인 500,000bps 사용!)
    // Serial.begin(500000);
    Serial.begin(921600); // 921,600bps로 설정 (Arduino Uno 16MHz 기준 오차율 0%)
    // 2. I2C 버스 오버클럭 (Fast Mode 400kHz) - 병목 제거의 핵심!
    Wire.begin();
    Wire.setClock(400000);

    // 패킷 고정값 초기화
    packet.sync_head = 0xAA;
    packet.packet_type = 0x01;
    packet.status_flags = 0x00;
    packet.reserved = 0x00;
    packet.sync_tail = 0x55;

    // INA219 전력 센서가 없다면 더미 데이터로 채움
    packet.voltage_mv = 5000; // 5.0V
    packet.current_ma = 120;  // 120mA
}

void loop() {
    uint32_t current_time = micros();

    // 1ms(1000 마이크로초)마다 한 번씩만 실행 (Deterministic Tick)
    if (current_time - last_loop_time >= 1000) {
        last_loop_time = current_time; // 시간 갱신

        // 1. 시퀀스 및 타임스탬프 갱신
        packet.seq_num = sequence_counter++;
        packet.timestamp_us = current_time;

        // --- 임시 더미 데이터 (센서 연결 전 테스트용) ---
        packet.accel[0] = 100; packet.accel[1] = 200; packet.accel[2] = 300;
        packet.gyro[0] = 10; packet.gyro[1] = 20; packet.gyro[2] = 30;
        packet.temperature = 2543; // 25.43도
        packet.humidity = 6512;    // 65.12%
        // ------------------------------------------------

        // 3. 체크섬 계산 (체크섬 필드, 예약어, 꼬리 제외한 28바이트 크기만큼 계산)
        packet.checksum = calculateChecksum((uint8_t*)&packet, 28);

        // 4. 구조체 메모리 통째로 덤프 (최고속 전송) 32바이트 덩어리를 문자열 변환 없이 그대로 긁어냄
        Serial.write((uint8_t*)&packet, sizeof(packet));
        /*
        &packet: 우리 데이터 패킷(구조체)이 메모리 어디에 저장되어 있는지 그 시작 주소를 알려줍니다.

        (uint8_t*): "지금부터 이 패킷을 데이터 구조체로 보지 말고, 그냥 1바이트짜리 숫자들이 나열된 덩어리(uint8_t)로 봐라"라고 강제로 형태를 변환(Casting)하는 겁니다.

        sizeof(packet): "전체 크기가 32바이트니까, 주소부터 시작해서 딱 32바이트만 긁어라"라는 뜻입니다.
        */
    }
}