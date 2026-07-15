// 传感器数据页面（SSE 实时数据）
const deviceMacApi = require('../../api/deviceMac.js')
const SSEClient = require('../../utils/sse.js')
const config = require('../../config/env.js')

Page({
  data: {
    deviceList: [], // 设备列表
    selectedDevice: null, // 当前选中的设备
    selectedIndex: 0, // 选中的设备索引
    
    // 实时数据
    realtimeData: {
      aqi: 0,
      tvoc: 0,
      eco2: 0,
      temperature: 0,
      humidity: 0
    },
    
    // 图表数据
    charts: [],
    
    loading: false,
    showDevicePicker: false
  },

  sseClients: {}, // SSE客户端实例集合（按指标分开）
  _isPageActive: false, // 页面是否处于活跃状态

  /**
   * 初始化图表配置
   */
  /**
   * 获取基础图表配置
   */
  getBaseChartOpts() {
    return {
      color: ["#1890FF"],
      enableScroll: false,
      dataLabel: false,
      update: true,
      animation: false,
      duration: 0,
      xAxis: {
        disableGrid: true,
        type: 'grid',
        gridType: 'dash',
        itemCount: 10, // 显示10个格子
        scrollBackgroundColor: '#00000000',
        scrollColor: '#DEE7F7',
        rotateLabel: false,
      },
      yAxis: {
        gridType: "dash",
        dashLength: 2,
      },
      extra: {
        line: {
          type: "curve",
          width: 2,
          activeType: "hollow"
        }
      }
    }
  },

  /**
   * 获取指标配置
   */
  getMetricsConfig() {
    return [
      { id: 'aqi', name: 'AQI', unit: '级', color: '#1890FF' },
      { id: 'tvoc', name: 'TVOC', unit: 'ppb', color: '#91CB74' },
      { id: 'eco2', name: 'eCO₂', unit: 'ppm', color: '#FAC858' },
      { id: 'temperature', name: '温度', unit: '°C', color: '#EE6666' },
      { id: 'humidity', name: '湿度', unit: '%', color: '#73C0DE' }
    ]
  },

  /**
   * 初始化图表配置
   */
  initCharts() {
    const baseOpts = this.getBaseChartOpts()
    const metrics = this.getMetricsConfig()

    const charts = metrics.map(metric => {
      const opts = JSON.parse(JSON.stringify(baseOpts))
      opts.color = [metric.color]
      
      // 初始化10个空数据点
      const categories = Array.from({ length: 10 }, (_, i) => '')
      const data = Array.from({ length: 10 }, () => 0)
      
      return {
        id: `chart-${metric.id}`,
        metricId: metric.id,
        name: metric.name,
        unit: metric.unit,
        opts: opts,
        chartData: {
          categories: categories,
          series: [{
            name: metric.name,
            data: data
          }]
        }
      }
    })

    this.setData({ charts })
  },

  /**
   * 重置图表数据
   */
  resetCharts() {
    const charts = this.data.charts || []
    const updates = {}
    
    charts.forEach((chart, index) => {
      // 重置为空数据
      const categories = Array.from({ length: 10 }, () => '')
      const data = Array.from({ length: 10 }, () => 0)
      
      updates[`charts[${index}].chartData`] = {
        categories: categories,
        series: [{
          name: chart.name,
          data: data
        }]
      }
    })
    
    if (Object.keys(updates).length > 0) {
      this.setData(updates)
    }
  },

  onLoad() {
    this._isPageActive = true
    this.initCharts()
    this.loadDeviceList()
  },

  onShow() {
    // 页面显示时标记为活跃
    this._isPageActive = true
    // 如果有选中的设备但SSE未连接，重新连接
    if (this.data.selectedDevice && Object.keys(this.sseClients).length === 0) {
      this.initSSE()
    }
  },

  onHide() {
    // 页面隐藏时标记为非活跃并关闭SSE连接以节省资源
    this._isPageActive = false
    this.closeSSE()
  },

  onUnload() {
    // 页面卸载时关闭SSE连接并标记为非活跃
    this._isPageActive = false
    this.closeSSE()
  },

  /**
   * 加载设备列表
   */
  async loadDeviceList() {
    try {
      wx.showLoading({ title: '加载中...' })
      const res = await deviceMacApi.selectAllEnabledDeviceMacAddress()
      
      if (res.data && Array.isArray(res.data.device_mac_addresses) && res.data.device_mac_addresses.length > 0) {
        const list = res.data.device_mac_addresses

        this.setData({
          deviceList: list,
          selectedDevice: list[0],
          selectedIndex: 0
        })
        console.log(this.data.deviceList);
        
        // 加载第一个设备的数据
        this.loadDeviceData()
      } else {
        wx.showToast({
          title: '暂无设备',
          icon: 'none'
        })
      }
    } catch (err) {
      console.error('加载设备列表失败', err)
      wx.showToast({
        title: '加载失败',
        icon: 'none'
      })
    } finally {
      wx.hideLoading()
    }
  },

  /**
   * 显示设备选择器
   */
  showDeviceSelector() {
    this.setData({ showDevicePicker: true })
  },

  /**
   * 选择设备
   */
  onDeviceChange(e) {
    // 支持 picker 的 e.detail.value 和自定义列表的 data-index
    const index = (e && e.detail && e.detail.value) != null
      ? e.detail.value
      : (e && e.currentTarget && e.currentTarget.dataset
        ? e.currentTarget.dataset.index
        : undefined)
    console.log('onDeviceChange index:', index)
    if (index === undefined || index === null) return

    const idx = Number(index)
    if (Number.isNaN(idx)) return

    // 先关闭SSE连接
    this.closeSSE()
    
    // 清空实时数据
    this.setData({
      selectedIndex: idx,
      selectedDevice: this.data.deviceList[idx],
      showDevicePicker: false,
      realtimeData: {
        aqi: 0,
        tvoc: 0,
        eco2: 0,
        temperature: 0,
        humidity: 0
      }
    })
    
    // 重置图表数据
    this.resetCharts()
    
    // 延迟一下再重新连接，确保旧连接完全关闭
    setTimeout(() => {
      this.loadDeviceData()
    }, 100)
  },

  /**
   * 取消选择
   */
  onPickerCancel() {
    this.setData({ showDevicePicker: false })
  },

  // 阻止事件冒泡的空方法
  noop() {},

  /**
   * 加载设备数据
   */
  async loadDeviceData() {
    if (!this.data.selectedDevice) return

    this.setData({ loading: true })

    try {
      this.initSSE()
    } catch (err) {
      console.error('初始化SSE失败', err)
    } finally {
      this.setData({ loading: false })
    }
  },

  /**
   * 初始化SSE连接
   */
  initSSE() {
    if (!this.data.selectedDevice) return

    const mac = this.data.selectedDevice.mac_address
    console.log(mac);
    const headers = {
      'Content-Type': 'application/json',
      'Accept': 'text/event-stream'
    }

    // 指标分组：int64 与 float64
    const int64Metrics = ['aqi', 'tvoc', 'eco2']
    const float64Metrics = ['temperature', 'humidity']

    // 先关闭旧连接
    this.closeSSE()
    this.sseClients = {}

    const makeClient = (metric, url) => new SSEClient(url, {
      method: 'POST',
      headers,
      // 每次连接从“现在”开始（秒）
      dataBuilder: () => ({
        mac_address: mac,
        field: metric,
        timestamp: Math.floor(Date.now() / 1000)
      }),
      onOpen: () => {
        console.log(`[SSE:${metric}] 连接成功`)
      },
      onMessage: (payload) => {
        // 服务器可能发送 {list:[{value,create_time},...]} 或单点 {value,create_time}
        this.handleMetricStreamData(metric, payload)
      },
      onError: (err) => {
        console.error(`[SSE:${metric}] 连接错误`, err)
      },
      onClose: () => {
        console.log(`[SSE:${metric}] 连接关闭`)
      },
      maxReconnectAttempts: 5,
      reconnectDelay: 3000
    })

    // 建立 int64 流
    int64Metrics.forEach(metric => {
      const url = `${config.sseBase}/sse/int64`
      const client = makeClient(metric, url)
      this.sseClients[metric] = client
      client.connect()
    })

    // 建立 float64 流
    float64Metrics.forEach(metric => {
      const url = `${config.sseBase}/sse/float64`
      const client = makeClient(metric, url)
      this.sseClients[metric] = client
      client.connect()
    })
  },

  // 处理单个指标流数据
  handleMetricStreamData(metric, payload) {
    if (!metric) return

    const pushPoint = (value, ts) => {
      // 检查页面是否还处于活跃状态
      if (!this._isPageActive) return
      
      const v = Number(value)
      if (!Number.isFinite(v)) return

      // 兼容秒/毫秒时间戳
      const tsNum = Number(ts)
      let ms = Date.now()
      if (Number.isFinite(tsNum)) {
        ms = tsNum > 1e12 ? tsNum : tsNum * 1000
      }

      // 更新实时卡片
      const realtimeData = { ...this.data.realtimeData }
      realtimeData[metric] = v
      this.setData({ realtimeData })

      // 更新图表数据
      this.updateChartData(metric, v, ms)
    }

    if (payload && Array.isArray(payload.list)) {
      payload.list.forEach(item => pushPoint(item.value, item.create_time))
      return
    }
    if (payload && (payload.value !== undefined)) {
      pushPoint(payload.value, payload.create_time)
    }
  },

  /**
   * 更新图表数据
   */
  updateChartData(metric, value, timestamp) {
    // 检查页面是否还处于活跃状态
    if (!this._isPageActive) return
    
    const charts = this.data.charts || []
    const chartIndex = charts.findIndex(c => c.metricId === metric)
    if (chartIndex === -1) return

    const chart = charts[chartIndex]
    const chartData = chart.chartData || { categories: [], series: [] }
    
    // 复制当前数据
    let categories = Array.isArray(chartData.categories) ? chartData.categories.slice() : []
    let series = Array.isArray(chartData.series) ? chartData.series.map(s => ({ 
      name: s.name, 
      data: (s.data || []).slice() 
    })) : []

    if (!series.length) {
      series = [{ name: chart.name, data: [] }]
    }

    // 生成时间标签
    const date = new Date(timestamp)
    const timeLabel = `${String(date.getHours()).padStart(2, '0')}:${String(date.getMinutes()).padStart(2, '0')}:${String(date.getSeconds()).padStart(2, '0')}`
    
    // 添加新数据点
    categories.push(timeLabel)
    series[0].data.push(value)

    // 保持固定长度（10个点）
    const maxLen = 10
    while (categories.length > maxLen) {
      categories.shift()
    }
    while (series[0].data.length > maxLen) {
      series[0].data.shift()
    }

    // 更新数据
    const updates = {}
    updates[`charts[${chartIndex}].chartData`] = {
      categories: categories,
      series: series
    }
    this.setData(updates)
  },

  /**
   * 关闭SSE连接
   */
  closeSSE() {
    if (this.sseClients) {
      Object.keys(this.sseClients).forEach(k => {
        try { 
          this.sseClients[k].close() 
          this.sseClients[k].abort()
          this.sseClients[k].offChunkReceived()
          console.log(`[SSE:${k}] 连接已关闭`)
        } catch (e) {
          console.error(`[SSE:${k}] 关闭失败`, e)
        }
      })
    }
    this.sseClients = {}
  },

  /**
   * 刷新数据
   */
  onRefresh() {
    // 避免重复连接，刷新前关闭现有 SSE
    this.closeSSE()
    this.loadDeviceData()
  },

  /**
   * 下拉刷新
   */
  onPullDownRefresh() {
    this.loadDeviceData()
    setTimeout(() => {
      wx.stopPullDownRefresh()
    }, 1000)
  }
})
