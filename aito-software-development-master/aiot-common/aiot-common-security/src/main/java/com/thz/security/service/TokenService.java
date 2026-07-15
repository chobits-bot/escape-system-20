package com.thz.security.service;

import cn.hutool.core.util.StrUtil;
import com.thz.common.core.constant.HttpConstants;
import com.thz.common.core.constant.JwtConstants;
import com.thz.common.core.constant.RedisConstants;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.pojo.LoginUser;
import com.thz.common.core.result.Result;
import com.thz.common.core.utlis.JwtUtils;
import com.thz.common.core.utlis.UUIDUtil;
import com.thz.exception.ApplicationException;
import com.thz.redis.utlis.RedisUtil;
import io.jsonwebtoken.Claims;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/2
 */
@Slf4j
@Service
public class TokenService {
    @Autowired
    private RedisUtil redisUtil;

    public String createToken(Long userId, String secret, Byte identity, String nickName, String avatarUrl, String email) {
        Map<String, Object> claims = new HashMap<>();
        String userKey = UUIDUtil.UUID_32();
        // 1.1随机生成 token 作为登入令牌
        claims.put(JwtConstants.LOGIN_USER_ID, userId);
        claims.put(JwtConstants.LOGIN_USER_KEY, userKey);
        String token = JwtUtils.createToken(claims, secret);
        LoginUser loginUserBuild = LoginUser.builder()
                .identity(identity)
                .nickName(nickName)
                .avatarUrl(avatarUrl)
                .email(email)
                .build();
        // 1.2存储
        String tokenKey = getTokenKey(userKey);
        // 2.1将用户信息存入 redis
        redisUtil.setCacheObject(tokenKey, loginUserBuild, RedisConstants.LOGIN_TOKEN_TTL, TimeUnit.MINUTES);
        return token;
    }


    // 延长token的有效时间，就是延长redis当中从存储的用于用户身份认证的敏感信息的有效时间
    public void extendToken(Claims claims) {
        String userKey = getUserKey(claims);
        if (userKey == null) {
            return;
        }
        // 从JWT令牌中获取token中UUID标识符
        String tokenKey = getTokenKey(userKey);
        // 720min  12个小时      剩余  180min 时候对它进行延长
        Long expire = redisUtil.getExpire(tokenKey, TimeUnit.MINUTES);
        if (expire != null && expire < RedisConstants.REFRESH_TTL) {
            redisUtil.expire(tokenKey, RedisConstants.LOGIN_TOKEN_TTL, TimeUnit.MINUTES);
        }
    }

    private String getTokenKey(String token) {
        return redisUtil.buildKey(RedisConstants.LOGIN_TOKEN_KEY,token);
    }

    // 获取jwt中的key
    public Long getUserId(Claims claims) {
        if (claims == null) return null;
        return Long.valueOf(JwtUtils.getUserId(claims));
    }

    // 获取jwt中的key
    public String getUserKey(Claims claims) {
        if (claims == null) return null;
        return JwtUtils.getUserKey(claims);
    }

    // 获取jwt中的key
    private String getUserKey(String token, String secret) {
        Claims claims = getClaims(token, secret);
        if (claims == null) return null;
        return JwtUtils.getUserKey(claims);
    }

    // 获取令牌中信息
    public Claims getClaims(String token, String secret) {
        Claims claims;
        try {
            claims = JwtUtils.parseToken(token, secret);
            if (claims == null) {
                log.error("解析token：{}, 出现异常", token);
                return null;
            }
        } catch (Exception e) {
            log.error("解析token：{}, 出现异常", token, e);
            return null;
        }
        return claims;
    }

    public LoginUser getLoginUser(String token, String secret) {
        String userKey = getUserKey(token, secret);
        if (userKey == null) {
            return null;
        }
        return redisUtil.getCacheObject(getTokenKey(userKey), LoginUser.class);
    }

    public boolean deleteLoginUser(String token, String secret) {
        String userKey = getUserKey(token, secret);
        if (userKey == null) {
            return false;
        }
        return redisUtil.deleteObject(getTokenKey(userKey));
    }

    public LoginUser analyzeLoginUser(String token,String secret) {
        // 1.截取token并且解析
        if (StrUtil.isNotEmpty(token) && token.startsWith(HttpConstants.PREFIX)) {
            token = token.replaceFirst(HttpConstants.PREFIX, StrUtil.EMPTY);
        }
        // 2.解析token
        LoginUser loginUser = getLoginUser(token, secret);
        if (loginUser == null) {
            log.warn(ResultCode.USER_GET_MESSAGE_ERROR.toString());
            throw new ApplicationException(Result.failed(ResultCode.USER_GET_MESSAGE_ERROR));
        }
        return loginUser;
    }

}
