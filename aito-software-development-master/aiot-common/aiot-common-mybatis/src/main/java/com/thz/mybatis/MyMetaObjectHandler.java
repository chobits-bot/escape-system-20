package com.thz.mybatis;

import com.baomidou.mybatisplus.core.handlers.MetaObjectHandler;
import com.thz.common.core.enums.UserIdentity;
import org.apache.ibatis.reflection.MetaObject;
import org.springframework.stereotype.Component;

import java.time.LocalDateTime;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/3
 */
@Component
public class MyMetaObjectHandler implements MetaObjectHandler {
    @Override
    public void insertFill(MetaObject metaObject) {
        this.strictInsertFill(metaObject, "createTime", LocalDateTime.class, LocalDateTime.now());
        this.strictInsertFill(metaObject, "deleteState", Byte.class, (byte) 0);
        this.strictInsertFill(metaObject, "identity", Byte.class, UserIdentity.ORDINARY.getValue());
        this.strictInsertFill(metaObject, "avatarUrl", String.class, "https://dongfangs.oss-cn-beijing.aliyuncs.com/forum/avatar01.jpeg");
    }

    @Override
    public void updateFill(MetaObject metaObject) {
        this.strictUpdateFill(metaObject, "updateTime", LocalDateTime.class, LocalDateTime.now());
    }
}
