# Edge-Telemetry Engine

> 프로젝트 기간: 26/07/27

## 1. 프로젝트 개요
> * 본 프로젝트는 물리적 센서(아두이노)로 부터 발생하는 고빈도 데이터를 리눅스 기반 엣지 서버(라즈베리 파이)에서 수집하여 1ms 미만의 결정적 지연 시간내에 다수의 네트워크 클라이언트에게 스트리밍하는 고성능 시스템 소프트웨어 구축을 목표로 합니다. 

## 2. 핵심 기술 목표
> * 초저지연: 하드웨어 인터럽트 발생부터 네트워크 패킷 송출까지 전 구간 지연 시간을 1ms 미만으로 유지
> * 고가용성 처리량: 초당 10,000개 이상의 바이너리 패킷을 유실 없이 처리
> * 결정성 확보: CPU Isolation 및 Real-time Scheduling을 통해 지연 시간의 표준 편차 최소화

## 3. 시스템 아키텍처
> 1. Hardware Layer: Arduino --> Uart Serial(921,600bps) --> Raspberry Pi
> 2. OS & Kernel Layer:
> 3. Memory Layer: 
> 4. Network Layer:

## 4. 상세 기술 스택
> * 언어: C++ 20, C
> * 환경: Raspberry Pi OS, Ubuntu Linux, VS Code

