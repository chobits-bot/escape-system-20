package com.thz.exception;

import com.thz.common.core.result.Result;
import lombok.Getter;

@Getter
public class ApplicationException extends RuntimeException {

    // 在异常中持有一个错误信息对象
    protected Result errorResult;

    /**
     * 构造方法
     * @param errorResult
     */
    public ApplicationException(Result errorResult) {
        super(errorResult.getMessage());
        this.errorResult = errorResult;
    }

    public ApplicationException(String message) {
        super(message);
    }

    public ApplicationException(String message, Throwable cause) {
        super(message, cause);
    }

    public ApplicationException(Throwable cause) {
        super(cause);
    }

}
