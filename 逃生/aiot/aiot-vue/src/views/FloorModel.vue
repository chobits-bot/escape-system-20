<template>
  <div class="page floor-page">
    <div class="page-header">
      <div>
        <div class="page-title-text">Floor Model</div>
      </div>
      <div class="toolbar">
        <el-radio-group v-model="viewMode" size="small">
          <el-radio-button label="3d">3D</el-radio-button>
          <el-radio-button label="2d">2D</el-radio-button>
        </el-radio-group>
      </div>
    </div>

    <!-- 逃生路径计算工具栏 -->
    <div class="panel escape-toolbar">
      <div class="toolbar-row">
        <el-select v-model="selectedAlgo" size="small" style="width:140px">
          <el-option label="Ant Colony (ACO)" value="aco" />
          <el-option label="Particle Swarm (PSO)" value="pso" disabled />
          <el-option label="Simulated Annealing (SA)" value="sa" disabled />
          <el-option label="Genetic Algorithm (GA)" value="ga" disabled />
        </el-select>
        <div class="fire-switch" :class="{ active: fireMode }">
          <el-switch v-model="fireMode" size="small" active-color="#ef4444" @change="onFireModeChange" />
          <span class="fire-label">🔥 Fire Mode</span>
          <el-tooltip v-if="fireMode" content="Fire mode enabled: all elevators disabled, stairs only" placement="bottom">
            <el-tag type="danger" size="small" effect="dark">Elevators Disabled</el-tag>
          </el-tooltip>
        </div>
        <el-select v-model="startNode" size="small" placeholder="Select Start Point" style="width:200px" filterable>
          <el-option v-for="n in availableStartNodes" :key="n.nodeKey || n.node_key" :label="`${n.name} (${n.nodeKey || n.node_key})`" :value="n.nodeKey || n.node_key" />
        </el-select>
        <el-button type="danger" size="small" :loading="computing" @click="onCalculate">{{ fireMode ? '🔥 Calculate Escape Path' : 'Calculate Path' }}</el-button>
        <el-button size="small" @click="onClearPath">Clear Path</el-button>
        <el-tag v-if="lastResult" type="success" size="small">Computed in {{ lastResult.computeTimeMs || lastResult.compute_time_ms }}ms · {{ lastResult.paths.length }} paths</el-tag>
      </div>
      <!-- 路径结果摘要 -->
      <div v-if="lastResult && lastResult.paths.length" class="path-results">
        <div v-for="(p, i) in lastResult.paths" :key="i" class="path-item">
          <el-tag size="small" :type="i === 0 ? 'danger' : 'info'">{{ i === 0 ? 'Optimal' : `#${i+1}` }}</el-tag>
          <span class="path-exit">→ {{ p.exitName || p.exit_name }}</span>
          <span class="path-cost">Cost {{ p.totalCost || p.total_cost }}</span>
          <span class="path-time">≈ {{ p.estimatedTime || p.estimated_time }}</span>
        </div>
      </div>
    </div>

    <!-- 模拟控制工具栏 -->
    <div class="panel simulation-toolbar">
      <div class="toolbar-row">
        <span class="sim-title">Simulation</span>
        <el-select v-model="simulation.smokeOrigin" size="small" placeholder="Smoke Origin" style="width:200px" clearable filterable>
          <el-option v-for="n in smokeOriginOptions" :key="n.nodeKey || n.node_key" :label="`${n.name}`" :value="n.nodeKey || n.node_key" />
        </el-select>
        <el-button type="success" size="small" :disabled="simulation.active" @click="startSimulation">
          ▶ Start
        </el-button>
        <el-button size="small" :disabled="!simulation.active" @click="pauseSimulation">
          {{ simulation.paused ? '▶ Resume' : '⏸ Pause' }}
        </el-button>
        <el-button size="small" @click="resetSimulation">↻ Reset</el-button>
        <div class="sim-speed">
          <span class="sim-speed-label">Speed</span>
          <el-slider v-model="simulation.speed" :min="0.5" :max="3" :step="0.1" size="small" style="width:100px" />
          <span class="sim-speed-val">{{ simulation.speed.toFixed(1) }}x</span>
        </div>
      </div>
    </div>

    <div class="panel floor-panel">
      <div class="panel-header">
        <span class="panel-title">Floor Browser</span>
        <div class="panel-actions">
          <el-tag :type="graphLoaded ? 'success' : 'info'">{{ graphLoaded ? `Loaded ${graphNodes.length} nodes` : 'Loading...' }}</el-tag>
          <el-tag type="warning">Current: {{ activeFloor.name }}</el-tag>
        </div>
      </div>

      <!-- ═══ 3D 视图 + 控制面板 ═══ -->
      <div v-show="viewMode === '3d'" class="three-container">
        <div ref="canvasWrap" class="three-wrap" />
        <!-- 相机控制面板 -->
        <div class="cam-panel">
          <div class="cam-section">
            <div class="cam-title">View Presets</div>
            <div class="cam-presets">
              <el-button size="small" @click="setCamPreset('perspective')">Perspective</el-button>
              <el-button size="small" @click="setCamPreset('top')">Top</el-button>
              <el-button size="small" @click="setCamPreset('front')">Front</el-button>
              <el-button size="small" @click="setCamPreset('side')">Side</el-button>
              <el-button size="small" @click="setCamPreset('fit')">Fit</el-button>
            </div>
          </div>
          <div class="cam-section">
            <div class="cam-title">Position (X / Y / Z)</div>
            <div class="cam-row">
              <span class="cam-axis x">X</span>
              <el-slider v-model="camPos.x" :min="-60" :max="60" :step="0.5" size="small" @input="applyCamPos" />
              <span class="cam-val">{{ camPos.x.toFixed(1) }}</span>
            </div>
            <div class="cam-row">
              <span class="cam-axis y">Y</span>
              <el-slider v-model="camPos.y" :min="0" :max="80" :step="0.5" size="small" @input="applyCamPos" />
              <span class="cam-val">{{ camPos.y.toFixed(1) }}</span>
            </div>
            <div class="cam-row">
              <span class="cam-axis z">Z</span>
              <el-slider v-model="camPos.z" :min="-60" :max="60" :step="0.5" size="small" @input="applyCamPos" />
              <span class="cam-val">{{ camPos.z.toFixed(1) }}</span>
            </div>
          </div>
          <div class="cam-section">
            <div class="cam-title">Target Focus</div>
            <div class="cam-row">
              <span class="cam-axis x">X</span>
              <el-slider v-model="camTarget.x" :min="-30" :max="30" :step="0.5" size="small" @input="applyCamTarget" />
              <span class="cam-val">{{ camTarget.x.toFixed(1) }}</span>
            </div>
            <div class="cam-row">
              <span class="cam-axis y">Y</span>
              <el-slider v-model="camTarget.y" :min="-5" :max="40" :step="0.5" size="small" @input="applyCamTarget" />
              <span class="cam-val">{{ camTarget.y.toFixed(1) }}</span>
            </div>
            <div class="cam-row">
              <span class="cam-axis z">Z</span>
              <el-slider v-model="camTarget.z" :min="-30" :max="30" :step="0.5" size="small" @input="applyCamTarget" />
              <span class="cam-val">{{ camTarget.z.toFixed(1) }}</span>
            </div>
          </div>
          <div class="cam-hint">Left-click rotate · Right-click pan · Scroll zoom<br/>WASD / Arrow keys to move camera</div>
        </div>
      </div>

      <!-- ═══ 2D 俯视图 ═══ -->
      <div v-show="viewMode === '2d'" class="plan2d-container">
        <div class="plan2d-floor-tabs">
          <el-radio-group v-model="activeFloorId" size="small">
            <el-radio-button v-for="f in floorList" :key="f.id" :label="f.id">{{ f.name }}</el-radio-button>
          </el-radio-group>
        </div>
        <div class="plan2d-canvas-wrap">
          <canvas ref="plan2dCanvas" class="plan2d-canvas" />
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed, nextTick, onMounted, onUnmounted, reactive, ref, watch } from 'vue'
import * as THREE from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js'
import { ElMessage } from 'element-plus'
import { getGraphNodes, getGraphEdges, calculateEscape } from '@/api/escape'
import { MallBuilder } from '@/utils/mallBuilder'
import { PathRenderer } from '@/utils/pathRenderer'

