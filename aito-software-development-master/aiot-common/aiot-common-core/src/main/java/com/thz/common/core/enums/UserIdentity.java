package com.thz.common.core.enums;

import lombok.Getter;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/2
 */
@Getter
public enum UserIdentity {

    ORDINARY((byte) 1, "用户"),

    ADMIN((byte)2, "管理");

    private Byte value;

    private String des;

    UserIdentity(Byte value, String des) {
        this.value = value;
        this.des = des;
    }
}

