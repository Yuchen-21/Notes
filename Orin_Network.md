# 多传感器多IP配置
## 如何一个设备多个IP
主要针对AGX ORIN,假设你的网口叫 eth0，你可以给这一个网口绑定多个 IP。推荐使用 nmcli 命令，这样配置是永久生效的。
### 查看当前连接
```bash
nmcli con show
```
内容如下：
```bash
yyc_orin@ubuntu:~$ nmcli con show
NAME                    UUID                                  TYPE      DEVICE  
X9 Pro                  d2c7c659-10ee-4776-9568-116b7f3e6f17  wifi      wlan0   
Mihomo                  834c7706-845c-49b9-9e01-e447daa6b42b  tun       Mihomo  
l4tbr0                  e4f4b861-1276-463b-8e8c-a81457635c86  bridge    l4tbr0  
docker0                 0269fec1-9150-4031-8353-02345e629122  bridge    docker0 
butterfry               3bf0176c-6db7-47a6-b12d-3c479d55fc89  wifi      --      
IPHONE MATE X200 ULTRA  140a42e2-1ae8-4828-a3f4-19ee588ed625  wifi      --      
nova 11                 500e7542-969e-4e41-9f25-dad2492c7d03  wifi      --      
test                    4cef7734-dea8-4c15-8f51-ea145e3e6bbd  wifi      --      
UFI-681A                fac061a4-6d79-42cb-9b37-5e96367c6222  wifi      --      
Wired connection 1      9f7afcfd-474b-3dca-a5bd-c2ed4989ed74  ethernet  --      
X9pr                    786b0ea4-2651-4ca7-9292-4be7cfb674ec  wifi      --      
X9pr 1                  4d5ddf5e-432d-485c-99dd-d8e9f7f88a2e  wifi      --      
ych                     519eb978-1405-4a04-9e6f-86ccf2cd4606  wifi      --      
ych 1                   7589a0c7-be09-48e7-9679-57315cb94709  wifi      --     
```
### 配置双IP
我们要把 .2 网段设为主 IP（因为传感器多），把 .1 网段设为副 IP。同时，网关(Gateway) 应该指向电台（因为你要通过电台上网或远程通信）。
假设：

    电台 IP（也是网关）：192.168.1.1

    传感器 IP：192.168.2.x
```bash
# 1. 设置主 IP (传感器网段) - 注意：这里不设网关，防止路由冲突
sudo nmcli con mod "Wired connection 1" ipv4.method manual ipv4.addresses 192.168.2.100/24

# 2. 追加副 IP (电台网段) - 并设置网关指向电台
sudo nmcli con mod "Wired connection 1" +ipv4.addresses 192.168.1.100/24
sudo nmcli con mod "Wired connection 1" ipv4.gateway 192.168.1.1

# 3. 设置 DNS (如果电台能上网)
sudo nmcli con mod "Wired connection 1" ipv4.dns "8.8.8.8"

# 4. 重启连接生效
sudo nmcli con up "Wired connection 1"
```
### 验证
```bash
ip addr 
inet 192.168.2.100/24 ...
inet 192.168.1.100/24 ...
```
## 查看当前网络状态
```bash
ip link
sudo apt install ethtool
sudo ethtool eth0
#关注输出中的 Speed, Duplex, Auto-negotiation 以及最后的 Link detected。
```
# ORIN 网络热点

```bash
iw dev wlan0 get power_save
# 如果输出 Power save: on，说明省电模式开着（
sudo vim /etc/NetworkManager/conf.d/default-wifi-powersave-on.conf
把里面的值改为 2（0=默认，1=忽略，2=关闭，3=开启）：
[connection]
wifi.powersave = 2
sudo systemctl restart NetworkManager
```

export QT_QUICK_BACKEND=software
export QT_QPA_PLATFORM=xcb

# orin 连不上外网
你之前手动配置静态 IP 时（比如为了连电台），填写了 Gateway（网关）：

    错误示范：

        IP: 192.168.1.100

        Netmask: 255.255.255.0

        Gateway: 192.168.1.1 <--- 这是问题的根源

如果你填写了 Gateway，系统就会认为 192.168.1.1 能带它去外网。对于纯局域网设备（传感器、不带路由功能的电台），配置静态 IP 时，Gateway 这一栏必须留空（或者填 0.0.0.0）！

## 网关和DNS
⚠️ 注意点一：只有“能上网”的那个网口，才配拥有网关

    Wi-Fi / 4G 路由器：必须有网关（通常自动获取）。

    激光雷达 / 相机 / 机械臂 / 电台：绝对不要填网关！

        在 Ubuntu 的 IPv4 设置里，Gateway 那一行直接空着。

        在命令行里，就是 ipv4.gateway ""。

⚠️ 注意点二：网段千万别撞车 (IP Conflict)

这是新手最容易犯的错误。

    错误示范：

        手机热点（Wi-Fi）：192.168.1.105 (网段 192.168.1.x)

        有线传感器（Eth0）：192.168.1.200 (网段 192.168.1.x)

    后果：

        Orin 会彻底精神分裂。当它要找 192.168.1.1 时，它不知道该走 Wi-Fi 还是走网线。通常会导致两边都断断续续。

    正确做法：

        如果手机热点默认是 192.168.1.x（很多华为/苹果手机默认是 43 或 1），那你规划传感器网络时，请务必避开这个号段。

        例如：把传感器和电台全部设在 192.168.10.x 或 10.0.0.x 这种冷门网段。

⚠️ 注意点三：DNS 也不要乱填

在配置静态 IP 的界面，通常还有一行叫 DNS。

    对于局域网设备（雷达/电台）：DNS 也要留空！

        如果你在雷达的网口上填了 8.8.8.8，Orin 在解析域名时，可能会试图通过这根网线去问谷歌“百度网址是多少”，结果当然是超时。

    只有通外网的那个网口（Wi-Fi），才需要 DNS。