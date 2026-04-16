# webproxy_lab_docker

`webproxy_lab_docker`는 CS:APP Proxy Lab을 Docker + VSCode DevContainer 환경에서 구현하고 디버깅하기 쉽게 정리한 교육용 저장소입니다. 핵심 과제는 `webproxy-lab/` 아래에서 Tiny 웹 서버와 HTTP 프록시 서버를 완성하고, 자동 채점 스크립트로 동작을 검증하는 것입니다.

이 저장소는 "완성된 웹 프록시 프로젝트"가 아니라 "실습용 개발 환경 + 과제 골격 코드"에 가깝습니다. 따라서 README도 설치 안내만 나열하지 않고, 현재 코드베이스가 어디까지 준비되어 있고 어디부터 직접 구현해야 하는지를 기준으로 설명합니다.

## 한눈에 보기

- 과제 주제: C로 Tiny 웹 서버와 HTTP 프록시 서버 구현
- 핵심 작업 위치: `webproxy-lab/proxy.c`, `webproxy-lab/tiny/tiny.c`
- 제공되는 것: CS:APP 보조 라이브러리, Tiny 테스트 자원, 자동 채점 스크립트, DevContainer, VSCode 디버깅 설정
- 제공되지 않는 것: 완성된 Tiny 구현, 완성된 프록시 구현, 바로 통과 가능한 정답 코드
- 권장 실행 환경: VSCode DevContainer 내부 Ubuntu
- 자동 채점 총점: 70점 (`Basic 40 + Concurrency 15 + Cache 15`)

## 이 저장소가 실제로 제공하는 것

이 저장소는 크게 두 층으로 나뉩니다.

1. 루트 레벨 개발 환경
   - `.devcontainer/`: Ubuntu 기반 컨테이너 정의
   - `.vscode/`: Tiny/Proxy 전용 빌드 및 디버깅 설정
   - `README.md`: 현재 문서
2. 실제 실습 코드
   - `webproxy-lab/`: Proxy Lab 과제 본체

즉, 루트는 "실습 환경", `webproxy-lab/`는 "실습 대상 코드"입니다.

## 저장소 구조

```text
webproxy_lab_docker/
├── .devcontainer/
│   ├── Dockerfile
│   └── devcontainer.json
├── .vscode/
│   ├── launch.json
│   └── tasks.json
├── webproxy-lab/
│   ├── Makefile
│   ├── README.md
│   ├── csapp.c
│   ├── csapp.h
│   ├── driver.sh
│   ├── free-port.sh
│   ├── nop-server.py
│   ├── port-for-user.pl
│   ├── proxy.c
│   └── tiny/
│       ├── Makefile
│       ├── README
│       ├── csapp.c
│       ├── csapp.h
│       ├── home.html
│       ├── godzilla.gif
│       ├── godzilla.jpg
│       ├── tiny.c
│       └── cgi-bin/
│           ├── Makefile
│           └── adder.c
└── README.md
```

자동 채점이나 프록시 실행 중에는 `webproxy-lab/.proxy/`, `webproxy-lab/.noproxy/` 같은 임시 디렉터리가 생성될 수 있습니다.

## 핵심 파일 설명

### 1. `webproxy-lab/proxy.c`

학생이 최종적으로 구현해야 하는 프록시 서버 엔트리 포인트입니다.

현재 스냅샷 기준으로는 다음 정도만 들어 있습니다.

- 캐시 상수 정의
  - `MAX_CACHE_SIZE`
  - `MAX_OBJECT_SIZE`
- 과제에서 요구하는 `User-Agent` 헤더 문자열
- `main()`에서 해당 헤더를 출력하고 종료하는 최소 골격

즉, 현재 `proxy.c`는 네트워크 프록시가 아니라 "과제용 상수/문자열만 남아 있는 스타터 파일"입니다.

### 2. `webproxy-lab/tiny/tiny.c`

Tiny 웹 서버 구현 파일입니다. 프록시가 요청을 전달할 대상(origin server) 역할을 합니다.

이 파일은 현재 다음 상태입니다.

- `main()` 루프는 존재
- 연결 수락(`Accept`) 후 `doit(connfd)` 호출 구조는 존재
- 하지만 `doit`, `read_requesthdrs`, `serve_static`, `serve_dynamic` 등의 실제 구현 본문은 빠져 있음

