package com.thz.device.pojo.device.vo;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.Getter;
import lombok.Setter;
import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/11
 */
@Setter
@Getter
public class DeviceMacAddressVO {
    @Schema(description = "设备号")
    private String macAddress;
    @Schema(description = "设备昵称")
    private String macAddressName;
    @Schema(description = "设备模块名称")
    private Integer boardId;
    @Schema(description = "设备样式")
    private String img;
}
