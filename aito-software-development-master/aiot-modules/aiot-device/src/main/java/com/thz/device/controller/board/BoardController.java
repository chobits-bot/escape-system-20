package com.thz.device.controller.board;

import com.thz.common.core.result.Result;
import com.thz.device.pojo.board.Board;
import com.thz.device.service.board.IBoardService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/13
 */
@RestController
@RequestMapping("/board")
@Tag(name="模块")
@Slf4j
public class BoardController {

    @Autowired
    private IBoardService boardService;

    @Operation(summary = "模块列表展示", description = "模块列表展示")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/topList")
    public Result<List<Board>> topList () {
        List<Board> boardList =  boardService.getBoards();
        return Result.ok(boardList);
    }
}
