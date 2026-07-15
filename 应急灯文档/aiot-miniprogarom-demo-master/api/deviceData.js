// 设备数据相关API
const request = require('../utils/request.js')
const config = require('../config/env.js')

const baseUrl = config.deviceDataApiBase

/**
 * 添加设备数据
 */
function addIotDeviceData(data) {
  return request.post(baseUrl, data)
}

/**
 * 根据ID获取设备数据
 */
function getIotDeviceDataById(id) {
  return request.get(`${baseUrl}/${id}`)
}

/**
 * 查询eco2历史数据
 * @param {Object} data - { mac_address, start_time, end_time }
 */
function getEco2(data) {
  return request.post(`${baseUrl}/eco2`, data)
}

/**
 * 查询湿度历史数据
 * @param {Object} data - { mac_address, start_time, end_time }
 */
function getHumidity(data) {
  return request.post(`${baseUrl}/humidity`, data)
}

/**
 * 查询温度历史数据
 * @param {Object} data - { mac_address, start_time, end_time }
 */
function getTemperature(data) {
  return request.post(`${baseUrl}/temperature`, data)
}

/**
 * 查询tvoc历史数据
 * @param {Object} data - { mac_address, start_time, end_time }
 */
function getTvoc(data) {
  return request.post(`${baseUrl}/tvoc`, data)
}

/**
 * 查询aqi历史数据
 * @param {Object} data - { mac_address, start_time, end_time }
 */
function getAqi(data) {
  return request.post(`${baseUrl}/aqi`, data)
}

/**
 * 获取所有告警设备
 */
function getDeviceWarning() {
  return request.get(`${baseUrl}/warnings`)
}

module.exports = {
  addIotDeviceData,
  getIotDeviceDataById,
  getEco2,
  getHumidity,
  getTemperature,
  getTvoc,
  getAqi,
  getDeviceWarning
}
