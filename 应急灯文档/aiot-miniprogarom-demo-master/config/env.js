// 环境配置
const ENV = {
  // 开发环境
  dev: {
    deviceMacApiBase: 'http://127.0.0.1:8003/api/v1/deviceMac',
    deviceDataApiBase: 'http://127.0.0.1:8002/api/v1/deviceData',
    deviceStatusApiBase: 'http://127.0.0.1:8004/api/v1/deviceStatus',
    boardApiBase: 'http://127.0.0.1:8001/api/v1/board',
    sseBase: 'http://127.0.0.1:8002'
  },
  // 生产环境
  prod: {
    deviceMacApiBase: 'https://api.yourdomain.com/api/v1/deviceMac',
    deviceDataApiBase: 'https://api.yourdomain.com/api/v1/deviceData',
    deviceStatusApiBase: 'https://api.yourdomain.com/api/v1/deviceStatus',
    boardApiBase: 'https://api.yourdomain.com/api/v1/board',
    sseBase: 'https://api.yourdomain.com'
  }
}

// 当前环境 dev | prod
const currentEnv = 'dev'

module.exports = {
  ...ENV[currentEnv],
  isDev: currentEnv === 'dev'
}
