package com.thz.system.pojo.sysuser.dto;

import jakarta.validation.constraints.Email;
import lombok.Data;
import lombok.Getter;
import lombok.Setter;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/11
 */
@Data
@Setter
@Getter
public class SysUserEmailDTO {
    @Email(message = "邮箱格式不正确")
    private String email;
}