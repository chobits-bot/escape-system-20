package com.thz.common.core.utlis;

import com.alibaba.fastjson2.JSON;
import lombok.extern.slf4j.Slf4j;
import org.springframework.util.StringUtils;

/**
 * @Description
 * @Author thz
 * @Date 2025/1/14
 */
@Slf4j
public class JsonUtil<T> {
    /**
     * 字符串转对象
     */
    public static <T> T parseJson(String json, Class<T> clazz){
        if (!StringUtils.hasLength(json) || clazz==null){
            return null;
        }
        try {
            return JSON.parseObject(json, clazz);
        }catch (Exception e){
            log.error("JsonUtil parseJson error, e:", e);
            return null;
        }

    }

    /**
     * 对象转字符串
     */
    public static String toJson(Object o){
        try{
            return o==null ? null: JSON.toJSONString(o);
        }catch (Exception e){
            log.error("JsonUtil toJson error, e:", e);
            return null;
        }
    }
}
