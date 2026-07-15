package com.thz.common.core.pojo.dto;

import lombok.*;

import java.io.Serializable;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/17
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class DeviceWarningDTO implements Serializable {
    private String macAddress;
    private Integer fireAlarm;
    private Integer electricityAlarm;
}


