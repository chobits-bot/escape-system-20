package v1

import "github.com/gogf/gf/v2/frame/g"

type CalculateEscapeReq struct {
	g.Meta    `path:"/api/escape/calculate" method:"post" tags:"Escape" summary:"Calculate escape path"`
	Algorithm string                 `json:"algorithm" v:"required" d:"aco"`
	StartNode string                 `json:"startNode" v:"required"`
	FloorId   int                    `json:"floorId"   v:"required"`
	FireMode  bool                   `json:"fireMode"  d:"true"`
	Params    map[string]interface{} `json:"params"`
}
type CalculateEscapeRes struct {
	Algorithm     string `json:"algorithm"`
	Paths         any    `json:"paths"`
	ComputeTimeMs int    `json:"computeTimeMs"`
}

type GetGraphNodesReq struct {
	g.Meta  `path:"/api/graph/nodes" method:"get" tags:"Graph" summary:"Get graph nodes"`
	FloorId int `json:"floorId" in:"query" d:"0"`
}
type GetGraphNodesRes struct {
	List any `json:"list"`
}

type GetGraphEdgesReq struct {
	g.Meta `path:"/api/graph/edges" method:"get" tags:"Graph" summary:"Get graph edges"`
}
type GetGraphEdgesRes struct {
	List any `json:"list"`
}
