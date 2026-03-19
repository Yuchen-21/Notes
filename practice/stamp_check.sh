#!/usr/bin/env bash
set -euo pipefail

# ====== 可修改参数（也支持命令行传参）======
DURATION="${1:-60}"                # 运行时长（秒），默认60
TOPIC="${2:-}"                     # 指定topic（可选）
BAG="${3:-0}"                      # 1=同时录bag，0=不录
WS_SETUP="${4:-}"                  # 可选：你的工作空间setup路径，比如 ~/catkin_ws/devel/setup.bash
LOG_DIR="${5:-/tmp/ros_stamp_check}"  # 日志目录

mkdir -p "$LOG_DIR"
TS="$(date +%Y%m%d_%H%M%S)"
LOG="$LOG_DIR/stamp_check_${TS}.log"
BAGFILE="$LOG_DIR/cam_issue_${TS}.bag"

echo "[INFO] duration=${DURATION}s bag=${BAG} log=${LOG}"
[ -n "$WS_SETUP" ] && echo "[INFO] sourcing workspace: $WS_SETUP"

# ====== ROS env ======
if [ -f /opt/ros/noetic/setup.bash ]; then
  source /opt/ros/noetic/setup.bash
elif [ -f /opt/ros/melodic/setup.bash ]; then
  source /opt/ros/melodic/setup.bash
elif [ -f /opt/ros/kinetic/setup.bash ]; then
  source /opt/ros/kinetic/setup.bash
else
  echo "[ERROR] Cannot find /opt/ros/*/setup.bash"
  exit 1
fi

if [ -n "$WS_SETUP" ] && [ -f "$WS_SETUP" ]; then
  # shellcheck disable=SC1090
  source "$WS_SETUP"
fi

# ====== Basic check ======
if ! command -v rostopic >/dev/null 2>&1; then
  echo "[ERROR] rostopic not found. Is ROS sourced correctly?"
  exit 1
fi

if ! rostopic list >/dev/null 2>&1; then
  echo "[ERROR] Cannot talk to ROS master. Is roscore running? ROS_MASTER_URI correct?"
  echo "        Try: export ROS_MASTER_URI=http://<master_ip>:11311"
  exit 1
fi

# ====== Discover topics ======
echo "[INFO] Scanning image topics..."
TOPICS=$(rostopic list | egrep -i "image|camera|rgb|color|raw|compressed" || true)

if [ -z "$TOPICS" ]; then
  echo "[ERROR] No image-like topics found."
  exit 1
fi

echo "========== Candidate topics =========="
echo "$TOPICS" | nl -w2 -s'. '
echo "======================================"

# ====== Choose topic ======
if [ -z "$TOPIC" ]; then
  # 优先挑 raw，其次 compressed
  DEFAULT="$(echo "$TOPICS" | egrep -i "image_raw$|/image$|/image_raw/" | head -n1 || true)"
  [ -z "$DEFAULT" ] && DEFAULT="$(echo "$TOPICS" | head -n1)"

  echo -n "[PROMPT] Enter topic number (empty = default: $DEFAULT): "
  read -r CHOICE || true
  if [ -n "${CHOICE:-}" ]; then
    TOPIC="$(echo "$TOPICS" | sed -n "${CHOICE}p" || true)"
  else
    TOPIC="$DEFAULT"
  fi
fi

if [ -z "$TOPIC" ]; then
  echo "[ERROR] Topic selection failed."
  exit 1
fi

echo "[INFO] Selected topic: $TOPIC"

# ====== Detect message type ======
TYPE="$(rostopic type "$TOPIC" 2>/dev/null || true)"
if [ -z "$TYPE" ]; then
  echo "[ERROR] Cannot get message type of $TOPIC"
  exit 1
fi

MODE="raw"
if [[ "$TYPE" == "sensor_msgs/CompressedImage" ]]; then
  MODE="compressed"
elif [[ "$TYPE" == "sensor_msgs/Image" ]]; then
  MODE="raw"
