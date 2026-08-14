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
> 3. Memory Layer: Zero-copy 기반 32B 구조체 다이렉트 캐스팅 및 원자적 연산(std::atomic) 기반 Lock-free 링 버퍼(Ring Buffer) IPC 아키텍처
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

# Phase 2 Progress: 리눅스 수신부(Receiver) 엔진 구축 및 IPC 파이프라인 구축
> * 시리얼 환경 최적화: 라즈베리파이 리눅스 환경 C++ POSIX 시리얼 통신 개방 및 OS 간섭을 차단하는 Raw Mode 세팅 완료
> * 초고속 데이터 수신: 921,600bps 환경에서 1ms 주기 데이터 스트림 Zero-Drop 수신 검증
> * 바이너리 해독: 프레임 동기화(Sync Head/Tail) 탐색 알고리즘 적용 및 제로 카피(Zero-copy) 기반 32Bytes 구조체 메모리 캐스팅 파싱 검증 완료
> * 무결성 검증: CRC16 무결성 검증 로직 추가 
> * IPC 공유 메모리 파이프 라인 개통: IPC(공유 메모리) 기반 데이터 스트리밍 파이프라인 구축
> * SPSC Lock-free Ring Buffer 및 원자적 연산: 'std::atomic'과 'Acquire-Release' 메모리 모델을 적용하여 OS의 Mutex Lock 없이 동기화 달성. 생산자-소비자 간의 데이터 찢어짐(Data Tearing)을 하드웨어 레벨에서 원천 차단

# phase 3 Progress: 데이터 소비 어플리케이션 구축 (진행....)
> * phase 2 에서 만든 리눅스 IPC 공유메모리에 접근하여 OS의 개입이나 복사 과정 없이 실시간 센서 데이터를 고속으로 소비하는 어플리케이션 구축
> * Zero-Copy 데이터 리딩 엔진: 수신부가 갱신 중인 물리적 RAM 영역(/dev/shm)에 포인터를 직접 맵핑하여 데이터 획득 검증
> * 접근 권한 통제: 생상자의 데이터 오염 방지 및 무결성 보호를 위해 소비자는 읽기 전용으로만 메모리 접근
> * 비동기 초고속 릴레이: 파일 I/O나 소켓 통신 없이 메모리 직접 참조를 통해 수신 엔진에 어떠한 overhead도 주지 않고 비동기 통신
> * 이기종 확장성 확보: C++ Python 등 다중 프로세스가 지연 없이 동시에 데이터를 퍼갈 수 있는 1:N 스트리밍 브로드 캐스트 아키텍처 기반 마련

