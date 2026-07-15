<template>
  <div class="page realtime-page">
    <div class="page-header">
      <div>
        <div class="page-title-text">实时数据</div>
        <div class="page-subtitle">单设备实时折线图，连接后端 /api/ws/device-data 获取实时传感器数据</div>
      </div>
      <div class="header-actions">
        <el-tag :type="connected ? 'success' : 'info'">{{ connected ? 'WS 已连接' : 'WS 未连接' }}</el-tag>
        <el-button :type="connected ? 'danger' : 'primary'" @click="toggleWs">{{ connected ? '断开连接' : '连接 WS' }}</el-button>
      </div>
    </div>

    <div class="panel toolbar">
      <el-form inline>
        <el-form-item label="设备 MAC">
          <el-select v-model="selectedMac" filterable clearable style="width:420px" placeholder="请选择一个传感器 MAC">
            <el-option v-for="item in sensors" :key="item.macAddress" :label="`${item.macAddress}${item.address ? ` · ${item.address}` : ''}`" :value="item.macAddress" />
          </el-select>
        </el-form-item>
        <el-form-item label="显示字段">
          <el-checkbox-group v-model="selectedMetrics" class="metric-check-group">
            <el-checkbox v-for="item in metricOptions" :key="item.value" :label="item.value">{{ item.label }}</el-checkbox>
          </el-checkbox-group>
        </el-form-item>
      </el-form>
    </div>

    <div class="stats-grid">
      <div v-for="card in cards" :key="card.label" class="iot-card stat-card">
        <div class="stat-label">{{ card.label }}</div>
        <div class="stat-value" :style="{ color: card.color }">{{ card.value }}</div>
      </div>
    </div>

    <div class="panel chart-panel">
      <div class="panel-header">
        <span class="panel-title">实时折线图</span>
        <span class="panel-sub">当前设备：{{ selectedMac || '未选择' }} · 已显示 {{ selectedMetrics.length }} 项 · 最近 {{ rows.length }} 条</span>
      </div>
      <div ref="chartRef" class="chart-box" />
    </div>

    <div class="panel">
      <div class="panel-header">
        <span class="panel-title">最新数据</span>
        <span class="panel-sub">仅展示当前订阅设备的最新值</span>
      </div>
      <el-descriptions :column="4" border>
        <el-descriptions-item label="时间">{{ latest.time || '—' }}</el-descriptions-item>
        <el-descriptions-item label="MAC">{{ latest.macAddress || '—' }}</el-descriptions-item>
        <el-descriptions-item label="温度">{{ latest.temp ?? '—' }}</el-descriptions-item>
        <el-descriptions-item label="湿度">{{ latest.humidity ?? '—' }}</el-descriptions-item>
        <el-descriptions-item label="AQI">{{ latest.aqi ?? '—' }}</el-descriptions-item>
        <el-descriptions-item label="ECO2">{{ latest.eco2 ?? '—' }}</el-descriptions-item>
        <el-descriptions-item label="TVOC">{{ latest.tvoc ?? '—' }}</el-descriptions-item>
        <el-descriptions-item label="电量">{{ latest.electricity ?? '—' }}</el-descriptions-item>
      </el-descriptions>
    </div>
  </div>
</template>

<script setup>
import { computed, nextTick, onMounted, onUnmounted, ref, watch } from 'vue'
import * as echarts from 'echarts'
import { ElMessage } from 'element-plus'
import { listSensors } from '@/api/sensor'

const WS_URL = `${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/api/ws/device-data`
const MAX_POINTS = 40

const sensors = ref([])
const selectedMac = ref('')
const selectedMetrics = ref(['temp', 'humidity', 'aqi', 'eco2', 'tvoc', 'electricity'])
const connected = ref(false)
const rows = ref([])
const chartRef = ref(null)
let ws = null
let chart = null
let resizeHandler = null

const metricOptions = [
  { label: '温度', value: 'temp', color: '#ef4444', unit: '°C' },
  { label: '湿度', value: 'humidity', color: '#3b82f6', unit: '%' },
  { label: 'AQI', value: 'aqi', color: '#f59e0b', unit: '' },
  { label: 'ECO2', value: 'eco2', color: '#8b5cf6', unit: 'ppm' },
  { label: 'TVOC', value: 'tvoc', color: '#10b981', unit: 'ppb' },
  { label: '电量', value: 'electricity', color: '#06b6d4', unit: '' }
]

const latest = computed(() => rows.value[0] || {})
const cards = computed(() => {
  return [
    { label: '当前设备', value: selectedMac.value || '未选择', color: '#6366f1' },
    { label: '在线流状态', value: connected.value ? '实时接入中' : '未连接', color: connected.value ? '#22c55e' : '#94a3b8' },
    { label: '消息条数', value: rows.value.length, color: '#f59e0b' }
  ]
})

