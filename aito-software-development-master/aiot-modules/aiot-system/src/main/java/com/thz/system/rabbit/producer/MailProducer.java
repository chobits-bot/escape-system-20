package com.thz.system.rabbit.producer;

import cn.hutool.core.util.RandomUtil;
import com.thz.common.core.constant.RabbitConstants;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.pojo.dto.SysUserEmailDTO;
import com.thz.common.core.result.Result;
import com.thz.common.core.utlis.JsonUtil;
import com.thz.exception.ApplicationException;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import static java.util.Objects.requireNonNull;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/8
 */
@Slf4j
@Component
public class MailProducer {

    @Autowired
    private RabbitTemplate rabbitTemplate;

    public String getCode(String email) {
        // 校验符合，生成验证码
        String code = RandomUtil.randomNumbers(6);
        SysUserEmailDTO sysUserEmailDTO = SysUserEmailDTO.builder()
                .email(email)
                .code(code).build();
        try {
            rabbitTemplate.convertAndSend(RabbitConstants.AIOT_MAIL_EXCHANGE, RabbitConstants.AIOT_MAIL_KEY, requireNonNull(JsonUtil.toJson(sysUserEmailDTO)));
        } catch (Exception e) {
            log.warn(ResultCode.EMAIL_SEND_ERROR.toString());
            throw new ApplicationException(Result.failed(ResultCode.EMAIL_SEND_ERROR));
        }
        return code;
    }

}
