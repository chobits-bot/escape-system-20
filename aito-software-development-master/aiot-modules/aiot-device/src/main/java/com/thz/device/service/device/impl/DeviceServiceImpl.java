package com.thz.device.service.device.impl;

import com.thz.common.core.constant.DeviceConstants;
import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.common.core.result.Result;
import com.thz.device.config.DeviceMacAddressConfig;
import com.thz.device.mapper.device.DeviceMapper;
import com.thz.device.pojo.device.dto.DeviceGetTimeDTO;
import com.thz.device.pojo.device.vo.*;
import com.thz.device.pojo.device.vo.get.GetEco2VO;
import com.thz.device.pojo.device.vo.get.GetHumidityVO;
import com.thz.device.pojo.device.vo.get.GetTemperatureVO;
import com.thz.device.pojo.device.vo.get.GetTvocVO;
import com.thz.device.service.device.IDeviceService;
import com.thz.device.service.device.service.DeviceDetailsService;
import com.thz.device.service.device.service.DeviceRedisService;
import com.thz.device.service.device.service.ToListService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.math.BigDecimal;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/10
 */
@Slf4j
@Service
public class DeviceServiceImpl implements IDeviceService {

    @Autowired
    private DeviceMapper deviceMapper;

    @Autowired
    private DeviceDetailsService deviceDetailsService;

    @Autowired
    private ToListService toListService;

    @Autowired
    private DeviceRedisService deviceRedisService;

    @Autowired
    private DeviceMacAddressConfig deviceMacAddressConfig;

    @Override
    public List<DeviceMacAddressVO> list(Integer boardId) {
        List<DeviceMacAddressVO> deviceMacAddressList = deviceMacAddressConfig.getMacAddresslist();
        if (deviceMacAddressList == null) {
            deviceMacAddressList = new ArrayList<>();
        }
        List<DeviceMacAddressVO> deviceMacAddressVOList = new ArrayList<>();
        for (DeviceMacAddressVO deviceMacAddress : deviceMacAddressList) {
            if (deviceMacAddress.getBoardId().equals(boardId)) {
                String deviceKey = deviceMacAddress.getMacAddress();
                if(!deviceRedisService.HasKeyDeviceRedis(deviceKey)){
                    deviceRedisService.setDeviceRedis(deviceMacAddress);
                }
                deviceMacAddressVOList.add(deviceRedisService.getDeviceRedis(deviceKey));
            }
        }
        return deviceMacAddressVOList;
    }

    @Override
    public DeviceDetailsVO details(String macAddress) {
        return deviceDetailsService.deviceDetails(macAddress);
    }

    @Override
    public GetTemperatureVO getTemperature(DeviceGetTimeDTO deviceGetTimeDTO) {
        List<BigDecimal> temperatureList = toListService.toBigDecimalList(deviceGetTimeDTO,"temperature");
        List<LocalDateTime> createTimeList = deviceMapper.createTimeList(deviceGetTimeDTO);
        if (createTimeList == null) {
            createTimeList = new ArrayList<>();
        }
        return GetTemperatureVO.builder()
                .temperature(temperatureList)
                .createTime(createTimeList)
                .build();
    }

    @Override
    public GetHumidityVO getHumidity(DeviceGetTimeDTO deviceGetTimeDTO) {
        List<BigDecimal> humidityList = toListService.toBigDecimalList(deviceGetTimeDTO,"humidity");
        List<LocalDateTime> createTimeList = deviceMapper.createTimeList(deviceGetTimeDTO);
        if (createTimeList == null) {
            createTimeList = new ArrayList<>();
        }
        return GetHumidityVO.builder()
                .humidity(humidityList)
                .createTime(createTimeList)
                .build();
    }

    @Override
    public GetTvocVO getTvoc(DeviceGetTimeDTO deviceGetTimeDTO) {
        List<Integer> tvocList = toListService.toIntegerList(deviceGetTimeDTO,"tvoc");
        List<LocalDateTime> createTimeList = deviceMapper.createTimeList(deviceGetTimeDTO);
        if (createTimeList == null) {
            createTimeList = new ArrayList<>();
        }
        return GetTvocVO.builder()
                .tvoc(tvocList)
                .createTime(createTimeList)
                .build();
    }

    @Override
    public GetEco2VO getEco2(DeviceGetTimeDTO deviceGetTimeDTO) {
        List<Integer> eco2List = toListService.toIntegerList(deviceGetTimeDTO,"eco2");
        List<LocalDateTime> createTimeList = deviceMapper.createTimeList(deviceGetTimeDTO);
        if (createTimeList == null) {
            createTimeList = new ArrayList<>();
        }
        return GetEco2VO.builder()
                .eco2(eco2List)
                .createTime(createTimeList)
                .build();
    }

    @Override
    public List<String> warningReportList() {
        DeviceWarningDTO deviceWarningDTO = deviceDetailsService.foreWarning();
        List<DeviceMacAddressVO> deviceMacAddressList = deviceMacAddressConfig.getMacAddresslist();
        List<String> warningReportList = new ArrayList<>();
        for (DeviceMacAddressVO deviceMacAddress : deviceMacAddressList) {
            if (deviceRedisService.checkWarning(deviceMacAddress.getMacAddress())){
                warningReportList.add(deviceDetailsService.buildSendWarning(deviceWarningDTO));
            }
        }
        return warningReportList;
    }

    @Override
    public String warningReport(String macAddress) {
        DeviceWarningDTO deviceWarningDTO = deviceDetailsService.foreWarning();
        StringBuilder warningReport = new StringBuilder();
        if (deviceRedisService.checkWarning(macAddress)){
            warningReport.append( deviceDetailsService.buildSendWarning(deviceWarningDTO));
        }
        return warningReport.toString();
    }
}
