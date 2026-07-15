package escape

import (
	"aiot/internal/dao"
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"github.com/gogf/gf/v2/frame/g"
)

// AlgoNode mirrors the Python GraphNode model.
type AlgoNode struct {
	NodeKey  string  `json:"node_key"`
	Name     string  `json:"name"`
	NodeType string  `json:"node_type"`
	X        float64 `json:"x"`
	Y        float64 `json:"y"`
	Z        float64 `json:"z"`
	FloorId  int     `json:"floor_id"`
	IsExit   bool    `json:"is_exit"`
	Capacity *int    `json:"capacity,omitempty"`
}

// AlgoEdge mirrors the Python GraphEdge model.
type AlgoEdge struct {
	FromNodeKey   string  `json:"from_node_key"`
	ToNodeKey     string  `json:"to_node_key"`
	Distance      float64 `json:"distance"`
	BaseWeight    float64 `json:"base_weight"`
	Width         float64 `json:"width,omitempty"`
	Bidirectional bool    `json:"bidirectional"`
}

// AlgoSensorReading mirrors the Python SensorReading model.
type AlgoSensorReading struct {
	Mac             string   `json:"mac"`
	NodeKey         string   `json:"node_key"`
	Temperature     *float64 `json:"temperature,omitempty"`
	Humidity        *float64 `json:"humidity,omitempty"`
	AQI             *float64 `json:"aqi,omitempty"`
	Flame           bool     `json:"flame"`
	InfluenceRadius float64  `json:"influence_radius"`
}

// CalculateRequest is the request to the algorithm service.
type CalculateRequest struct {
	Algorithm  string                 `json:"algorithm"`
	StartNode  string                 `json:"start_node"`
	FloorId    int                    `json:"floor_id"`
	Nodes      []AlgoNode             `json:"nodes"`
	Edges      []AlgoEdge             `json:"edges"`
	SensorData []AlgoSensorReading    `json:"sensor_data"`
	Params     map[string]interface{} `json:"params,omitempty"`
}

// PathResult mirrors the Python PathResult.
type PathResult struct {
	ExitNode      string   `json:"exit_node"`
	ExitName      string   `json:"exit_name"`
	Nodes         []string `json:"nodes"`
	TotalCost     float64  `json:"total_cost"`
	EstimatedTime string   `json:"estimated_time"`
}

// CalculateResponse is the response from the algorithm service.
type CalculateResponse struct {
	Algorithm     string       `json:"algorithm"`
	Paths         []PathResult `json:"paths"`
	ComputeTimeMs int          `json:"compute_time_ms"`
}

// Calculate loads graph data from DB, calls the Python algorithm service, and returns the result.
// When fireMode is true, elevator nodes and their connected edges are excluded (fire safety: stairs only).
func Calculate(ctx context.Context, algorithm string, startNode string, floorId int, fireMode bool, params map[string]interface{}) (*CalculateResponse, error) {
	// 1. Load graph nodes and edges from DB
	allNodes, err := dao.GraphNode.ListAll(ctx)
	if err != nil {
		return nil, fmt.Errorf("load graph nodes: %w", err)
	}
	allEdges, err := dao.GraphEdge.ListAll(ctx)
	if err != nil {
		return nil, fmt.Errorf("load graph edges: %w", err)
	}

	// Convert to algorithm request format, filtering out elevators in fire mode
	excludedKeys := make(map[string]bool)
	nodes := make([]AlgoNode, 0, len(allNodes))
	for _, n := range allNodes {
		// 火灾模式：禁止使用电梯（垂直电梯、扶手电梯等）
		// g.Log().Info(ctx, "11111", fireMode, n.NodeType)
		if fireMode && n.NodeType == "elevator" {
			excludedKeys[n.NodeKey] = true
			continue
		}
		nodes = append(nodes, AlgoNode{
			NodeKey:  n.NodeKey,
			Name:     n.Name,
			NodeType: n.NodeType,
			X:        n.X,
			Y:        n.Y,
			Z:        n.Z,
			FloorId:  n.FloorId,
			IsExit:   n.IsExit == 1,
			Capacity: n.Capacity,
		})
	}

	edges := make([]AlgoEdge, 0, len(allEdges))
	for _, e := range allEdges {
		// 跳过连接到被排除节点的边
		if excludedKeys[e.FromNodeKey] || excludedKeys[e.ToNodeKey] {
			continue
		}
		edges = append(edges, AlgoEdge{
			FromNodeKey:   e.FromNodeKey,
			ToNodeKey:     e.ToNodeKey,
			Distance:      e.Distance,
			BaseWeight:    e.BaseWeight,
			Width:         e.Width,
			Bidirectional: e.Bidirectional == 1,
		})
	}

	// 2. Load sensor-node bindings and build sensor readings (placeholder, can be enriched with real-time data)
	bindings, _ := dao.SensorNodeBinding.ListAll(ctx)
	sensorData := make([]AlgoSensorReading, 0, len(bindings))
	for _, b := range bindings {
		sensorData = append(sensorData, AlgoSensorReading{
			Mac:             fmt.Sprintf("sensor-%d", b.SensorId),
			NodeKey:         b.NodeKey,
			InfluenceRadius: b.InfluenceRadius,
		})
	}

	// 3. Build request
	req := CalculateRequest{
		Algorithm:  algorithm,
		StartNode:  startNode,
		FloorId:    floorId,
		Nodes:      nodes,
		Edges:      edges,
		SensorData: sensorData,
		Params:     params,
	}

	// 4. Call algorithm service
	algoURL := g.Cfg().MustGet(ctx, "algorithm.url").String()
	timeout := g.Cfg().MustGet(ctx, "algorithm.timeout", 30).Int()

	body, _ := json.Marshal(req)
	endpoint := fmt.Sprintf("%s/api/escape/calculate", algoURL)

	client := &http.Client{Timeout: time.Duration(timeout) * time.Second}
	resp, err := client.Post(endpoint, "application/json", bytes.NewReader(body))
	if err != nil {
		return nil, fmt.Errorf("algorithm service call failed: %w", err)
	}
	defer resp.Body.Close()

	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("read algorithm response: %w", err)
	}

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("algorithm service error (status %d): %s", resp.StatusCode, string(respBody))
	}

	var result CalculateResponse
	if err = json.Unmarshal(respBody, &result); err != nil {
		return nil, fmt.Errorf("parse algorithm response: %w", err)
	}
	return &result, nil
}
