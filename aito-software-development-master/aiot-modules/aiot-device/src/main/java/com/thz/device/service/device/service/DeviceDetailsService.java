package com.thz.device.service.device.service;

import com.thz.common.core.constant.DeviceConstants;
import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.device.mapper.device.DeviceMapper;
import com.thz.device.pojo.device.Device;
import com.thz.device.pojo.device.vo.DeviceDetailsVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;


/**
 * @Description
 * @Author thz
 * @Date 2025/3/10
 */
@Service
public class DeviceDetailsService {
    @Autowired
    private DeviceMapper deviceMapper;

    @Autowired
    private DeviceRedisService deviceRedisService;

    public DeviceDetailsVO deviceDetails (String macAddress) {
        Device details = deviceMapper.details(macAddress);
        return DeviceDetailsVO.builder()
                .macAddress(macAddress)
                .electricityAlarm(checkForewarning(details.getElectricityAlarm()))
                .fireAlarm(checkForewarning(details.getFireAlarm()))
                .aqi(checkAqi(details.getAqi()))
                .eco2(details.getEco2())
                .tvoc(details.getTvoc())
                .humidity(details.getHumidity())
                .temperature(details.getTemperature())
                .createTime(details.getCreateTime())
                .electricity(details.getElectricity())
                .img(deviceRedisService.getDeviceRedis(macAddress).getImg())
                .build();
    }

    private String checkForewarning(Integer num) {
        if (num == 0) {
            return DeviceConstants.FOREWARNING_0;
        }
        return DeviceConstants.FOREWARNING_1;
    }

    private String checkAqi(Integer aqi){
        String aqiStr = null;
        switch(aqi){
            case 1 :
                aqiStr  = DeviceConstants.API_1;
                break;
            case 2 :
                aqiStr = DeviceConstants.API_2;
                break;
            case 3 :
                aqiStr = DeviceConstants.API_3;
                break;
            case 4 :
                aqiStr = DeviceConstants.API_4;
                break;
            case 5 :
                aqiStr = DeviceConstants.API_5;
                break;
        }
        return aqiStr;
    }

    public DeviceWarningDTO foreWarning() {
        return deviceMapper.getDeviceWarning();
    }

    public String buildSendWarning(DeviceWarningDTO deviceWarningDTO) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("<h1 style=\"color:#FF0000;\">").append("尊敬的").append("管理员你好").append("</h1>");
        stringBuilder.append("<br>").append("当前").append(deviceWarningDTO.getMacAddress())
                .append("设备检测到");
        if(deviceWarningDTO.getElectricityAlarm().equals(DeviceConstants.FOREWARNING)){
            stringBuilder.append("漏电报警，");
        }
        if(deviceWarningDTO.getFireAlarm().equals(DeviceConstants.FOREWARNING)){
            stringBuilder.append("火灾报警，");
        }
        stringBuilder.append("请及时处理").append("</br>");
        return stringBuilder.toString();
    }

}
