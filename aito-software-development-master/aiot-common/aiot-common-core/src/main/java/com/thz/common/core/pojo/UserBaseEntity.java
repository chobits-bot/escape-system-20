package com.thz.common.core.pojo;

import com.baomidou.mybatisplus.annotation.FieldFill;
import com.baomidou.mybatisplus.annotation.TableField;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import java.time.LocalDateTime;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/26
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class UserBaseEntity {
    /*
     * 创建时间
     */
    @TableField(fill = FieldFill.INSERT)
    private LocalDateTime createTime;

    /*
     * 注销 0否,1是
     */
    @TableField(fill = FieldFill.INSERT)
    private Byte deleteState;

    /*
     * 注销 1 - 用户, 2 - 管理
     */
    @TableField(fill = FieldFill.INSERT)
    private Byte identity;

    // 初始头像 https://dongfangs.oss-cn-beijing.aliyuncs.com/forum/avatar01.jpeg
    @TableField(fill = FieldFill.INSERT)
    private String avatarUrl;

    /*
     * 更新时间
     */
    @TableField(fill = FieldFill.UPDATE)
    private LocalDateTime updateTime;
}
