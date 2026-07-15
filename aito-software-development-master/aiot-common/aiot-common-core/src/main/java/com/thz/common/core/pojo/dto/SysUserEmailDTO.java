package com.thz.common.core.pojo.dto;

import lombok.*;

import java.io.Serializable;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/16
 */
@Builder
@Data
@AllArgsConstructor
@NoArgsConstructor
public class SysUserEmailDTO implements Serializable {
    private String email;
    private String code;
}