const COLORS = {
  indicatorGreen: 0x22c55e,
  indicatorYellow: 0xfbbf24,
  indicatorRed: 0xef4444,
}

/* ═══ 基础状态 ═══ */
const viewMode = ref('3d')
const canvasWrap = ref()
const plan2dCanvas = ref()
const graphLoaded = ref(false)
const graphNodes = ref([])
const graphEdges = ref([])
const selectedAlgo = ref('aco')
const startNode = ref('')
const computing = ref(false)
const lastResult = ref(null)
const fireMode = ref(true)   // 火灾模式：默认开启，禁用电梯

/* ═══ 模拟状态 ═══ */
const simulation = reactive({
  active: false,
  paused: false,
  speed: 1.0,
  progress: 0,
  path: [],
  smokeNodes: [],
  smokeOrigin: '',
})
let personGroup = null
let smokeParticles = []
let simulationAnimId = null
const simulationClock = new THREE.Clock()

/* ═══ 起点选择器：火灾模式下过滤电梯节点 ═══ */
const availableStartNodes = computed(() => {
  if (!fireMode.value) return graphNodes.value
  return graphNodes.value.filter(n => (n.nodeType ?? n.node_type) !== 'elevator')
})

/* ═══ 相机控制面板状态 ═══ */
const camPos = reactive({ x: 25, y: 30, z: 35 })
const camTarget = reactive({ x: 0, y: 2, z: 0 })

/* ═══ 楼层 ═══ */
const floorList = computed(() => {
  const map = new Map()
  for (const n of graphNodes.value) {
    const fid = n.floorId ?? n.floor_id
    const ntype = n.nodeType ?? n.node_type
    if (!map.has(fid)) map.set(fid, { id: fid, name: `${fid}F`, rooms: [] })
    if (ntype === 'room') map.get(fid).rooms.push(n)
  }
  return [...map.values()].sort((a, b) => a.id - b.id)
})

const activeFloor = ref({ id: 1, name: '1F', rooms: [] })
const activeFloorId = ref(1)

watch(activeFloorId, id => {
  const f = floorList.value.find(f => f.id === id)
  if (f) selectFloor(f)
  drawPlan2d()
})

/* ═══ Three.js 实例 ═══ */
let scene, camera, renderer, controls, animationId
let mallBuilder = null
let pathRenderer = null
const keysPressed = new Set()

function selectFloor(floor) {
  activeFloor.value = floor
  activeFloorId.value = floor.id
  if (mallBuilder) mallBuilder.highlightFloor(floor.id)
}

/* ═══ 灯光 ═══ */
function addLights() {
  scene.add(new THREE.HemisphereLight(0x8ecae6, 0x3b4252, 0.6))
  const dl = new THREE.DirectionalLight(0xffffff, 1.6)
  dl.position.set(18, 30, 22)
  dl.castShadow = true
  dl.shadow.mapSize.set(1024, 1024)
  dl.shadow.camera.left = -30
  dl.shadow.camera.right = 30
  dl.shadow.camera.top = 30
  dl.shadow.camera.bottom = -30
  scene.add(dl)
  const fill = new THREE.DirectionalLight(0xb0c4de, 0.5)
  fill.position.set(-10, 15, -10)
  scene.add(fill)
  scene.add(new THREE.AmbientLight(0xffffff, 0.4))
  scene.add(new THREE.GridHelper(70, 70, 0x334155, 0x1e293b))
}

/* ═══ 相机适配 ═══ */
function fitCamera() {
  if (!scene || !controls || !camera) return
  const box = new THREE.Box3()
  scene.traverse(child => {
    if (child.isMesh || child.isLine) box.expandByObject(child)
  })
  if (box.isEmpty()) return
  const size = box.getSize(new THREE.Vector3()).length()
  const center = box.getCenter(new THREE.Vector3())
  controls.target.copy(center)
  camera.position.set(center.x + size * 0.5, center.y + size * 0.4, center.z + size * 0.7)
  camera.lookAt(center)
  controls.update()
  syncCamUI()
}

/* ═══ 相机 ↔ 面板 同步 ═══ */
function syncCamUI() {
  if (!camera || !controls) return
  camPos.x = Math.round(camera.position.x * 2) / 2
  camPos.y = Math.round(camera.position.y * 2) / 2
  camPos.z = Math.round(camera.position.z * 2) / 2
  camTarget.x = Math.round(controls.target.x * 2) / 2
  camTarget.y = Math.round(controls.target.y * 2) / 2
  camTarget.z = Math.round(controls.target.z * 2) / 2
}

