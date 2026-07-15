// 设备管理页
const deviceStatusApi = require('../../api/deviceStatus.js')

Page({
  data: {
    signboardList: [], // 指示牌列表
    loading: false
  },

  onLoad() {
    this.loadSignboardList()
  },

  onShow() {
    // 每次显示时刷新列表
    this.loadSignboardList()
    // 设置自定义tabBar选中状态
    if (typeof this.getTabBar === 'function' && this.getTabBar()) {
      this.getTabBar().setData({
        selected: 1
      })
    }
  },

  /**
   * 加载指示牌列表
   */
  async loadSignboardList() {
    this.setData({ loading: true })
    
    try {
      const res = await deviceStatusApi.searchSignboard({page:0,limit:-1})
      if (res.data.list.length) {
        this.setData({
          signboardList: Array.isArray(res.data.list) ? res.data.list : []
        })
        console.log(this.data.signboardList);
      }
    } catch (err) {
      console.error('加载指示牌列表失败', err)
      wx.showToast({
        title: '加载失败',
        icon: 'none'
      })
    } finally {
      this.setData({ loading: false })
    }
  },

  /**
   * 查看设备详情
   */
  viewDetail(e) {
    const id = e.currentTarget.dataset.id
    console.log(id);
    wx.navigateTo({
      url: `/pages/signboard/detail/detail?id=${id}`
    })
  },

  /**
   * 添加设备
   */
  addDevice() {
    wx.navigateTo({
      url: '/pages/signboard/detail/detail'
    })
  },

  /**
   * 删除设备
   */
  deleteDevice(e) {
    const id = e.currentTarget.dataset.id
    const name = e.currentTarget.dataset.name
    
    wx.showModal({
      title: '确认删除',
      content: `确定要删除设备"${name}"吗？`,
      confirmColor: '#ff6b6b',
      success: async (res) => {
        if (res.confirm) {
          try {
            await deviceStatusApi.deleteSignboard(id)
            wx.showToast({
              title: '删除成功',
              icon: 'success'
            })
            this.loadSignboardList()
          } catch (err) {
            console.error('删除失败', err)
            wx.showToast({
              title: '删除失败',
              icon: 'none'
            })
          }
        }
      }
    })
  },

  /**
   * 下拉刷新
   */
  onPullDownRefresh() {
    this.loadSignboardList()
    setTimeout(() => {
      wx.stopPullDownRefresh()
    }, 1000)
  }
})
