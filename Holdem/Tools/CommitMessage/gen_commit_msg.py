#!/usr/bin/env python3
import os, sys, subprocess, re, json

def run_git(args, cwd):
    try:
        cp = subprocess.run(["git"] + args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, encoding=sys.getfilesystemencoding() or 'utf-8', errors='replace')
        if cp.returncode != 0:
            return ""
        return cp.stdout
    except Exception:
        return ""

def repo_root(start_dir):
    out = run_git(["rev-parse", "--show-toplevel"], start_dir).strip()
    return out if out else start_dir

def staged_name_status(root):
    out = run_git(["diff", "--cached", "--name-status"], root)
    files = []
    for line in out.splitlines():
        m = re.match(r"^(?P<status>[ACDMRTU])\s+(?P<path>.+)$", line)
        if m:
            files.append({"status": m.group("status"), "path": m.group("path")})
    return files

def staged_numstat(root):
    out = run_git(["diff", "--cached", "--numstat"], root)
    stats = {}
    for line in out.splitlines():
        m = re.match(r"^(?P<adds>\d+|-)\t(?P<dels>\d+|-)\t(?P<path>.+)$", line)
        if m:
            adds = 0 if m.group("adds") == '-' else int(m.group("adds"))
            dels = 0 if m.group("dels") == '-' else int(m.group("dels"))
            stats[m.group("path")] = {"adds": adds, "dels": dels}
    return stats

def guess_scope(path):
    """Holdem 프로젝트 구조에 맞춘 scope 추론"""
    # Source/Holdem/ 내부 - Public/Private 구분
    m = re.match(r"^Source/Holdem/(?:Public|Private)/([^/]+)/", path)
    if m:
        folder = m.group(1).lower()
        return folder  # components, game, tester, ui
    
    # Source/Holdem/ 직접 하위
    if path.startswith("Source/Holdem/"):
        return "core"
    
    # Content/ 하위
    m = re.match(r"^Content/([^/]+)/", path)
    if m:
        folder = m.group(1).lower()
        if folder == "blueprints":
            return "bp"
        elif folder == "assets":
            return "assets"
        elif folder == "maps":
            return "maps"
        elif folder == "player":
            return "player"
        elif folder == "ui":
            return "ui"
        else:
            return "content"
    
    # Config/
    if path.startswith("Config/"):
        return "config"
    
    # .claude/
    if path.startswith(".claude/"):
        return "claude"
    
    # AgentLog/
    if path.startswith("AgentLog/"):
        return "log"
    
    # 프로젝트 파일
    if path.endswith(".uproject"):
        return "project"
    
    # 문서
    if re.match(r"^(CLAUDE\.md|README\.md|docs?/)", path, re.I):
        return "docs"
    
    # 기타
    return "core"

def guess_type(files, branch):
    """커밋 타입 추론"""
    # 브랜치 이름으로부터 타입 추론
    m = re.match(r"^(feat|fix|refactor|docs|build|ci|test|perf|chore)(/|$)", branch or "")
    if m:
        return m.group(1)
    
    # 파일 패턴으로 타입 추론
    paths = [f["path"] for f in files]
    
    # 새 파일 추가 = feat
    if any(f["status"] == 'A' for f in files):
        return 'feat'
    
    # 설정 파일만 = chore
    if all(f["path"].startswith("Config/") for f in files):
        return 'chore'
    
    # 문서만 = docs
    if all(re.match(r"^(CLAUDE\.md|README|docs?/)", f["path"], re.I) for f in files):
        return 'docs'
    
    # 테스트 관련 = test
    if any("Tester" in f["path"] or "test" in f["path"].lower() for f in files):
        return 'test'
    
    # Content/Blueprints만 = feat
    if all(f["path"].startswith("Content/Blueprints/") for f in files):
        return 'feat'
    
    # 기본값
    return 'chore'

def build_subject(commit_type, scope, adds, dels, count):
    """커밋 메시지 subject 생성"""
    prefix = f"{commit_type}({scope}): " if scope else f"{commit_type}: "
    summary = f"파일 {count}개 변경 (+{adds}/-{dels})"
    limit = 72
    avail = max(1, limit - len(prefix))
    if len(summary) > avail:
        summary = summary[:max(0, avail-3)] + "..."
    return prefix + summary