function normalizeMessage(message) {
  const mac = Object.keys(message || {})[0]
  const data = mac ? message[mac] || {} : {}
  return {
    time: new Date().toLocaleTimeString(),
    macAddress: mac || '--',
    temp: data.temp ?? null,
    humidity: data.hum ?? null,
    aqi: data.AQI ?? null,
    eco2: data.ECO2 ?? null,
    tvoc: data.TVOC ?? null,
    electricity: data.electricity ?? null
  }
}
function initChart() {
  if (!chartRef.value) return
  chart = echarts.init(chartRef.value, null, { renderer: 'canvas' })
  resizeHandler = () => chart?.resize()
  window.addEventListener('resize', resizeHandler)
  renderChart()
}
function renderChart() {
  if (!chart) return
  const enabledMetrics = metricOptions.filter(item => selectedMetrics.value.includes(item.value))
  chart.setOption({
    backgroundColor: 'transparent',
    grid: { top: 24, right: 20, bottom: 32, left: 48 },
    tooltip: {
      trigger: 'axis',
      backgroundColor: '#131e34',
      borderColor: '#1e3456',
      textStyle: { color: '#e2e8f0', fontSize: 12 }
    },
    legend: {
      top: 0,
      textStyle: { color: '#94a3b8', fontSize: 12 }
    },
    xAxis: {
      type: 'category',
      data: rows.value.map(item => item.time).reverse(),
      axisLine: { lineStyle: { color: '#1e3456' } },
      axisLabel: { color: '#64748b', fontSize: 11 }
    },
    yAxis: {
      type: 'value',
      axisLabel: { color: '#64748b', fontSize: 11 },
      splitLine: { lineStyle: { color: '#1e345633' } }
    },
    series: enabledMetrics.map(item => ({
      name: item.label,
      type: 'line',
      smooth: true,
      symbol: 'circle',
      symbolSize: 6,
      data: rows.value.map(row => row[item.value] ?? null).reverse(),
      lineStyle: { color: item.color, width: 3 },
      itemStyle: { color: item.color }
    }))
  })
}
function sendSubscribe() {
  if (!ws || ws.readyState !== WebSocket.OPEN || !selectedMac.value) return
  ws.send(JSON.stringify({ subscribe: [selectedMac.value] }))
}
function connect() {
  if (!selectedMac.value) {
    ElMessage.warning('请先选择一个设备')
    return
  }
  if (ws) ws.close()
  ws = new WebSocket(WS_URL)
  ws.onopen = () => {
    connected.value = true
    sendSubscribe()
  }
  ws.onmessage = (event) => {
    try {
      const row = normalizeMessage(JSON.parse(event.data))
      if (row.macAddress !== selectedMac.value) return
      rows.value.unshift(row)
      rows.value = rows.value.slice(0, MAX_POINTS)
      renderChart()
    } catch {
      // ignore invalid message
    }
  }
  ws.onerror = () => ElMessage.error('WebSocket 连接失败')
  ws.onclose = () => {
    connected.value = false
    ws = null
  }
}
function toggleWs() {
  if (connected.value) ws?.close()
  else connect()
}
async function loadSensors() {
  const res = await listSensors()
  sensors.value = res?.list ?? res ?? []
  if (!selectedMac.value && sensors.value.length) selectedMac.value = sensors.value[0].macAddress
}

watch(selectedMac, (val, oldVal) => {
  if (val === oldVal) return
  rows.value = []
  renderChart()
  if (connected.value && val) connect()
})
watch(selectedMetrics, () => renderChart(), { deep: true })

onMounted(async () => {
  await loadSensors()
  await nextTick()
  initChart()
})
onUnmounted(() => {
  ws?.close()
  window.removeEventListener('resize', resizeHandler)
  chart?.dispose()
})
</script>

<style scoped>
.realtime-page { gap: 16px; }
.page-subtitle { margin-top: 6px; font-size: 12px; color: var(--el-text-color-secondary); }
.header-actions { display: flex; align-items: center; gap: 12px; }
.toolbar { padding-bottom: 4px; }
.stats-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 16px; }
.stat-card { padding: 18px; min-width: 350px; }
.stat-label { font-size: 12px; color: var(--el-text-color-secondary); margin-bottom: 10px; }
.stat-value { font-size: 24px; font-weight: 700; word-break: break-all; }
.chart-panel { display: flex; flex-direction: column; }
.chart-box { height: 420px; }
@media (max-width: 1200px) { .stats-grid { grid-template-columns: 1fr; } }
</style>
