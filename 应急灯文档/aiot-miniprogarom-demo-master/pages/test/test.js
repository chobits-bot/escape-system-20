// 折线图页面（uCharts 折线图 + 自动更新，可选 SSE 实时数据）
const util = require("../../utils/util.js")
const SSEClient = require("../../utils/sse.js")

Page({
  // 页面的初始数据
  data: {
    // 多图表数组：每项包含 { id, opts, chartData }
    charts: []
  },

  // 初始化多个图表数据；如提供 ?count=<n> 控制数量，?sse=<url> 则连接 SSE 实时数据（作用于第1个图表）
  onLoad(options = {}) {
    const count = Math.max(1, Math.min(6, parseInt(options.count || '3', 10) || 3))
    // 基础 opts（与单图保持一致，且遵循你已设置的参数）
    const baseOpts = {
      color: ["#1890FF", "#91CB74"],
      enableScroll: true,
      dataLabel: false,
      update: true,
      animation: false,
      duration: 0,
      xAxis: {
        disableGrid: true,
        type: 'grid',
        gridType: 'dash',
        itemCount: 6,
        scrollBackgroundColor: '#00000000',
        scrollColor: '#DEE7F7',
        rotateLabel: true,
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

    // 初始数据模板
    const baseData = {
      categories: ["2016","2017","2018","2019","2020","2021"],
      series: [
        { name: "目标值", data: [35,36,31,33,13,34] },
        { name: "完成量", data: [18,27,21,24,6,28] }
      ]
    }

    const charts = Array.from({ length: count }).map((_, i) => ({
      id: `myChart-${i}`,
      opts: JSON.parse(JSON.stringify(baseOpts)),
      chartData: JSON.parse(JSON.stringify(baseData))
    }))
    this.setData({ charts })

    // 可选：页面参数传入 SSE 地址则优先使用实时数据（更新第1个图表）
    const raw = options && options.sse
    if (raw) {
      let url = ''
      try { url = decodeURIComponent(raw) } catch (e) { url = raw }
      this.connectSSE(url)
    }
  },

  // 页面显示时启动自动更新
  onShow() {
    this.startAutoUpdate()
  },

  // 页面隐藏/卸载时停止自动更新
  onHide() {
    this.stopAutoUpdate()
  },
  onUnload() {
    this.stopAutoUpdate()
    this.closeSSE()
  },

  // 启动自动更新：每秒为每个图表追加一个点并保持固定长度
  startAutoUpdate() {
    if (this._timer) clearInterval(this._timer)
    this._timer = setInterval(() => {
      const charts = this.data.charts || []
      if (!charts.length) return
      const updates = {}
      charts.forEach((c, idx) => {
        const cd = c.chartData || {}
        let categories = Array.isArray(cd.categories) ? cd.categories.slice() : []
        let series = Array.isArray(cd.series) ? cd.series.map(s => ({ name: s.name, data: s.data ? s.data.slice() : [] })) : []
        if (!categories.length || !series.length) {
          categories = ["2016","2017","2018","2019","2020","2021"]
          series = [
            { name: "目标值", data: [35,36,31,33,13,34] },
            { name: "完成量", data: [18,27,21,24,6,28] }
          ]
        }
        // 生成下一个 x 轴标签
        let nextLabel = ""
        const last = categories[categories.length - 1]
        const lastYear = Number(last)
        if (!isNaN(lastYear)) nextLabel = String(lastYear + 1)
        else {
          const now = new Date()
          nextLabel = `${now.getHours()}:${String(now.getMinutes()).padStart(2,'0')}:${String(now.getSeconds()).padStart(2,'0')}`
        }
        // 计算新值
        const updatedSeries = series.map(s => {
          const arr = s.data.slice()
          const prev = arr.length ? Number(arr[arr.length - 1]) : 20
          const delta = Math.round((Math.random() - 0.5) * 6)
          const nextVal = Math.max(0, prev + delta)
          arr.push(nextVal)
          return { name: s.name, data: arr }
        })
        categories.push(nextLabel)
        // 固定窗口长度
        const maxLen = 12
        while (categories.length > maxLen) categories.shift()
        const trimmedSeries = updatedSeries.map(s => {
          const diff = s.data.length - maxLen
          return diff > 0 ? { name: s.name, data: s.data.slice(diff) } : s
        })
        const nextData = { categories, series: trimmedSeries }
        updates[`charts[${idx}].chartData`] = JSON.parse(JSON.stringify(nextData))
      })
      if (Object.keys(updates).length) this.setData(updates)
    }, 1000)
  },

  // 停止自动更新
  stopAutoUpdate() {
    if (this._timer) {
      clearInterval(this._timer)
      this._timer = null
    }
  }
  ,

  // 连接 SSE（当提供地址时优先使用 SSE 更新图表）
  connectSSE(url) {
    if (!url || this._sse) return
    this._sse = new SSEClient(url, {
      method: 'GET',
      headers: { 'Accept': 'text/event-stream' },
      onOpen: () => {
        // 建立连接后，停止本地模拟
        this.stopAutoUpdate()
      },
      onMessage: (msg) => {
        // 期望后端直接推送 { categories:[], series:[] }
        if (msg && Array.isArray(msg.categories) && Array.isArray(msg.series)) {
          this.setData({ 'charts[0].chartData': JSON.parse(JSON.stringify(msg)) })
          return
        }
        // 兜底：若推送的是单点数值，则按时间轴追加
        if (typeof msg === 'number') {
          this.appendPointToChart(0, msg)
        } else if (msg && typeof msg.value === 'number') {
          this.appendPointToChart(0, msg.value)
        }
      },
      onError: () => {
        // 出错时回落到本地模拟
        if (!this._timer) this.startAutoUpdate()
      },
      onClose: () => {
        // 连接关闭时，如无定时器则恢复本地模拟
        if (!this._timer) this.startAutoUpdate()
      }
    })
    this._sse.connect()
  },

  // 关闭 SSE 连接
  closeSSE() {
    if (this._sse) {
      try { this._sse.close() } catch (e) {}
      this._sse = null
    }
  },

  // 将单点值追加到第二条曲线（完成量），同时平滑目标值
  appendPointToChart(index, val) {
    const charts = this.data.charts || []
    const c = charts[index]
    if (!c) return
    const cd = c.chartData || { categories: [], series: [] }
    const categories = Array.isArray(cd.categories) ? cd.categories.slice() : []
    const series = Array.isArray(cd.series) ? cd.series.map(s => ({ name: s.name, data: (s.data||[]).slice() })) : []
    if (!series.length) return

    // 生成时间标签
    const now = new Date()
    const label = `${now.getHours()}:${String(now.getMinutes()).padStart(2,'0')}:${String(now.getSeconds()).padStart(2,'0')}`
    categories.push(label)

    // 平滑追加：第一条作为目标值，按上一个值小幅波动；第二条使用实时值
    const maxLen = 12
    const s1 = series[0]
    const s2 = series[1] || { name: '完成量', data: [] }
    const prev1 = s1.data.length ? Number(s1.data[s1.data.length - 1]) : 30
    const delta1 = Math.round((Math.random() - 0.5) * 4)
    s1.data.push(Math.max(0, prev1 + delta1))
    s2.data.push(Number(val))

    while (categories.length > maxLen) categories.shift()
    series.forEach(s => {
      const diff = s.data.length - maxLen
      if (diff > 0) s.data = s.data.slice(diff)
    })

    this.setData({ [`charts[${index}].chartData`]: JSON.parse(JSON.stringify({ categories, series })) })
  }
})
