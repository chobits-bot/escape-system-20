// 设备MAC地址相关API
const request = require('../utils/request.js')
const config = require('../config/env.js')

const baseUrl = config.deviceMacApiBase

/**
 * 添加设备MAC地址
 */
function addDeviceMacAddress(data) {
  return request.post(baseUrl, data)
}

/**
 * 更新设备MAC地址
 */
function updateDeviceMacAddress(data) {
  return request.put(baseUrl, data)
}

/**
 * 删除设备MAC地址
 */
function delDeviceMacAddress(id) {
  return request.del(`${baseUrl}/${id}`)
}

/**
 * 根据ID获取设备MAC地址
 */
function getDeviceMacAddressById(id) {
  return request.get(`${baseUrl}/${id}`)
}

/**
 * 获取所有启用的设备MAC地址列表
 */
function selectAllEnabledDeviceMacAddress() {
  return request.get(`${baseUrl}/list`)
}

/**
 * 根据MAC地址查询设备
 */
function selectByMacAddress(mac) {
  return request.get(`${baseUrl}/mac/${mac}`)
}

module.exports = {
  addDeviceMacAddress,
  updateDeviceMacAddress,
  delDeviceMacAddress,
  getDeviceMacAddressById,
  selectAllEnabledDeviceMacAddress,
  selectByMacAddress
}
