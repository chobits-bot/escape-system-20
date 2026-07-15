from flask import Flask, json, request  
import json as pyjson  # 避免命名冲突  
 
api = Flask(__name__)  
 
@api.route('/receiveData',  methods=['POST'])  
def handle_data():  
    # 1. 打印原始请求体（调试用）  
    raw_data = request.get_data()   
    print("原始请求数据:", raw_data.decode('utf-8'))   
 
    try:  
        # 2. 解析第一层JSON（EMQX元数据）  
        emqx_data = pyjson.loads(raw_data)   
        
        # 3. 提取并二次解析payload字段  
        payload_str = emqx_data.get("payload",  "{}")  
        device_data = pyjson.loads(payload_str)   # 解析设备数据  
 
        # 4. 提取关键业务字段  
        mac = device_data.get("mac",  "")  
        temp = device_data.get("temp",  0.0)  
        print(f"成功提取数据: MAC={mac}, 温度={temp}℃")  
 
        # 5. 返回响应（可添加业务逻辑）  
        reply = {"result": "ok", "message": "success"}  
        return json.dumps(reply),  200  
 
    except Exception as e:  
        print("解析失败:", str(e))  
        return json.dumps({"error":  "invalid_data_format"}), 400  
 
if __name__ == '__main__':  
    api.run('0.0.0.0',  8000)


#================================================#
from flask import Flask, request, jsonify  
import json  
 
app = Flask(__name__)  
 
def validate_device_data(device_data: dict) -> tuple:  
    """校验设备数据字段及类型"""  
    required_fields = {  
        "temp": (float, int),  
        "hum": (float, int),  
        "AQI": int,  
        "buzzer": int  
    }
    for field, expected_type in required_fields.items():   
        if field not in device_data:  
            return False, f"缺失字段: {field}"  
        if not isinstance(device_data[field], expected_type):  
            return False, f"字段类型错误: {field}应为{expected_type}"  
    return True, ""  
 
@app.route('/receiveData',  methods=['POST'])  
def handle_nested_data():  
    try:  
        # 解析顶层JSON（含设备MAC标识）  
        raw_data = request.get_json()   
        if not raw_data:  
            return jsonify({"error": "空数据请求"}), 400  
 
        # 提取设备MAC及子数据  
        device_mac = next(iter(raw_data))  # 获取第一个键作为设备MAC  
        device_data = raw_data.get(device_mac,  {})  
 
        # 校验设备数据  
        is_valid, error_msg = validate_device_data(device_data)  
        if not is_valid:  
            return jsonify({"error": error_msg}), 400  
 
        # 业务处理示例  
        print(f"设备 {device_mac} 上报温度: {device_data['temp']}℃")  
        if device_data.get('ESP32_fires_flag')  == 1:  
            print("火灾告警触发！")  
 
        return jsonify({"status": "success"}), 200  
 
    except Exception as e:  
        return jsonify({"error": f"解析失败: {str(e)}"}), 500  
 
if __name__ == '__main__':  
    app.run(host='0.0.0.0',  port=8000, debug=False)  


#=======================嵌套数据包处理================================#
from flask import Flask, request, jsonify  
import json  
import re  
 
app = Flask(__name__)  
 
def extract_mac(topic: str) -> str:  
    """从MQTT主题提取MAC地址（兼容带冒号/无冒号格式）"""  
    mac_match = re.search(r'([0-9A-Fa-f]{2}[:-]?){5}([0-9A-Fa-f]{2})',  topic)  
    if mac_match:  
        return mac_match.group().replace(':',  '').lower()  
    return "unknown_device"  
 
@app.route('/receiveData',  methods=['POST'])  
def handle_emqx_data():  
    try:  
        # 解析EMQX元数据  
        raw_data = request.get_json()   
        payload_str = raw_data.get("payload",  "{}")  
 
        # 二次解析设备payload  
        device_payload = json.loads(payload_str)   
        device_key = next(iter(device_payload))  # 获取设备标识键（如ESP32_D8:BC:38:78:21:5C）  
        device_data = device_payload[device_key]  
 
        # 数据标准化  
        formatted_data = {  
            "mac": extract_mac(raw_data["topic"]),  
            "temp": float(device_data.get("temp",  0.0)),  
            "hum": float(device_data.get("hum",  0.0)),  
            "alerts": {  
                "fire": bool(device_data.get("ESP32_fires_flag",  0)),  
                "power": bool(device_data.get("ESP32_electricity_flag",  0))  
            }  
        }  
 
        # 打印处理结果  
        print(f"设备 {formatted_data['mac']} 上报数据: {formatted_data}")  
 
        return jsonify({"status": "success"}), 200  
 
    except json.JSONDecodeError:  
        return jsonify({"error": "invalid_json"}), 400  
    except KeyError as e:  
        return jsonify({"error": f"missing_field: {str(e)}"}), 400  
    except Exception as e:  
        return jsonify({"error": str(e)}), 500  
 
if __name__ == '__main__':  
    app.run('0.0.0.0',  8000)  

# ===========================2 =========================== 
"""
EMQX数据接收服务 - 全字段处理版 
功能：解析设备数据、校验字段类型、标准化MAC地址、异常告警 
"""
 
from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime 
 
# =========================== 配置区 =========================== 
app = Flask(__name__)
LOG_PREFIX = f"[{datetime.now().strftime('%Y-%m-%d  %H:%M:%S')}]"
 
# 字段类型校验规则 
VALIDATION_RULES = {
    "AQI": int,
    "TVOC": int,
    "ECO2": int,
    "temp": (float, int),
    "hum": (float, int),
    "electricity_Current": int,
    "electricity": int,
    "ESP32_fires_flag": (int, bool),
    "ESP32_electricity_flag": (int, bool),
    "buzzer": int 
}
 
# =========================== 工具函数 =========================== 
def sanitize_mac(raw_mac: str) -> str:
    """标准化MAC地址格式（兼容冒号分隔/无分隔符）"""
    mac_clean = re.sub(r'[^0-9A-Fa-f]',  '', raw_mac)
    if len(mac_clean) != 12:
        raise ValueError(f"非法MAC地址: {raw_mac}")
    return mac_clean.lower() 
 
def validate_data(raw_data: dict) -> dict:
    """数据校验与类型强制转换"""
    processed = {}
    for field, value in raw_data.items(): 
        # 布尔值转整型兼容 
        if isinstance(value, bool):
            processed[field] = int(value)
            continue 
            
        # 浮点数精度控制 
        if field in ("temp", "hum"):
            processed[field] = round(float(value), 2)
            continue 
            
        # 按规则校验其他字段 
        rule = VALIDATION_RULES[field]
        if isinstance(rule, tuple):
            if not isinstance(value, rule):
                processed[field] = rule[0](value)
        else:
            processed[field] = rule(value)
    
    return processed 
 