else
  echo "[ERROR] Unsupported message type: $TYPE"
  echo "        Only supports sensor_msgs/Image or sensor_msgs/CompressedImage"
  exit 1
fi

echo "[INFO] Topic type: $TYPE (mode=$MODE)"

# ====== Write python checker to temp ======
PY="$LOG_DIR/stamp_check_${TS}.py"
cat > "$PY" <<'PYEOF'
#!/usr/bin/env python3
import os, sys, time
import rospy

MODE = os.environ.get("MODE", "raw").strip()
TOPIC = os.environ.get("TOPIC", "").strip()
DURATION = float(os.environ.get("DURATION", "60"))
LOGFILE = os.environ.get("LOGFILE", "")

if not TOPIC:
    print("[PY][ERROR] TOPIC env not set")
    sys.exit(2)

last = None
backward_cnt = 0
max_backward = 0.0
total = 0
t0 = time.time()

def log(msg):
    s = time.strftime("%Y-%m-%d %H:%M:%S") + " " + msg
    print(s, flush=True)
    if LOGFILE:
        with open(LOGFILE, "a", encoding="utf-8") as f:
            f.write(s + "\n")

def cb(msg):
    global last, backward_cnt, max_backward, total
    total += 1
    t = msg.header.stamp.to_sec()
    if last is not None and t < last:
        backward_cnt += 1
        diff = last - t
        max_backward = max(max_backward, diff)
        log(f"[WARN] STAMP BACKWARD: {last:.6f} -> {t:.6f} (back {diff:.6f}s) cnt={backward_cnt}")
    last = t

def main():
    rospy.init_node("stamp_monotonic_checker", anonymous=True, disable_signals=True)
    log(f"[INFO] Start checking topic={TOPIC} mode={MODE} duration={DURATION}s")

    if MODE == "compressed":
        from sensor_msgs.msg import CompressedImage
        rospy.Subscriber(TOPIC, CompressedImage, cb, queue_size=200)
    else:
        from sensor_msgs.msg import Image
        rospy.Subscriber(TOPIC, Image, cb, queue_size=200)

    rate = rospy.Rate(10)
    while not rospy.is_shutdown():
        if time.time() - t0 >= DURATION:
            break
        rate.sleep()

    log(f"[RESULT] total_msgs={total} backward_cnt={backward_cnt} max_backward={max_backward:.6f}s")
    if backward_cnt > 0:
        log("[RESULT] FAIL: stamp is NOT monotonic (time went backwards). Upstream (camera/SDK/ROS node) issue likely.")
        sys.exit(1)
    else:
        log("[RESULT] PASS: stamp monotonic (no backward).")
        sys.exit(0)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        log(f"[PY][ERROR] {e}")
        sys.exit(3)
PYEOF

chmod +x "$PY"

# ====== Optional bag record ======
BAG_PID=""
if [ "$BAG" = "1" ]; then
  if ! command -v rosbag >/dev/null 2>&1; then
    echo "[WARN] rosbag not found, skip bag recording."
  else
    echo "[INFO] Recording rosbag to $BAGFILE (topic: $TOPIC) ..."
    rosbag record -O "$BAGFILE" "$TOPIC" >/dev/null 2>&1 &
    BAG_PID="$!"
  fi
fi

# ====== Run checker ======
echo "[INFO] Running checker... (will auto-exit in ${DURATION}s)"
echo "[INFO] Log: $LOG"
set +e
MODE="$MODE" TOPIC="$TOPIC" DURATION="$DURATION" LOGFILE="$LOG" python3 "$PY"
RC=$?
set -e

# ====== Stop bag ======
if [ -n "$BAG_PID" ]; then
  echo "[INFO] Stopping rosbag (pid=$BAG_PID) ..."
  kill -INT "$BAG_PID" >/dev/null 2>&1 || true
  sleep 1
  echo "[INFO] Bag saved: $BAGFILE"
fi

echo "========== SUMMARY =========="
tail -n 10 "$LOG" || true
echo "============================="
echo "[INFO] Exit code: $RC (0=PASS, 1=FAIL stamp backward)"
exit "$RC"