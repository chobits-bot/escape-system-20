package com.thz.device.service.device.service;

import com.thz.common.core.constant.RedisConstants;
import com.thz.device.pojo.device.vo.DeviceMacAddressVO;
import com.thz.redis.utlis.RedisUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.concurrent.TimeUnit;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/13
 */
@Slf4j
@Service
public class DeviceRedisService {

    @Autowired
    private RedisUtil redisUtil;

    public void setDeviceRedis(DeviceMacAddressVO deviceMacAddressVO){
        String deviceKey = getDeviceRedisKey(deviceMacAddressVO.getMacAddress());
        redisUtil.setCacheObject(deviceKey, deviceMacAddressVO, RedisConstants.DEVICE_TTL, TimeUnit.MINUTES);
    }

    public DeviceMacAddressVO getDeviceRedis(String macAddress){
        String deviceKey = getDeviceRedisKey(macAddress);
        log.info(":{}", redisUtil.getCacheObject(deviceKey,DeviceMacAddressVO.class));
        return redisUtil.getCacheObject(deviceKey,DeviceMacAddressVO.class);
    }

    public Boolean HasKeyDeviceRedis(String macAddress){
        String deviceKey = getDeviceRedisKey(macAddress);
        return redisUtil.hasKey(deviceKey);
    }

    public String getDeviceRedisKey(String macAddress){
        return redisUtil.buildKey(RedisConstants.DEVICE_KEY,macAddress);
    }

    public Boolean checkWarning(String macAddress){
        String userKey = redisUtil.buildKey(RedisConstants.WARNING_USER_KEY,macAddress);
        return redisUtil.hasKey(userKey);
    }

}
