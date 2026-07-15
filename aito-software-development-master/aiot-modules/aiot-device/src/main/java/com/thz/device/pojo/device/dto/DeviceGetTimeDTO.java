package com.thz.device.pojo.device.dto;

import com.fasterxml.jackson.annotation.JsonFormat;
import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

import java.time.LocalDateTime;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/12
 */
@Setter
@Getter
@Builder
public class DeviceGetTimeDTO {
    @Schema(description = "设备号")
    @NotBlank(message = "设备名称不能为空")
    private String macAddress;

    @Schema(description = "输入的起始时间")
    @JsonFormat(pattern = "yyyy-MM-dd HH:mm:ss")
    @NotNull(message = "起始时间不能为空")
    private LocalDateTime beginTime;

    @Schema(description = "输入的结束时间")
    @JsonFormat(pattern = "yyyy-MM-dd HH:mm:ss")
    @NotNull(message = "结束时间不能为空")
    private LocalDateTime endTime;
}