function applyCamPos() {
  if (!camera || !controls) return
  camera.position.set(camPos.x, camPos.y, camPos.z)
  controls.update()
}

function applyCamTarget() {
  if (!controls) return
  controls.target.set(camTarget.x, camTarget.y, camTarget.z)
  controls.update()
}

/* ═══ 视角预设 ═══ */
function setCamPreset(preset) {
  if (!camera || !controls) return
  const box = new THREE.Box3()
  scene.traverse(c => { if (c.isMesh || c.isLine) box.expandByObject(c) })
  const center = box.isEmpty() ? new THREE.Vector3() : box.getCenter(new THREE.Vector3())
  const size = box.isEmpty() ? 30 : box.getSize(new THREE.Vector3()).length()

  switch (preset) {
    case 'top':
      camera.position.set(center.x, center.y + size * 0.9, center.z + 0.01)
      controls.target.copy(center)
      break
    case 'front':
      camera.position.set(center.x, center.y + size * 0.2, center.z + size * 0.8)
      controls.target.copy(center)
      break
    case 'side':
      camera.position.set(center.x + size * 0.8, center.y + size * 0.2, center.z)
      controls.target.copy(center)
      break
    case 'perspective':
      camera.position.set(center.x + size * 0.4, center.y + size * 0.35, center.z + size * 0.6)
      controls.target.copy(center)
      break
    case 'fit':
      fitCamera()
      return
  }
  camera.lookAt(controls.target)
  controls.update()
  syncCamUI()
}

/* ═══ 键盘移动 ═══ */
function onKeyDown(e) {
  keysPressed.add(e.code)
}
function onKeyUp(e) {
  keysPressed.delete(e.code)
}
function processKeys() {
  if (!camera || !controls || keysPressed.size === 0) return
  const speed = 0.4
  const forward = new THREE.Vector3()
  camera.getWorldDirection(forward)
  forward.y = 0
  forward.normalize()
  const right = new THREE.Vector3().crossVectors(forward, camera.up).normalize()

  const move = new THREE.Vector3()
  if (keysPressed.has('KeyW') || keysPressed.has('ArrowUp'))    move.add(forward)
  if (keysPressed.has('KeyS') || keysPressed.has('ArrowDown'))  move.sub(forward)
  if (keysPressed.has('KeyA') || keysPressed.has('ArrowLeft'))  move.sub(right)
  if (keysPressed.has('KeyD') || keysPressed.has('ArrowRight')) move.add(right)
  if (keysPressed.has('KeyQ') || keysPressed.has('Space'))      move.y += 1
  if (keysPressed.has('KeyE') || keysPressed.has('ShiftLeft'))  move.y -= 1

  if (move.lengthSq() > 0) {
    move.normalize().multiplyScalar(speed)
    camera.position.add(move)
    controls.target.add(move)
    controls.update()
    syncCamUI()
  }
}

/* ═══ 数据加载 ═══ */
async function loadGraphData() {
  try {
    const [nodesRes, edgesRes] = await Promise.all([getGraphNodes(), getGraphEdges()])
    graphNodes.value = Array.isArray(nodesRes) ? nodesRes : (nodesRes?.list || nodesRes?.data?.list || [])
    graphEdges.value = Array.isArray(edgesRes) ? edgesRes : (edgesRes?.list || edgesRes?.data?.list || [])
    graphLoaded.value = true

    if (graphNodes.value.length && scene) {
      mallBuilder = new MallBuilder(scene)
      mallBuilder.build(graphNodes.value, graphEdges.value)
      mallBuilder.setFireMode(fireMode.value)
      fitCamera()

      if (floorList.value.length) {
        activeFloor.value = floorList.value[0]
        activeFloorId.value = floorList.value[0].id
      }
      if (graphNodes.value.length && !startNode.value) {
        startNode.value = graphNodes.value[0].nodeKey || graphNodes.value[0].node_key
      }
      drawPlan2d()
    }
  } catch (e) {
    console.error('Failed to load graph data', e)
    ElMessage.warning('Failed to load building graph data, please check backend service')
  }
}

/* ═══ 路径计算 ═══ */
async function onCalculate() {
  if (!startNode.value) { ElMessage.warning('Please select a start point'); return }
  computing.value = true
  lastResult.value = null
  try {
    const res = await calculateEscape({
      algorithm: selectedAlgo.value,
      startNode: startNode.value,
      floorId: activeFloor.value.id,
      fireMode: fireMode.value,
      smokeOrigin: simulation.smokeOrigin || undefined,
    })
    lastResult.value = res?.paths ? res : (res?.data || res)
    if (mallBuilder && lastResult.value) {
      if (!pathRenderer) pathRenderer = new PathRenderer(scene, mallBuilder.getNodePositions())
      pathRenderer.render(lastResult.value)
    }
    drawPlan2d()
  } catch (e) {
    console.error('路径计算失败', e)
    ElMessage.error('Escape path calculation failed: ' + (e.response?.data?.message || e.message))
  } finally {
    computing.value = false
  }
}

function onFireModeChange(val) {
  if (mallBuilder) mallBuilder.setFireMode(val)

  // 如果当前起点是电梯节点，切换到火灾模式时清空起点
  if (val && startNode.value) {
    const currentNode = graphNodes.value.find(n => (n.nodeKey ?? n.node_key) === startNode.value)
    if (currentNode && (currentNode.nodeType ?? currentNode.node_type) === 'elevator') {
      startNode.value = ''
    }
  }

  // 清除旧路径（旧路径可能包含电梯节点，不再有效）
  if (pathRenderer) pathRenderer.clear()
  const hadResult = !!lastResult.value
  lastResult.value = null

  // 如果之前已有计算结果，自动重新计算
  if (hadResult && startNode.value) {
    onCalculate()
  }

  drawPlan2d()
}

function onClearPath() {
  if (pathRenderer) pathRenderer.clear()
  lastResult.value = null
  stopSimulation()
  drawPlan2d()
}

/* ═══ 模拟控制 ═══ */
const smokeOriginOptions = computed(() => {
  return graphNodes.value.filter(n => (n.nodeType ?? n.node_type) !== 'exit')
})

