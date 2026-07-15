package com.thz.device.service.device.service;

import com.thz.device.mapper.device.DeviceMapper;
import com.thz.device.pojo.device.dto.DeviceGetTimeDTO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.stereotype.Service;

import java.math.BigDecimal;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/11
 */
@Service
public class ToListService {
    @Autowired
    private DeviceMapper deviceMapper;

    public List<BigDecimal> toBigDecimalList(DeviceGetTimeDTO deviceGetTimeDTO, String toListStr){
        return deviceMapper.toBigDecimalList(deviceGetTimeDTO.getBeginTime(), deviceGetTimeDTO.getEndTime(), deviceGetTimeDTO.getMacAddress() ,toListStr);
    }

    public List<Integer> toIntegerList(DeviceGetTimeDTO deviceGetTimeDTO, String toListStr){
        return deviceMapper.toIntegerList(deviceGetTimeDTO.getBeginTime(), deviceGetTimeDTO.getEndTime(), deviceGetTimeDTO.getMacAddress() ,toListStr);
    }

}
