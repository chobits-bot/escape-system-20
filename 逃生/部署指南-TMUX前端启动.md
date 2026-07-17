# 前端 tmux 启动指南

## 背景问题

SSH 连接服务器后，直接运行 `npm run dev` 启动前端，一旦电脑休眠或 SSH 断开，进程会被杀掉，每次都需要重新登录、重新启动。

## 解决方案：使用 tmux

tmux 是一个终端会话管理器，可以在后台保持进程运行，SSH 断开后进程不会被杀掉。

---

## 一、安装 tmux

```bash
sudo apt install tmux -y
```

## 二、首次启动前端

```bash
# 杀掉可能残留的旧进程
pkill -f "vite"

# 用 tmux 后台启动前端
tmux new -d -s frontend "cd ~/escape-project/aiot/aiot-vue && npm run dev -- --host 0.0.0.0 --port 5173"
```

- `-d`：后台运行，不占用当前终端
- `-s frontend`：给会话取名 `frontend`，方便后续操作

## 三、日常使用

### 查看前端是否在运行

```bash
tmux ls
```

输出示例：
```
frontend: 1 windows (created Thu Jul 17 10:00:00 2026)
```

### 进入前端日志界面

```bash
tmux attach -t frontend
```

可以实时看到 `npm run dev` 的输出日志。

### 退出日志界面（不停止进程）

按 `Ctrl+B`，松开后按 `D`

会显示 `[detached from frontend]`，前端继续在后台运行。

### 重启前端（前端挂了的情况）

```bash
# 杀掉旧进程
pkill -f "vite"

# 重新用 tmux 启动
tmux new -d -s frontend "cd ~/escape-project/aiot/aiot-vue && npm run dev -- --host 0.0.0.0 --port 5173"
```

---

## 四、命令速查表

| 操作 | 命令 |
|------|------|
| 查看所有 tmux 会话 | `tmux ls` |
| 进入前端日志 | `tmux attach -t frontend` |
| 退出日志（不关进程） | `Ctrl+B` → `D` |
| 杀掉前端会话 | `tmux kill-session -t frontend` |
| 启动前端 | `tmux new -d -s frontend "cd ~/escape-project/aiot/aiot-vue && npm run dev -- --host 0.0.0.0 --port 5173"` |

---

## 五、完整流程（从零开始）

```bash
# 1. SSH 连接服务器
ssh ubuntu@42.193.218.29

# 2. 启动后端容器
cd ~/escape-project
docker compose up -d

# 3. 启动算法服务
cd ~/escape-project/aiot/algorithm-service
nohup python3 main.py > /tmp/algo.log 2>&1 &

# 4. 用 tmux 启动前端
pkill -f "vite" 2>/dev/null
tmux new -d -s frontend "cd ~/escape-project/aiot/aiot-vue && npm run dev -- --host 0.0.0.0 --port 5173"

# 5. 访问前端
# 浏览器打开 http://42.193.218.29:5173
```

**不是永远不会掉。** tmux 能防的情况：

| 场景 | 会掉吗 |
|------|--------|
| 电脑休眠 | 不会 |
| SSH 断开 | 不会 |
| 关闭终端窗口 | 不会 |
| **服务器重启** | **会掉** |

服务器重启后 tmux 会话也会消失，需要重新启动。如果想要服务器重启后自动启动前端，可以设置 systemd 服务，需要的话告诉我。