def analyze_changes(root):
    """변경 사항 상세 분석"""
    out = run_git(["diff", "--cached", "-U0"], root)
    
    # Unreal Engine 매크로/키워드 감지
    tokens = {
        "UFUNCTION": 0,
        "UPROPERTY": 0,
        "UCLASS": 0,
        "UENUM": 0,
        "USTRUCT": 0,
        "Blueprint": 0
    }
    
    for line in out.splitlines():
        if line.startswith('+') and not line.startswith('+++'):
            if 'UFUNCTION' in line:
                tokens["UFUNCTION"] += 1
            if 'UPROPERTY' in line:
                tokens["UPROPERTY"] += 1
            if 'UCLASS' in line:
                tokens["UCLASS"] += 1
            if 'UENUM' in line:
                tokens["UENUM"] += 1
            if 'USTRUCT' in line:
                tokens["USTRUCT"] += 1
            if 'Blueprint' in line:
                tokens["Blueprint"] += 1
    
    # 상위 디렉토리 분석 (최대 2단계)
    names = run_git(["diff", "--cached", "--name-only"], root).splitlines()
    dir_counts = {}
    for p in names:
        if not p:
            continue
        d = os.path.dirname(p).replace('\\', '/')
        parts = [seg for seg in d.split('/') if seg]
        seg = '/'.join(parts[:min(len(parts), 2)]) if parts else '.'
        dir_counts[seg] = dir_counts.get(seg, 0) + 1
    
    top_dirs = [k for k, _ in sorted(dir_counts.items(), key=lambda x: x[1], reverse=True)[:3]]
    
    # 확장자별 변경량
    num = staged_numstat(root)
    ext_counts = {}
    for p, st in num.items():
        _, ext = os.path.splitext(p)
        ext = ext.lower()
        ext_counts[ext] = ext_counts.get(ext, 0) + st['adds'] + st['dels']
    
    top_ext = [f"{k}:{v}" for k, v in sorted(ext_counts.items(), key=lambda x: x[1], reverse=True)[:3] if k]
    
    return tokens, top_dirs, top_ext