// 当起火点变化时，自动重新计算路径
watch(() => simulation.smokeOrigin, (newOrigin) => {
  if (newOrigin && startNode.value) {
    onCalculate()
  }
})

function createPerson() {
  if (personGroup) {
    scene.remove(personGroup)
    personGroup = null
  }

  personGroup = new THREE.Group()
  personGroup.name = 'escape-person'

  // 身体
  const body = new THREE.Mesh(
    new THREE.CylinderGeometry(0.12, 0.15, 0.35, 8),
    new THREE.MeshStandardMaterial({ color: 0x3b82f6, roughness: 0.6 })
  )
  body.position.y = 0.35
  body.name = 'body'
  personGroup.add(body)

  // 头部
  const head = new THREE.Mesh(
    new THREE.SphereGeometry(0.1, 12, 12),
    new THREE.MeshStandardMaterial({ color: 0xfbbf24, roughness: 0.5 })
  )
  head.position.y = 0.62
  head.name = 'head'
  personGroup.add(head)

  // 左腿
  const leftLeg = new THREE.Mesh(
    new THREE.CylinderGeometry(0.05, 0.05, 0.3, 6),
    new THREE.MeshStandardMaterial({ color: 0x1e293b, roughness: 0.7 })
  )
  leftLeg.position.set(-0.06, 0.15, 0)
  leftLeg.name = 'leftLeg'
  personGroup.add(leftLeg)

  // 右腿
  const rightLeg = new THREE.Mesh(
    new THREE.CylinderGeometry(0.05, 0.05, 0.3, 6),
    new THREE.MeshStandardMaterial({ color: 0x1e293b, roughness: 0.7 })
  )
  rightLeg.position.set(0.06, 0.15, 0)
  rightLeg.name = 'rightLeg'
  personGroup.add(rightLeg)

  // 左臂
  const leftArm = new THREE.Mesh(
    new THREE.CylinderGeometry(0.035, 0.035, 0.25, 6),
    new THREE.MeshStandardMaterial({ color: 0x3b82f6, roughness: 0.6 })
  )
  leftArm.position.set(-0.2, 0.38, 0)
  leftArm.name = 'leftArm'
  personGroup.add(leftArm)

  // 右臂
  const rightArm = new THREE.Mesh(
    new THREE.CylinderGeometry(0.035, 0.035, 0.25, 6),
    new THREE.MeshStandardMaterial({ color: 0x3b82f6, roughness: 0.6 })
  )
  rightArm.position.set(0.2, 0.38, 0)
  rightArm.name = 'rightArm'
  personGroup.add(rightArm)

  personGroup.visible = false
  scene.add(personGroup)
}

function createSmoke(originNode) {
  clearSmoke()
  if (!originNode) return

  const pos = mallBuilder.nodePositions.get(originNode.nodeKey || originNode.node_key)
  if (!pos) return

  // 在起火点及相邻节点生成烟雾
  const originKey = originNode.nodeKey || originNode.node_key
  const neighbors = mallBuilder._edgeIndex.get(originKey) || []
  const smokePositions = [pos.clone()]

  for (const nk of neighbors) {
    const nPos = mallBuilder.nodePositions.get(nk)
    if (nPos) smokePositions.push(nPos.clone())
  }

  for (const sp of smokePositions) {
    const count = 15
    const particles = []
    for (let i = 0; i < count; i++) {
      const size = 0.2 + Math.random() * 0.4
      const particle = new THREE.Mesh(
        new THREE.SphereGeometry(size, 8, 8),
        new THREE.MeshStandardMaterial({
          color: 0x6b7280,
          transparent: true,
          opacity: 0.3 + Math.random() * 0.3,
          roughness: 1,
        })
      )
      particle.position.set(
        sp.x + (Math.random() - 0.5) * 2,
        sp.y + Math.random() * 1.5,
        sp.z + (Math.random() - 0.5) * 2
      )
      particle.userData = {
        baseY: particle.position.y,
        speed: 0.3 + Math.random() * 0.5,
        drift: (Math.random() - 0.5) * 0.02,
        phase: Math.random() * Math.PI * 2,
      }
      scene.add(particle)
      particles.push(particle)
    }
    smokeParticles.push(...particles)
  }
}

function clearSmoke() {
  for (const p of smokeParticles) {
    scene.remove(p)
    if (p.geometry) p.geometry.dispose()
    if (p.material) p.material.dispose()
  }
  smokeParticles = []
}

function startSimulation() {
  if (!lastResult.value || !lastResult.value.paths.length) {
    ElMessage.warning('Please calculate escape path first')
    return
  }

  const optimalPath = lastResult.value.paths[0]
  const nodeKeys = optimalPath.nodes || []
  const points = nodeKeys.map(k => mallBuilder.nodePositions.get(k)).filter(Boolean)
  if (points.length < 2) return

  simulation.path = points
  simulation.progress = 0
  simulation.active = true
  simulation.paused = false

  createPerson()
  personGroup.visible = true

  // 创建烟雾
  if (simulation.smokeOrigin) {
    const originNode = graphNodes.value.find(n =>
      (n.nodeKey ?? n.node_key) === simulation.smokeOrigin
    )
    if (originNode) createSmoke(originNode)
  }

  // 开始模拟动画
  simulationClock.start()
  animateSimulation()
}

