// 语言包配置
export const messages = {
  'zh-CN': {
    // 通用
    common: {
      save: '保存',
      cancel: '取消',
      confirm: '确认',
      delete: '删除',
      edit: '编辑',
      search: '搜索',
      reset: '重置',
      loading: '加载中...',
      noData: '暂无数据',
      operation: '操作'
    },
    
    // 导航菜单
    nav: {
      dashboard: '数据总览',
      sensors: '传感器',
      boards: '控制板',
      signboards: '指示牌',
      alerts: '设备告警',
      notifications: '通知日志',
      reports: '数据报表',
      users: '用户管理'
    },
    
    // 页签相关
    tabs: {
      closeCurrent: '关闭当前',
      closeOther: '关闭其他',
      closeAll: '关闭所有',
      refresh: '刷新'
    },
    
    // 设置
    settings: {
      language: '语言',
      theme: '主题',
      showTabs: '显示页签栏',
      logout: '退出登录',
      confirmLogout: '确定退出登录？'
    },
    
    // 用户
    user: {
      profile: '个人资料',
      settings: '设置'
    }
  },
  
  'en-US': {
    // Common
    common: {
      save: 'Save',
      cancel: 'Cancel',
      confirm: 'Confirm',
      delete: 'Delete',
      edit: 'Edit',
      search: 'Search',
      reset: 'Reset',
      loading: 'Loading...',
      noData: 'No data',
      operation: 'Operation'
    },
    
    // Navigation
    nav: {
      dashboard: 'Dashboard',
      sensors: 'Sensors',
      boards: 'Control Boards',
      signboards: 'Signboards',
      alerts: 'Device Alerts',
      notifications: 'Notification Logs',
      reports: 'Data Reports',
      users: 'User Management'
    },
    
    // Tabs
    tabs: {
      closeCurrent: 'Close Current',
      closeOther: 'Close Other',
      closeAll: 'Close All',
      refresh: 'Refresh'
    },
    
    // Settings
    settings: {
      language: 'Language',
      theme: 'Theme',
      showTabs: 'Show Tabs Bar',
      logout: 'Logout',
      confirmLogout: 'Are you sure to logout?'
    },
    
    // User
    user: {
      profile: 'Profile',
      settings: 'Settings'
    }
  }
}

// 获取当前语言的翻译
export function useI18n() {
  const { language } = useSettingsStore?.() || { language: { value: 'zh-CN' } }
  
  return {
    t: (key) => {
      const keys = key.split('.')
      let result = messages[language.value]
      
      for (const k of keys) {
        if (result && result[k]) {
          result = result[k]
        } else {
          return key // 如果找不到翻译，返回原key
        }
      }
      
      return result
    }
  }
}

// 在组件中使用的组合式函数
export function useTranslate() {
  const { t } = useI18n()
  return { t }
}