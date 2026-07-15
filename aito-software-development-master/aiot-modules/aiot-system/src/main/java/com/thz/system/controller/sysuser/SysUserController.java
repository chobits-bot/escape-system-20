package com.thz.system.controller.sysuser;

import com.thz.common.core.constant.HttpConstants;
import com.thz.common.core.pojo.vo.LoginUserVO;
import com.thz.common.core.result.Result;
import com.thz.common.core.controller.BaseController;
import com.thz.system.pojo.sysuser.dto.SysUserEmailDTO;
import com.thz.system.pojo.sysuser.dto.SysUserLoginDTO;
import com.thz.system.pojo.sysuser.dto.SysUserSaveDTO;
import com.thz.system.service.sysuser.ISysUserService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@RestController
@RequestMapping("/sysUser")
@Tag(name="用户接口文档")
@Slf4j
public class SysUserController extends BaseController {

    @Autowired
    private ISysUserService sysUserService;

    @Operation(summary = "用户登入", description = "根据账号和密码登入后台管理")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @PostMapping("/login")
    public Result<String> UserLogin(@Validated SysUserLoginDTO userLoginDTO){
        // 实现登入功能
        return Result.ok(sysUserService.login(userLoginDTO.getUserAccount(),userLoginDTO.getPassword()));
    }


    @Operation(summary = "用户登出", description = "解析token获取JWT令牌中的userKey删除redis中的信息")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "3000", description = "操作失败")
    @DeleteMapping("/logout")
    public Result logout(@RequestHeader(HttpConstants.AUTHENTICATION) String token) {
        return ToResult(sysUserService.logout(token));
    }


    @Operation(summary = "新增用户", description = "根据提供的信息新增⽤⼾")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @ApiResponse(responseCode = "3005", description = "⽤⼾已存在")
    @PostMapping("/register")
    public Result register(@Validated SysUserSaveDTO saveDTO) {
        return IToResult(sysUserService.register(saveDTO));
    }

    @Operation(summary = "获取当前用户信息", description = "解析token获取JWT令牌中的用户信息")
    @GetMapping("/info")
    public Result<LoginUserVO> info(@RequestHeader(HttpConstants.AUTHENTICATION) String token) {
        return Result.ok(sysUserService.info(token)) ;
    }

    @Operation(summary = "获取邮箱验证码", description = "通过用户提供的邮箱信息获取注册权限")
    @GetMapping("/code")
    public Result<String> sendCode(@Validated SysUserEmailDTO emailDTO) {
        // 发送邮箱验证码并保存验证码
        return Result.ok(sysUserService.sendCode(emailDTO.getEmail()));
    }
}
