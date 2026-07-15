package escape

import (
	v1 "aiot/api/escape/v1"
	"aiot/internal/dao"
	"aiot/internal/errs"
	escapeLogic "aiot/internal/logic/escape"
	"context"
)

type ControllerV1 struct{}

func NewV1() *ControllerV1 { return &ControllerV1{} }

func (c *ControllerV1) CalculateEscape(ctx context.Context, req *v1.CalculateEscapeReq) (res *v1.CalculateEscapeRes, err error) {
	result, err := escapeLogic.Calculate(ctx, req.Algorithm, req.StartNode, req.FloorId, req.FireMode, req.Params)
	if err != nil {
		return nil, errs.Wrap(ctx, err, "逃生路径计算失败")
	}
	return &v1.CalculateEscapeRes{
		Algorithm:     result.Algorithm,
		Paths:         result.Paths,
		ComputeTimeMs: result.ComputeTimeMs,
	}, nil
}

func (c *ControllerV1) GetGraphNodes(ctx context.Context, req *v1.GetGraphNodesReq) (res *v1.GetGraphNodesRes, err error) {
	var list interface{}
	if req.FloorId > 0 {
		list, err = dao.GraphNode.ListByFloor(ctx, req.FloorId)
	} else {
		list, err = dao.GraphNode.ListAll(ctx)
	}
	if err != nil {
		return nil, errs.Wrap(ctx, err, "获取图节点失败")
	}
	return &v1.GetGraphNodesRes{List: list}, nil
}

func (c *ControllerV1) GetGraphEdges(ctx context.Context, req *v1.GetGraphEdgesReq) (res *v1.GetGraphEdgesRes, err error) {
	list, err := dao.GraphEdge.ListAll(ctx)
	if err != nil {
		return nil, errs.Wrap(ctx, err, "获取图边失败")
	}
	return &v1.GetGraphEdgesRes{List: list}, nil
}
