# `parse_uri`를 위한 C 문자열 헬퍼 정리

이 문서는 프록시의 `parse_uri` 또는 `parse_absolute_uri`를 설계할 때 자주 쓰는 C 표준 문자열 함수들을 빠르게 다시 보기 위한 참고용이다.

목표는 다음과 같은 입력을 다룰 때 필요한 도구를 정리하는 것이다.

```text
http://localhost:8080/home.html?x=1&y=2
```

여기서 보통 관심 있는 조각은 다음과 같다.

- scheme: `http://`
- host: `localhost`
- port: `8080`
- path + query: `/home.html?x=1&y=2`

주의:

- `strstr`, `strchr`는 "인덱스"를 반환하지 않는다.
- 둘 다 문자열 내부의 특정 위치를 가리키는 `char *` 포인터를 반환한다.
- 인덱스가 필요하면 `found - base`처럼 포인터 차이를 사용한다.

## 1. `strstr`

```c
char *strstr(const char *haystack, const char *needle);
```

역할:

- 문자열 `haystack` 안에서 부분 문자열 `needle`이 처음 나타나는 위치를 찾는다.

성공 시:

- `needle`이 시작되는 위치를 가리키는 포인터를 반환한다.

실패 시:

- `NULL`을 반환한다.

`parse_uri`에서 주로 쓰는 경우:

- `"http://"`가 포함되어 있는지 확인
- `"//"` 같은 구분자를 찾기

예시:

```c
char *p = strstr(uri, "://");
```

해석:

- `p != NULL`이면 `"://"`를 찾은 것
- `p == NULL`이면 기대한 형태가 아닐 수 있음

주의:

- 빈 문자열 `""`을 찾으면 항상 시작 위치가 반환된다.

## 2. `strchr`

```c
char *strchr(const char *s, int c);
```

역할:

- 문자열 `s` 안에서 문자 `c`가 처음 나타나는 위치를 찾는다.

성공 시:

- 해당 문자를 가리키는 포인터를 반환한다.

실패 시:

- `NULL`을 반환한다.

`parse_uri`에서 주로 쓰는 경우:

- `':'` 찾기: 포트 구분
- `'/'` 찾기: path 시작점
- `'?'` 찾기: query 시작점

예시:

```c
char *colon = strchr(host_begin, ':');
char *slash = strchr(host_begin, '/');
```

해석:

- `colon != NULL`이면 이후에 포트가 있을 가능성이 있음
- `slash != NULL`이면 path 시작점이 있음
- 둘 다 없으면 host만 있고 path가 없을 수 있음

## 3. `strrchr`

```c
char *strrchr(const char *s, int c);
```

역할:

- 문자열 `s` 안에서 문자 `c`가 마지막으로 나타나는 위치를 찾는다.

성공 시:

- 마지막 등장 위치를 가리키는 포인터를 반환한다.

실패 시:

- `NULL`을 반환한다.

`parse_uri`에서 꼭 필요한 건 아니지만 도움이 될 수 있는 경우:

- 마지막 구분자를 기준으로 뒤쪽을 보고 싶을 때

주의:

- 일반적인 `http://host:port/path` 파싱에서는 보통 `strchr`로도 충분하다.

## 4. `strncmp`

```c
int strncmp(const char *s1, const char *s2, size_t n);
```

역할:

- 앞에서부터 `n`글자만 비교한다.

성공 시:

- 두 문자열의 앞 `n`글자가 같으면 `0`

실패 시:

- 다르면 `0`이 아닌 값

`parse_uri`에서 주로 쓰는 경우:

- 입력이 `"http://"`로 시작하는지 확인

예시:

```c
if (strncmp(uri, "http://", 7) == 0) {
    /* http scheme 시작 */
}
```

해석:

- `== 0`: 기대한 접두어가 맞음
- `!= 0`: 기대한 형식과 다름

주의:

- 문자열 비교에서 "성공"은 보통 `0`이다. 이 점이 헷갈리기 쉽다.

## 5. `strlen`

```c
size_t strlen(const char *s);
```

역할:

- 널 종료 문자를 제외한 문자열 길이를 구한다.

성공 시:

- 길이를 반환한다.

실패 시:

- 별도의 실패 반환값은 없다.
- 하지만 `s == NULL`이면 잘못된 사용이다.

`parse_uri`에서 주로 쓰는 경우:

- 복사 길이 계산
- 경계 검사

주의:

- 반환형은 `size_t`
- 널 종료 문자를 포함하지 않는다.

## 6. 포인터 차이로 길이 구하기

문자열 함수와 함께 자주 쓰는 패턴이다.

예시:

```c
size_t host_len = slash - host_begin;
```

의미:

- `host_begin`부터 `slash` 바로 전까지의 길이

전제:

- `slash`가 `NULL`이 아니어야 한다.
- 두 포인터가 같은 문자열 내부를 가리켜야 한다.

주의:

- `NULL - host_begin` 같은 계산은 하면 안 된다.

## 7. `sscanf`

