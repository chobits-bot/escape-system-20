package com.thz.system.pojo.sysuser.dto;

import com.baomidou.mybatisplus.annotation.TableField;
import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class SysUserSaveDTO {
    @NotBlank(message = "⽤⼾账号不能为空")
    @Size(min = 8, max = 16, message = "账号⻓度不能少于8位，不能⼤于16位")
    @Schema(description = "用户账号")
    private String userAccount;

    @NotBlank(message = "⽤⼾密码不能为空")
    @Size(min = 8, max = 16, message = "密码⻓度不能少于8位，不能⼤于16位")
    @Schema(description = "用户密码")
    private String password;

    @NotBlank(message = "⽤⼾邮箱不能为空")
    @Schema(description = "用户邮箱")
    private String email;

    @NotBlank(message = "⽤⼾昵称不能为空")
    @Schema(description = "用户昵称")
    private String nickName;

    @TableField(exist = false)
    @NotBlank(message = "验证码不能为空")
    @Schema(description = "邮箱验证码")
    private String code;
}