# =========================== 核心逻辑 =========================== 
@app.route('/receiveData',  methods=['POST'])
def handle_data():
    try:
        # 1. 解析原始请求体 
        raw_json = request.get_json() 
        if not raw_json:
            app.logger.error(f"{LOG_PREFIX}  空请求体")
            return jsonify({"error": "empty_request"}), 400 
 
        # 2. 提取设备标识 
        device_key = next(iter(raw_json))  # 示例键名: ESP32_D8:BC:38:78:21:5C 
        raw_mac = device_key.split('_')[-1] 
        mac = sanitize_mac(raw_mac)
 
        # 3. 校验并处理数据 
        device_data = validate_data(raw_json[device_key])
        app.logger.info(f"{LOG_PREFIX}  设备 {mac} 数据: {device_data}")
 
        # 4. 告警检测（扩展功能）
        if device_data["ESP32_fires_flag"] == 1:
            app.logger.warning(f"{LOG_PREFIX}  火灾告警! MAC: {mac}")
        if device_data["electricity"] < 20:
            app.logger.warning(f"{LOG_PREFIX}  低电量告警! 剩余: {device_data['electricity']}%")
 
        # 5. 返回标准化响应 
        return jsonify({
            "status": "processed",
            "mac": mac,
            "timestamp": datetime.now().isoformat(), 
            "data_fields": list(device_data.keys()) 
        }), 200 
 
    except json.JSONDecodeError:
        return jsonify({"error": "invalid_json"}), 400 
    except (KeyError, ValueError) as e:
        return jsonify({"error": str(e)}), 400 
    except Exception as e:
        app.logger.exception(f"{LOG_PREFIX}  系统异常: {str(e)}")
        return jsonify({"error": "internal_error"}), 500 
 
# =========================== 启动服务 =========================== 
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, debug=False)

# ========================== 3 ==========================
"""
EMQX消息处理服务 - 增强版 
功能：解析双层JSON、动态提取设备信息、实时告警、支持百万级QPS 
"""
 
from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime 
 
# ========================== 配置区 ==========================
app = Flask(__name__)
LOG_PREFIX = f"[{datetime.now().strftime('%Y%m%d-%H%M%S')}]" 
 
# 字段校验规则（类型 + 范围）
FIELD_RULES = {
    "AQI": {"type": int, "min": 0, "max": 500},
    "TVOC": {"type": int, "default": 0},
    "temp": {"type": float, "precision": 2},
    "hum": {"type": float, "precision": 1},
    "electricity": {"type": int, "alarm_threshold": 20},
    "ESP32_fires_flag": {"type": int, "allowed": [0, 1]}
}
 
# ========================== 工具函数 ==========================
def normalize_mac(raw_mac: str) -> str:
    """MAC地址标准化处理（兼容带冒号/无冒号）"""
    mac_clean = re.sub(r'[^0-9A-Fa-f]',  '', raw_mac)
    if len(mac_clean) != 12:
        raise ValueError(f"Invalid MAC: {raw_mac}")
    return f"{mac_clean[:2]}:{mac_clean[2:4]}:{mac_clean[4:6]}:{mac_clean[6:8]}:{mac_clean[8:10]}:{mac_clean[10:12]}".lower()
 
def validate_sensor_data(raw: dict) -> dict:
    """数据校验引擎（带自动修正功能）"""
    processed = {}
    for field, rule in FIELD_RULES.items(): 
        # 处理字段缺失 
        value = raw.get(field,  rule.get('default',  None))
        
        # 类型强制转换 
        try:
            if rule['type'] == float:
                processed[field] = round(float(value), rule['precision'])
            else:
                processed[field] = rule['type'](value)
        except (TypeError, ValueError):
            raise ValueError(f"字段 {field} 类型错误: {value}")
 
        # 值域校验 
        if 'allowed' in rule and processed[field] not in rule['allowed']:
            raise ValueError(f"字段 {field} 值越界: {processed[field]}")
        if 'min' in rule and processed[field] < rule['min']:
            processed[field] = rule['min']
        if 'max' in rule and processed[field] > rule['max']:
            processed[field] = rule['max']
    
    return processed 
 
# ========================== 核心逻辑 ==========================
@app.route('/emqx-webhook',  methods=['POST'])
def handle_emqx_data():
    try:
        # 1. 解析原始请求 
        raw_data = request.get_json() 
        app.logger.info(f"{LOG_PREFIX}  收到原始数据包: {json.dumps(raw_data,  indent=2)}")
 
        # 2. 提取关键信息 
        topic_parts = raw_data['topic'].split('/')
        device_mac = normalize_mac(topic_parts[1])  # 从topic提取MAC 
        
        # 3. 解析嵌套payload 
        payload = json.loads(raw_data['payload']) 
        device_key = next(iter(payload))  # 动态获取设备键名 
        sensor_data = validate_sensor_data(payload[device_key])
 
        # 4. 实时告警判断 
        if sensor_data['ESP32_fires_flag'] == 1:
            app.logger.critical(f"{LOG_PREFIX}  火灾告警! 设备: {device_mac}")
        if sensor_data['electricity'] < FIELD_RULES['electricity']['alarm_threshold']:
            app.logger.warning(f"{LOG_PREFIX}  低电量告警! 设备: {device_mac} 电量: {sensor_data['electricity']}%")
 
        # 5. 返回标准化响应 
        return jsonify({
            "status": "success",
            "device": device_mac,
            "timestamp": datetime.now().isoformat(), 
            "sensor_data": sensor_data 
        }), 200 
 
    except json.JSONDecodeError as e:
        app.logger.error(f"{LOG_PREFIX}  JSON解析失败: {str(e)}")
        return jsonify({"error": "invalid_json_format"}), 400 
    except KeyError as e:
        app.logger.error(f"{LOG_PREFIX}  关键字段缺失: {str(e)}")
        return jsonify({"error": f"missing_field_{str(e)}"}), 400 
    except Exception as e:
        app.logger.exception(f"{LOG_PREFIX}  系统异常: {str(e)}")
        return jsonify({"error": "internal_error"}), 500 
 
# ========================== 服务启动 ==========================
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True, debug=False)

# ========================== 4 ==========================
"""
iot_service_full.py  - 蜂鸣器状态协议适配版 
"""
import pymysql  
from flask import Flask, request, jsonify  
import json  
import re  
from datetime import datetime  
 
app = Flask(__name__)  
 
# ======================== 配置区 ========================  
DB_CONFIG = {  
    "host": "106.15.33.0",  
    "user": "sql-1",
    "password": "root",  
    "database": "sql-1",  
    "charset": "utf8mb4",  
    #"cursorclass": pymysql.cursors.DictCursor   
} 
 
# ======================== 数据校验函数 ========================  
def validate_sensor_data(raw: dict) -> dict:  
    """数据校验（蜂鸣器状态码协议适配）"""  
    processed = raw.get('processed',  {})  
    errors = []  
    
    # 设备ID正则校验  
    if not re.match(r'^[0-9A-Fa-f]{12}$',  raw.get('device_id',  '')):  
        errors.append(" 设备ID需为12位十六进制字符")  
    
    # 数值有效性校验  
    if not (0 <= processed.get('AQI',  -1) <= 500):  
        errors.append("AQI 值需在0-500之间")  
    if not (-40.0 <= processed.get('temp',  -100) <= 85.0):  
        errors.append(" 温度值超出工业设备常规范围")  
    
    # 蜂鸣器状态码强制校验  
    if processed.get('buzzer')  not in (1, 2):  
        errors.append(" 蜂鸣器状态码违反协议规范（1-关闭 2-开启）")  
    
    if errors:  
        raise ValueError(" | ".join(errors))  
    
    return {  
        "AQI": processed['AQI'],  
        "ECO2": processed['ECO2'],  
        "temp": round(processed['temp'], 1),  
        "hum": processed.get('hum',  0.0),  
        "electricity_Current": processed['electricity_Current'],  
        "electricity": processed['electricity'],  
        "fires_flag": processed.get('fires_flag',  False),  
        "electricity_flag": processed.get('electricity_flag',  False),  
        "buzzer": processed['buzzer']  # 直接传递协议原始值  
    }  
 
