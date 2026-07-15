package com.thz.common.core.pojo;


import lombok.Builder;
import lombok.Getter;
import lombok.Setter;
/**
 * @Description
 * @Author thz
 * @Date 2025/3/2
 */
@Getter
@Setter
@Builder
public class LoginUser {

    private String nickName;   // 用户昵称

    private Byte identity;  // 1 表示普通用户  2 表示管理员用户

    private String avatarUrl;  // 用户头像

    private String email;
}
