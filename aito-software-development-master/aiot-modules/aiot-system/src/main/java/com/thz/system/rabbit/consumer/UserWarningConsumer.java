package com.thz.system.rabbit.consumer;

import com.rabbitmq.client.Channel;
import com.thz.common.core.constant.DeviceConstants;
import com.thz.common.core.constant.RabbitConstants;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.common.core.result.Result;
import com.thz.common.core.utlis.JsonUtil;
import com.thz.exception.ApplicationException;
import com.thz.system.pojo.SysUser;
import com.thz.rabbitmq.producer.MailUtil;
import com.thz.system.service.sysuser.ISysUserService;
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

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/17
 */
@Slf4j
@Component
public class UserWarningConsumer {

    @Autowired
    private MailUtil mailUtil;


    @Autowired
    private ISysUserService sysUserService;


    @Value("${spring.mail.properties.warning_subject}")
    private String subject;

    @RabbitListener(bindings = @QueueBinding(
            value = @Queue(value = RabbitConstants.AIOT_WARNING_QUEUE , durable = "true"),
            exchange = @Exchange(value = RabbitConstants.AIOT_WARNING_EXCHANGE , type = ExchangeTypes.FANOUT),
            key = {RabbitConstants.AIOT_WARNING_KEY}
    ))
    public void ConsumerMessage(Message message, Channel channel) throws Exception {
        long deliveryTag = message.getMessageProperties().getDeliveryTag();
        try {
            String body = new String(message.getBody());
            DeviceWarningDTO deviceWarningDTO = JsonUtil.parseJson(body, DeviceWarningDTO.class);

            List<SysUser> sysUserList = sysUserService.getAdminUser();
            if(sysUserList == null){
                throw new ApplicationException(Result.failed(ResultCode.BULLETIN_SENDING_FAILURE));
            }

            if (deviceWarningDTO != null) {
                for (SysUser sysUser : sysUserList) {
                    mailUtil.sendMail(sysUser.getEmail(),subject,buildSendWarning(deviceWarningDTO,sysUser.getNickName()));
                }
            }
            // 确认
            channel.basicAck(deliveryTag, true);
        }catch (Exception e){
            // 否定确认
            channel.basicNack(deliveryTag, true, true);
            log.error("报警发送失败, e:", e);
        }
    }


    public String buildSendWarning(DeviceWarningDTO deviceWarningDTO, String nickName) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("<h1>").append("尊敬的").append(nickName).append("管理员你好").append("</h1>");
        stringBuilder.append("<br>").append("当前").append(deviceWarningDTO.getMacAddress())
                     .append("设备检测到");
        if(deviceWarningDTO.getElectricityAlarm().equals(DeviceConstants.FOREWARNING)){
            stringBuilder.append("漏电报警，");
        }
        if(deviceWarningDTO.getFireAlarm().equals(DeviceConstants.FOREWARNING)){
            stringBuilder.append("火灾报警，");
        }
        stringBuilder.append("请及时处理").append("</br>");
        return stringBuilder.toString();
    }
}
