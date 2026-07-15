package com.thz.common.core.constant;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/4
 */
public class RegexConstants {
    // 密码必须包含字母、数字和特殊字符
    public static final String regEx2 = "^(?![0-9a-zA-Z]+$)[a-zA-Z0-9~!@#$%^&*?_-]{1,50}$";
}
