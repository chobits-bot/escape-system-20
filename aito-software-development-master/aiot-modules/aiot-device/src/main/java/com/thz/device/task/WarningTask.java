package com.thz.device.task;


import com.thz.common.core.constant.DeviceConstants;
import com.thz.common.core.constant.RedisConstants;
import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.device.rabbit.producer.DeviceWarningProducer;
import com.thz.device.service.device.IDeviceService;
import com.thz.device.service.device.service.DeviceDetailsService;
import com.thz.redis.utlis.RedisUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/18
 */
@Slf4j
@Component
public class WarningTask {
    @Autowired
    private RedisUtil redisUtil;

    @Autowired
    private DeviceDetailsService deviceDetailsService;

    @Autowired
    private DeviceWarningProducer deviceWarningProducer;

    public void Task() {
        DeviceWarningDTO deviceWarningDTO = deviceDetailsService.foreWarning();
        // 1. 检查判断该设备是否报警
        String userKey = redisUtil.buildKey(RedisConstants.WARNING_USER_KEY,deviceWarningDTO.getMacAddress());
        Boolean buildHasKey = redisUtil.hasKey(userKey);
        // 2. 有一个条件符合就报警，将该设备的空值存入redis（限制发信息的时间）
        if(deviceWarningDTO.getFireAlarm().equals(DeviceConstants.FOREWARNING) || deviceWarningDTO.getElectricityAlarm().equals(DeviceConstants.FOREWARNING)){
            // 如果redis没有就发送短信，如果redis中有就跳过
            if (!buildHasKey){
                deviceWarningProducer.produceMessage(deviceWarningDTO);
            }
        }else {
            // 3.1 如果该设备之前报警，经过完善之后的处理
            // 3.2 删除原来的redis的值
            if (buildHasKey){
                redisUtil.delete(userKey);
            }
        }

    }

}