function animateSimulation() {
  if (!simulation.active || simulation.paused) return

  simulationAnimId = requestAnimationFrame(animateSimulation)

  const delta = simulationClock.getDelta()
  const speed = simulation.speed
  const pathLength = simulation.path.length - 1

  simulation.progress += delta * speed * 0.15
  if (simulation.progress >= pathLength) {
    simulation.progress = pathLength
    simulation.active = false
  }

  // 计算当前位置
  const idx = Math.floor(simulation.progress)
  const t = simulation.progress - idx
  const p1 = simulation.path[Math.min(idx, pathLength)]
  const p2 = simulation.path[Math.min(idx + 1, pathLength)]

  if (p1 && p2 && personGroup) {
    personGroup.position.lerpVectors(p1, p2, t)
    personGroup.position.y = p1.y + 0.01

    // 面向移动方向
    const dir = new THREE.Vector3().subVectors(p2, p1).normalize()
    const angle = Math.atan2(dir.x, dir.z)
    personGroup.rotation.y = angle

    // 行走动画（腿部摆动）
    const walkTime = simulationClock.getElapsedTime() * 8 * speed
    const leftLeg = personGroup.getObjectByName('leftLeg')
    const rightLeg = personGroup.getObjectByName('rightLeg')
    const leftArm = personGroup.getObjectByName('leftArm')
    const rightArm = personGroup.getObjectByName('rightArm')

    if (leftLeg) leftLeg.rotation.x = Math.sin(walkTime) * 0.4
    if (rightLeg) rightLeg.rotation.x = -Math.sin(walkTime) * 0.4
    if (leftArm) leftArm.rotation.x = -Math.sin(walkTime) * 0.3
    if (rightArm) rightArm.rotation.x = Math.sin(walkTime) * 0.3
  }

  // 更新烟雾动画
  for (const p of smokeParticles) {
    const ud = p.userData
    p.position.y = ud.baseY + Math.sin(simulationClock.getElapsedTime() * ud.speed + ud.phase) * 0.3
    p.position.x += ud.drift
    p.material.opacity = 0.2 + Math.sin(simulationClock.getElapsedTime() * 0.5 + ud.phase) * 0.15
  }

  // 更新指示灯颜色（沿路径的灯变绿，其他变黄）
  updateIndicatorLightsForSimulation()

  // 自动切换楼层
  if (personGroup && personGroup.position.y !== undefined) {
    const currentFloor = Math.round(personGroup.position.y / 4.5) + 1
    if (currentFloor >= 1 && currentFloor <= 6 && currentFloor !== activeFloorId.value) {
      activeFloorId.value = currentFloor
    }
  }
}

function updateIndicatorLightsForSimulation() {
  if (!personGroup || !mallBuilder) return

  const personPos = personGroup.position
  const allIndicators = mallBuilder.getIndicatorLights()

  for (const [floorId, lights] of allIndicators) {
    const states = []
    lights.forEach((ind, idx) => {
      const dist = ind.position.distanceTo(personPos)
      let color = COLORS.indicatorYellow

      // 路径附近的灯变绿
      if (simulation.path.some(p => p.distanceTo(ind.position) < 3)) {
        color = COLORS.indicatorGreen
      }
      // 靠近烟雾的灯变红
      if (simulation.smokeNodes.some(sn => {
        const snPos = mallBuilder.nodePositions.get(sn)
        return snPos && snPos.distanceTo(ind.position) < 4
      })) {
        color = COLORS.indicatorRed
      }

      states.push({ index: idx, color })
    })
    mallBuilder.setIndicatorColors(new Map([[floorId, states]]))
  }
}

function pauseSimulation() {
  simulation.paused = !simulation.paused
  if (!simulation.paused && simulation.active) {
    simulationClock.start()
    animateSimulation()
  }
}

function resetSimulation() {
  stopSimulation()
  simulation.progress = 0
  simulation.active = false
  simulation.paused = false
  if (personGroup) {
    personGroup.visible = false
  }
  mallBuilder?.resetIndicatorColors()
  clearSmoke()
}

function stopSimulation() {
  simulation.active = false
  simulation.paused = false
  if (simulationAnimId) {
    cancelAnimationFrame(simulationAnimId)
    simulationAnimId = null
  }
  if (personGroup) {
    scene.remove(personGroup)
    personGroup = null
  }
  mallBuilder?.resetIndicatorColors()
  clearSmoke()
}

/* ═══ Three.js 初始化 ═══ */
function initThree() {
  if (!canvasWrap.value) return
  scene = new THREE.Scene()
  scene.background = new THREE.Color('#0f172a')
  camera = new THREE.PerspectiveCamera(45, canvasWrap.value.clientWidth / 520, 0.1, 500)
  renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false })
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2))
  renderer.shadowMap.enabled = true
  renderer.shadowMap.type = THREE.PCFSoftShadowMap
  renderer.toneMapping = THREE.ACESFilmicToneMapping
  renderer.toneMappingExposure = 1.1
  renderer.setSize(canvasWrap.value.clientWidth, 520)
  canvasWrap.value.innerHTML = ''
  canvasWrap.value.appendChild(renderer.domElement)

  controls = new OrbitControls(camera, renderer.domElement)
  controls.enableDamping = true
  controls.dampingFactor = 0.08
  controls.screenSpacePanning = true
  controls.addEventListener('change', syncCamUI)

  addLights()
  camera.position.set(25, 30, 35)
  controls.update()
  syncCamUI()
  onResize()
  animate()
  loadGraphData()
}

function animate() {
  animationId = requestAnimationFrame(animate)
  processKeys()
  controls?.update()
  renderer?.render(scene, camera)
}

function onResize() {
  if (!renderer || !camera || !canvasWrap.value) return
  const width = canvasWrap.value.clientWidth
  camera.aspect = width / 520
  camera.updateProjectionMatrix()
  renderer.setSize(width, 520)
}

/* ═══════════════════════════════════════════════════
   2D 俯视图 Canvas 绘制
   ═══════════════════════════════════════════════════ */
