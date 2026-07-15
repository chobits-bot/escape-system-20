package com.thz.system.pojo;

import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;
import com.thz.common.core.pojo.UserBaseEntity;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/26
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@Builder
@TableName("tb_sys_user")
public class SysUser extends UserBaseEntity {
    @TableId(value = "USER_ID", type = IdType.ASSIGN_ID)
    private Long userId;
    private String userAccount;
    private String password;
    private String nickName;
    private String salt;
    private String remark;
    private String email;
}
