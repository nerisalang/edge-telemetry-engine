# Edge-Telemetry Engine

> 프로젝트 기간: 26/07/27

## 1. 프로젝트 개요
> * 본 프로젝트는 물리적 센서(아두이노)로 부터 발생하는 고빈도 데이터를 리눅스 기반 엣지 서버(라즈베리 파이)에서 수집하여 1ms 미만의 결정적 지연 시간내에 다수의 네트워크 클라이언트에게 스트리밍하는 고성능 시스템 소프트웨어 구축을 목표로 합니다. 

<p align="left">
  <img src="https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white"/>
  <img src="https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black"/>
  <img src="https://img.shields.io/badge/Raspberry%20Pi-A22846?style=flat-square&logo=Raspberry%20Pi&logoColor=white"/>
</p>

## 2. 핵심 기술 목표
> * 초저지연: 하드웨어 인터럽트 발생부터 네트워크 패킷 송출까지 전 구간 지연 시간을 1ms 미만으로 유지
> * 고가용성 처리량: 초당 10,000개 이상의 바이너리 패킷을 유실 없이 처리
> * 결정성 확보: CPU Isolation 및 Real-time Scheduling을 통해 지연 시간의 표준 편차 최소화

## 3. 시스템 아키텍처
> 1. Hardware Layer: Arduino(송신) --> Uart Serial(921,600bps) --> Raspberry Pi(수신)
> 2. OS & Kernel Layer: Linux POSIX Serial API 제어 및 termios Raw Mode 적용 (커널 간섭 및 컨텍스트 스위칭 오버헤드 원천 차단)
> 3. Memory Layer: Zero-copy 기반 32B 구조체 다이렉트 캐스팅 및 IPC(Inter-Process Communication) 공유 메모리(Shared Memory) 링 버퍼 구축 (예정)
> 4. Network Layer: 비동기(Asynchronous) 소켓 프로그래밍을 통한 다중 클라이언트 초저지연 TCP/UDP 스트리밍 (예정)

## 4. 상세 기술 스택
> * 언어: C++ 20, C
> * 환경: Raspberry Pi OS, Ubuntu Linux, VS Code

## 5. 프로젝트 구조 (Directory Structure)
```text
edge-telemetry-engine/
├── README.md              # 프로젝트 메인 설명서
├── PROTOCOL.md            # 32B 바이너리 패킷 규격서
├── EdgeSender/            # [Phase 1] 아두이노 송신 펌웨어 (PlatformIO)
│   ├── include/packet.h   # 패킷 구조체 정의
│   └── src/main.cpp       # 1ms 정밀 제어 및 고속 시리얼 송신 로직
└── engine/                # [Phase 2] 라즈베리파이 C++ 수신 및 파싱 엔진
    ├── include/packet.h   # 아두이노와 100% 동일한 패킷 구조체 (메모리 규격 공유)
    └── src/receiver.cpp   # 리눅스 시리얼 포트 제어(Raw 모드) 및 바이너리 파싱 로직
```

# Phase 1 Progress: 하드웨어 통신 및 인터페이스 정의
> * 32Byte 바이너리 프로토콜 설계: Header(8Bytes) + Payload(20Bytes) + Footer(4Bytes)
> * UART 통신 규격 확립: 921,600 bps 설정
> * 프로젝트 빌드 환경인 platformio.ini 설정
> * 송신 펌웨어인 EdgeSender 구현 완료

# Phase 2 Progress: 리눅스 수신부(Receiver) 엔진 구축 및 파싱
> * 시리얼 환경 최적화: 라즈베리파이 리눅스 환경 C++ POSIX 시리얼 통신 개방 및 OS 간섭을 차단하는 Raw Mode 세팅 완료
> * 초고속 데이터 수신: 921,600bps 환경에서 1ms 주기 데이터 스트림 Zero-Drop 수신 검증
> * 바이너리 해독: 프레임 동기화(Sync Head/Tail) 탐색 알고리즘 적용 및 제로 카피(Zero-copy) 기반 32Bytes 구조체 메모리 캐스팅 파싱 검증 완료
> * 무결성 검증: CRC16 무결성 검증 로직 추가 및 IPC(공유 메모리) 기반 데이터 스트리밍 파이프라인 구축 (미완)
