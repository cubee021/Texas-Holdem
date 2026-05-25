---
description: Staged 파일을 분석하여 Conventional Commit 메시지를 생성하고 자동으로 커밋합니다
---

# Auto Commit with Generated Message

**참고**: `Tools/CommitMessage/gen_commit_msg.py`

## 실행 내용

1. Staged 파일 확인
2. Conventional Commit 형식 메시지 자동 생성
3. 사용자에게 메시지 확인
4. 자동 커밋 실행  

## 작업 지시

다음 절차를 따라 자동 커밋을 수행하세요:

### 1. Staged 파일 확인

먼저 staged 파일이 있는지 확인합니다:

```bash
git diff --cached --name-status
```

**staged 파일이 없으면**:
- 사용자에게 알림: "커밋할 파일이 없습니다. `git add`로 파일을 stage하세요."
- 종료  

### 2. 변경 내용 분석  
Staged 파일의 변경 내용을 분석하여:  
- 어떤 type인지 결정 (feat/fix/docs/style/refactor/test/chore/perf/ci/build)  
- 핵심 변경 사항 파악  
- 변경 이유와 목적 이해  

### 3. 커밋 메시지 생성

`Tools/CommitMessage/gen_commit_msg.py` 스크립트를 실행합니다:

```bash
python Tools/CommitMessage/gen_commit_msg.py
```

**스크립트가 없으면:**  
`gen_commit_msg.py`가 없는 경우, 다음과 같이 직접 메시지를 생성합니다:
  
**Conventional Commit 형식:**  
```
<type>: <subject>

<body>

<footer>
```

**규칙**:
- **Subject**: 50자 이내, 소문자 시작, 명령형, 마침표 없음
- **Body**: 72자마다 줄바꿈, 왜 변경했는지 설명 (선택사항)
- **Footer**: 이슈 참조, Breaking Changes (선택사항)  

**Type 선택 가이드**

| Type | 사용 시점 |
|------|-----------|
| **feat** | 새로운 기능 추가 |
| **fix** | 버그 수정 |
| **docs** | 문서만 변경 (코드 변경 없음) |
| **style** | 코드 포맷팅, 공백 등 (동작 변경 없음) |
| **refactor** | 코드 리팩토링 (기능 변경 없음) |
| **test** | 테스트 코드 추가/수정 |
| **chore** | 빌드, 설정 파일 변경 |
| **perf** | 성능 개선 |
| **ci** | CI 설정 변경 |
| **build** | 빌드 시스템 변경 |

```bash
# Staged 파일 분석
git diff --cached --stat

# 간단한 메시지 생성
# - 변경 파일 수
# - 추가/삭제 라인 수
# - 주요 경로
# 를 기반으로 type과 scope 추론

예: chore: 파일 5개 변경 (+120/-45)
```

**주의사항**

1. **전체 diff가 아닌 staged 파일만 확인**: `git diff --cached` 사용
2. **컨텍스트 활용**: 이전 대화에서 작업한 내용을 고려
3. **명확하고 간결하게**: 불필요한 설명 제거
4. **일관된 스타일**: 프로젝트의 기존 커밋 메시지 스타일 따르기  


### 4. 생성된 메시지 읽기

스크립트가 생성한 메시지를 읽습니다:

**파일 경로**:
- `.git/COMMIT_EDITMSG` (우선)
- `Tools/CommitMessage/commit_message.txt` (대체)

메시지에서 주석(`#`로 시작하는 줄)을 제거하고 실제 커밋 메시지만 추출합니다.

### 5. 사용자에게 메시지 확인

생성된 커밋 메시지를 사용자에게 보여줍니다:

```
📝 생성된 커밋 메시지:
────────────────────────────────
feat(docs): 파일 3개 변경 (+45/-12)
────────────────────────────────

이 메시지로 커밋하시겠습니까? (y/n)
```

**중요**: 사용자 확인을 받지 않고 바로 커밋하지 마세요!

### 6. 커밋 실행