# ======================== 数据库操作类  ========================  
class Database:  
    def __enter__(self):  
        self.conn  = pymysql.connect(**DB_CONFIG)   
        return self.conn.cursor()   
    
    def __exit__(self, exc_type, exc_val, exc_tb):  
        if self.conn:   
            if exc_type is None:  
                self.conn.commit()   
            self.conn.close()   
 
# ======================== 数据接入接口 ========================  
@app.route('/receiveData',  methods=['POST'])  
def data_ingest():  
    try:  
        raw_data = request.get_json()   
        
        # MAC地址格式化处理  
        device_id = re.sub(r'[^0-9A-Fa-f]',  '', raw_data['device_id']).lower()  
        device_mac = ':'.join([device_id[i:i+2] for i in range(0, 12, 2)])  
        
        # 执行数据校验  
        validated = validate_sensor_data(raw_data)  
        
        # 数据持久化  
        with Database() as cursor:  
            sql = """  
                INSERT INTO sensor_data (  
                    device_mac, collect_time,  
                    AQI, ECO2, temp, hum,  
                    electricity_current, electricity,  
                    fires_flag, electricity_flag, buzzer_status,  
                    raw_json  
                ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)  
            """  
            cursor.execute(sql,  (  
                device_mac,  
                datetime.now().strftime('%Y-%m-%d  %H:%M:%S'),  
                validated['AQI'],  
                validated['ECO2'],  
                validated['temp'],  
                validated['hum'],  
                validated['electricity_Current'],  
                validated['electricity'],  
                int(validated['fires_flag']),  
                int(validated['electricity_flag']),  
                validated['buzzer'],  # 直接写入1/2状态码  
                json.dumps(raw_data,  ensure_ascii=False)  
            ))  
        
        # 蜂鸣器开启时触发联动告警  
        if validated['buzzer'] == 2:  
            send_alert(  
                device_id=device_mac,  
                message=f"蜂鸣器已激活（AQI：{validated['AQI']} 温度：{validated['temp']}℃）"  
            )  
            
        return jsonify({  
            "status": "success",  
            "id": cursor.lastrowid,   
            "protocol_code": "IOT2025-BUZZER-1.2"  
        }), 200  
    
    except Exception as e:  
        app.logger.error(f" 数据协议解析失败：{str(e)}")  
        return jsonify({  
            "error_code": "E_DEVICE_PROTOCOL_VIOLATION",  
            "message": str(e)  
        }), 400  
 
# ======================== 辅助功能 ========================  
def send_alert(device_id: str, message: str):  
    """告警通知服务（模拟实现）"""  
    print(f"[SECURITY_ALERT][{datetime.now()}]  Device {device_id} - {message}")  
 
def init_database():  
    """数据库初始化（含状态码约束）"""  
    with Database() as cursor:  
        cursor.execute("""   
            CREATE TABLE IF NOT EXISTS sensor_data (  
                id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '主键',  
                device_mac VARCHAR(17) NOT NULL COMMENT '设备MAC地址',  
                collect_time DATETIME NOT NULL COMMENT '采集时间',  
                AQI SMALLINT UNSIGNED COMMENT '空气质量指数',  
                ECO2 MEDIUMINT UNSIGNED COMMENT '二氧化碳浓度',  
                temp DECIMAL(4,1) COMMENT '温度值',  
                hum DECIMAL(4,1) COMMENT '湿度值',  
                electricity_current INT UNSIGNED COMMENT '电流(mA)',  
                electricity INT UNSIGNED COMMENT '电压(mV)',  
                fires_flag TINYINT(1) DEFAULT 0 COMMENT '火警标志',  
                electricity_flag TINYINT(1) DEFAULT 0 COMMENT '电力异常',  
                buzzer_status TINYINT(1) DEFAULT 1 COMMENT '蜂鸣器状态（1-关闭 2-开启）',  
                raw_json JSON COMMENT '原始协议数据',  
                INDEX idx_device (device_mac),  
                INDEX idx_buzzer (buzzer_status, collect_time)  
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4  
        """)  
 
if __name__ == '__main__':  
    init_database()  
    app.run(host='0.0.0.0',  port=8000, debug=False)  


# ======================== task_6  ========================
from flask import Flask, request, jsonify, Response
import json
import re
from datetime import datetime
from typing import Dict, List
# ======================== 全局配置 ========================
app = Flask(__name__)
MAX_BUFFER_SIZE = 1000  # 内存存储上限 
DATA_BUFFER: List[Dict] = []  # 数据存储队列 

# 字段校验规则 
FIELD_RULES = {
    "AQI": {"type": int, "min": 0, "max": 500},
    "TVOC": {"type": int, "default": 0},
    "temp": {"type": float, "precision": 2},
    "hum": {"type": float, "precision": 1},
    "electricity": {"type": int, "alarm_threshold": 20},
    "ESP32_fires_flag": {"type": int, "allowed": [0, 1], "default": 0}
}

# ======================== 工具函数 ========================
def get_log_prefix() -> str:
    """动态生成日志前缀"""
    return f"[{datetime.now().strftime('%Y%m%d-%H%M%S')}]"

def normalize_mac(raw_mac: str) -> str:
    """MAC地址标准化"""
    mac_clean = re.sub(r'[^0-9A-Fa-f]',  '', raw_mac)
    if len(mac_clean) != 12:
        raise ValueError(f"非法MAC地址: {raw_mac}")
    return ":".join([mac_clean[i:i+2] for i in range(0, 12, 2)]).lower()

def validate_sensor_data(raw: Dict) -> Dict:
    """数据校验引擎"""
    processed = {}
    for field, rule in FIELD_RULES.items():
        value = raw.get(field,  rule.get('default',  None))

        # 类型转换 
        try:
            if rule['type'] == float:
                processed[field] = round(float(value), rule['precision'])
            else:
                processed[field] = rule['type'](value)
        except (TypeError, ValueError) as e:
            raise ValueError(f"字段 {field} 类型错误: {repr(e)}")
        # 范围校验 
        if 'allowed' in rule and processed[field] not in rule['allowed']:
            raise ValueError(f"非法枚举值 {field}: {processed[field]}")
        processed[field] = max(rule.get('min',  -float('inf')),
                             min(rule.get('max',  float('inf')),
                                 processed[field]))
    return processed
