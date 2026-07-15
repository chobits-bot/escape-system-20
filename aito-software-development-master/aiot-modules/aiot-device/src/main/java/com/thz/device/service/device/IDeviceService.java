package com.thz.device.service.device;

import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.common.core.result.Result;
import com.thz.device.pojo.device.dto.DeviceGetTimeDTO;
import com.thz.device.pojo.device.vo.*;
import com.thz.device.pojo.device.vo.get.GetEco2VO;
import com.thz.device.pojo.device.vo.get.GetHumidityVO;
import com.thz.device.pojo.device.vo.get.GetTemperatureVO;
import com.thz.device.pojo.device.vo.get.GetTvocVO;

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/10
 */
public interface IDeviceService {
    List<DeviceMacAddressVO> list(Integer boardId);

    DeviceDetailsVO details(String macAddress);

    GetTemperatureVO getTemperature(DeviceGetTimeDTO deviceGetTimeDTO);

    GetHumidityVO getHumidity(DeviceGetTimeDTO deviceGetTimeDTO);

    GetTvocVO getTvoc(DeviceGetTimeDTO deviceGetTimeDTO);

    GetEco2VO getEco2(DeviceGetTimeDTO deviceGetTimeDTO);

    List<String> warningReportList();

    String warningReport(String macAddress);
}
