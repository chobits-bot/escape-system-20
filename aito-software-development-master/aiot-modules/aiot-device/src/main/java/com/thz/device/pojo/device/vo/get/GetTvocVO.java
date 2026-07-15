package com.thz.device.pojo.device.vo.get;

import com.fasterxml.jackson.annotation.JsonFormat;
import io.swagger.v3.oas.annotations.media.Schema;
import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

import java.time.LocalDateTime;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/13
 */
@Setter
@Getter
@Builder
public class GetTvocVO {
    @Schema(description = "tvoc 浓度")
    private List<Integer> tvoc;
    @Schema(description = "创建时间")
    @JsonFormat(pattern = "yyyy-MM-dd HH:mm:ss")
    private List<LocalDateTime> createTime;
}
