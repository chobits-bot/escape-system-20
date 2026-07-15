package com.thz.system.pojo.sysuser.dto;

import jakarta.validation.constraints.NotBlank;
import lombok.*;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class SysUserLoginDTO {
    @NotBlank(message = "⽤⼾账号不能为空")
    private String userAccount;
    @NotBlank(message = "⽤⼾密码不能为空")
    private String password;
}
