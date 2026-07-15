// MQTT直连版本后端
const app = getApp()
var mqtt = require("../../utils/utils/mqtt.min")

const options = {
  connectTimeout: 3000,
  clientId: function () {
    return Math.random().toString(36).substr(2, 15)
  },
};
const deviceConfig = {
  deviceName: "hardware",
};

var client;

Page({
  /**
   * 页面的初始数据
   */
  data: {
    Light: 0,
    LightLevel: 0,    
    Temperature_Light:37.6,
    Humidity_Light: 49.7,
    CO2_Light: 424,
    TVOC_Light: 35, 
    PM2_Light: 1,
    AIRlevel_Light: 3,

    Temperature_Mini:37.4,
    Humidity_Mini: 42.6,
    HCHO_Mini: 0.01,
    TVOC_Mini: 44.04,
    AIRlevel_Mini: 3,
    Cell_Mini: 80,

    HCHO_Sixto1: 38.15,
    TVOC_Sixto1: 70.1,
    Temperature_Sixto1: 35.3,
    Humidity_Sixto1: 45.31,
    AIRlevel_Sixto1: 4,
    Cell_Sixto1: 100,
    isFrist:0,
  },  

  /**
   * 生命周期函数--监听页面加载
   */
  async onLoad() {
    wx.showToast({
      title: '等待',
      icon:"loading"
    })


    let that = this;
    //创建websocket,需要域名、ssl证书
    client = mqtt.connect('wx://124.220.31.151:8083/mqtt',options)
    client.on('connect', function () {
      console.log('连接服务器成功')
      //修改订阅主题
      client.subscribe('ESP32toplight/#', function (err) {
        if (!err) {
           console.log('订阅成功！');
        }
      })
    })
	//接收消息监听
  client.on('message', function (topic, message) {
      // message is Buffer
      let msg = message.toString();
      console.log('收到消息：'+msg);
      //解析json数据
      //console.log(typeof msg)
      try{
        let temp  =JSON.parse(msg);
        that.setData({
          data: temp ,                //将所有数据存储到data
          //由于微信小程序的局限性，无法同步data.(mqValue).()和（mqValue）.（）的值，所以手动更新
          //toFixed为了只让数据显示小数点后一位
          //吸顶疼
          Temperature_Light:temp.Temperature_Light.toFixed(1),
          Humidity_Light:temp.Humidity_Light.toFixed(1),
          Light:temp.Light,
          LightLevel:temp.LightLevel,
          //Mini
          Temperature_Mini:temp.Temperature_Mini.toFixed(1),
          //SixTo1
          Temperature_Sixto1:temp.Temperature_Sixto1.toFixed(1),
        });
      }
      catch(err){
       console.log("++a")
      }
      
    if(that.data.isFrist == 0){
that.setData({
  isFrist:1,
})
    }

     //console.log(that.data.isFrist)
      if(that.data.isFrist == 1){
        wx.showToast({
          title: '加载成功',
          icon:"success"
        })
        that.setData({
          isFrist:3
        })
      }
     //关闭连接 client.end()
    })
  
  },

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady() {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow() {
    if (typeof this.getTabBar === 'function' &&
    this.getTabBar()) {
    this.getTabBar().setData({
      selected: 0
    })
  }
  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide() {

  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload() {

  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh() {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom() {

  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage() {

  },

  onLightChange: function(event) {
    //var that = this
    // 要发布的主题
    let topicdown = `ESP32topsub/public`
    let lightoff = `0`
    let lighton  = `1`
    let light_level = 1000; //开灯默认是最大亮度
    let light_New = this.data.Light;  //获取目前灯的开关状态 
    var light_f = '{"Light":'
    var light_b = '"LightLevel":'
    console.log(light_New);

    if (light_New == '1') {
      client.publish(topicdown,light_f+lightoff+","+light_b+lightoff+"}", function (err) {
        if (!err) {
          console.log('成功下发命令 关灯!');
        }
      })
    } else {
     
      // "LightLevel":"+lighton+"}
      client.publish(topicdown,light_f+lighton+","+light_b+light_level+"}", function (err) {
        if (!err) {
          console.log('成功下发命令 开灯!');

        }
      })
    }
  },

  // onLightChange: function(event) {
  //   //var that = this
  //   // 要发布的主题
  //   let topicdown = `sub`
  //   let lightoff = `0`
  //   let lighton  = `1`
    

  //   if (this.data.mqValue.Light == '1') {
  //     client.publish(topicdown,lightoff, function (err) {
  //       if (!err) {
  //         console.log('成功下发命令 关灯!');
  //       }
  //     })
  //   } else {
  //     client.publish(topicdown,lighton, function (err) {
  //       if (!err) {
  //         console.log('成功下发命令 开灯!');
  //       }
  //     })
  //   }
  // },

  //---------备份
  // let water = `0`,
  // onLightChange: function(event) {
  //   //var that = this
  //   console.log(event.detail)
  //   let sw = event.detail.value
  //   // 要发布的主题
  //   let topicdown = `sub`
  //   let lightoff = `0`
  //   let lighton  = `1`

  //   if (sw==0) {
  //     client.publish(topicdown, lightoff, function (err) {
  //       if (!err) {
  //         console.log('成功下发命令 关灯!');
  //       }
  //     })
  //   } else {
  //     client.publish(topicdown,lighton, function (err) {
  //       if (!err) {
  //         console.log('成功下发命令 开灯!');
  //       }
  //     })
  //   }
  // },


  SliderLight_Change(e) {
       //var that = this
    // 要发布的主题
    let topicdown = `ESP32topsub/public`
    let lightoff = `0`
    let lighton  = `1`
    // LightLevel:e.detail.value;  //更新灯亮度数据
    let light_level = e.detail.value; //获取目前灯亮度状态
    let light_New = this.data.Light;  //获取目前灯的开关状态 
    var light_f = '{"Light":'
    var light_b = '"LightLevel":'

    if (light_New == '1') {
      client.publish(topicdown,light_f+lighton+","+light_b+light_level+"}", function (err) {
        if (!err) {
          console.log('成功下发命令 调光!');
        }
      })
    } else {
      // "LightLevel":"+lighton+"}
      client.publish(topicdown,light_f+lighton+","+light_b+light_level+"}", function (err) {
        if (!err) {
          console.log('灯未开启 已亮灯并且调光成功调光!');
         }
       })
    }
  },

// //改变灯光亮度
//   SliderLight_Change(e) {
//     var t = this
//     console.log(e.detail)
//     let ss = e.detail.value
//     // 要发布的主题
//     let topicdown = `sub`
//     //转换number-->let以便client.publish发布
//     let num = e.detail.value
//     let str = String(num)

//     if (ss) {
//       client.publish(topicdown,str, function (err) {
//         if (ss) {
//           console.log('成功下发命令 改变亮度!')
//         }
//       })
//     }
//   },


  gotoLight: function (options) {

    wx.navigateTo({
      url: '../Group/Group', //要跳转到的页面路径
    })
  },
  gotoMini: function (options) {

    wx.navigateTo({
      url: '../miniQRS/miniQRS', //要跳转到的页面路径
    })
  },
  gotoSixTo1: function (options) {

    wx.navigateTo({
      url: '../SixTo1/SixTo1', //要跳转到的页面路径
    })
  },

})

// ChooseRight({
//   data: {
//     // 假设这是你的条件变量
//     condition: true,
//     // 定义两种情况下要显示的文本
//     textForConditionTrue: '条件为真时的文本',
//     textForConditionFalse: '条件为假时的文本'
//   }
// });

//跳转路径

// index.js
// 获取应用实例
const app1 = getApp()

