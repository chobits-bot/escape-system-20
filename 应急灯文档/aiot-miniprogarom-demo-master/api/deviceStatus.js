// 设备状态(指示牌)相关API
const request = require('../utils/request.js')
const config = require('../config/env.js')

const baseUrl = `${config.deviceStatusApiBase}/signboard`

/**
 * 添加指示牌
 */
function addSignboard(data) {
  return request.post(baseUrl, data)
}

/**
 * 更新指示牌
 */
function updateSignboard(data) {
  return request.put(baseUrl, data)
}

/**
 * 删除指示牌
 */
function deleteSignboard(id) {
  return request.del(`${baseUrl}/${id}`)
}

/**
 * 根据ID获取指示牌
 */
function getSignboardById(id) {
  return request.get(`${baseUrl}/${id}`)
}

/**
 * 搜索指示牌列表
 * @param {Object} params - 查询参数
 */
function searchSignboard(params = {}) {
  return request.get(`${baseUrl}/search`, params)
}

module.exports = {
  addSignboard,
  updateSignboard,
  deleteSignboard,
  getSignboardById,
  searchSignboard
}
