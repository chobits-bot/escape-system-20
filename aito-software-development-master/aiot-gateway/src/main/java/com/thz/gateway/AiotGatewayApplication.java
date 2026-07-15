package com.thz.gateway;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/26
 */
@SpringBootApplication(exclude = { DataSourceAutoConfiguration.class })
public class AiotGatewayApplication {
    public static void main(String[] args) {
        SpringApplication.run(AiotGatewayApplication.class, args);
    }
}
