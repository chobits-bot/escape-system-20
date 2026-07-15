package com.thz.system.service.sysuser.service;

import com.thz.common.core.constant.RedisConstants;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.result.Result;
import com.thz.exception.ApplicationException;
import com.thz.redis.utlis.RedisUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.concurrent.TimeUnit;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/8
 */
@Service
public class UserRegisterService {

    @Autowired
    private RedisUtil redisUtil;

    public void sendCode(String email, String code) {
        // 保存验证码到 redis
        redisUtil.setCacheObject(getMailKey(email), code, RedisConstants.LOGIN_CODE_TTL, TimeUnit.MINUTES);
    }

    public Boolean getCode(String email,String code) {
        if (!redisUtil.hasKey(redisUtil.buildKey(RedisConstants.REGISTER_CODE_KEY,email))) {
            throw new ApplicationException(Result.failed(ResultCode.NOT_EMAIL_ACQUIRED));
        }
        return redisUtil.getCacheObject(getMailKey(email), String.class).equals(code);
    }

    private String getMailKey(String email) {
        return redisUtil.buildKey(RedisConstants.REGISTER_CODE_KEY,email);
    }
}
