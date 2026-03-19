# ftp 服务器使用命令
## 安装及连接
```bash
# 安装 ftp 客户端（如果尚未安装）
sudo apt update
sudo apt install ftp
# 连接到服务器
ftp 192.168.43.79
``` 
输入用户名和密码
## 基本命令
    ls - 列出文件
    cd 目录名 - 切换目录
    get 文件名 - 下载文件
    put 文件名 - 上传文件
    bye - 断开连接
put <本地文件路径> <远程文件路径>
