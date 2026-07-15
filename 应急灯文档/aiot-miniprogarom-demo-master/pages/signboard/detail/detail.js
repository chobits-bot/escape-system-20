// 设备详情页
const deviceStatusApi = require('../../../api/deviceStatus.js')

Page({
  data: {
    id: null, // 设备ID，null表示新增
    isEdit: false, // 是否为编辑模式
    
    formData: {
      mac_address: '',
      display: 1,
      duty_cycle: 512,
      brightness: 50,
      address: '',
      remark: ''
    },
    
    displayOptions: [
      { value: 1, label: 'left' },
      { value: 2, label: 'right' },
      { value: 3, label: 'center' },
      { value: 4, label: 'up' },
      { value: 5, label: 'down' },
      { value: 6, label: 'Left' },
      { value: 7, label: 'Right' },
      { value: 8, label: 'Center' },
      { value: 9, label: 'Up' },
      { value: 10, label: 'Down' }
    ],
    
    loading: false
  },

  onLoad(options) {
    console.log(options)
    if (options.id) {
      this.setData({
        id: options.id,
        isEdit: true
      })
      console.log(options.id);
      this.loadDeviceDetail(options.id)
    }
  },

  /**
   * 加载设备详情
   */
  async loadDeviceDetail(id) {
    this.setData({ loading: true })
    
    try {
      const res = await deviceStatusApi.getSignboardById(id)
      console.log(res)
      if (res.data) {
        this.setData({
          formData: {
            mac_address: res.data.signboard.mac_address || '',
            display: res.data.signboard.display || 1,
            duty_cycle: res.data.signboard.duty_cycle || 512,
            brightness: res.data.signboard.brightness || 50,
            address: res.data.signboard.address || '',
            remark: res.data.signboard.remark || ''
          }
        })
      }
    } catch (err) {
      wx.showToast({
        title: '加载失败',
        icon: 'none'
      })
    } finally {
      this.setData({ loading: false })
    }
  },

  /**
   * 输入框变化
   */
  onInputChange(e) {
    const field = e.currentTarget.dataset.field
    const value = e.detail.value
    this.setData({
      [`formData.${field}`]: value
    })
  },

  /**
   * 滑块变化
   */
  onSliderChange(e) {
    const field = e.currentTarget.dataset.field
    const value = e.detail.value
    this.setData({
      [`formData.${field}`]: value
    })
  },

  /**
   * 选择器变化
   */
  onPickerChange(e) {
    const value = parseInt(e.detail.value) + 1 // 选择器索引从0开始，display值从1开始
    this.setData({
      'formData.display': value
    })
  },

  /**
   * 保存设备
   */
  async saveDevice() {
    const { formData, id, isEdit } = this.data

    // 验证必填项
    if (!formData.mac_address) {
      wx.showToast({
        title: '请输入MAC地址',
        icon: 'none'
      })
      return
    }

    this.setData({ loading: true })

    try {
      if (isEdit) {
        // 更新设备
        await deviceStatusApi.updateSignboard({
          id: parseInt(id),
          ...formData
        })
        wx.showToast({
          title: '更新成功',
          icon: 'success'
        })
      } else {
        // 新增设备
        const res = await deviceStatusApi.addSignboard(formData)
        if (res.code===200){
          wx.showToast({
            title: '添加成功',
            icon: 'success'
          })
        }else{
          throw new Error('设备ID无效')
        }
      }

      setTimeout(() => {
        wx.navigateBack()
      }, 1500)
    } catch (err) {
      console.error('保存失败', err)
      wx.showToast({
        title: '保存失败',
        icon: 'none'
      })
    } finally {
      this.setData({ loading: false })
    }
  }
})
