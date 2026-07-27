# 공통 헤더 파일 (hacket.h)
> 프로젝트에서는 32 바이트 구조체를 사용

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