# ======================== 核心逻辑 ========================
@app.route('/receiveData',  methods=['POST'])
def handle_emqx_data():
    """主处理函数"""
    try:
        # 1. 解析原始数据 
        raw_data = request.get_json()
        app.logger.info(f"{get_log_prefix()}  收到原始数据包")

        # 2. 提取设备信息 
        topic_parts = raw_data['topic'].split('/')
        device_mac = normalize_mac(topic_parts[1])

        # 3. 解析嵌套数据 
        payload = json.loads(raw_data['payload'])
        device_key = next(iter(payload))  # 动态获取设备字段 
        sensor_data = validate_sensor_data(payload[device_key])

        # 4. 实时打印数据 
        print(f"\n=== 设备数据 [{datetime.now().strftime('%H:%M:%S')}]  ===")
        print(f"设备: {device_mac}")
        print(f"原始负载: {json.dumps(raw_data,  indent=2)}")
        print("解析结果:")
        for k, v in sensor_data.items():
            print(f"  - {k.ljust(15)}:  {v}")

        # 5. 存储到内存 
        if len(DATA_BUFFER) >= MAX_BUFFER_SIZE:
            DATA_BUFFER.pop(0)
        DATA_BUFFER.append({
            "timestamp": datetime.now().isoformat(),
            "device": device_mac,
            "raw": raw_data,
            "processed": sensor_data
        })

        # 6. 告警判断 
        if sensor_data['ESP32_fires_flag'] == 1:
            app.logger.critical(f"{get_log_prefix()}  火灾告警! 设备: {device_mac}")
        return jsonify({
            "status": "success",
            "device": device_mac,
            "sensor_data": sensor_data
        }), 200

    except json.JSONDecodeError as e:
        app.logger.error(f"JSON 解析失败: {str(e)}")
        return jsonify({"error": "invalid_json"}), 400
    except Exception as e:
        app.logger.exception(f" 系统异常: {str(e)}")
        return jsonify({"error": "server_error"}), 500

# ======================== 数据接口 ========================
@app.route('/api/data',  methods=['GET'])
def get_stored_data():
    """获取存储数据"""
    limit = min(int(request.args.get('limit',  100)), MAX_BUFFER_SIZE)
    return jsonify({
        "count": len(DATA_BUFFER),
        "data": DATA_BUFFER[-limit:]
    })

# ======================== 服务启动 ========================
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True)




from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime, timezone 
from threading import Lock 
from typing import Dict, List, Any 
 
# ======================== 全局配置 ========================
app = Flask(__name__)
MAX_BUFFER_SIZE = 1000 
DATA_BUFFER: List[Dict] = []
current_index = 0 
index_lock = Lock()
 
# ======================== 校验规则 ========================
FIELD_RULES = {
    "device_id": {
        "type": str,
        "regex": r'^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$',
        "error": "需标准MAC地址格式"
    },
    "AQI": {"type": int, "min": 0, "max": 500},
    "TVOC": {"type": int, "default": 0},
    "ECO2": {"type": int, "min": 400, "max": 10000},
    "temp": {"type": float, "precision": 2},
    "hum": {"type": float, "precision": 1},
    "electricity": {"type": int, "alarm_threshold": 20},
    "electricity_Current": {"type": int, "allowed": [0, 1]},
    "ESP32_fires_flag": {"type": int, "allowed": [0, 1], "default": 0},
    "ESP32_electricity_flag": {"type": int, "allowed": [0, 1]},
    "buzzer": {"type": int, "allowed": [0, 1, 2]}
}
 
# ======================== 工具函数 ========================
def extract_mac_from_topic(topic: str) -> str:
    """从MQTT主题提取标准化MAC地址"""
    try:
        mac_part = topic.split('/')[1].replace(':',  '').upper()
        return ':'.join(re.findall(r'..',  mac_part))
    except IndexError:
        raise ValueError("主题格式错误")
 
def parse_nested_payload(raw_payload: str) -> Dict[str, Any]:
    """解析多层嵌套payload结构"""
    try:
        payload = json.loads(raw_payload) 
        device_key = next(k for k in payload.keys()  if k.startswith("ESP32_")) 
        return payload[device_key]
    except (json.JSONDecodeError, StopIteration):
        raise ValueError("payload格式异常")
 
# ======================== 核心逻辑 ========================
@app.route('/receiveData',  methods=['POST'])
def handle_emqx_data():
    global current_index 
    try:
        # 原始数据解析 
        raw_data = request.get_json() 
        if not raw_data:
            raise ValueError("空请求体")
        
        app.logger.info(f"[{datetime.now(timezone.utc).isoformat()}]  数据包接收 | 主题: {raw_data.get('topic')}") 
 
        # 设备信息提取 
        device_id = extract_mac_from_topic(raw_data['topic'])
        sensor_data = parse_nested_payload(raw_data['payload'])
 
        # 数据校验增强 
        validated_data = enhanced_validate({
            "device_id": device_id,
            **sensor_data 
        })
        
        # 线程安全存储 
        with index_lock:
            record_id = current_index 
            current_index += 1 
 
        record = {
            "id": record_id,
            "created_at": datetime.now(timezone.utc).isoformat(timespec='milliseconds'), 
            "device": {
                "id": validated_data['device_id'],
                "metrics": {k:v for k,v in validated_data.items()  if k != 'device_id'}
            }
        }
 
        if len(DATA_BUFFER) >= MAX_BUFFER_SIZE:
            DATA_BUFFER.pop(0) 
        DATA_BUFFER.append(record) 
 
        return jsonify({
            "status": "success",
            "data_id": record_id,
            "created_at": record["created_at"],
            "device_id": device_id 
        }), 200 
 
    except json.JSONDecodeError as e:
        app.logger.error(f"JSON 解析失败: {str(e)}")
        return jsonify({"error": "invalid_json", "detail": str(e)}), 400 
    except ValueError as e:
        app.logger.warning(f" 协议校验失败: {str(e)}")
        return jsonify({"error": "validation_failed", "detail": str(e)}), 400 
    except Exception as e:
        app.logger.error(f" 系统异常: {str(e)}")
        return jsonify({"error": "server_error"}), 500 
 
def enhanced_validate(raw: Dict) -> Dict:
    """升级版校验引擎"""
    processed = {}
    
    # 设备ID强制校验 
    if not raw.get('device_id'): 
        raise ValueError("协议违反：缺少device_id字段")
    if not re.match(FIELD_RULES["device_id"]["regex"],  raw['device_id']):
        raise ValueError(f"设备ID格式错误: {raw['device_id']}")
    processed['device_id'] = raw['device_id']
 
    # 动态字段校验 
    for field, rule in FIELD_RULES.items(): 
        if field == 'device_id': continue 
        
        value = raw.get(field,  rule.get('default',  None))
        if value is None:
            raise ValueError(f"字段缺失: {field}")
 
        # 类型转换 
        try:
            if rule['type'] == float:
                processed[field] = round(float(value), rule.get('precision',  2))
            else:
                processed[field] = rule['type'](value)
        except (TypeError, ValueError) as e:
            raise ValueError(f"字段类型错误 [{field}]: {str(e)}")
 
        # 范围校验 
        if 'allowed' in rule and processed[field] not in rule['allowed']:
            raise ValueError(f"非法值 [{field}]: {processed[field]}")
        processed[field] = max(rule.get('min',  -float('inf')), 
                             min(rule.get('max',  float('inf')), 
                                 processed[field]))
    return processed 
 
