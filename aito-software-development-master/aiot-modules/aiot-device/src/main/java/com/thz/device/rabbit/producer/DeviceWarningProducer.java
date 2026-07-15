package com.thz.device.rabbit.producer;

import com.thz.common.core.constant.RabbitConstants;
import com.thz.common.core.constant.RedisConstants;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.common.core.result.Result;
import com.thz.common.core.utlis.JsonUtil;
import com.thz.exception.ApplicationException;
import com.thz.redis.utlis.RedisUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.TimeUnit;

import static java.util.Objects.requireNonNull;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/17
 */
@Slf4j
@Component
public class DeviceWarningProducer {
    @Autowired
    private RabbitTemplate rabbitTemplate;

    @Autowired
    private RedisUtil redisUtil;

    public void produceMessage(DeviceWarningDTO deviceWarningDTO) {
        try {
            rabbitTemplate.convertAndSend(RabbitConstants.AIOT_WARNING_EXCHANGE, RabbitConstants.AIOT_WARNING_KEY ,requireNonNull(JsonUtil.toJson(deviceWarningDTO)));
            String userKey = redisUtil.buildKey(RedisConstants.WARNING_USER_KEY,deviceWarningDTO.getMacAddress());
            redisUtil.setCacheObject(userKey,"",RedisConstants.WARNING_USER_TTL, TimeUnit.MINUTES);
        } catch (Exception e) {
            log.error("生产者发送消息异常", e);
            throw new ApplicationException((Result.failed(ResultCode.BULLETIN_SENDING_FAILURE)));
        }
    }

}
