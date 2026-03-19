# MQTT调试
## 原因
因未知原因导致，app_ss无法启动，逐步排查发现mosquitto.log文件不见了
## 排查过程
查看日志
```bash
sudo systemctl status mosquitto
journalctl -u mosquitto -f
```
显示结果如下
```bash
titan@titan-ubuntu1:~$ sudo systemctl status mosquitto
[sudo] password for titan: 
● mosquitto.service - Mosquitto MQTT v3.1/v3.1.1 Broker
     Loaded: loaded (/lib/systemd/system/mosquitto.service; enabled; vendor pre>
     Active: failed (Result: exit-code) since Thu 2025-07-24 15:40:34 CST; 1min>
       Docs: man:mosquitto.conf(5)
             man:mosquitto(8)
    Process: 2175 ExecStart=/usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.con>
   Main PID: 2175 (code=exited, status=1/FAILURE)

Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Scheduled restart >
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: Stopped Mosquitto MQTT v3.1/v3.1.1 Br>
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Start request repe>
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Failed with result>
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: Failed to start Mosquitto MQTT v3.1/v>

titan@titan-ubuntu1:~$ journalctl -u mosquitto -f
-- Logs begin at Wed 2023-11-22 05:10:21 CST. --
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: Starting Mosquitto MQTT v3.1/v3.1.1 Broker...
Jul 24 15:40:34 titan-ubuntu1 mosquitto[2175]: 1753342834: Error: Unable to open log file /var/log/mosquitto/mosquitto.log for writing.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Main process exited, code=exited, status=1/FAILURE
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Failed with result 'exit-code'.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: Failed to start Mosquitto MQTT v3.1/v3.1.1 Broker.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Scheduled restart job, restart counter is at 5.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: Stopped Mosquitto MQTT v3.1/v3.1.1 Broker.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Start request repeated too quickly.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: mosquitto.service: Failed with result 'exit-code'.
Jul 24 15:40:34 titan-ubuntu1 systemd[1]: Failed to start Mosquitto MQTT v3.1/v3.1.1 Broker.

```

发现问题显示
```bash
Jul 24 15:40:34 titan-ubuntu1 mosquitto[2175]: 1753342834: Error: Unable to open log file /var/log/mosquitto/mosquitto.log for writing.
```
寻找mosquitto.log发现已经没有了。

## 解决方法
```bash
# 创建日志目录（如果不存在）
sudo mkdir -p /var/log/mosquitto
# 设置正确的所有者和权限
sudo chown -R mosquitto:mosquitto /var/log/mosquitto
sudo chmod -R 755 /var/log/mosquitto
# 创建空日志文件并授权
sudo touch /var/log/mosquitto/mosquitto.log
sudo chown mosquitto:mosquitto /var/log/mosquitto/mosquitto.log
sudo chmod 644 /var/log/mosquitto/mosquitto.log

# 重置失败状态计数
sudo systemctl reset-failed mosquitto
# 启动服务
sudo systemctl start mosquitto
# 检查状态
sudo systemctl status mosquitto
```

