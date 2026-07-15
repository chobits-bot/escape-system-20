package com.thz.device.task;

import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Async;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/18
 */
@Slf4j
@Component
public class MultiThreadTask {

    @Autowired
    private WarningTask warningTask;

    @Scheduled(cron ="*/6 * * * * ?")
    public void execute() {
        warningTask.Task();
    }

}
