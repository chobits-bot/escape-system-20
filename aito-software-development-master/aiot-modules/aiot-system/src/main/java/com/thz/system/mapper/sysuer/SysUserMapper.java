package com.thz.system.mapper.sysuer;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.thz.system.pojo.SysUser;
import org.apache.ibatis.annotations.Mapper;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@Mapper
public interface SysUserMapper extends BaseMapper<SysUser> {
    SysUser selectSysUserByUserAccount(String userAccount);
}
