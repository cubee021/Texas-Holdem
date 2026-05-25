# Compact Log 생성

## 실행 내용

1. 최근 Start Log 또는 이전 Compact Log 이후 작업 분석
2. Compact Log 작성 및 저장
3. Commit 메시지 추천
4. 다음 ToDo 제시 (필요시)

## 작업 지시

다음 절차를 따라 Compact Log를 생성하세요:

### 1. 최근 작업 분석

다음 정보를 파악합니다:
- 변경된 파일 목록 (git status, git diff 등)
- 작성한 코드 및 기능
- 해결한 문제
- 수정한 버그
- 추가한 테스트

### 2. Compact Log 카운트 확인

현재 로그 파일에서 마지막 Compact Log 번호를 확인합니다.
- 첫 번째 Compact Log: `Compact Log 1`
- 두 번째 Compact Log: `Compact Log 2`
- ...

### 3. Git 사용자 정보 및 시간 확인
```bash
git config user.name
```

현재 시간 (UTC+09:00) 계산

### 4. 로그 파일 경로 찾기

**중요**: Start Log가 작성된 파일에 추가합니다.
- 예: `AgentLog/[작업자GitID]/[작업자GitID]_251107.md`
- 예시: `AgentLog/dopple/dopple_251109.md`
- 날짜가 바뀌어도 현재 Start Log가 있는 파일에 계속 기록

### 5. Compact Log 작성

**작성 내용**:
```markdown
## Compact Log N
- YYYYMMDD hh:mm

### 요약 내용

[작업 내용을 구체적으로 요약]

**변경 파일**:
- `file1.cpp`: 기능 추가
- `file2.h`: 클래스 선언 수정
- `file3.md`: 문서 업데이트

**주요 작업**:
- 기능 A 구현 완료
- 버그 B 수정
- 테스트 C 추가

### Commit 정보

**Summary (EN)**:
Brief one-line summary of all changes

**세부 내역**:
1. "[변경 범주 1]"
   - [범주 1의 상세 설명]
2. "[변경 범주 2]"
   - [범주 2의 상세 설명]

```

### 6. 다음 ToDo 제시 (선택사항)

이전 ToDo를 완료했다면, 다음 단계의 ToDo를 제시합니다:

```markdown
### Next To Do

1. [다음 작업 1]
2. [다음 작업 2]
```

### 7. 사용자에게 알림

Compact Log가 작성되었음을 알리고, 작업을 계속합니다.

---

**중요**:
- Compact Log는 작업 중간중간 필요할 때마다 생성할 수 있습니다
- 큰 작업 완료 시, 중간 체크포인트로 활용합니다
- Commit 전에 Compact Log를 작성하면 커밋 메시지 작성에 도움이 됩니다
