package com.thz.common.core.utlis;

import java.util.UUID;

/*
 * @description: 生成扰动字符的随机数
 * @author: thz
 * @date: 2024/12/12 22:48
 * @param:
 * @return:
 **/
public class UUIDUtil {

    /**
     * 生成一个标准的UUID
     *
     * @return
     */
    public static String UUID_36 () {
        return UUID.randomUUID().toString();
    }

    /**
     * 生成一个32位的UUID
     * @return
     */
    public static String UUID_32 () {
        return UUID.randomUUID().toString().replace("-", "");
    }
}
