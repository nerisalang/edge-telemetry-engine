# Phase 1
## 공통 헤더 파일 (packet.h)
> 프로젝트에서는 32 바이트 구조체를 사용

```cpp
#pragma pack(push, 1)
struct TelemetryPacket32B {
    // 1. 헤더 (8 Bytes)
    uint8_t  sync_head;      // 0xAA
    uint8_t  seq_num;
    uint8_t  packet_type;    // 0x01
    uint8_t  status_flags;
    uint32_t timestamp_us;

    // 2. 페이로드 (20 Bytes)
    int16_t  accel[3];       // MPU6050
    int16_t  gyro[3];        // MPU6050
    int16_t  temperature;    // BME280 (*100)
    uint16_t humidity;       // BME280 (*100)
    uint16_t voltage_mv;     // INA219
    int16_t  current_ma;     // INA219

    // 3. 푸터 (4 Bytes)
    uint16_t checksum;       // CRC16
    uint8_t  reserved;
    uint8_t  sync_tail;      // 0x55
};
#pragma pack(pop)
```

### 헤더 8Bytes
>sync_head – 1Byte, 패킷의 시작을 알리는 고정 바이트, 데이터가 밀렸을 때 동기화를 맞추는 핵심

>seq_num – 1Byte, 0부터 255까지 순환하는 패킷 번호, 이 번호가 끊기면 패킷 유실을 감지할 수 있음

>packet_type – 1Byte, 패킷의 종류를 식별

>status_flags – 1Byte, 아두이노의 상태나 센서 에러 정보를 비트로 표시

>timestamp_us – 4Bytes, 아두이노 내부의 micros()값, 라즈베리파이 수신 시점과 대조하여 정확한 통신 지연 시간을 측정하는데 필수

### 페이로드 20Bytes
>accel[3] – 6Bytes, MPU6050의 X,Y,Z 축 가속도 데이터임, int16_t 3개

>gyro[3] – 6Bytes, MPU6050의 X,Y,Z축 자이로 데이터임, init16_t 3개

>temperature – 2Bytes, BME280의 온도 데이터임, 아두이노에서 실수에 100을 곱해 정수로 전송함(4Bytes에서 2Bytes로 줄이기 위해) 그리고 라즈베리파이에서 다시 100.0로 나눔

>humidity – 2Bytes, BME280의 습도 데이터임, 온도와 마찬가지로 실수에 100을 곱해 전송

>voltage_mv – 2Bytes, 전압 데이터임, 소수점 계산없이 mV 그대로 전송, uint16_t

>current_ma - 2Bytes, 전류 데이터임, 방향이 있을 수 있으므로 부호 있는 정수로 mA 전송, int16_t

### 푸터 구역 4Bytes
>checksum – 2Bytes, 헤더부터 페이로드까지의 데이터 무결성을 검증하기 위한 값, CRC16

>reserved – 1Byte, 빈 공간임

>sync_tail – 1Byte, 패킷의 종료를 알리는 고정 바이트임, 시작과 끝을 모두 확인하여 데이터 신뢰성을 극대화함


## 921,600 bps 설정
> 전송시간 = (패킷크기(Bytes) * 10(bit/Bytes)) / 보레이트(bps)
> UART 통신 동기화를 맞추는 클럭이 없기 때문에 수신부에 '전송'과 '전송 완료'를 알려주는 신호가 필요하기 때문에 10
> 따라서 총 1Byte에 붙는 비트는

>Start Bit 1
>Data Bit 8
>Parity Bit 0
>Stop Bit 1

> 따라서 전송 시간 = (32 * 10) / 921,600 = 약 0.347ms
> 서버쪽에 할당되는 처리 시간은 약 0.653ms
> 처리 시간 안에
> 1. 하드웨어 인터럽트
> 2. 컨텍스트 스위칭
> 3. 버퍼 처리
> 4. 네트워크 송신

## 송신 펌웨어 EdgeSender
> packet.h에서 설계한 32바이트짜리 구조체 틀에 타임스탬프와 시퀀스 번호를 채워 데이터 변환 과정 없이 메모리에 올라간 32바이트를 Serial.write()를 통해 라즈베리파이 쪽으로 송신합니다.

> 처리 성능을 높이기 위해 메모리 자체를 바이너리 형태로 송신하는 방식이기 때문에 시리얼 모니터에서는 외계어처럼 나옵니다.

> ** 실행화면
![시리얼 모니터 화면](./images/EdgeSender%20Serial%20Monitor.png)

```cpp
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
    Serial.begin(500000);

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
```

# Phase 2
## 수신부 프로젝트 구조 세팅
>아두이노와 라즈베리파이가 똑같은 메모리 규격를 쓰게 만듦

## 리눅스 시리얼 포트 제어
>C++의 시스템 헤더(<termios.h>, <fcntl.h>)를 사용해 아두이노가 연결된 장치를 엽니다. 
>
>통신 속도는 921,600 로 고정함

## 동기화 및 메모리 확보

## 구조체 덮어씌우기