function drawPlan2d() {
  const canvas = plan2dCanvas.value
  if (!canvas) return
  const wrap = canvas.parentElement
  if (!wrap) return
  const dpr = window.devicePixelRatio || 1
  const W = wrap.clientWidth
  const H = wrap.clientHeight || 520
  canvas.width = W * dpr
  canvas.height = H * dpr
  canvas.style.width = W + 'px'
  canvas.style.height = H + 'px'
  const ctx = canvas.getContext('2d')
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
  ctx.clearRect(0, 0, W, H)

  // 背景
  ctx.fillStyle = '#0f172a'
  ctx.fillRect(0, 0, W, H)

  const fid = activeFloorId.value
  const nodes = graphNodes.value.filter(n => (n.floorId ?? n.floor_id) === fid)
  const edges = graphEdges.value.filter(e => {
    const fk = e.fromNodeKey ?? e.from_node_key
    const tk = e.toNodeKey ?? e.to_node_key
    return nodes.some(n => (n.nodeKey ?? n.node_key) === fk) &&
           nodes.some(n => (n.nodeKey ?? n.node_key) === tk)
  })

  if (!nodes.length) {
    ctx.fillStyle = '#94a3b8'
    ctx.font = '16px sans-serif'
    ctx.textAlign = 'center'
    ctx.fillText('No node data for this floor', W / 2, H / 2)
    return
  }

  // 计算坐标范围，映射到 canvas
  let minX = Infinity, maxX = -Infinity, minZ = Infinity, maxZ = -Infinity
  for (const n of nodes) {
    const nx = Number(n.x) || 0, nz = Number(n.z) || 0
    minX = Math.min(minX, nx); maxX = Math.max(maxX, nx)
    minZ = Math.min(minZ, nz); maxZ = Math.max(maxZ, nz)
  }
  const pad = 5
  minX -= pad; maxX += pad; minZ -= pad; maxZ += pad
  const rangeX = maxX - minX || 1
  const rangeZ = maxZ - minZ || 1
  const margin = 60
  const scaleX = (W - margin * 2) / rangeX
  const scaleZ = (H - margin * 2) / rangeZ
  const scale = Math.min(scaleX, scaleZ)
  const offX = margin + ((W - margin * 2) - rangeX * scale) / 2
  const offZ = margin + ((H - margin * 2) - rangeZ * scale) / 2

  const toCanvas = (wx, wz) => [offX + (wx - minX) * scale, offZ + (wz - minZ) * scale]
  const nodeMap = new Map()
  for (const n of nodes) nodeMap.set(n.nodeKey ?? n.node_key, n)

  // ── 网格 ──
  ctx.strokeStyle = '#1e293b'
  ctx.lineWidth = 0.5
  const gridStep = 2
  for (let gx = Math.ceil(minX); gx <= maxX; gx += gridStep) {
    const [sx] = toCanvas(gx, 0)
    ctx.beginPath(); ctx.moveTo(sx, margin); ctx.lineTo(sx, H - margin); ctx.stroke()
  }
  for (let gz = Math.ceil(minZ); gz <= maxZ; gz += gridStep) {
    const [, sy] = toCanvas(0, gz)
    ctx.beginPath(); ctx.moveTo(margin, sy); ctx.lineTo(W - margin, sy); ctx.stroke()
  }

  // ── 边/通道 ──
  for (const e of edges) {
    const fn = nodeMap.get(e.fromNodeKey ?? e.from_node_key)
    const tn = nodeMap.get(e.toNodeKey ?? e.to_node_key)
    if (!fn || !tn) continue
    const [x1, y1] = toCanvas(Number(fn.x), Number(fn.z))
    const [x2, y2] = toCanvas(Number(tn.x), Number(tn.z))

    // 走廊宽度条带
    const ew = Math.min(Number(e.width) || 2, 4) * scale
    ctx.strokeStyle = 'rgba(67,76,94,0.5)'
    ctx.lineWidth = ew
    ctx.lineCap = 'round'
    ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke()

    // 中心线
    ctx.strokeStyle = 'rgba(94,129,172,0.6)'
    ctx.lineWidth = 1
    ctx.setLineDash([6, 4])
    ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke()
    ctx.setLineDash([])
  }

  // ── 节点 ──
  const typeColors = {
    room: { fill: 'rgba(96,165,250,0.25)', stroke: '#60a5fa', text: '#e0e7ff' },
    corridor: { fill: 'rgba(148,163,184,0.15)', stroke: '#64748b', text: '#94a3b8' },
    exit: { fill: 'rgba(34,197,94,0.3)', stroke: '#22c55e', text: '#bbf7d0' },
    stair: { fill: 'rgba(251,146,60,0.3)', stroke: '#fb923c', text: '#fed7aa' },
    elevator: { fill: 'rgba(167,139,250,0.3)', stroke: '#a78bfa', text: '#ddd6fe' },
    fire_hydrant: { fill: 'rgba(220,38,38,0.3)', stroke: '#dc2626', text: '#fecaca' },
    monitor_room: { fill: 'rgba(14,165,233,0.3)', stroke: '#0ea5e9', text: '#bae6fd' },
    electrical_room: { fill: 'rgba(234,179,8,0.3)', stroke: '#eab308', text: '#fef08a' },
  }
  const roomW = 5 * scale, roomD = 4 * scale

  for (const n of nodes) {
    const ntype = n.nodeType ?? n.node_type
    const nkey = n.nodeKey ?? n.node_key
    const nx = Number(n.x) || 0, nz = Number(n.z) || 0
    const [cx, cy] = toCanvas(nx, nz)
    const c = typeColors[ntype] || typeColors.room

    if (ntype === 'room') {
      ctx.fillStyle = c.fill
      ctx.strokeStyle = c.stroke
      ctx.lineWidth = 2
      ctx.beginPath()
      ctx.roundRect(cx - roomW / 2, cy - roomD / 2, roomW, roomD, 4)
      ctx.fill(); ctx.stroke()

      // 门洞标记
      const neighbors = graphEdges.value.filter(e =>
        (e.fromNodeKey ?? e.from_node_key) === nkey || (e.toNodeKey ?? e.to_node_key) === nkey
      )
      for (const e of neighbors) {
        const otherKey = (e.fromNodeKey ?? e.from_node_key) === nkey
          ? (e.toNodeKey ?? e.to_node_key) : (e.fromNodeKey ?? e.from_node_key)
        const other = nodeMap.get(otherKey)
        if (!other) continue
        const dx = Number(other.x) - nx, dz = Number(other.z) - nz
        ctx.fillStyle = '#88c0d0'
        const doorLen = 0.8 * scale
        if (Math.abs(dx) > Math.abs(dz)) {
          const wx = dx > 0 ? cx + roomW / 2 : cx - roomW / 2
          ctx.fillRect(wx - 2, cy - doorLen / 2, 4, doorLen)
        } else {
          const wz = dz > 0 ? cy + roomD / 2 : cy - roomD / 2
          ctx.fillRect(cx - doorLen / 2, wz - 2, doorLen, 4)
        }
      }
    } else if (ntype === 'exit') {
      // 绿色三角
      ctx.fillStyle = c.fill
      ctx.strokeStyle = c.stroke
      ctx.lineWidth = 2.5
      const s = 10
      ctx.beginPath()
      ctx.moveTo(cx, cy - s); ctx.lineTo(cx + s, cy + s); ctx.lineTo(cx - s, cy + s)
      ctx.closePath(); ctx.fill(); ctx.stroke()
    } else if (ntype === 'stair') {
      // 台阶图标
      const sw = 12, sh = 14
      ctx.strokeStyle = c.stroke; ctx.lineWidth = 1.5
      for (let i = 0; i < 4; i++) {
        const sy = cy - sh / 2 + i * (sh / 4)
        ctx.beginPath(); ctx.moveTo(cx - sw / 2, sy); ctx.lineTo(cx + sw / 2, sy); ctx.stroke()
      }
      ctx.strokeStyle = c.stroke; ctx.lineWidth = 2
      ctx.strokeRect(cx - sw / 2, cy - sh / 2, sw, sh)
    } else if (ntype === 'elevator') {
      const isDisabled = fireMode.value
      ctx.fillStyle = isDisabled ? 'rgba(153,27,27,0.3)' : c.fill
      ctx.strokeStyle = isDisabled ? '#991b1b' : c.stroke
      ctx.lineWidth = 2
      ctx.beginPath(); ctx.arc(cx, cy, 10, 0, Math.PI * 2); ctx.fill(); ctx.stroke()
      ctx.fillStyle = isDisabled ? '#ef4444' : c.stroke
      ctx.font = 'bold 10px sans-serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
      ctx.fillText('E', cx, cy)
      if (isDisabled) {
        // 红色 X 禁用标记
        ctx.strokeStyle = '#ef4444'; ctx.lineWidth = 2.5; ctx.lineCap = 'round'
        ctx.beginPath(); ctx.moveTo(cx - 7, cy - 7); ctx.lineTo(cx + 7, cy + 7); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx + 7, cy - 7); ctx.lineTo(cx - 7, cy + 7); ctx.stroke()
      }
    } else if (ntype === 'fire_hydrant') {
      // 消防栓：红色菱形
      ctx.fillStyle = c.fill
      ctx.strokeStyle = c.stroke
      ctx.lineWidth = 2
      ctx.beginPath()
      ctx.moveTo(cx, cy - 8); ctx.lineTo(cx + 8, cy); ctx.lineTo(cx, cy + 8); ctx.lineTo(cx - 8, cy)
      ctx.closePath(); ctx.fill(); ctx.stroke()
      ctx.fillStyle = c.stroke
      ctx.font = 'bold 9px sans-serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
      ctx.fillText('F', cx, cy)
    } else if (ntype === 'monitor_room') {
      // 监控室：蓝色圆角矩形 + 屏幕图标
      ctx.fillStyle = c.fill
      ctx.strokeStyle = c.stroke
      ctx.lineWidth = 2
      ctx.beginPath()
      ctx.roundRect(cx - 12, cy - 10, 24, 20, 4)
      ctx.fill(); ctx.stroke()
      // 小屏幕
      ctx.fillStyle = '#22d3ee'
      ctx.fillRect(cx - 6, cy - 5, 12, 8)
    } else if (ntype === 'electrical_room') {
      // 配电间：黄色三角警示
      ctx.fillStyle = c.fill
      ctx.strokeStyle = c.stroke
      ctx.lineWidth = 2
      ctx.beginPath()
      ctx.moveTo(cx, cy - 10); ctx.lineTo(cx + 10, cy + 7); ctx.lineTo(cx - 10, cy + 7)
      ctx.closePath(); ctx.fill(); ctx.stroke()
      ctx.fillStyle = c.stroke
      ctx.font = 'bold 9px sans-serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
      ctx.fillText('⚡', cx, cy + 1)
    } else {
      // corridor dot
      ctx.fillStyle = c.stroke
      ctx.beginPath(); ctx.arc(cx, cy, 4, 0, Math.PI * 2); ctx.fill()
    }

    // 标签
    ctx.fillStyle = c.text
    ctx.font = '12px "Microsoft YaHei", sans-serif'
    ctx.textAlign = 'center'
    ctx.textBaseline = 'top'
    const labelY = ntype === 'room' ? cy + roomD / 2 + 4 : cy + 14
    ctx.fillText(n.name, cx, labelY)
  }

  // ── 路径叠加 ──
  if (lastResult.value && lastResult.value.paths) {
    const algoColors = { aco: '#ef4444', pso: '#3b82f6', sa: '#f59e0b', ga: '#8b5cf6' }
    const algo = lastResult.value.algorithm
    lastResult.value.paths.forEach((p, pi) => {
      const pNodes = p.nodes || []
      const pts = pNodes.map(k => {
        const nd = nodeMap.get(k)
        return nd ? toCanvas(Number(nd.x), Number(nd.z)) : null
      }).filter(Boolean)
      if (pts.length < 2) return

      const col = pi === 0 ? (algoColors[algo] || '#ef4444') : '#94a3b8'
      ctx.strokeStyle = col
      ctx.lineWidth = pi === 0 ? 4 : 2
      ctx.lineCap = 'round'; ctx.lineJoin = 'round'
      ctx.setLineDash([])
      ctx.globalAlpha = pi === 0 ? 0.9 : 0.5
      ctx.beginPath()
      ctx.moveTo(pts[0][0], pts[0][1])
      for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1])
      ctx.stroke()

      // 箭头
      if (pi === 0) {
        for (let i = 0; i < pts.length - 1; i++) {
          const [ax, ay] = pts[i], [bx, by] = pts[i + 1]
          const mx = (ax + bx) / 2, my = (ay + by) / 2
          const angle = Math.atan2(by - ay, bx - ax)
          const aLen = 8
          ctx.fillStyle = col
          ctx.beginPath()
          ctx.moveTo(mx + Math.cos(angle) * aLen, my + Math.sin(angle) * aLen)
          ctx.lineTo(mx + Math.cos(angle + 2.5) * aLen * 0.6, my + Math.sin(angle + 2.5) * aLen * 0.6)
          ctx.lineTo(mx + Math.cos(angle - 2.5) * aLen * 0.6, my + Math.sin(angle - 2.5) * aLen * 0.6)
          ctx.closePath(); ctx.fill()
        }
      }

      // 起终点
      ctx.globalAlpha = 1
      const [sx, sy] = pts[0], [ex, ey] = pts[pts.length - 1]
      ctx.fillStyle = '#22c55e'; ctx.beginPath(); ctx.arc(sx, sy, 6, 0, Math.PI * 2); ctx.fill()
      ctx.fillStyle = '#ef4444'; ctx.beginPath(); ctx.arc(ex, ey, 6, 0, Math.PI * 2); ctx.fill()
      ctx.fillStyle = '#fff'; ctx.font = 'bold 9px sans-serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
      ctx.fillText('S', sx, sy); ctx.fillText('E', ex, ey)
    })
    ctx.globalAlpha = 1
  }

  // ── 楼层标题 ──
  ctx.fillStyle = '#e2e8f0'
  ctx.font = 'bold 18px "Microsoft YaHei", sans-serif'
  ctx.textAlign = 'left'
  ctx.textBaseline = 'top'
  ctx.fillText(`${fid}F Floor Plan`, 16, 16)

  // ── 图例 ──
  const legends = [
    { label: 'Room', color: '#60a5fa' },
    { label: 'Corridor', color: '#64748b' },
    { label: 'Exit', color: '#22c55e' },
    { label: 'Stairs', color: '#fb923c' },
    { label: 'Elevator', color: '#a78bfa' },
    { label: 'Fire Hydrant', color: '#dc2626' },
    { label: 'Monitor Room', color: '#0ea5e9' },
    { label: 'Electrical Room', color: '#eab308' },
  ]
  const lx = W - 16
  ctx.textAlign = 'right'
  ctx.font = '12px sans-serif'
  legends.forEach((l, i) => {
    const ly = 18 + i * 20
    ctx.fillStyle = l.color
    ctx.fillRect(lx - 50, ly, 10, 10)
    ctx.fillStyle = '#cbd5e1'
    ctx.fillText(l.label, lx, ly + 9)
  })
}

