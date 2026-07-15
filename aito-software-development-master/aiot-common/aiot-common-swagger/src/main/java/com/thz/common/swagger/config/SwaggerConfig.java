package com.thz.common.swagger.config;

import io.swagger.v3.oas.models.OpenAPI;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import io.swagger.v3.oas.models.info.Info;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@Configuration
public class SwaggerConfig {
    @Bean
    public OpenAPI openAPI() {
        return new OpenAPI()
                .info(new Info()
                        .title("AIOT消防安全管理系统")
                        .description("AIOT消防安全管理系统接⼝⽂档")
                        .version("v1"));
    }
}