따라서 현재 저장소 상태 그대로는 Tiny가 링크되지 않습니다.

### 3. `webproxy-lab/driver.sh`

Proxy Lab 자동 채점 스크립트입니다. 이 저장소를 이해할 때 가장 중요한 파일 중 하나입니다. 무엇을 구현해야 하는지가 이 스크립트에 그대로 드러납니다.

검증 항목은 다음 세 가지입니다.

- `Basic` 40점
  - Tiny에서 직접 받은 파일과 프록시를 거쳐 받은 파일이 동일한지 비교
  - 비교 대상: `home.html`, `csapp.c`, `tiny.c`, `godzilla.jpg`, `tiny`
- `Concurrency` 15점
  - 응답하지 않는 `nop-server.py`를 하나 띄워 프록시 요청 하나를 묶어 둔 뒤
  - 동시에 Tiny의 `home.html` 요청도 정상 처리되는지 확인
  - 본질적으로 head-of-line blocking 없이 동시성을 처리하는지 보는 테스트
- `Cache` 15점
  - 프록시를 통해 `tiny.c`, `home.html`, `csapp.c`를 먼저 가져와 캐시를 채운 뒤
  - Tiny를 종료하고
  - 다시 `home.html`을 요청했을 때 프록시 캐시에서 응답하는지 확인

즉, 이 과제의 구현 목표는 단순 포워딩이 아니라 아래 세 가지를 만족하는 프록시입니다.

- 올바른 HTTP 전달
- 동시 처리
- 메모리 캐시

### 4. `webproxy-lab/nop-server.py`

Concurrency 테스트 전용 보조 서버입니다. 연결을 받은 뒤 무한 대기하므로, 프록시가 한 요청 때문에 전체가 막히는지 확인할 수 있습니다.

### 5. `webproxy-lab/csapp.c`, `webproxy-lab/csapp.h`

CS:APP 교재에서 제공하는 래퍼 함수와 Robust I/O(RIO) 유틸리티가 들어 있습니다. 소켓 생성, 리스닝, 클라이언트 연결, 에러 처리, 라인 단위 입출력 등을 직접 구현하지 않고 재사용하는 기반입니다.

### 6. `webproxy-lab/tiny/cgi-bin/adder.c`

Tiny의 동적 콘텐츠 예제로 쓰이는 CGI 프로그램입니다. 쿼리 스트링에서 두 숫자를 읽어 더한 결과를 HTML로 반환합니다.

### 7. `.devcontainer/`

루트 `Dockerfile`은 Ubuntu 기반 환경에 다음 도구를 설치합니다.

- `build-essential`, `gcc`, `make`, `cmake`
- `gdb`, `valgrind`
- `curl`, `wget`, `git`
- `python3`
- `net-tools`, `iproute2`

이 구성은 단순 편의가 아니라 실질적으로 필요합니다. 아래 "왜 DevContainer가 사실상 필수인가" 섹션을 꼭 보세요.

### 8. `.vscode/launch.json`, `.vscode/tasks.json`

VSCode에서 바로 Tiny와 Proxy를 디버깅할 수 있게 설정되어 있습니다.

- `Debug Tiny Web Server`
  - 실행 파일: `webproxy-lab/tiny/tiny`
  - 기본 포트: `8000`
  - 사전 작업: `Tiny server clean-and-build`
- `Debug Proxy Server`
  - 실행 파일: `webproxy-lab/proxy`
  - 기본 포트: `4500`
  - 사전 작업: `Proxy server clean-and-build`

## 왜 DevContainer가 사실상 필수인가

이 저장소는 겉으로는 "C 프로젝트"지만, 실제로는 Linux 기반 실습 환경을 전제로 작성돼 있습니다.

핵심 이유는 다음과 같습니다.

- `driver.sh`와 `free-port.sh`가 GNU/Linux 스타일 `netstat` 옵션을 사용합니다.
- 루트 `Dockerfile`은 그 전제를 만족하도록 `net-tools`를 설치합니다.
- VSCode 디버깅 설정도 컨테이너 기준 경로(`/workspaces/webproxy_lab_docker`)를 사용합니다.

실제로 macOS 호스트에서 직접 `netstat --numeric-ports --numeric-hosts -a --protocol=tcpip`를 실행하면 옵션 오류가 납니다. 즉, macOS 터미널에서 바로 `driver.sh`를 돌리는 방식은 이 저장소의 기본 사용 경로가 아닙니다.

