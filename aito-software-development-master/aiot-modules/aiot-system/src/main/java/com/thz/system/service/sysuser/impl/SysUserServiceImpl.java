package com.thz.system.service.sysuser.impl;

import cn.hutool.core.util.StrUtil;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.thz.common.core.constant.HttpConstants;
import com.thz.common.core.enums.UserIdentity;
import com.thz.common.core.pojo.LoginUser;
import com.thz.common.core.pojo.vo.LoginUserVO;
import com.thz.common.core.result.Result;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.utlis.MD5Util;
import com.thz.common.core.utlis.UUIDUtil;
import com.thz.exception.ApplicationException;
import com.thz.security.service.TokenService;
import com.thz.system.mapper.sysuer.SysUserMapper;
import com.thz.system.pojo.SysUser;
import com.thz.system.pojo.sysuser.dto.SysUserSaveDTO;
import com.thz.system.rabbit.producer.MailProducer;
import com.thz.system.service.sysuser.ISysUserService;
import com.thz.system.service.sysuser.service.UserRegisterService;
import com.thz.system.utlis.LoginUtils;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.cloud.context.config.annotation.RefreshScope;
import org.springframework.stereotype.Service;
import org.springframework.beans.factory.annotation.Value;

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@Slf4j
@Service
@RefreshScope
public class SysUserServiceImpl implements ISysUserService {
    @Autowired
    private SysUserMapper sysUserMapper;

    @Autowired
    private TokenService tokenService;

    @Autowired
    private UserRegisterService userRegisterService;

    @Autowired
    private MailProducer mailProducer;

    @Value("${jwt.secret}")
    private String secret;


    @Override
    public String login(String userAccount, String password) {
        // 1.1利用用户名去查找用户信息
        SysUser loginUser = sysUserMapper.selectSysUserByUserAccount(userAccount);

        // 1.2判断该用户有没有被注册
        if(loginUser == null){
            // 如果用户不存在
            log.warn(ResultCode.FAILED_USER_NOT_EXISTS.toString() + ", 用户名不存在 userAccount:" + userAccount);
            throw new ApplicationException(Result.failed(ResultCode.FAILED_USER_NOT_EXISTS));
        }

        // 2.1 判断密码是否正确
        // 2.2 用密文和数据库中存的用户密码进行比较
        LoginUtils.checkLoginPassword(password,loginUser.getPassword(),loginUser.getSalt());


        return tokenService.createToken(loginUser.getUserId(), secret,
                loginUser.getIdentity(),
                loginUser.getNickName(),
                loginUser.getAvatarUrl(),
                loginUser.getEmail());
    }

    @Override
    public Boolean logout(String token) {
        if (StrUtil.isNotEmpty(token) && token.startsWith(HttpConstants.PREFIX)) {
            token = token.replaceFirst(HttpConstants.PREFIX, StrUtil.EMPTY);
        }
        return tokenService.deleteLoginUser(token, secret);
    }

    @Override
    public int register(SysUserSaveDTO saveDTO) {
        // 1.校验参数 判断账号与密码是否符合规范
        LoginUtils.checkAddAndAddParameter(saveDTO.getUserAccount(),saveDTO.getPassword());

        // 2.判断该用户有没有被注册
        SysUser sysUser = sysUserMapper.selectSysUserByUserAccount(saveDTO.getUserAccount());

        if(sysUser != null){
            // 如果用户已存在
            log.warn(ResultCode.AILED_USER_EXISTS.toString() + ", 用户名已存在 userAccount:" + saveDTO.getUserAccount());
            throw new ApplicationException(Result.failed(ResultCode.AILED_USER_EXISTS));
        }

        // 3.校验邮箱验证码
        if (!userRegisterService.getCode(saveDTO.getEmail(),saveDTO.getCode())){
            log.warn(ResultCode.EMAIL_ERROR.toString() + ", 验证码错误 email:" + saveDTO.getEmail());
            throw new ApplicationException(Result.failed(ResultCode.EMAIL_ERROR));
        }

        // 4.1校验完成
        // 4.2生成盐(随机生成32个字符)
        String salt = UUIDUtil.UUID_32();
        // 4.3生成密码的密文
        String ciphertext = MD5Util.encryptPasswordSalt(saveDTO.getPassword(), salt);
        SysUser sysUserBuild = SysUser.builder()
                .userAccount(saveDTO.getUserAccount())
                .password(ciphertext)
                .salt(salt)
                .email(saveDTO.getEmail())
                .nickName(saveDTO.getNickName())
                .build();

        return sysUserMapper.insert(sysUserBuild);
    }

    @Override
    public String sendCode(String email) {
        String code = mailProducer.getCode(email);
        userRegisterService.sendCode(email,code);
        return code;
    }

    @Override
    public LoginUserVO info(String token) {
        LoginUser loginUser = tokenService.analyzeLoginUser(token,secret);
        String identityName = loginUser.getIdentity().equals(UserIdentity.ORDINARY.getValue())  ? UserIdentity.ORDINARY.getDes() : UserIdentity.ADMIN.getDes();
        return LoginUserVO.builder()
                .identityName(identityName)
                .nickName(loginUser.getNickName())
                .avatarUrl(loginUser.getAvatarUrl())
                .build();
    }

    @Override
    public List<SysUser> getAdminUser() {
        LambdaQueryWrapper<SysUser> queryWrapper = new LambdaQueryWrapper<>();
        return sysUserMapper.selectList(queryWrapper
                .select(SysUser::getEmail, SysUser::getNickName, SysUser::getUserAccount)
                .eq(SysUser::getIdentity, UserIdentity.ADMIN.getValue()));
    }
}
