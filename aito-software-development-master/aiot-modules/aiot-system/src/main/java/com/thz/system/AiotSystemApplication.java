package com.thz.system;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/26
 */
@SpringBootApplication
@MapperScan("com.thz.**.mapper")
public class AiotSystemApplication {
    public static void main(String[] args) {
        SpringApplication.run(AiotSystemApplication.class, args);
    }
}
