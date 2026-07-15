package com.thz.system.service.sysuser;

import com.thz.common.core.pojo.vo.LoginUserVO;
import com.thz.system.pojo.SysUser;
import com.thz.system.pojo.sysuser.dto.SysUserSaveDTO;

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
public interface ISysUserService {

    String login(String userAccount, String password);

    int register(SysUserSaveDTO saveDTO);

    LoginUserVO info(String token);

    Boolean logout(String token);

    String sendCode(String email);

    List<SysUser> getAdminUser();
}
