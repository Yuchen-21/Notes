# github 命令
## 创建
因为现在 github 有 tokens 机制，所以需要配置 tokens 才能使用 github 命令，不支持使用用户名和密码登录。
```bash
cd /你的/文件夹路径
git init
git add .
git commit -m "first commit"
git branch -M main
git remote add origin https://github.com/你的用户名/你的仓库名.git
git push -u origin main
用户名 Yuchen-21
密码改用 tokens 
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
```
## git 命令
```bash
# 看哪些文件被修改了，哪些还没提交。
git status

git add .
git commit -m "写清这次改了什么"
git pull --rebase origin main
git push origin main

```
### 提交
提交信息尽量写清楚，比如：
```bash
git commit -m "这次修改的说明"
# 如果你的分支不是 main，就换成对应分支名。
git push origin main
```
## 常用命令
```bash
git status
git add .
git commit -m "message"
git pull --rebase origin main
git push origin main
git log --oneline
git diff
git branch
把暂存区中的文件都踢出来
git reset
```
## 易混淆概念
![alt text](image-1.png)


    workspace：工作区
    staging area：暂存区/缓存区
    local repository：版本库或本地仓库
    remote repository：远程仓库

所以add是把工作区中的文件添加到暂存区，commit是把暂存区中的文件提交到本地库，而push是把本地库中的文件推送到远程仓库。
## git 忽略文件
创建一个 .gitignore 文件，把要忽略的文件写在文件中，
比如：
build/
install/
log/
完成后添加规则