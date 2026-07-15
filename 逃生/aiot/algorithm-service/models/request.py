"""API 请求/响应模型"""

from __future__ import annotations

from typing import Optional

from pydantic import BaseModel

from .graph import GraphEdge, GraphNode, SensorReading


class EscapeRequest(BaseModel):
    """逃生路径计算请求"""
    algorithm: str = "aco"
    start_node: str
    floor_id: int
    nodes: list[GraphNode]
    edges: list[GraphEdge]
    sensor_data: list[SensorReading] = []
    params: Optional[dict] = None


class PathResult(BaseModel):
    """单条逃生路径结果"""
    exit_node: str
    exit_name: str
    nodes: list[str]
    total_cost: float
    estimated_time: str


class EscapeResponse(BaseModel):
    """逃生路径计算响应"""
    algorithm: str
    paths: list[PathResult]
    compute_time_ms: int
