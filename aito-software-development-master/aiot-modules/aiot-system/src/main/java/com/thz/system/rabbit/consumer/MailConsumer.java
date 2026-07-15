package com.thz.system.rabbit.consumer;

import com.rabbitmq.client.Channel;
import com.thz.common.core.constant.RabbitConstants;
import com.thz.common.core.constant.RedisConstants;
import com.thz.common.core.pojo.dto.SysUserEmailDTO;
import com.thz.common.core.utlis.JsonUtil;
import com.thz.rabbitmq.producer.MailUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.amqp.core.ExchangeTypes;
import org.springframework.amqp.core.Message;
import org.springframework.amqp.rabbit.annotation.Exchange;
import org.springframework.amqp.rabbit.annotation.Queue;
import org.springframework.amqp.rabbit.annotation.QueueBinding;
import org.springframework.amqp.rabbit.annotation.RabbitListener;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/16
 */
@Slf4j
@Component
public class MailConsumer {
    @Autowired
    private MailUtil mailUtil;

    @Value("${spring.mail.properties.register_subject}")
    private String subject;

    @RabbitListener(bindings = @QueueBinding(
            value = @Queue(value = RabbitConstants.AIOT_MAIL_QUEUE , durable = "true"),
            exchange = @Exchange(value = RabbitConstants.AIOT_MAIL_EXCHANGE , type = ExchangeTypes.FANOUT),
            key = {RabbitConstants.AIOT_MAIL_KEY}
    ))
    public void handler(Message message, Channel channel) throws Exception {
        long deliveryTag = message.getMessageProperties().getDeliveryTag();
        try {
            String body = new String(message.getBody());
            SysUserEmailDTO sysUserEmailVO = JsonUtil.parseJson(body, SysUserEmailDTO.class);
            if(sysUserEmailVO != null) {
                mailUtil.sendMail(sysUserEmailVO.getEmail(),subject,buildContent(sysUserEmailVO.getCode()));
            }
            //确认
            channel.basicAck(deliveryTag, true);
        }catch (Exception e){
            //否定确认
            channel.basicNack(deliveryTag, true, true);
            log.error("邮件发送失败, e:", e);
        }
    }

    private String buildContent(String code){
        return  "<br>" + "感谢您将注册成为我们AIOT的一员! 我们很高兴，您加入我们的团队!" + "<br/>" +
                "<br>" + "您的验证码为" + code + "。" + RedisConstants.LOGIN_CODE_TTL + "分钟内有效，请勿泄露和转发。如非本人操作，请忽略此短信" + "<br/>" +
                "<br>" + "为了确保您的账户安全，请妥善保管您的验证码。如果使⽤过程中, 遇到任何问题, 欢迎联系我们的⽀持团队。" + "<br/>" +
                "<br>" + "AIOT团队" + "<br/>";
    }
}