정리하면:

- Windows/macOS 호스트: Docker Desktop + DevContainer 사용 권장
- 컨테이너 내부 Ubuntu: 빌드/디버그/자동 채점의 기준 환경

## 빠른 시작

### 1. 저장소 클론

```bash
git clone --depth=1 https://github.com/krafton-jungle/webproxy_lab_docker.git
cd webproxy_lab_docker
```

### 2. VSCode에서 열기

- VSCode로 루트 폴더를 엽니다.
- `Dev Containers: Reopen in Container`를 실행합니다.

### 3. 컨테이너 내부에서 작업

실제 작업 디렉터리는 `webproxy-lab/`입니다.

```bash
cd webproxy-lab
```

### 4. 빌드

```bash
make
cd tiny
make
```

### 5. 자동 채점

```bash
cd ../
./driver.sh
```

### 6. VSCode 디버깅

- Tiny 디버깅: `Debug Tiny Web Server`
- Proxy 디버깅: `Debug Proxy Server`

포트 충돌이 있으면 `.vscode/launch.json`에서 `8000`, `4500`을 다른 값으로 바꿔 사용하면 됩니다.

## 현재 스냅샷에서 직접 확인한 상태

아래는 현재 작업 트리에서 실제로 확인한 결과입니다.

| 확인 항목 | 결과 | 의미 |
| --- | --- | --- |
| `cd webproxy-lab && make` | 성공 | `proxy.c`는 골격 수준이지만 컴파일은 됨 |
| `cd webproxy-lab/tiny && make` | 실패 | `tiny.c`에 `doit` 등 실제 구현이 없어 링크 실패 |
| `cd webproxy-lab && ./driver.sh` | 실패 | Tiny 실행 파일이 없어서 자동 채점 시작 단계에서 중단 |
| macOS 호스트에서 `netstat ... --protocol=tcpip` | 실패 | DevContainer/Linux 환경 사용이 사실상 필요 |

현재 Tiny 빌드 실패의 직접 원인은 `tiny.c`에 선언만 있고 정의가 없는 함수들입니다.

## 추천 작업 순서

이 저장소를 과제 풀이 관점에서 다루려면 아래 순서가 가장 자연스럽습니다.

1. `webproxy-lab/tiny/tiny.c`를 교재 기준으로 복원하거나 완성합니다.
2. `webproxy-lab/tiny`에서 `make`가 통과하는지 확인합니다.
3. `webproxy-lab/proxy.c`에 요청 파싱, 서버 연결, 헤더 재작성, 응답 전달 로직을 구현합니다.
4. 동시성 모델(예: 스레드 기반)을 추가해 `Concurrency` 테스트를 통과시킵니다.
5. 객체 크기/전체 캐시 크기 제한을 지키는 메모리 캐시를 구현합니다.
6. `./driver.sh`로 70점 만점을 목표로 반복 검증합니다.

## 알려진 제한사항과 주의점

- 현재 `proxy.c`는 프록시 서버가 아니라 과제 골격 파일입니다.
- 현재 `tiny.c`는 불완전해서 기본 상태로는 링크되지 않습니다.
- 자동 채점 스크립트는 Linux 계열 네트워킹 도구 사용을 전제로 합니다.
- `webproxy-lab/Makefile`의 `handin` 규칙은 `proxylab-handout` 디렉터리를 패키징하도록 되어 있어, 현재 저장소 디렉터리 이름(`webproxy-lab`)과 바로 맞지 않습니다. 제출용 tarball 생성 전에는 이 부분을 다시 확인하는 편이 안전합니다.

## 참고 문서

- [`webproxy-lab/README.md`](./webproxy-lab/README.md): Proxy Lab 원본 안내
- [`webproxy-lab/tiny/README`](./webproxy-lab/tiny/README): Tiny 서버 설명

## 요약

이 저장소의 본질은 "프록시 과제를 풀기 위한 개발 환경과 골격 코드"입니다. 루트 설정은 컨테이너 기반 Linux 실습을 지원하고, `webproxy-lab/driver.sh`는 과제 요구사항을 Basic/Concurrency/Cache 세 축으로 검증합니다. 다만 현재 스냅샷은 완성본이 아니라 스타터 상태이므로, Tiny와 Proxy 모두 실제 구현을 채워 넣어야 정상 빌드와 채점이 가능합니다.
