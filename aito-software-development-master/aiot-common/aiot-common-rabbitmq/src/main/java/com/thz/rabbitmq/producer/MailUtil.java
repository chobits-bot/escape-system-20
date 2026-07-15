package com.thz.rabbitmq.producer;

import jakarta.mail.MessagingException;
import jakarta.mail.internet.MimeMessage;
import org.springframework.boot.autoconfigure.mail.MailProperties;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.MimeMessageHelper;

import java.io.UnsupportedEncodingException;
import java.util.Optional;

/**
 * @Description
 * @Author thz
 * @Date 2025/1/14
 */
public class MailUtil {
    private final JavaMailSender javaMailSender;
    private final MailProperties mailProperties;

    public MailUtil(JavaMailSender javaMailSender, MailProperties mailProperties) {
        this.javaMailSender = javaMailSender;
        this.mailProperties = mailProperties;
    }

    public void sendMail(String to, String subject, String content) throws MessagingException, UnsupportedEncodingException {
        MimeMessage mimeMessage = javaMailSender.createMimeMessage();
        MimeMessageHelper helper = new MimeMessageHelper(mimeMessage,false);
        // 设置收件人
        helper.setTo(to);
        // 设置发送人
        String personal = Optional.ofNullable(mailProperties.getProperties().get("personal")).orElse(mailProperties.getUsername());
        helper.setFrom(mailProperties.getUsername(), personal);
        // 设置邮件主题
        helper.setSubject(subject);
        helper.setText(content, true);
        javaMailSender.send(mimeMessage);
    }

}