/* ═══ 生命周期 ═══ */
watch(viewMode, async value => {
  if (value === '3d') { await nextTick(); onResize() }
  if (value === '2d') { await nextTick(); drawPlan2d() }
})

onMounted(() => {
  initThree()
  window.addEventListener('resize', onResize)
  window.addEventListener('keydown', onKeyDown)
  window.addEventListener('keyup', onKeyUp)
})
onUnmounted(() => {
  cancelAnimationFrame(animationId)
  stopSimulation()
  window.removeEventListener('resize', onResize)
  window.removeEventListener('keydown', onKeyDown)
  window.removeEventListener('keyup', onKeyUp)
  if (mallBuilder) mallBuilder.clear()
  if (pathRenderer) pathRenderer.clear()
  controls?.dispose()
  renderer?.dispose()
})
</script>

<style scoped>
.floor-page { gap: 16px; }
.toolbar, .panel-actions { display: flex; gap: 12px; align-items: center; }
.escape-toolbar { padding: 14px 18px; }
.simulation-toolbar { padding: 14px 18px; }
.sim-title { font-size: 14px; font-weight: 700; color: var(--el-text-color-primary); margin-right: 8px; }
.sim-speed { display: flex; align-items: center; gap: 6px; }
.sim-speed-label { font-size: 12px; color: var(--el-text-color-secondary); }
.sim-speed-val { font-size: 12px; color: var(--el-text-color-secondary); width: 30px; }
.fire-switch { display: flex; align-items: center; gap: 6px; padding: 4px 10px; border-radius: 8px; border: 1px solid transparent; transition: .2s; }
.fire-switch.active { background: rgba(239,68,68,0.08); border-color: rgba(239,68,68,0.25); }
.fire-label { font-size: 13px; font-weight: 600; white-space: nowrap; }
.toolbar-row { display: flex; gap: 10px; align-items: center; flex-wrap: wrap; }
.path-results { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 10px; }
.path-item { display: flex; align-items: center; gap: 8px; padding: 6px 12px; border-radius: 8px; background: var(--el-fill-color-light); font-size: 13px; }
.path-exit { font-weight: 600; }
.path-cost, .path-time { color: var(--el-text-color-secondary); }
.floor-panel { padding: 18px; }
.panel-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 14px; gap: 12px; }

