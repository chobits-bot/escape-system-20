"""蚁群算法 (Ant Colony Optimization) 实现逃生路径求解"""

from __future__ import annotations

import math
import random
from typing import Optional

import numpy as np

from models.graph import GraphEdge, GraphNode, SensorReading


class AntColonyOptimizer:
    """蚁群算法求解器"""

    # 默认参数
    DEFAULT_PARAMS = {
        "ant_count": 30,
        "alpha": 1.0,       # 信息素权重
        "beta": 3.0,        # 启发式权重
        "rho": 0.3,         # 信息素蒸发率
        "Q": 100.0,         # 信息素常量
        "max_iterations": 200,
        "danger_weight": 10.0,  # 危险系数权重
    }

    def __init__(
        self,
        nodes: list[GraphNode],
        edges: list[GraphEdge],
        sensor_data: list[SensorReading],
        params: Optional[dict] = None,
    ):
        self.params = {**self.DEFAULT_PARAMS, **(params or {})}
        self.nodes = {n.node_key: n for n in nodes}
        self.edges = edges
        self.sensor_data = sensor_data

        # 构建邻接表
        self.adj: dict[str, list[tuple[str, float]]] = {}
        self._build_graph()

        # 信息素矩阵 (edge_key -> pheromone)
        self.pheromone: dict[str, float] = {}
        for edge in self.edges:
            key = f"{edge.from_node_key}->{edge.to_node_key}"
            self.pheromone[key] = 1.0
            if edge.bidirectional:
                rev_key = f"{edge.to_node_key}->{edge.from_node_key}"
                self.pheromone[rev_key] = 1.0

        # 出口节点列表
        self.exits = [k for k, n in self.nodes.items() if n.is_exit]

    def _build_graph(self):
        """构建加权邻接表，结合传感器数据动态调整权重"""
        # 传感器影响：计算每个节点的危险系数
        danger_map: dict[str, float] = {}
        for sr in self.sensor_data:
            if sr.flame:
                # 火焰区域：极高危险
                danger_map[sr.node_key] = 100.0
            elif sr.temperature and sr.temperature > 50:
                danger_map[sr.node_key] = max(
                    danger_map.get(sr.node_key, 0),
                    (sr.temperature - 50) * 2.0
                )
            elif sr.aqi and sr.aqi > 200:
                danger_map[sr.node_key] = max(
                    danger_map.get(sr.node_key, 0),
                    (sr.aqi - 200) * 0.1
                )

        danger_w = self.params["danger_weight"]

        for edge in self.edges:
            # 基础权重 = 距离 * base_weight
            weight = edge.distance * edge.base_weight

            # 叠加目标节点的危险系数
            to_danger = danger_map.get(edge.to_node_key, 0)
            from_danger = danger_map.get(edge.from_node_key, 0)
            avg_danger = (to_danger + from_danger) / 2.0
            weight += avg_danger * danger_w

            # 火焰区域几乎不可通行
            if to_danger >= 100.0:
                weight = 99999.0

            if edge.from_node_key not in self.adj:
                self.adj[edge.from_node_key] = []
            self.adj[edge.from_node_key].append((edge.to_node_key, weight))

            if edge.bidirectional:
                # 反向边：考虑起点的危险系数
                rev_weight = edge.distance * edge.base_weight
                rev_weight += avg_danger * danger_w
                if from_danger >= 100.0:
                    rev_weight = 99999.0
                if edge.to_node_key not in self.adj:
                    self.adj[edge.to_node_key] = []
                self.adj[edge.to_node_key].append((edge.from_node_key, rev_weight))

    def _edge_key(self, from_key: str, to_key: str) -> str:
        return f"{from_key}->{to_key}"

    def _get_weight(self, from_key: str, to_key: str) -> float:
        """获取边权重"""
        neighbors = self.adj.get(from_key, [])
        for nk, w in neighbors:
            if nk == to_key:
                return w
        return 99999.0

    def _heuristic(self, from_key: str, to_key: str) -> float:
        """启发式函数：1/distance，距离越短吸引力越大"""
        w = self._get_weight(from_key, to_key)
        if w <= 0:
            return 1e-10
        return 1.0 / w

    def _select_next(self, current: str, visited: set[str]) -> Optional[str]:
        """蚂蚁选择下一个节点"""
        neighbors = self.adj.get(current, [])
        candidates = [(nk, w) for nk, w in neighbors if nk not in visited]

        if not candidates:
            return None

        alpha = self.params["alpha"]
        beta = self.params["beta"]

        probabilities = []
        for nk, _w in candidates:
            ek = self._edge_key(current, nk)
            tau = self.pheromone.get(ek, 1.0)
            eta = self._heuristic(current, nk)
            prob = (tau ** alpha) * (eta ** beta)
            probabilities.append(prob)

        total = sum(probabilities)
        if total == 0:
            return random.choice(candidates)[0]

        probabilities = [p / total for p in probabilities]

        r = random.random()
        cumsum = 0.0
        for i, (nk, _w) in enumerate(candidates):
            cumsum += probabilities[i]
            if r <= cumsum:
                return nk
        return candidates[-1][0]

    def _run_ant(self, start: str, target_exit: str) -> Optional[tuple[list[str], float]]:
        """单只蚂蚁从起点走到目标出口"""
        path = [start]
        visited = {start}
        total_cost = 0.0
        current = start
        max_steps = len(self.nodes) * 2  # 防止无限循环

        for _ in range(max_steps):
            if current == target_exit:
                return path, total_cost

            next_node = self._select_next(current, visited)
            if next_node is None:
                return None  # 死路

            cost = self._get_weight(current, next_node)
            total_cost += cost
            path.append(next_node)
            visited.add(next_node)
            current = next_node

        return None  # 未能到达

    def _update_pheromone(self, all_paths: list[tuple[list[str], float]]):
        """更新信息素"""
        rho = self.params["rho"]
        Q = self.params["Q"]

        # 蒸发
        for key in self.pheromone:
            self.pheromone[key] *= (1 - rho)
            self.pheromone[key] = max(self.pheromone[key], 1e-10)

        # 加强
        for path, cost in all_paths:
            if cost <= 0:
                continue
            deposit = Q / cost
            for i in range(len(path) - 1):
                ek = self._edge_key(path[i], path[i + 1])
                self.pheromone[ek] = self.pheromone.get(ek, 1e-10) + deposit

    def solve(self, start_node: str) -> dict[str, tuple[list[str], float]]:
        """
        求解从 start_node 到所有出口的最优路径。

        返回: {exit_key: (path_nodes, total_cost)}
        """
        if start_node not in self.nodes:
            return {}

        ant_count = self.params["ant_count"]
        max_iter = self.params["max_iterations"]

        # 每个出口维护最优路径
        best: dict[str, tuple[list[str], float]] = {}

        for _iteration in range(max_iter):
            iteration_paths: list[tuple[list[str], float]] = []

            for _ant in range(ant_count):
                for exit_key in self.exits:
                    result = self._run_ant(start_node, exit_key)
                    if result is not None:
                        path, cost = result
                        iteration_paths.append((path, cost))

                        if exit_key not in best or cost < best[exit_key][1]:
                            best[exit_key] = (path, cost)

            if iteration_paths:
                self._update_pheromone(iteration_paths)

            # 早停：如果连续迭代没有改善
            # (简化实现，完整版可加收敛检测)

        return best

    @staticmethod
    def estimate_time(cost: float, speed: float = 1.5) -> str:
        """估算逃生时间 (假设步行速度 1.5 m/s)"""
        seconds = cost / speed
        if seconds < 60:
            return f"{int(seconds)}s"
        minutes = int(seconds // 60)
        secs = int(seconds % 60)
        return f"{minutes}m{secs}s"