```c
int sscanf(const char *str, const char *format, ...);
```

역할:

- 문자열에서 형식에 맞는 값을 읽어온다.

성공 시:

- 성공적으로 읽어온 항목 수를 반환한다.

실패 시:

- 기대한 개수보다 작은 값을 반환한다.

`parse_uri`에서의 위치:

- 사용할 수는 있지만, 초반에는 오히려 문자열 경계를 직접 잡는 방식보다 덜 직관적일 수 있다.

예시 해석:

```c
int n = sscanf(s, "%15[^:]:%15s", host, port);
```

- `n == 2`: host와 port 모두 읽은 것
- `n < 2`: 기대한 형식과 다름

주의:

- 폭 제한을 두지 않으면 버퍼 오버플로 위험이 있다.
- 이 과제에서는 `strchr`와 포인터 계산이 더 이해하기 쉬운 경우가 많다.

## 8. `snprintf`

```c
int snprintf(char *str, size_t size, const char *format, ...);
```

역할:

- 버퍼 크기를 넘지 않도록 안전하게 문자열을 만든다.

성공 시:

- 출력하려고 했던 문자열 길이를 반환한다.

실패 시:

- 음수를 반환할 수 있다.

`parse_uri`에서 주로 쓰는 경우:

- 잘라낸 host, port, path를 목적 버퍼에 복사
- 기본 포트 `"80"`를 넣기
- 기본 path `"/"`를 넣기

주의:

- 반환값이 `size` 이상이면 잘렸을 가능성이 있다.

## 9. `memcpy`

```c
void *memcpy(void *dest, const void *src, size_t n);
```

역할:

- 정확히 `n`바이트를 복사한다.

성공 시:

- `dest`를 반환한다.

실패 시:

- 별도 실패 반환값은 없다.
- 잘못된 길이 또는 잘못된 포인터면 문제가 난다.

`parse_uri`에서 주로 쓰는 경우:

- 포인터 구간 `[begin, end)`를 잘라서 버퍼에 옮길 때

주의:

- 문자열로 쓸 거면 복사 후 직접 `'\0'`를 붙여야 한다.
- `strncpy`보다 의도가 분명한 경우가 많다.

예시 패턴:

```c
memcpy(host, host_begin, host_len);
host[host_len] = '\0';
```

## 10. `strtol`

```c
long strtol(const char *nptr, char **endptr, int base);
```

역할:

- 숫자 문자열을 정수로 바꾼다.

성공 시:

- 변환된 정수를 반환한다.

실패 시:

- 변환이 전혀 안 되면 `endptr`가 입력 시작 위치를 가리킨다.
- 범위를 넘으면 `errno` 확인이 필요하다.

`parse_uri`에서의 위치:

- 포트를 문자열로 저장할 수도 있고 정수로 검증할 수도 있다.

주의:

- 단순히 문자열 `"80"`를 보관하는 것과, 실제 숫자로 유효성을 검사하는 것은 별개의 단계다.

## `parse_uri` 관점에서 특히 자주 쓰는 조합

### 1. 접두어 확인

- `strncmp(uri, "http://", 7)`

### 2. host 시작점 잡기

- `host_begin = uri + 7`

### 3. host 이후 첫 구분자 찾기

- `strchr(host_begin, ':')`
- `strchr(host_begin, '/')`

### 4. 구간 길이 계산

- `end - begin`

### 5. 결과 버퍼에 안전하게 복사

- `memcpy(...); result[len] = '\0';`
- 또는 `snprintf(...)`

## 많이 헷갈리는 점

### `strstr`, `strchr`는 인덱스를 반환하지 않는다

잘못된 이해:

```c
int idx = strchr(s, '/');
```

올바른 이해:

```c
char *p = strchr(s, '/');
if (p != NULL) {
    size_t idx = p - s;
}
```

### 문자열 비교에서 `0`이 "같다"를 뜻한다

```c
if (strncmp(a, b, n) == 0) {
    /* 같음 */
}
```

### `memcpy`는 문자열 종료를 자동으로 붙여주지 않는다

```c
memcpy(dst, src, len);
dst[len] = '\0';
```

## 지금 단계에서 추천하는 최소 도구 세트

초기 구현에서는 아래 정도만 잘 써도 충분하다.

- `strncmp`
- `strchr`
- `strlen`
- 포인터 차이
- `memcpy` 또는 `snprintf`

이 조합만으로도 다음 판단을 꽤 분명하게 할 수 있다.

- `http://`로 시작하는가
- host는 어디까지인가
- port가 있는가
- path가 있는가
- path가 없으면 기본값 `/`를 둘 것인가

## 스스로 점검할 질문

- 지금 내가 찾고 싶은 건 "부분 문자열"인가, "문자 하나"인가?
- 반환값이 인덱스인가, 포인터인가?
- 못 찾았을 때 `NULL` 가능성을 먼저 검사했는가?
- 잘라낸 뒤 널 종료 문자를 붙였는가?
- 버퍼 길이를 넘길 가능성을 고려했는가?
