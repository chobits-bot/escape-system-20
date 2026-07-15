package com.thz.device.service.board.impl;

import com.thz.device.config.BoardNameConfig;
import com.thz.device.pojo.board.Board;
import com.thz.device.pojo.device.vo.DeviceMacAddressVO;
import com.thz.device.service.board.IBoardService;
import lombok.Setter;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/13
 */
@Service
public class BoardServiceImpl implements IBoardService {

    @Autowired
    private BoardNameConfig boardNameConfig;

    @Override
    public List<Board> getBoards() {
        List<Board> boardList = boardNameConfig.getBoardList();
        if (boardList == null) {
            boardList = new ArrayList<>();
        }
        return boardList;
    }
}
