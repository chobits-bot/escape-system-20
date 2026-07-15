package com.thz.security.interceptor;


import cn.hutool.core.util.StrUtil;
import com.thz.common.core.constant.HttpConstants;
import com.thz.common.core.pojo.LoginUser;
import com.thz.security.service.TokenService;
import io.jsonwebtoken.Claims;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;
import org.springframework.web.servlet.HandlerInterceptor;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/3
 */
@Slf4j
@Component
public class TokenInterceptor implements HandlerInterceptor {

    @Autowired
    private TokenService tokenService;

    @Value("${jwt.secret}")
    private String secret;

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {
        String token = getToken(request);  //请求头中获取token
        if (StrUtil.isEmpty(token)) {
            return true;
        }
        Claims claims = tokenService.getClaims(token, secret);
        tokenService.extendToken(claims);
        return true;
    }

    private String getToken(HttpServletRequest request) {
        // 获取请求头的JWT令牌
        String token = request.getHeader(HttpConstants.AUTHENTICATION);
        // 如果前端设置了令牌前缀，则裁剪掉前缀
        if (StrUtil.isNotEmpty(token) && token.startsWith(HttpConstants.PREFIX)) {
            token = token.replaceFirst(HttpConstants.PREFIX, StrUtil.EMPTY);
        }
        return token;
    }
}