def main():
    if os.environ.get('SKIP_AUTO_COMMIT_MSG'):
        return 0
    
    start = os.getcwd()
    root = repo_root(start)
    msg_path = None
    
    # 인자 파싱
    args = sys.argv[1:]
    allow_overwrite = False
    to_clip = False
    i = 0
    while i < len(args):
        a = args[i]
        if a.lower() == '-msgpath' and i + 1 < len(args):
            msg_path = args[i + 1]
            i += 2
            continue
        if a.lower() == '-allowoverwrite':
            allow_overwrite = True
            i += 1
            continue
        if a.lower() == '-toclipboard':
            to_clip = True
            i += 1
            continue
        i += 1
    
    if not msg_path:
        msg_path = os.path.join(root, '.git', 'COMMIT_EDITMSG')
    
    # Staged 파일 가져오기
    files = staged_name_status(root)
    num = staged_numstat(root)
    adds = sum(st.get('adds', 0) for st in num.values()) if num else 0
    dels = sum(st.get('dels', 0) for st in num.values()) if num else 0
    count = len(files)
    
    # Staged 파일 없으면 플레이스홀더
    if not files:
        subject = "chore: 파일 0개 변경 (+0/-0)"
        lines = [subject, "", "# 변경 없음 (staged 없음)"]
        content = "\r\n".join(lines) + "\r\n"
        with open(msg_path, 'w', encoding='utf-8-sig', newline='\r\n') as f:
            f.write(content)
        out_txt = os.path.join(os.path.dirname(__file__), 'commit_message.txt')
        with open(out_txt, 'w', encoding='utf-8-sig', newline='\r\n') as f:
            f.write(content)
        return 0
    
    # 브랜치 이름
    branch = run_git(["rev-parse", "--abbrev-ref", "HEAD"], root).strip()
    
    # 타입과 scope 추론
    ctype = guess_type(files, branch)
    scopes = [guess_scope(f["path"]) for f in files]
    
    # 가장 많이 나타나는 scope 선택
    scope = None
    if scopes:
        freq = {}
        for s in scopes:
            freq[s] = freq.get(s, 0) + 1
        scope = max(freq.items(), key=lambda x: x[1])[0]
    
    # Subject 생성
    subject = build_subject(ctype, scope if scope != 'core' else None, adds, dels, count)
    
    # 상세 분석
    tokens, top_dirs, top_ext = analyze_changes(root)
    
    # 지능적 요약 생성
    try:
        ranked = sorted(
            ({'path': p, 'adds': st.get('adds', 0), 'dels': st.get('dels', 0)} for p, st in num.items()),
            key=lambda x: x['adds'] + x['dels'],
            reverse=True
        )[:10]
        top_files = [f['path'] for f in ranked[:3]]
        
        # 자연스러운 한국어 요약
        total_delta = sum((st.get('adds', 0) + st.get('dels', 0)) for st in num.values()) or 1
        ext_mix = {}
        for p, st in num.items():
            _, e = os.path.splitext(p)
            e = e.lower()
            ext_mix[e] = ext_mix.get(e, 0) + st.get('adds', 0) + st.get('dels', 0)
        
        dominant = sorted(ext_mix.items(), key=lambda x: x[1], reverse=True)
        phrases = []
        
        if dominant:
            top_ext_name, top_val = dominant[0]
            share = top_val / total_delta
            
            if top_ext_name == '.uasset' and share >= 0.4:
                phrases.append('블루프린트/에셋 중심 변경')
            elif top_ext_name in ('.cpp', '.h') and share >= 0.5:
                phrases.append('C++ 코드 중심 변경')
            elif top_ext_name == '.ini' and share >= 0.5:
                phrases.append('설정 파일 업데이트')
            elif top_ext_name == '.md' and share >= 0.5:
                phrases.append('문서 업데이트')
        
        if adds and not dels:
            phrases.append('추가 위주')
        elif dels and not adds:
            phrases.append('삭제 위주')
        
        if any(tokens.values()):
            phrases.append('UE 매크로/타입 변경 포함')
        
        if not phrases:
            phrases.append('변경 사항 업데이트')
        
        one_liner = ', '.join(dict.fromkeys(phrases))
        
        # 최종 메시지 구성
        lines = [
            subject,
            "",
            "# 요약",
            f"# - 요약: {one_liner}",
            f"# - 총 변경량: +{adds}/-{dels}, 파일: {count}",
        ]
        
        if top_files:
            lines.append("# - 주요 파일: " + ', '.join(top_files))
        if top_dirs:
            lines.append("# - 영향 경로 상위: " + ', '.join(top_dirs))
        if top_ext:
            lines.append("# - 변경 확장자 상위: " + ', '.join(top_ext))
        if any(tokens.values()):
            lines.append("# - 코드 토큰: " + ", ".join([f"{k}:{v}" for k, v in tokens.items() if v > 0]))
        
        lines.append("")
        lines.append("# 변경 내역 상위 10개")
        for f in ranked:
            st = next((x['status'] for x in files if x['path'] == f['path']), 'M')
            lines.append(f"# - {st} {f['path']} (+{f['adds']}/-{f['dels']})")
        
        if branch:
            lines += ["", f"# 브랜치: {branch}"]
        
        content = "\r\n".join(lines) + "\r\n"
    except Exception:
        # 오류 발생 시 기본 메시지
        content = subject + "\r\n"
    
    # UTF-8 BOM으로 저장 (Windows 호환)
    with open(msg_path, 'w', encoding='utf-8-sig', newline='\r\n') as f:
        f.write(content)
    
    # Tools/CommitMessage/commit_message.txt에도 저장
    out_txt = os.path.join(os.path.dirname(__file__), 'commit_message.txt')
    try:
        with open(out_txt, 'w', encoding='utf-8-sig', newline='\r\n') as f:
            f.write(content)
    except Exception:
        pass
    
    # 클립보드 복사 (선택사항)
    if to_clip:
        try:
            import subprocess
            p = subprocess.Popen(['clip'], stdin=subprocess.PIPE)
            p.communicate(input=content.encode('utf-8'))
        except Exception:
            pass
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
