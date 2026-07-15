package com.thz.exception.handler;

import cn.hutool.core.collection.CollUtil;
import cn.hutool.core.util.StrUtil;

import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.result.Result;
import com.thz.exception.ApplicationException;
import jakarta.servlet.http.HttpServletRequest;
import lombok.extern.slf4j.Slf4j;
import org.springframework.context.support.DefaultMessageSourceResolvable;
import org.springframework.dao.DuplicateKeyException;
import org.springframework.validation.BindException;
import org.springframework.web.HttpRequestMethodNotSupportedException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.util.Collection;
import java.util.Objects;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * 全局异常处理器
 */
@RestControllerAdvice
@Slf4j
public class GlobalExceptionHandler {
    /**
     * 请求⽅式不⽀持
     */
    @ExceptionHandler(HttpRequestMethodNotSupportedException.class)
    public Result<?>
    handleHttpRequestMethodNotSupported(HttpRequestMethodNotSupportedException e, HttpServletRequest request) {
        String requestURI = request.getRequestURI();
        log.error("请求地址'{}',不⽀持'{}'请求", requestURI, e.getMethod());
        return Result.failed(ResultCode.ERROR.toString());
    }

    /**
     * 拦截运⾏时异常
     */
    @ExceptionHandler(RuntimeException.class)
    public Result<?> handleRuntimeException(RuntimeException e, HttpServletRequest
            request) {
        String requestURI = request.getRequestURI();
        log.error("请求地址'{}',发⽣异常.", requestURI, e);
        return Result.failed(ResultCode.ERROR);
    }

    /**
     * 自定义异常
     */
    @ExceptionHandler(ApplicationException.class)
    public Result<?> ApplicationException(Exception e, HttpServletRequest request) {
        String requestURI = request.getRequestURI();
        log.error("请求地址'{}',发⽣异常.", requestURI, e);
        return Result.failed(e.getMessage());
    }

    @ResponseBody
    @ExceptionHandler(DuplicateKeyException.class)
    public Result SQLIntegrityHandler(DuplicateKeyException ex){
        String msg = ex.getMessage();
        if(msg.contains("Duplicate entry")){
            String[] split = msg.split(" ");
            String key = null;
            for(int i = 0 ; i < split.length ; i++){
                if(split[i].equals("entry")){
                    key = split[i+1];
                }
            }
            return Result.failed(key , ResultCode.FAILED_USER_EMAIL_EXISTS);
        }
        return Result.failed(ResultCode.ERROR);
    }

    /**
     * 系统异常
     */
    @ExceptionHandler(Exception.class)
    public Result<?> handleException(Exception e, HttpServletRequest request) {
        String requestURI = request.getRequestURI();
        log.error("请求地址'{}',发⽣异常.", requestURI, e);
        return Result.failed(ResultCode.ERROR);
    }

    @ExceptionHandler(BindException.class)
    public Result<Void> handleBindException(BindException e) {
        log.error(e.getMessage());
        String message = join(e.getAllErrors(),
                DefaultMessageSourceResolvable::getDefaultMessage, ", ");
        return Result.fail(ResultCode.FAILED_PARAMS_VALIDATE.getCode(), message);
    }

    private <E> String join(Collection<E> collection, Function<E, String>
            function, CharSequence delimiter) {
        if (CollUtil.isEmpty(collection)) {
            return StrUtil.EMPTY;
        }
        return collection.stream().map(function).filter(Objects::nonNull).collect(Collectors.joining(delimiter));
    }
}