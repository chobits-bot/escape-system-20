package com.thz.common.core.utlis;


import org.apache.commons.codec.digest.DigestUtils;

/**
 * 加密算法⼯具类
 */
public class MD5Util {
    /**
     * ⽣成加密后密⽂
     *
     * @param password 密码
     * @return 加密字符串
     */
    public static String encryptPassword(String password) {
        return DigestUtils.md5Hex(password);
    }

    /**
     * 对用户密码进行二次加密
     * @param password 密码明文
     * @param salt     扰动字符
     * @return 密文
     */
    public static String encryptPasswordSalt (String password,String salt) {
        return encryptPassword(encryptPassword(password) + salt);
    }

    /**
     * 判断密码是否相同
     *
     * @param rawPassword 真实密码
     * @param encodedPassword 加密后密⽂
     * @return 结果
     */
    public static boolean matchesPassword(String rawPassword, String encodedPassword,String salt) {
        rawPassword = encryptPasswordSalt(rawPassword,salt);
        return rawPassword.equals(encodedPassword);
    }
}