/* ═══ 3D 容器 + 控制面板 ═══ */
.three-container { display: flex; gap: 14px; }
.three-wrap { flex: 1; min-width: 0; height: 520px; border-radius: 16px; overflow: hidden; background: #0f172a; }
.cam-panel {
  width: 240px; flex-shrink: 0;
  background: var(--el-bg-color); border: 1px solid var(--el-border-color);
  border-radius: 14px; padding: 14px; display: flex; flex-direction: column; gap: 12px;
  overflow-y: auto; max-height: 520px;
}
.cam-section { display: flex; flex-direction: column; gap: 6px; }
.cam-title { font-size: 12px; font-weight: 700; color: var(--el-text-color-secondary); text-transform: uppercase; letter-spacing: 0.5px; }
.cam-presets { display: flex; flex-wrap: wrap; gap: 6px; }
.cam-presets .el-button { flex: 1; min-width: 60px; }
.cam-row { display: flex; align-items: center; gap: 8px; }
.cam-row .el-slider { flex: 1; }
.cam-axis {
  width: 20px; height: 20px; border-radius: 4px; display: flex; align-items: center; justify-content: center;
  font-size: 11px; font-weight: 700; color: #fff; flex-shrink: 0;
}
.cam-axis.x { background: #ef4444; }
.cam-axis.y { background: #22c55e; }
.cam-axis.z { background: #3b82f6; }
.cam-val { font-size: 11px; color: var(--el-text-color-secondary); width: 36px; text-align: right; flex-shrink: 0; font-variant-numeric: tabular-nums; }
.cam-hint { font-size: 11px; color: var(--el-text-color-placeholder); line-height: 1.5; text-align: center; margin-top: auto; padding-top: 8px; border-top: 1px solid var(--el-border-color-lighter); }

/* ═══ 2D 俯视图 ═══ */
.plan2d-container { display: flex; flex-direction: column; gap: 10px; }
.plan2d-floor-tabs { display: flex; justify-content: center; }
.plan2d-canvas-wrap { width: 100%; height: 520px; border-radius: 16px; overflow: hidden; background: #0f172a; }
.plan2d-canvas { display: block; width: 100%; height: 100%; }

@media (max-width: 1100px) {
  .three-container { flex-direction: column; }
  .cam-panel { width: 100%; max-height: none; flex-direction: row; flex-wrap: wrap; }
  .cam-section { min-width: 200px; flex: 1; }
}
@media (max-width: 900px) { .panel-header, .toolbar, .toolbar-row { flex-wrap: wrap; } }
</style>
