# Holdem - AI Commit Automation
팀 프로젝트 협업 과정에서 사용했던 AI 기반 Commit 자동화 시스템을 재구성한 개인 프로젝트입니다.  

## Features  
- Conventional Commit 메시지 자동 생성  
- Markdown 기반 작업 기록 저장  
- 이전 작업 로그 참조 지원  
- Fork → GitHub Desktop 환경 재구성  

## Project Structure  
/prompts  
 ├─ [CLAUDE.md](https://github.com/cubee021/Texas-Holdem/blob/652c4f32b49eb0612cc4abd8776b603dd35a88c2/Holdem/CLAUDE.md)  
 ├─ [commit.md](https://github.com/cubee021/Texas-Holdem/blob/652c4f32b49eb0612cc4abd8776b603dd35a88c2/Holdem/.claude/commands/commit.md)  
 └─ [compact.md](https://github.com/cubee021/Texas-Holdem/blob/6ca5d56292252a9b53e357f8e366b2c06fea7226/Holdem/.claude/commands/compact.md)  

/scripts  
 └─ [gen_commit_msg.py](https://github.com/cubee021/Texas-Holdem/blob/652c4f32b49eb0612cc4abd8776b603dd35a88c2/Holdem/Tools/CommitMessage/gen_commit_msg.py)
