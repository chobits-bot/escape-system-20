"""逃生路径算法微服务 - FastAPI 入口"""

import time

from fastapi import FastAPI, HTTPException

from algorithms.aco import AntColonyOptimizer
from models.request import EscapeRequest, EscapeResponse, PathResult

app = FastAPI(
    title="AIoT 逃生路径算法服务",
    description="提供蚁群算法等多种逃生路径计算",
    version="1.0.0",
)


@app.get("/health")
async def health():
    return {"status": "ok"}


@app.post("/api/escape/aco", response_model=EscapeResponse)
async def escape_aco(req: EscapeRequest):
    """蚁群算法计算逃生路径"""
    start_ms = time.time()

    # 验证起点存在
    node_keys = {n.node_key for n in req.nodes}
    if req.start_node not in node_keys:
        raise HTTPException(status_code=400, detail=f"起点 {req.start_node} 不存在于图节点中")

    # 验证至少有一个出口
    exits = [n for n in req.nodes if n.is_exit]
    if not exits:
        raise HTTPException(status_code=400, detail="图中没有出口节点")

    solver = AntColonyOptimizer(
        nodes=req.nodes,
        edges=req.edges,
        sensor_data=req.sensor_data,
        params=req.params,
    )

    results = solver.solve(req.start_node)

    paths = []
    for exit_key, (path_nodes, cost) in sorted(results.items(), key=lambda x: x[1][1]):
        exit_node = next((n for n in req.nodes if n.node_key == exit_key), None)
        paths.append(PathResult(
            exit_node=exit_key,
            exit_name=exit_node.name if exit_node else exit_key,
            nodes=path_nodes,
            total_cost=round(cost, 2),
            estimated_time=AntColonyOptimizer.estimate_time(cost),
        ))

    elapsed_ms = int((time.time() - start_ms) * 1000)

    return EscapeResponse(
        algorithm="aco",
        paths=paths,
        compute_time_ms=elapsed_ms,
    )


@app.post("/api/escape/calculate", response_model=EscapeResponse)
async def escape_calculate(req: EscapeRequest):
    """通用逃生路径计算入口，根据 algorithm 字段分发"""
    if req.algorithm == "aco":
        return await escape_aco(req)
    # TODO: pso, sa, ga
    raise HTTPException(status_code=400, detail=f"不支持的算法: {req.algorithm}")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=8090, reload=True)