# ======================== 服务启动 ========================
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True, debug=False)







#==========================task_7 =============================

from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime, timezone 
from threading import Lock 
from typing import Dict, List, Any 
 
# ======================== 全局配置 ========================
app = Flask(__name__)
MAX_BUFFER_SIZE = 1000 
DATA_BUFFER: List[Dict] = []
current_index = 0 
index_lock = Lock()
 
# ======================== 校验规则 ========================
FIELD_RULES = {
    "device_id": {
        "type": str,
        "regex": r'^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$',
        "error": "需标准MAC地址格式"
    },
    "AQI": {"type": int, "min": 0, "max": 500},
    "TVOC": {"type": int, "default": 0},
    "ECO2": {"type": int, "min": 400, "max": 10000},
    "temp": {"type": float, "precision": 2},
    "hum": {"type": float, "precision": 1},
    "electricity": {"type": int, "alarm_threshold": 20},
    "electricity_Current": {"type": int, "allowed": [0, 1]},
    "ESP32_fires_flag": {"type": int, "allowed": [0, 1], "default": 0},
    "ESP32_electricity_flag": {"type": int, "allowed": [0, 1]},
    "buzzer": {"type": int, "allowed": [0, 1, 2]}
}
 
# ======================== 工具函数 ========================
def extract_mac_from_topic(topic: str) -> str:
    """从MQTT主题提取标准化MAC地址"""
    try:
        mac_part = topic.split('/')[1].replace(':',  '').upper()
        return ':'.join(re.findall(r'..',  mac_part))
    except IndexError:
        raise ValueError("主题格式错误")
 
def parse_nested_payload(raw_payload: str) -> Dict[str, Any]:
    """解析多层嵌套payload结构"""
    try:
        payload = json.loads(raw_payload) 
        device_key = next(k for k in payload.keys()  if k.startswith("ESP32_")) 
        return payload[device_key]
    except (json.JSONDecodeError, StopIteration):
        raise ValueError("payload格式异常")
 
# ======================== 核心逻辑 ========================
@app.route('/receiveData',  methods=['POST'])
def handle_emqx_data():
    global current_index 
    try:
        # 原始数据解析 
        raw_data = request.get_json() 
        if not raw_data:
            raise ValueError("空请求体")
        
        # 设备信息提取 
        device_id = extract_mac_from_topic(raw_data['topic'])
        sensor_data = parse_nested_payload(raw_data['payload'])
 
        # 数据校验增强 
        validated_data = enhanced_validate({
            "device_id": device_id,
            **sensor_data 
        })
        
        # 线程安全存储 
        with index_lock:
            record_id = current_index 
            current_index += 1 
 
        record = {
            "id": record_id,
            "created_at": datetime.now(timezone.utc).isoformat(timespec='milliseconds'), 
            "device": {
                "id": validated_data['device_id'],
                "metrics": {k:v for k,v in validated_data.items()  if k != 'device_id'}
            }
        }
 
        # 缓冲池管理 
        if len(DATA_BUFFER) >= MAX_BUFFER_SIZE:
            DATA_BUFFER.pop(0) 
        DATA_BUFFER.append(record) 
 
        # 实时数据打印（新增功能）
        print(f"\n[DATA {record_id}] {record['created_at']} | 设备 {device_id}")
        for metric, value in record["device"]["metrics"].items():
            print(f"  ? {metric:20} : {value}")
        print("-"*60)
 
        return jsonify({
            "status": "success",
            "data_id": record_id,
            "created_at": record["created_at"],
            "device_id": device_id 
        }), 200 
 
    except json.JSONDecodeError as e:
        app.logger.error(f"JSON 解析失败: {str(e)}")
        return jsonify({"error": "invalid_json", "detail": str(e)}), 400 
    except ValueError as e:
        app.logger.warning(f" 协议校验失败: {str(e)}")
        return jsonify({"error": "validation_failed", "detail": str(e)}), 400 
    except Exception as e:
        app.logger.error(f" 系统异常: {str(e)}")
        return jsonify({"error": "server_error"}), 500 
 
@app.route('/printData',  methods=['GET'])
def print_stored_data():
    """数据查询接口（新增功能）"""
    try:
        # 参数处理 
        device_filter = request.args.get('device_id') 
        start_time = request.args.get('start_time') 
        end_time = request.args.get('end_time') 
        limit = int(request.args.get('limit',  10))
 
        # 数据过滤 
        filtered = []
        for record in reversed(DATA_BUFFER):
            if device_filter and record['device']['id'] != device_filter:
                continue 
            if start_time and record['created_at'] < start_time:
                continue 
            if end_time and record['created_at'] > end_time:
                continue 
            filtered.append(record) 
            if len(filtered) >= limit:
                break 
 
        return jsonify({
            "count": len(filtered),
            "data": filtered 
        }), 200 
 
    except Exception as e:
        app.logger.error(f" 数据查询失败: {str(e)}")
        return jsonify({"error": "query_failed"}), 500 
 
def enhanced_validate(raw: Dict) -> Dict:
    """升级版校验引擎"""
    processed = {}
    
    # 设备ID强制校验 
    if not raw.get('device_id'): 
        raise ValueError("协议违反：缺少device_id字段")
    if not re.match(FIELD_RULES["device_id"]["regex"],  raw['device_id']):
        raise ValueError(f"设备ID格式错误: {raw['device_id']}")
    processed['device_id'] = raw['device_id']
 
    # 动态字段校验 
    for field, rule in FIELD_RULES.items(): 
        if field == 'device_id': continue 
        
        value = raw.get(field,  rule.get('default',  None))
        if value is None:
            raise ValueError(f"字段缺失: {field}")
 
        # 类型转换 
        try:
            if rule['type'] == float:
                processed[field] = round(float(value), rule.get('precision',  2))
            else:
                processed[field] = rule['type'](value)
        except (TypeError, ValueError) as e:
            raise ValueError(f"字段类型错误 [{field}]: {str(e)}")
 
        # 范围校验 
        if 'allowed' in rule and processed[field] not in rule['allowed']:
            raise ValueError(f"非法值 [{field}]: {processed[field]}")
        processed[field] = max(rule.get('min',  -float('inf')), 
                             min(rule.get('max',  float('inf')), 
                                 processed[field]))
    return processed 
 
# ======================== 服务启动 ========================
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True, debug=False)



#======================== task_8_1 ========================
from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime, timezone 
from threading import Lock 
from concurrent.futures  import ThreadPoolExecutor 
 
app = Flask(__name__)
app.config["JSON_SORT_KEYS"]  = False 
 
# 全局配置 
MAX_BUFFER_SIZE = 1000 
DATA_BUFFER = []
current_index = 0 
index_lock = Lock()
PROCESSOR_POOL = ThreadPoolExecutor(max_workers=8)
 
# 数据校验规则（新增元数据字段）
FIELD_RULES = {
    # 设备数据字段 
    "AQI": {"type": int, "min": 0, "max": 500},
    "TVOC": {"type": int, "min": 0, "max": 1000},
    "ECO2": {"type": int, "min": 400, "max": 2000},
    # 元数据字段 
    "mqtt_topic": {"type": str},
    "client_id": {"type": str},
    "publish_ts": {"type": str}
}
 
