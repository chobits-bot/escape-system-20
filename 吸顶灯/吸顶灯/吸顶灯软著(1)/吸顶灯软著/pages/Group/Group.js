import * as echarts from '../../ec-canvas/echarts';


var len=10;
var the=666;
let res1 = [37.5,37.6,37.5,37.4,37.7,37.5,37.6,37.7,37.3,37.5]; //温度初始数据 
let res2 = [40.0,41.8,41.6,41.5,42,43,45,41.6,42,41]; //湿度初始数据
function setOption(chart) {
  len++;
  //调用Page内部函数
   const page = getCurrentPages()
  // console.log((page[page.length-1].data.x))

//
  const categories = (function () {
    let now = new Date();
    let res = [];
    let len = 10;
    while (len--) {
      res.unshift(now.toLocaleTimeString().replace(/^\D*/, ''));
      now = new Date(+now - 2000);
    }
    return res;
  })();

  const categories2 = (function () {
    let res = [];
    let len = 10;
    while (len--) {
      res.push(10 - len - 1);
    }
    return res;
  })();

  const data = (function () {
    //将每次获取的最新数据给到res[9]，再将之前的数据向前推进

    let tmp = 0;
    //数据向前推进
    while(tmp<9)
    {
      res1[tmp++]  = res1[tmp];
    }
    res1[9] =  page[page.length-1].data.Temperature_Light;
    return res1;
  })();
  const data2 = (function () {

//将每次获取的最新数据给到res[9]，再将之前的数据向前推进

let tmp = 0;
//数据向前推进
while(tmp<9)
{
  res2[tmp++]  = res2[tmp];
}
res2[9] =  page[page.length-1].data.Humidity_Light;
    return res2;
  })();


  var option = {
    title: {
      text: '温湿度图示',
      backgroundColor: '#C2F628', //主题色
      radius:'55%'
    },
    
    tooltip: {
      trigger: 'axis',
      axisPointer: {
        type: 'cross',
        label: {
          backgroundColor: '#283b56'  
        }
      }
    },
    legend: {},
    toolbox: {
      show: true,
      feature: {
        dataView: { readOnly: false },
        restore: {},
        saveAsImage: {}
      }
    },
    dataZoom: {
      show: false,
      start: 0,
      end: 100
    },
  //标识颜色
    legend: {
      data: ['温度', '湿度'],
      textStyle:{
        color:"#CCCCCC"
      }

    },
    grid: {
      left: 20,
      right: 20,
      bottom: 15,
      top: 40,
      containLabel: true
    },
    xAxis: [
      {
        type: 'category',
        boundaryGap: true,
        data: categories,
      },
      {
        type: 'category',
        boundaryGap: true,
        data: categories2,
      },
    ],
    yAxis: [
      {
        // 折线图
        type: 'value',
        scale: true,
        max: 100,
        min: 0,
        axisLabel:{//修改坐标系字体颜色
          show:true,
          textStyle:{
            color:"#bee283"
          }
          },
        boundaryGap: [0.2, 0.2]
      },
      //柱状图
      {
        type: 'value',
        scale: true,
        max: 50,
        min: 0,
        axisLabel:{//修改坐标系字体颜色
          show:true,
          textStyle:{
            color:"#8DB6DB"
          }
          },
        boundaryGap: [0.2, 0.2]
      }
    ],
    
    series: [
      {
        itemStyle: {
          normal: {
            //柱形图圆角设置
            barBorderRadius:[20,20, 0, 0]//柱形图圆角设置
            }
        },
        name: '温度',
        type: 'bar',
        xAxisIndex: 1,
        yAxisIndex: 1,
        data: data
      },
      {

        name: '湿度',
        type: 'line',
        data: data2
      }
    ]
  };

  chart.setOption(option);
}

