package com.thz.device.mapper.device;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.thz.device.pojo.device.Device;
import com.thz.device.pojo.device.dto.DeviceGetTimeDTO;
import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import org.apache.ibatis.annotations.Mapper;

import java.math.BigDecimal;
import java.time.LocalDateTime;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/10
 */
@Mapper
public interface DeviceMapper extends BaseMapper<Device> {
    Device details(String macAddress);

    List<LocalDateTime> createTimeList(DeviceGetTimeDTO deviceGetTimeDTO);

    List<BigDecimal> toBigDecimalList(LocalDateTime beginTime, LocalDateTime endTime, String macAddress, String toListStr);

    List<Integer> toIntegerList(LocalDateTime beginTime, LocalDateTime endTime, String macAddress, String toListStr);

    DeviceWarningDTO getDeviceWarning();
}