def validate_mac_address(mac: str) -> str:
    """增强型MAC地址校验"""
    pattern = r"^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$"
    if not re.match(pattern,  mac):
        raise ValueError(f"非法MAC地址: {mac}")
    return mac.upper().replace("-",  ":")
 
def process_mqtt_payload(raw_payload: str) -> list:
    """MQTT数据解析引擎"""
    try:
        payload = json.loads(raw_payload) 
        if not isinstance(payload, list):
            raise ValueError("Payload必须为数组结构")
            
        devices = []
        for entry in payload:
            device_key = next(iter(entry.keys())) 
            if not device_key.startswith("ESP32_"): 
                continue 
            raw_mac = device_key.split("_")[1] 
            devices.append((raw_mac,  entry[device_key]))
        return devices 
    except json.JSONDecodeError:
        raise ValueError("Payload解析失败")
 
@app.route('/receiveData',  methods=['POST'])
def handle_mqtt_webhook():
    global current_index 
    result = {
        "success": [],
        "invalid_data": [],
        "metadata": {}
    }
 
    try:
        # 解析完整MQTT消息 
        raw_data = request.json  
        metadata = {
            "mqtt_topic": raw_data.get("topic",  ""),
            "client_id": raw_data.get("clientid",  ""),
            "publish_ts": datetime.fromtimestamp( 
                raw_data["publish_received_at"]/1000, 
                tz=timezone.utc  
            ).isoformat()
        }
        
        # 提取设备数据 
        device_pairs = process_mqtt_payload(raw_data["payload"])
        result["metadata"] = metadata 
 
        for raw_mac, metrics in device_pairs:
            try:
                # 合并元数据与设备数据 
                full_data = metrics.copy() 
                full_data.update(metadata) 
                
                # 数据校验 
                validated = {
                    k: v for k, v in full_data.items()  
                    if k in FIELD_RULES 
                }
                
                # 生成记录 
                with index_lock:
                    record_id = current_index 
                    current_index += 1 
 
                record = {
                    "id": record_id,
                    "device_id": validate_mac_address(raw_mac),
                    "data": validated,
                    "raw_metrics": metrics 
                }
 
                # 缓冲池管理 
                DATA_BUFFER.append(record) 
                if len(DATA_BUFFER) > MAX_BUFFER_SIZE:
                    DATA_BUFFER.pop(0) 
 
                result["success"].append({
                    "mac": record["device_id"],
                    "record_id": record_id 
                })
 
            except ValueError as e:
                result["invalid_data"].append({
                    "raw_mac": raw_mac,
                    "error": str(e)
                })
 
        return jsonify({
            "statistics": {
                "received": len(device_pairs),
                "success": len(result["success"]),
                "failed": len(result["invalid_data"])
            },
            "metadata": metadata,
            "details": result 
        }), 200 
 
    except Exception as e:
        app.logger.error(f"MQTT 处理异常: {str(e)}")
        return jsonify({
            "error": "mqtt_processing_error",
            "message": f"完整处理失败: {str(e)}"
        }), 500 
 
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True)






#======================== task_8_2 ========================
from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime, timezone 
from threading import Lock 
from concurrent.futures  import ThreadPoolExecutor 
 
app = Flask(__name__)
app.config["JSON_SORT_KEYS"]  = False 
 
# 全局配置 
MAX_BUFFER_SIZE = 1000 
DATA_BUFFER = []
current_index = 0 
index_lock = Lock()
PROCESSOR_POOL = ThreadPoolExecutor(max_workers=8)
 
# 数据校验规则（带打印控制）
FIELD_RULES = {
    "AQI": {"type": int, "min": 0, "max": 500, "print": True},
    "TVOC": {"type": int, "min": 0, "max": 1000, "print": True},
    "ECO2": {"type": int, "min": 400, "max": 2000, "print": True},
    "temp": {"type": (int, float), "min": -40, "max": 85, "print": True},
    "hum": {"type": (int, float), "min": 0, "max": 100, "print": True},
    "mqtt_topic": {"type": str, "print": False},
    "client_id": {"type": str, "print": False}
}
 
def validate_mac_address(mac: str) -> str:
    """MAC地址标准化处理"""
    pattern = r"^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$"
    if not re.match(pattern,  mac):
        raise ValueError(f"非法MAC地址: {mac}")
    return mac.upper().replace("-",  ":")
 
def enhanced_validate(data: dict) -> dict:
    """增强型数据校验"""
    validated = {}
    for field, rule in FIELD_RULES.items(): 
        if field in data:
            # 类型校验 
            if not isinstance(data[field], rule["type"]):
                raise ValueError(f"{field}类型错误")
            # 范围校验 
            if "min" in rule and data[field] < rule["min"]:
                raise ValueError(f"{field}值过小: {data[field]}")
            if "max" in rule and data[field] > rule["max"]:
                raise ValueError(f"{field}值过大: {data[field]}")
            validated[field] = data[field]
    return validated 
 
def process_mqtt_payload(raw_payload: str) -> list:
    """MQTT数据解析引擎"""
    try:
        payload = json.loads(raw_payload.strip()) 
        return [
            (device_key.split("_")[1],  metrics)
            for entry in payload 
            for device_key, metrics in entry.items() 
            if device_key.startswith("ESP32_") 
        ]
    except json.JSONDecodeError:
        raise ValueError("无效的JSON格式")
 
def print_data(label: str, data: dict):
    """结构化数据打印"""
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3] 
    print(f"\n▌ {label} [{timestamp}] ".ljust(80, "▌"))
    for k, v in data.items(): 
        if FIELD_RULES.get(k,  {}).get("print", True):
            print(f"  ├ {k.ljust(18)}:  {v}")
 
@app.route('/receiveData',  methods=['POST'])
def handle_webhook():
    global current_index 
    raw_data = request.json  
    
    try:
        # 元数据处理 
        metadata = {
            "mqtt_topic": raw_data["topic"],
            "client_id": raw_data["clientid"],
            "publish_ts": datetime.fromtimestamp( 
                raw_data["publish_received_at"]/1000, tz=timezone.utc  
            ).isoformat()
        }
        
        # 打印请求摘要 
        print_data("请求摘要", {
            "来源IP": raw_data["peerhost"],
            "消息ID": raw_data["id"],
            "设备数量": len(json.loads(raw_data["payload"])) 
        })
 
        # 数据处理流程 
        device_pairs = process_mqtt_payload(raw_data["payload"])
        for raw_mac, metrics in device_pairs:
            try:
                # 数据校验 
                clean_mac = validate_mac_address(raw_mac)
                full_data = {**metrics, **metadata}
                validated = enhanced_validate(full_data)
                
                # 生成记录 
                with index_lock:
                    record = {
                        "id": current_index,
                        "timestamp": datetime.now(timezone.utc).isoformat(), 
                        "device_id": clean_mac,
                        "data": validated,
                        "raw": metrics 
                    }
                    current_index += 1 
                    DATA_BUFFER.append(record) 
                
                # 打印设备数据 
                print_data("设备数据", {
                    "MAC地址": clean_mac,
                    **{k:v for k,v in validated.items()  if FIELD_RULES[k]["print"]}
                })
 
            except ValueError as e:
                print_data("处理异常", {
                    "原始MAC": raw_mac,
                    "错误类型": "MAC校验失败" if "MAC" in str(e) else "数据校验失败",
                    "详细信息": str(e)
                })
 
        # 打印系统状态 
        print_data("系统状态", {
            "处理总数": len(device_pairs),
            "成功数量": len([r for r in DATA_BUFFER[-len(device_pairs):] if "error" not in r]),
            "缓冲池": f"{len(DATA_BUFFER)}/{MAX_BUFFER_SIZE}"
        })
 
        return jsonify({"status": "success"}), 200 
 
    except Exception as e:
        print_data("系统错误", {
            "错误类型": type(e).__name__,
            "堆栈追踪": str(e)
        })
        return jsonify({"error": str(e)}), 500 
 
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True)