사용자가 승인하면 커밋을 실행합니다:

```bash
# 방법 1: 생성된 메시지 파일 사용
git commit -F .git/COMMIT_EDITMSG

# 방법 2: 메시지 직접 전달
git commit -m "생성된 메시지"
```

**커밋 성공 시**:
```
✅ 커밋이 완료되었습니다!

커밋 SHA: abc1234
메시지: feat(docs): 파일 3개 변경 (+45/-12)

💡 다음 단계: GitHub Desktop에서 "Push origin"을 눌러 원격 저장소에 반영하세요.
```

**커밋 실패 시**:
- 에러 메시지 출력
- 사용자에게 수동 커밋 방법 안내

### 실행 예시

**전체 플로우**:

```bash
# 1. 파일 staged 확인
$ git status --short
M  Documents/README.md
A  Documents/Planning/NewDoc.md
M  Source/Main.cpp

# 2. 메시지 자동 생성
$ python Tools/CommitMessage/gen_commit_msg.py
✅ 커밋 메시지 생성 완료

# 3. 생성된 메시지 확인
feat(docs): 파일 3개 변경 (+45/-12)

# 요약
# - 요약: 문서 대량 업데이트, C++ 코드 중심 변경
# - 총 변경량: +45/-12, 파일: 3
# ...

# 4. 사용자 확인 후 커밋
$ git commit -F .git/COMMIT_EDITMSG
[main abc1234] feat(docs): 파일 3개 변경 (+45/-12)
 3 files changed, 45 insertions(+), 12 deletions(-)

# 5. GitHub Desktop으로 전환하여 Push
```

---

## 주의사항

1. **사용자 확인 필수**: 자동으로 커밋하기 전에 반드시 사용자에게 메시지를 보여주고 승인을 받으세요.

2. **Pre-commit Hook 실행**: 커밋 시 pre-commit hook이 실행될 수 있습니다. Hook 실패 시 사용자에게 알립니다.

3. **메시지 수정 옵션**: 사용자가 메시지를 수정하고 싶어하면:
   ```
   메시지를 수정하시겠습니까? (y/n)
   > y
   수정할 메시지를 입력하세요:
   > [사용자 입력]
   ```

4. **빈 커밋 방지**: Staged 파일이 없으면 커밋하지 않습니다.

5. **Claude Code Signature**: 커밋 메시지 끝에 다음을 추가할 수 있습니다 (선택):
   ```
   
   🤖 Generated with Claude Code
   
   ```

---

## GitHub Desktop 사용자를 위한 안내

### CLI 커밋과 GitHub Desktop의 관계

- ✅ **안전함**: CLI로 커밋해도 GitHub Desktop에 자동으로 반영됩니다
- ✅ **동기화**: 같은 `.git` 폴더를 사용하므로 충돌 없음
- ⚠️ **새로고침**: GitHub Desktop이 열려있다면 창을 클릭하여 새로고침하세요

### 권장 워크플로우

1. **Claude Code에서**: `/commit` 명령으로 커밋 생성
2. **GitHub Desktop으로 전환**: 새 커밋 확인
3. **Push**: "Push origin" 버튼 클릭하여 GitHub에 반영

### Push는 별도로

CLI로 커밋해도 **자동 Push는 되지 않습니다**. 다음 중 선택:

- **옵션 A**: GitHub Desktop에서 "Push origin" 버튼 클릭 (추천)
- **옵션 B**: CLI에서 `git push` 실행

---

## 간소화 버전 (빠른 커밋)

사용자가 확인 없이 빠른 커밋을 원하면:

```bash
# 메시지 생성 + 즉시 커밋
python Tools/CommitMessage/gen_commit_msg.py && git commit -F .git/COMMIT_EDITMSG
```

**하지만**: Claude에서는 항상 사용자 확인을 받는 것을 권장합니다.

---

**참고**: 이 명령은 `commit-helper` skill이 설치된 경우 더 정확한 메시지를 생성합니다.
