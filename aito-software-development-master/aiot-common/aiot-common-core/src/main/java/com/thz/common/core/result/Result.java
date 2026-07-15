package com.thz.common.core.result;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.thz.common.core.enums.ResultCode;
import lombok.*;

/**
 * @Description
 * @Author thz
 * @Date 2024/12/12
 */
@Data
@NoArgsConstructor
public class Result<T> {
    // 状态码
    // 不论任何情况都参与JSON序列化
    @JsonInclude(JsonInclude.Include.ALWAYS)
    private int code;
    // 描述信息
    @JsonInclude(JsonInclude.Include.ALWAYS)
    private String message;
    // 具体的数据
    @JsonInclude(JsonInclude.Include.ALWAYS)
    private T data;

    /**
     * 构造方法
     * @param code
     * @param message
     */
    public Result(int code, String message) {
        this(code, message, null);
    }

    public Result(int code, String message, T data) {
        this.code = code;
        this.message = message;
        this.data = data;
    }

    /**
     * 成功
     * @return
     */
    public static Result success () {
        return new Result(ResultCode.SUCCESS.getCode(), ResultCode.SUCCESS.getMessage());
    }

    public static Result success (String message) {
        return new Result(ResultCode.SUCCESS.getCode(), message);
    }

    public static <T> Result<T> ok (T data) {
        return new Result<>(ResultCode.SUCCESS.getCode(), ResultCode.SUCCESS.getMessage(), data);
    }

    public static <T> Result<T> fail (String message, T data) {
        return new Result<>(ResultCode.SUCCESS.getCode(), message, data);
    }

    public static <T> Result<T> fail (int code,String message) {
        return new Result<>(code, message,null);
    }

    /**
     * 失败
     * @return
     */
    public static Result failed () {
        return new Result(ResultCode.FAILED.getCode(), ResultCode.FAILED.getMessage());
    }

    public static Result failed (String message) {
        return new Result(ResultCode.FAILED.getCode(), message);
    }

    public static Result failed (String key, ResultCode resultCode) {
        return new Result(resultCode.getCode(),key + resultCode.getMessage());
    }

    public static Result failed (ResultCode resultCode) {
        return new Result(resultCode.getCode(), resultCode.getMessage());
    }
}
