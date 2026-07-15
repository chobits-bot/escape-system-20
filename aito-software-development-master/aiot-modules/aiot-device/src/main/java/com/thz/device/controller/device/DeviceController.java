package com.thz.device.controller.device;

import com.thz.common.core.pojo.dto.DeviceWarningDTO;
import com.thz.common.core.result.Result;
import com.thz.device.pojo.device.dto.DeviceGetTimeDTO;
import com.thz.device.pojo.device.vo.*;
import com.thz.device.pojo.device.vo.get.GetEco2VO;
import com.thz.device.pojo.device.vo.get.GetHumidityVO;
import com.thz.device.pojo.device.vo.get.GetTemperatureVO;
import com.thz.device.pojo.device.vo.get.GetTvocVO;
import com.thz.device.service.device.IDeviceService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.tags.Tag;
import lombok.NonNull;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * @Description
 * @Author thz
 * @Date 2025/3/10
 */
@RestController
@RequestMapping("/device")
@Tag(name="设备信息管理")
@Slf4j
public class DeviceController {

    @Autowired
    private IDeviceService deviceService;

    @Operation(summary = "设备信息列表展示", description = "查找设备名称")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/list")
    public Result<List<DeviceMacAddressVO>> list(@NonNull Integer boardId){
        return Result.ok(deviceService.list(boardId));
    }

    @Operation(summary = "获取当前设备详细信息", description = "根据设备名称查找当前设备信息")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/details")
    public Result<DeviceDetailsVO> details(@NonNull String macAddress){
        return Result.ok(deviceService.details(macAddress));
    }

    @Operation(summary = "获取设备温度的折线图像", description = "根据设备名称查找对应温度的折现图")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/temperature")
    public Result<GetTemperatureVO> getTemperature(@Validated DeviceGetTimeDTO deviceGetTimeDTO){
        return Result.ok(deviceService.getTemperature(deviceGetTimeDTO));
    }

    @Operation(summary = "获取设备湿度的折线图像", description = "根据设备名称查找对应湿度的折现图")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/humidity")
    public Result<GetHumidityVO> getHumidity(@Validated DeviceGetTimeDTO deviceGetTimeDTO){
        return Result.ok(deviceService.getHumidity(deviceGetTimeDTO));
    }

    @Operation(summary = "获取tvoc浓度折线图像", description = "根据设备名称查找对应tvoc浓度的折现图")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/tvoc")
    public Result<GetTvocVO> getTvoc(@Validated DeviceGetTimeDTO deviceGetTimeDTO){
        return Result.ok(deviceService.getTvoc(deviceGetTimeDTO));
    }

    @Operation(summary = "获取eco2浓度折线图像", description = "根据设备名称查找对应eco2浓度的折现图")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/eco2")
    public Result<GetEco2VO> getEco2(@Validated DeviceGetTimeDTO deviceGetTimeDTO){
        return Result.ok(deviceService.getEco2(deviceGetTimeDTO));
    }

    @Operation(summary = "通过redis的信息检测设备是否报警", description = "如果报警就放出公告")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/warningReportList")
    public Result<List<String>> warningReportList(){
        return Result.ok(deviceService.warningReportList());
    }

    @Operation(summary = "通过redis的信息检测设备是否报警", description = "如果报警就放出公告")
    @ApiResponse(responseCode = "1000", description = "操作成功")
    @ApiResponse(responseCode = "2000", description = "服务繁忙请稍后重试")
    @GetMapping("/warningReport")
    public Result<String> warningReport(@NonNull String macAddress){
        return Result.ok(deviceService.warningReport(macAddress));
    }

}
