# 搭建openclaw环境

## 部署docker 版本
```bash

cd ~/extend/YYC/openclaw
sudo chown -R $USER:$USER .
export OPENCLAW_IMAGE=ghcr.io/openclaw/openclaw:latest
./docker-setup.sh
```
```bash
docker 权限问题
sudo groupadd docker 2>/dev/null || true
sudo usermod -aG docker $USER
newgrp docker
docker ps```
## 更新相关命令
```bash
cd ~/extend/YYC/openclaw
docker pull ghcr.io/openclaw/openclaw:latest
docker compose down
docker compose up -d openclaw-gateway
```

## 日常启动
```bash
docker compose ps
docker compose logs -f openclaw-gateway
docker compose restart openclaw-gateway
docker compose up -d openclaw-gateway
docker compose down
```
## 以管理员权限进docker
```bash
docker exec -u root -it openclaw-openclaw-gateway-1 bash
```


## Dashboard / 认证 / 配对
```bash
docker compose run --rm openclaw-cli dashboard --no-open
docker compose run --rm openclaw-cli config get gateway.auth.token
docker compose run --rm openclaw-cli devices list
docker compose run --rm openclaw-cli devices approve <requestId>
```

## 企业微信配置
https://work.weixin.qq.com/nl/index/openclaw
## skill 安装
```bash
    # 不如直接扔到目标位置里面
# npx clawhub search "calendar"
# npx clawhub install <skill-slug>

cd /home/yyc_orin/.openclaw/workspace/skills
# 解压 skill 到当前目录
```
