// SSE (Server-Sent Events) 客户端封装
// 参考: https://blog.csdn.net/weixin_44474834/article/details/148894446

class SSEClient {
  constructor(url, options = {}) {
    this.url = url
    this.options = options
    this.requestTask = null
    this.buffer = '' // 缓存未完成的数据
    this.isConnected = false
    this.reconnectAttempts = 0
    this.maxReconnectAttempts = options.maxReconnectAttempts || 5
    this.reconnectDelay = options.reconnectDelay || 3000
    this.onMessage = options.onMessage || (() => {})
    this.onError = options.onError || (() => {})
    this.onOpen = options.onOpen || (() => {})
    this.onClose = options.onClose || (() => {})
    // 允许自定义请求方法、头与请求体/构造器
    this.method = options.method || 'POST'
    this.headers = options.headers || {}
    this.data = options.data
    this.dataBuilder = options.dataBuilder || null
  }

  /**
   * 连接SSE
   */
  connect() {
    if (this.isConnected) {
      console.log('SSE already connected')
      return
    }

    const reqData = this.dataBuilder ? this.dataBuilder() : this.data

    this.requestTask = wx.request({
      url: this.url,
      method: this.method,
      header: this.headers,
      data: reqData,
      enableChunked: true, // 启用分块传输
      timeout: 0, // SSE需要长连接，设置为0表示不超时
      success: (res) => {
        console.log('SSE connection established', res)
        this.isConnected = true
        this.reconnectAttempts = 0
        this.onOpen()
      },
      fail: (err) => {
        console.error('SSE connection failed', err)
        this.isConnected = false
        this.onError(err)
        this.handleReconnect()
      }
    })

    // 监听分块数据
    this.requestTask.onChunkReceived((res) => {
      const chunk = this.arrayBufferToString(res.data)
      this.handleChunk(chunk)
    })

    // 监听头部接收完成
    this.requestTask.onHeadersReceived((res) => {
      console.log('SSE headers received', res)
    })
  }

  /**
   * 处理接收到的数据块
   */
  handleChunk(chunk) {
    this.buffer += chunk
    const lines = this.buffer.split('\n')
    
    // 保留最后一个可能不完整的行
    this.buffer = lines.pop() || ''

    for (let line of lines) {
      line = line.trim()
      
      // 跳过空行和注释行
      if (!line || line.startsWith(':')) {
        continue
      }

      // 解析data字段
      if (line.startsWith('data:')) {
        const data = line.substring(5).trim()
        try {
          const parsedData = JSON.parse(data)
          this.onMessage(parsedData)
        } catch (e) {
          // 如果不是JSON，直接传递原始数据
          this.onMessage(data)
        }
      }
    }
  }

  /**
   * ArrayBuffer转字符串
   */
  arrayBufferToString(buffer) {
    const uint8Array = new Uint8Array(buffer)
    let str = ''
    for (let i = 0; i < uint8Array.length; i++) {
      str += String.fromCharCode(uint8Array[i])
    }
    return str
  }

  /**
   * 重连处理
   */
  handleReconnect() {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.log('Max reconnect attempts reached')
      this.onError(new Error('Max reconnect attempts reached'))
      return
    }

    this.reconnectAttempts++
    const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1) // 指数退避
    
    console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts})`)
    
    setTimeout(() => {
      this.connect()
    }, delay)
  }

  /**
   * 关闭连接
   */
  close() {
    if (this.requestTask) {
      this.requestTask.abort()
      this.requestTask = null
    }
    this.isConnected = false
    this.buffer = ''
    this.reconnectAttempts = 0
    this.onClose()
  }

  /**
   * 重置连接
   */
  reset() {
    this.close()
    this.connect()
  }
}

module.exports = SSEClient