Page({
  data: {
    ec: {
      // 将 lazyLoad 设为 true 后，需要手动初始化图表
      lazyLoad: true,
      timer:''
    },
    isLoaded: false,
    isDisposed: false,
    Light: 0,
    LightLevel: 0,    
    Temperature_Light:37.6,
    Humidity_Light: 49.7,
    CO2_Light: 424,
    TVOC_Light: 35, 
    PM2_Light: 1,
    AIRlevel_Light: 1,

    Temperature_Mini:37.4,
    Humidity_Mini: 42.6,
    HCHO_Mini: 0.01,
    TVOC_Mini: 44.04,
    AIRlevel_Min: 1,
    Cell_Mini: 20,

    HCHO_Sixto1: 38.15,
    TVOC_Sixto1: 70.1,
    Temperature_Sixto1: 35.3,
    Humidity_Sixto1: 45.31,
    AIRlevel_Sixto1: 1,
    Cell_Sixto1: 20,
    isFrist:0,
  },

  onShareAppMessage: res => {
    return {
      title: 'ECharts 可以在微信小程序中使用啦！',
      path: '/pages/index/index',
      success: function () { },
      fail: function () { }
    }
  },


  setChartOption() {
    const app = getApp();
    //app.setOption(this.data.chartOption);
  },

  updateChartData() {
    // 更新图表数据
    //console.log(this.data.Temperature_Light);
    // this.data.chartOption.series[0].data = [len, the, 340, 344, 300, 320, 310]
    // this.data.chartOption.series[1].data = [120, 102, 141, 174, 190, 250, 220]
    // this.data.chartOption.series[2].data = [-20, -32, -21, -34, -90, -130, -110]
    // 更新图表
    this.setChartOption()
  },

  // getOption: function () {        
  //   var _this = this;
    
  //         console.log(res);
  //         _this.setData({      
  //           //将接口返回的数据data赋值给data
  //           data:res.data,     
  //         })
  //         var temperature=[];
  //         var date=[];
  //         for (var i = 0; i < res.data.length; ++i) {
  //             temperature[i]=res.data[i].temperature;
  //             date[i]=res.data[i].date;
  //          }
  //         _this.init_chart(date,temperature)           
  // },


  //如果全部注销就会排除init找不到的错位
  onLoad() {
  //   // 每2秒调用一次init函数
  //   this.timer = setInterval(() => {
  //     this.init();
  //   }, 2000);
  //  每2秒调用一次init函数
  //  console.log(this.x);
  var _this = this;
  //this.getOption();
  this.setData({                    //每隔10s刷新一次
    timer: setInterval(function () {
      _this.init();
  }, 1000)
  })


  //  var _this = this;
  //  this.data.x++;
  //  this.getOption();
  //  this.setData({                    //每隔10s刷新一次
  //   timer: setInterval(function () {
  //     _this.init();
  // }, 1000)

  //  this.timer = setInterval(() => {
  //   this.setData({
  //     xD:[...this.data.xD, this.data.x]
  //   });
  // }, 200);
  },

  onReady: function () {
    // 获取组件
    this.ecComponent = this.selectComponent('#mychart-dom-bar');
  },

  // 点击按钮后初始化图表
  init: function () {

    this.ecComponent.init((canvas, width, height, dpr) => {
      // 获取组件的 canvas、width、height 后的回调函数
      // 在这里初始化图表
      const chart = echarts.init(canvas, null, {
        width: width,
        height: height,
        devicePixelRatio: dpr // new
      });
      setOption(chart);
      // 将图表实例绑定到 this 上，可以在其他成员函数（如 dispose）中访问
      this.chart = chart;
      this.setData({
        isLoaded: true,
        isDisposed: false
      });
      // 注意这里一定要返回 chart 实例，否则会影响事件处理等
      return chart;
    });
  },

  dispose: function () {
    if (this.chart) {
      this.chart.dispose();
    }
    this.setData({
      isDisposed: true
    });
  },
  

  /**
   * 生命周期函数--监听页面加载
//    */
async onLoad() {
  var _this = this;
  //this.getOption();

  this.setData({                    //每隔10s刷新一次
    timer: setInterval(function () {
      _this.init();
  }, 5000)
  })
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

  onLightSwitch: function(event) {
    //var that = this
    // 要发布的主题
    let topicdown = `ESP32topsub/public`
    let lightoff = `0`
    let lighton  = `1`
    let light_level = 1000; //开灯默认是最大亮度
    let light_New = this.data.Light;  //获取目前灯的开关状态 
    var light_f = '{"Light":'
    var light_b = '"LightLevel":'
    var checkedValue = event.detail.value;  //获取当前开关状态
    console.log(checkedValue);  //打印开关状态
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


// MQTT直连版本后端





var mqtt = require("../../utils/utils/mqtt.min")
const options = {

  clientId: function () {
    return Math.random().toString(36).substr(2, 15)
  },
};
const deviceConfig = {
  deviceName: "hardware",
};
var client;
