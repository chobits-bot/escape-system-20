package com.thz.common.core.constant;

/**
 * @Description
 * @Author thz
 * @Date 2024/11/25
 */
public class RedisConstants {
    // 用户登入
    public static final String LOGIN_TOKEN_KEY = "login:token";
    public static final String REGISTER_CODE_KEY = "register:code";
    public static final String WARNING_USER_KEY = "warning:user";
    public static final Long LOGIN_TOKEN_TTL = 720L;
    public static final Long REFRESH_TTL = 180L;
    public static final Long LOGIN_CODE_TTL = 3L;
    public static final Long WARNING_USER_TTL = 180L;

    // 设备
    public static final String DEVICE_KEY = "device:macAddress";
    public static final Long DEVICE_TTL = 60L;
}