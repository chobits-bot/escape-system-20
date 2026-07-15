package com.thz.device.pojo.device.vo.get;

import com.fasterxml.jackson.annotation.JsonFormat;
import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.math.BigDecimal;
import java.time.LocalDateTime;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/11
 */
@Builder
@Data
@AllArgsConstructor
@NoArgsConstructor
public class GetTemperatureVO {
    @Schema(description = "温度")
    private List<BigDecimal> temperature;
    @Schema(description = "创建时间")
    @JsonFormat(pattern = "yyyy-MM-dd HH:mm:ss")
    private List<LocalDateTime> createTime;
}