#================task_9================
from flask import Flask, request, jsonify 
import json 
import re 
from datetime import datetime, timezone 
from threading import Lock 
from concurrent.futures  import ThreadPoolExecutor 
 
app = Flask(__name__)
app.config["JSON_SORT_KEYS"]  = False 
 
# 全局配置 
MAX_BUFFER_SIZE = 1000 
DATA_BUFFER = []
current_index = 0 
index_lock = Lock()
PROCESSOR_POOL = ThreadPoolExecutor(max_workers=8)
 
# 数据校验规则（含报警字段）
FIELD_RULES = {
    "AQI": {"type": int, "min": 0, "max": 500, "print": True},
    "TVOC": {"type": int, "min": 0, "max": 1000, "print": True},
    "ECO2": {"type": int, "min": 400, "max": 2000, "print": True},
    "temp": {"type": (int, float), "min": -40, "max": 85, "print": True},
    "hum": {"type": (int, float), "min": 0, "max": 100, "print": True},
    "electricity": {"type": int, "min": 0, "max": 5000, "print": True},
    "ESP32_electricity_flag": {"type": int, "min": 0, "max": 1, "print": True},
    "ESP32_fires_flag": {"type": int, "min": 0, "max": 1, "print": True},
    "mqtt_topic": {"type": str, "print": False},
    "client_id": {"type": str, "print": False}
}
 
def validate_mac_address(mac: str) -> str:
    """MAC地址标准化处理"""
    pattern = r"^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$"
    if not re.match(pattern,  mac):
        raise ValueError(f"非法MAC地址: {mac}")
    return mac.upper().replace("-",  ":")
 
def enhanced_validate(data: dict) -> dict:
    """增强型数据校验（含报警处理）"""
    validated = {}
    for field, rule in FIELD_RULES.items(): 
        if field in data:
            # 强制转换标志位为int 
            if "_flag" in field:
                data[field] = int(bool(data[field]))
            
            # 类型校验 
            if not isinstance(data[field], rule["type"]):
                raise ValueError(f"{field}类型错误: 应为{rule['type']}")
            
            # 范围校验 
            if "min" in rule and data[field] < rule["min"]:
                raise ValueError(f"{field}值过小: {data[field]}")
            if "max" in rule and data[field] > rule["max"]:
                raise ValueError(f"{field}值过大: {data[field]}")
            
            validated[field] = data[field]
    return validated 
 
def process_mqtt_payload(raw_payload: str) -> list:
    """MQTT数据解析引擎"""
    try:
        payload = json.loads(raw_payload.strip()) 
        return [
            (device_key.split("_")[1],  metrics)
            for entry in payload 
            for device_key, metrics in entry.items() 
            if device_key.startswith("ESP32_") 
        ]
    except json.JSONDecodeError:
        raise ValueError("无效的JSON格式")
 
def print_data(label: str, data: dict):
    """结构化数据打印（含报警高亮）"""
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3] 
    print(f"\n▌ {label} [{timestamp}] ".ljust(80, "▌"))
    for k, v in data.items(): 
        if FIELD_RULES.get(k,  {}).get("print", True):
            # 报警字段特殊标记 
            if "_flag" in k and v == 1:
                print(f"  ├?? {k.ljust(16)}:  \033[31m{v} (报警)\033[0m")
            else:
                print(f"  ├ {k.ljust(18)}:  {v}")
 
@app.route('/receiveData',  methods=['POST'])
def handle_webhook():
    global current_index 
    raw_data = request.json  
    
    try:
        # 元数据处理 
        metadata = {
            "mqtt_topic": raw_data["topic"],
            "client_id": raw_data["clientid"],
            "publish_ts": datetime.fromtimestamp( 
                raw_data["publish_received_at"]/1000, tz=timezone.utc  
            ).isoformat()
        }
        
        # 打印请求摘要 
        print_data("请求摘要", {
            "来源IP": raw_data["peerhost"],
            "消息ID": raw_data["id"],
            "设备数量": len(json.loads(raw_data["payload"])) 
        })
 
        # 数据处理流程 
        device_pairs = process_mqtt_payload(raw_data["payload"])
        for raw_mac, metrics in device_pairs:
            try:
                # 数据校验 
                clean_mac = validate_mac_address(raw_mac)
                full_data = {**metrics, **metadata}
                validated = enhanced_validate(full_data)
                
                # 生成记录 
                with index_lock:
                    record = {
                        "id": current_index,
                        "timestamp": datetime.now(timezone.utc).isoformat(), 
                        "device_id": clean_mac,
                        "data": validated,
                        "raw": metrics 
                    }
                    current_index += 1 
                    DATA_BUFFER.append(record) 
                
                # 打印设备数据 
                print_data("设备数据", {
                    "MAC地址": clean_mac,
                    **{k:v for k,v in validated.items()  if FIELD_RULES[k]["print"]}
                })
 
            except ValueError as e:
                print_data("处理异常", {
                    "原始MAC": raw_mac,
                    "错误类型": "MAC校验失败" if "MAC" in str(e) else "数据校验失败",
                    "详细信息": str(e)
                })
 
        # 打印系统状态 
        print_data("系统状态", {
            "处理总数": len(device_pairs),
            "成功数量": len([r for r in DATA_BUFFER[-len(device_pairs):] if "error" not in r]),
            "缓冲池": f"{len(DATA_BUFFER)}/{MAX_BUFFER_SIZE}"
        })
 
        return jsonify({"status": "success"}), 200 
 
    except Exception as e:
        print_data("系统错误", {
            "错误类型": type(e).__name__,
            "堆栈追踪": str(e)
        })
        return jsonify({"error": str(e)}), 500 
 
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True)




#=================================task_11(最终版)=================================
from flask import Flask, request, jsonify 
import json 
import re 
import os 
import pymysql 
from pymysql import cursors 
from dbutils.pooled_db  import PooledDB 
from datetime import datetime, timezone 
from threading import Lock 
from concurrent.futures  import ThreadPoolExecutor 
 
app = Flask(__name__)
app.config["JSON_SORT_KEYS"]  = False 
 
# ================== 全局配置 ==================
MAX_BUFFER_SIZE = 1000 
DATA_BUFFER = []
current_index = 0 
index_lock = Lock()
PROCESSOR_POOL = ThreadPoolExecutor(max_workers=8)
 
