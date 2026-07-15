package com.thz.common.core.pojo.vo;

import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/5
 */
@Getter
@Setter
@Builder
public class LoginUserVO {

    private String identityName;

    private String nickName;

    private String avatarUrl;

}