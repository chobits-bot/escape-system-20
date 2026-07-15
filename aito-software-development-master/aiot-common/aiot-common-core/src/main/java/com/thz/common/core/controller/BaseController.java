package com.thz.common.core.controller;

import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.result.Result;
import lombok.extern.slf4j.Slf4j;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/3
 */
@Slf4j
public class BaseController {
    public Result IToResult(int ret) {
        if(ret != 1){
            log.warn(ResultCode.FAILED_CREATE.toString());
            Result.failed(ResultCode.FAILED_CREATE);
        }
        return Result.success();
    }

    public Result UToResult(int ret) {
        if(ret != 1){
            log.warn(ResultCode.FAILED_UPDATE.toString());
            Result.failed(ResultCode.FAILED_UPDATE);
        }
        return Result.success();
    }

    public Result ToResult(Boolean rows) {
        if(rows){
            return Result.success();
        }
        return Result.failed();
    }

}
