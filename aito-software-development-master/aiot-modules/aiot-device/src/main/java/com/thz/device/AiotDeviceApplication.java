package com.thz.device;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/26
 */
@EnableScheduling
@SpringBootApplication
@MapperScan("com.thz.**.mapper")
public class AiotDeviceApplication {
    public static void main(String[] args) {
        SpringApplication.run(AiotDeviceApplication.class, args);
    }
}
