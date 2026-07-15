package com.thz.device.config;

import com.thz.device.pojo.board.Board;
import lombok.Data;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/13
 */
@Data
@Component
@ConfigurationProperties(prefix = "board")
public class BoardNameConfig {
    private List<Board> boardList;
}
