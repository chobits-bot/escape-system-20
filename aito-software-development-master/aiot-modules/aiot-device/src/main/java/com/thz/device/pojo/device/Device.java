package com.thz.device.pojo.device;

import com.fasterxml.jackson.annotation.JsonFormat;
import io.swagger.v3.oas.annotations.media.Schema;
import lombok.*;

import java.math.BigDecimal;
import java.security.Timestamp;
import java.time.LocalDateTime;
import java.util.Date;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/10
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class Device {
    private Integer id;
    @Schema(description = "设备号")
    private String macAddress;
    @Schema(description = "获取空气质量指数, 返回值：1-优，2-良，3-中，4-差，5-不健康")
    private Integer aqi;
    @Schema(description = "TVOC 浓度, 返回值范围: 0-65000，单位：ppb")
    private Integer tvoc;
    @Schema(description = "二氧化碳浓度返回值范围: 400-65000，单位：ppm")
    private Integer eco2;
    @Schema(description = "温度")
    private BigDecimal temperature;
    @Schema(description = "湿度")
    private BigDecimal humidity;
    @Schema(description = "电流值")
    private Integer electricity;
    @Schema(description = "火灾标志（1报警，0无报警）")
    private Integer fireAlarm;
    @Schema(description = "断电标志（1报警，0无报警）")
    private Integer electricityAlarm;
    @Schema(description = "创建时间")
    @JsonFormat(pattern = "yyyy-MM-dd HH:mm:ss")
    private LocalDateTime createTime;
}
