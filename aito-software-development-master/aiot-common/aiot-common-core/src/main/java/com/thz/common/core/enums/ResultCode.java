package com.thz.common.core.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * @Description
 * @Author thz
 * @Date 2024/12/12
 */
@AllArgsConstructor
@Getter
public enum ResultCode {
    // 操作成功
    SUCCESS (1000, "操作成功"),

    // 服务器内部错误，友好提⽰
    ERROR (2000, "服务繁忙请稍后重试"),

    // 操作失败，但是服务器不存在异常
    FAILED (3000, "操作失败"),
    FAILED_UNAUTHORIZED (3001, "未授权"),
    FAILED_PARAMS_VALIDATE (3002, "参数校验失败"),
    FAILED_NOT_EXISTS (3003, "资源不存在"),
    FAILED_ALREADY_EXISTS (3004, "资源已存在"),
    AILED_USER_EXISTS (3005, "⽤⼾已存在"),
    FAILED_USER_NOT_EXISTS (3006, "⽤⼾不存在"),
    FAILED_LOGIN (3007, "密码错误"),
    FAILED_USER_BANNED (3008, "您已被列⼊⿊名单, 请联系管理员"),
    FAILED_USER_EMAIL_EXISTS(3009, "该邮箱已被注册"),

    // 登入操作
    USER_LOGIN_IS_NULL(3200, "用户名或密码不能为空"),
    FAILED_UPDATE(3201, "更新失败"),
    FAILED_CREATE(3202, "新增失败"),
    ARGUMENT_REGEX(3203,"您的账号或密码复杂度太低，设置中必须包含字母、数字，特殊字符请及时修改！"),
    USER_GET_MESSAGE_ERROR(3204,"用户获取个人信息失败"),
    EMAIL_SEND_ERROR(3205, "邮箱发送失败"),
    NOT_EMAIL_ACQUIRED(3206, "验证码未发送"),
    EMAIL_ERROR(3207, "验证码错误"),
    BULLETIN_SENDING_FAILURE(3208, "公告发送失败");

    // 状态码
    private int code;
    // 描述信息
    private String message;
}