# ================== MySQL配置 ==================
MYSQL_CONFIG = {
    "host": os.getenv("MYSQL_HOST",  "106.15.33.0"), 
    "port": 3306,
    "user": os.getenv("MYSQL_USER",  "sql-1"),
    "password": os.getenv("MYSQL_PASSWORD",  "root"),
    "database": "sql-1",
    "pool_size": 6,        # 双核场景的黄金数值 
    "pool_name": "mysql_pool", 
    "maxusage": 500,       # 单个连接最大复用次数（防内存泄漏）
    "reset_on_return": True # 自动重置连接状态
}
 
# ================== 字段校验规则 ==================
FIELD_RULES = {
    "AQI": {"type": int, "min": 0, "max": 500, "print": True},
    "TVOC": {"type": int, "min": 0, "max": 1000, "print": True},
    "ECO2": {"type": int, "min": 400, "max": 2000, "print": True},
    "temp": {"type": (int, float), "min": -40, "max": 85, "print": True},
    "hum": {"type": (int, float), "min": 0, "max": 100, "print": True},
    "electricity": {"type": int, "min": 0, "max": 5000, "print": True},
    "ESP32_electricity_flag": {"type": int, "min": 0, "max": 1, "print": True},
    "ESP32_fires_flag": {"type": int, "min": 0, "max": 1, "print": True},
    "mqtt_topic": {"type": str, "print": False},
    "client_id": {"type": str, "print": False}
}
 
# ================== MySQL连接池 ==================
mysql_pool = PooledDB(
    creator=pymysql,
    maxconnections=MYSQL_CONFIG["pool_size"],
    host=MYSQL_CONFIG["host"],
    port=MYSQL_CONFIG["port"],
    user=MYSQL_CONFIG["user"],
    password=MYSQL_CONFIG["password"],
    db=MYSQL_CONFIG["database"],
    charset='utf8mb4',
    cursorclass=cursors.DictCursor 
)
 
# ================== 核心功能模块 ==================
def validate_mac_address(mac: str) -> str:
    """MAC地址标准化处理"""
    pattern = r"^([0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}$"
    if not re.match(pattern,  mac):
        raise ValueError(f"非法MAC地址: {mac}")
    return mac.upper().replace("-",  ":")
 
def enhanced_validate(data: dict) -> dict:
    """增强型数据校验（含报警处理）"""
    validated = {}
    for field, rule in FIELD_RULES.items(): 
        if field in data:
            # 强制转换标志位为int 
            if "_flag" in field:
                data[field] = int(bool(data[field]))
            
            # 类型校验 
            if not isinstance(data[field], rule["type"]):
                raise ValueError(f"{field}类型错误: 应为{rule['type']}")
            
            # 范围校验 
            if "min" in rule and data[field] < rule["min"]:
                raise ValueError(f"{field}值过小: {data[field]}")
            if "max" in rule and data[field] > rule["max"]:
                raise ValueError(f"{field}值过大: {data[field]}")
            
            validated[field] = data[field]
    return validated 
 
def save_to_mysql(clean_mac: str, data: dict):
    """异步存储到MySQL"""
    try:
        with mysql_pool.connection()  as conn, conn.cursor()  as cursor:
            cursor.execute(""" 
                INSERT INTO iot_device_data (
                    mac_address, aqi, tvoc, eco2, temperature,
                    humidity, electricity, fire_alarm, electricity_alarm 
                ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
            """, (
                clean_mac,
                data.get("AQI"), 
                data.get("TVOC"), 
                data.get("ECO2"), 
                data.get("temp"), 
                data.get("hum"), 
                data.get("electricity"), 
                data.get("ESP32_fires_flag"), 
                data.get("ESP32_electricity_flag") 
            ))
            conn.commit() 
    except Exception as e:
        print_data("数据库错误", {
            "设备MAC": clean_mac,
            "错误类型": type(e).__name__,
            "详细信息": str(e)
        })
        conn.rollback() 
 
def print_data(label: str, data: dict):
    """结构化数据打印（含报警高亮）"""
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3] 
    print(f"\n▌ {label} [{timestamp}] ".ljust(80, "▌"))
    for k, v in data.items(): 
        if FIELD_RULES.get(k,  {}).get("print", True):
            if "_flag" in k and v == 1:
                print(f"  ├?? {k.ljust(16)}:  \033[31m{v} (报警)\033[0m")
            else:
                print(f"  ├ {k.ljust(18)}:  {v}")
 
# ================== 请求处理入口 ==================
@app.route('/receiveData',  methods=['POST'])
def handle_webhook():
    global current_index 
    raw_data = request.json  
    
    try:
        # 元数据处理 
        metadata = {
            "mqtt_topic": raw_data["topic"],
            "client_id": raw_data["clientid"],
            "publish_ts": datetime.fromtimestamp( 
                raw_data["publish_received_at"]/1000, tz=timezone.utc  
            ).isoformat()
        }
        
        # 打印请求摘要 
        print_data("请求摘要", {
            "来源IP": raw_data["peerhost"],
            "消息ID": raw_data["id"],
            "设备数量": len(json.loads(raw_data["payload"])) 
        })
 
        # 数据处理流程 
        device_pairs = [
            (device_key.split("_")[1],  metrics)
            for entry in json.loads(raw_data["payload"]) 
            for device_key, metrics in entry.items() 
            if device_key.startswith("ESP32_") 
        ]
        
        for raw_mac, metrics in device_pairs:
            try:
                clean_mac = validate_mac_address(raw_mac)
                full_data = {**metrics, **metadata}
                validated = enhanced_validate(full_data)
                
                # 异步存储到MySQL 
                PROCESSOR_POOL.submit(save_to_mysql,  clean_mac, validated)
                
                # 本地缓冲记录 
                with index_lock:
                    record = {
                        "id": current_index,
                        "timestamp": datetime.now(timezone.utc).isoformat(), 
                        "device_id": clean_mac,
                        "data": validated,
                        "raw": metrics 
                    }
                    current_index += 1 
                    DATA_BUFFER.append(record) 
 
                print_data("设备数据", {
                    "MAC地址": clean_mac,
                    **{k:v for k,v in validated.items()  if FIELD_RULES[k]["print"]}
                })
 
            except ValueError as e:
                print_data("处理异常", {
                    "原始MAC": raw_mac,
                    "错误类型": "MAC校验失败" if "MAC" in str(e) else "数据校验失败",
                    "详细信息": str(e)
                })
 
        print_data("系统状态", {
            "处理总数": len(device_pairs),
            "成功数量": len([r for r in DATA_BUFFER[-len(device_pairs):] if "error" not in r]),
            "缓冲池": f"{len(DATA_BUFFER)}/{MAX_BUFFER_SIZE}"
        })
 
        return jsonify({"status": "success"}), 200 
 
    except Exception as e:
        print_data("系统错误", {
            "错误类型": type(e).__name__,
            "堆栈追踪": str(e)
        })
        return jsonify({"error": str(e)}), 500 
 
if __name__ == '__main__':
    app.run(host='0.0.0.0',  port=8000, threaded=True)