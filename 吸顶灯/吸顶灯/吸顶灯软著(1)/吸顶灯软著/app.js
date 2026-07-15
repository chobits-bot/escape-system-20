// app.js
App({
  onLaunch() {
    // 展示本地存储能力
    const logs = wx.getStorageSync('logs') || []
    logs.unshift(Date.now())
    wx.setStorageSync('logs', logs)

    // 登录
    wx.login({
      success: res => {
        // 发送 res.code 到后台换取 openId, sessionKey, unionId
      }
    })
  },
  globalData: {
    userInfo: null,
    GroupID: [],
    Group_co2: [],
    Group_CH2O: [],
    Group_TEMPER: [],
    Group_HUD: [],
    Group_TVOC: [],
    Group_PM2_5: [],
    Group_PM10: [],
    Group_DC: [],
    Group_time: []
  }
})