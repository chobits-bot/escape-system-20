package com.thz.system.utlis;

import com.thz.common.core.result.Result;
import com.thz.common.core.enums.ResultCode;
import com.thz.common.core.constant.RegexConstants;
import com.thz.exception.ApplicationException;
import lombok.extern.slf4j.Slf4j;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static com.thz.common.core.utlis.MD5Util.matchesPassword;

/**
 * @Description
 * @Author thz
 * @Date 2025/2/28
 */
@Slf4j
public class LoginUtils {

    public static void  checkAddAndAddParameter(String userAccount, String password){
        if (!(checkUserAccountAndPassword(userAccount) && checkUserAccountAndPassword(password))){
            // 抛出异常
            throw new ApplicationException(Result.failed(ResultCode.ARGUMENT_REGEX));
        }
    }

    private static boolean checkUserAccountAndPassword(String argument){
        Pattern pattern = Pattern.compile(RegexConstants.regEx2);
        Matcher matcher = pattern.matcher(argument);
        return matcher.matches();
    }

    public static void checkLoginPassword(String rawPassword, String encodedPassword, String salt){
        if (!matchesPassword(rawPassword,encodedPassword,salt)) {
            // 打印日志
            log.warn(ResultCode.FAILED_LOGIN.toString() + ", 密码错误 rawPassword = " + rawPassword);
            // 抛出异常
            throw new ApplicationException(Result.failed(ResultCode.FAILED_LOGIN));
        }
    }

}
