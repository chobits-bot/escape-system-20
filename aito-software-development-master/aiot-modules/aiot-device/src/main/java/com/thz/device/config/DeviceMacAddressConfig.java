package com.thz.device.config;

import com.thz.device.pojo.device.vo.DeviceMacAddressVO;
import lombok.Data;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/11
 */
@Data
@Component
@ConfigurationProperties(prefix = "device")
public class DeviceMacAddressConfig {
    private List<DeviceMacAddressVO> macAddresslist;
}
