import path from "path-browserify";

import React, { useCallback, useRef, useState, useEffect } from "react";
import ReactFlow, {
  ReactFlowProvider,
  addEdge,
  useNodesState,
  useEdgesState,
  Background,
  Controls,
  MiniMap,
  MarkerType,
  Handle,
  Position,
} from "reactflow";
import "reactflow/dist/style.css";
import "./App.css";
import { v4 as uuidv4 } from "uuid";

const CVBlock = ({ data }) => {
  return (
    <div
      style={{
        padding: "10px",
        background: "white",
        border: "1px solid #ccc",
        borderRadius: "6px",
        textAlign: "center",
        minWidth: 140,
        fontSize: 14,
        position: "relative",
        pointerEvents: "auto",
      }}
    >
      <Handle
        type="target"
        position={Position.Left}
        style={{
          background: "#007acc",
          width: 12,
          height: 12,
          borderRadius: "50%",
        }}
      />

      <div
        className="drag-handle"
        style={{ fontWeight: "bold", marginBottom: 6, cursor: "move" }}
      >
        {data.label}
      </div>

      {data.input_type && (
        <div style={{ fontSize: 10, color: "#555", marginBottom: 4 }}>
          <strong>Input:</strong> {data.input_type}
        </div>
      )}
      {data.output_type && (
        <div style={{ fontSize: 10, color: "#555", marginBottom: 6 }}>
          <strong>Output:</strong> {data.output_type}
        </div>
      )}

      {data.image && (
        <img
          src={data.image}
          alt="node-preview"
          style={{
            maxWidth: "100%",
            maxHeight: "80px",
            borderRadius: "4px",
            marginTop: "4px",
            objectFit: "contain",
            pointerEvents: "auto",
          }}
        />
      )}

      {data.params && data.updateParam && (
        <div
          style={{
            marginTop: "6px",
            fontSize: "12px",
            textAlign: "left",
            pointerEvents: "auto",
          }}
        >
          {data.params.map(({ name, default: defaultValue }, i) => {
            // get current value or fallback to default
            const currentValue = defaultValue;
            return (
              <div key={name} style={{ marginBottom: "6px" }}>
                <label style={{ fontWeight: "bold" }}>{name}</label>
                <input
                  type="number"
                  value={currentValue}
                  onChange={(e) => {
                    const val = e.target.value;
                    if (val === "") {
                      data.updateParam(name, 0);
                    } else {
                      const num = Number(val);
                      if (!isNaN(num)) data.updateParam(name, num);
                    }
                  }}
                  onPointerDown={(e) => e.stopPropagation()}
                  onPointerUp={(e) => e.stopPropagation()}
                  onMouseDown={(e) => e.stopPropagation()}
                  onMouseUp={(e) => e.stopPropagation()}
                  onClick={(e) => e.stopPropagation()}
                  style={{
                    width: "100%",
                    padding: "4px",
                    fontSize: "12px",
                    marginTop: "2px",
                    boxSizing: "border-box",
                  }}
                />
              </div>
            );
          })}
        </div>
      )}

      <Handle
        type="source"
        position={Position.Right}
        style={{
          background: "#2e7d32",
          width: 12,
          height: 12,
          borderRadius: "50%",
        }}
      />
    </div>
  );
};

const nodeTypes = {
  cvblock: CVBlock,
};

const initialNodes = [];
const initialEdges = [];

function FlowEditor() {
  const [availableBlocks, setAvailableBlocks] = useState([]);
  const reactFlowWrapper = useRef(null);
  const [nodes, setNodes, onNodesChange] = useNodesState(initialNodes);
  const [selectedNodes, setSelectedNodes] = useState([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState(initialEdges);
  const [reactFlowInstance, setReactFlowInstance] = useState(null);
  const [inputImagePath, setInputImagePath] = useState(null);

  useEffect(() => {
    fetch("blocks.json")
      .then((res) => res.json())
      .then((data) => setAvailableBlocks(data))
      .catch((err) =>
        console.error("[ERROR] Failed to load blocks.json:", err)
      );
  }, []);

  useEffect(() => {
    const handleKeyDown = (e) => {
      const activeTag = document.activeElement.tagName;
      if (
        ["INPUT", "TEXTAREA"].includes(activeTag) ||
        document.activeElement.isContentEditable
      )
        return;

      if (
        (e.key === "Delete" || e.key === "Backspace") &&
        selectedNodes.length > 0
      ) {
        setNodes((nds) => {
          const idsToDelete = new Set(selectedNodes.map((n) => n.id));
          const newNodes = nds.filter((n) => !idsToDelete.has(n.id));
          setEdges((eds) =>
            eds.filter(
              (e) => !idsToDelete.has(e.source) && !idsToDelete.has(e.target)
            )
          );
          return newNodes;
        });
      }
    };
    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [selectedNodes, setNodes, setEdges]);

  const exec = window.electronAPI.exec;

  const onDragStart = (event, block) => {
    event.dataTransfer.setData("application/reactflow", JSON.stringify(block));
    event.dataTransfer.effectAllowed = "move";
  };

  const onDrop = useCallback(
    (event) => {
      event.preventDefault();
      const raw = event.dataTransfer.getData("application/reactflow");
      if (!raw || !reactFlowInstance) return;
      const block = JSON.parse(raw);

      const bounds = reactFlowWrapper.current.getBoundingClientRect();
      const dropX = event.clientX - bounds.left;
      const dropY = event.clientY - bounds.top;

      const position = reactFlowInstance.project({
        x: dropX - 50,
        y: dropY - 20,
      });

      const id = uuidv4();

      // Store current param values in an object so we can update on changes
      const paramsValues = {};
      (block.params || []).forEach((p) => {
        paramsValues[p.name] = p.default;
      });

      const newNode = {
        id,
        type: "cvblock",
        position,
        data: {
          label: block.label,
          input_type: block.input_type || "Unknown",
          output_type: block.output_type || "Unknown",
          params: block.params || [],
          image: undefined,
          updateParam: (key, value) => {
            // update param in paramsValues
            paramsValues[key] = value;
            // update node params with updated values as array of objects with name/default updated
            setNodes((nds) =>
              nds.map((node) =>
                node.id === id
                  ? {
                      ...node,
                      data: {
                        ...node.data,
                        params: node.data.params.map((p) =>
                          p.name === key ? { ...p, default: value } : p
                        ),
                      },
                    }
                  : node
              )
            );
          },
        },
      };

      setNodes((nds) => nds.concat(newNode));
    },
    [reactFlowInstance]
  );

  const runPipeline = async () => {
    if (!inputImagePath) {
      alert("Please select an input image before running the pipeline.");
      return;
    }

    // filter edges so source and target exist in nodes
    const validNodeIds = new Set(nodes.map((n) => n.id));
    const filteredEdges = edges.filter(
      (e) => validNodeIds.has(e.source) && validNodeIds.has(e.target)
    );

    // convert params array of objects to params object for backend json
    const nodesForBackend = nodes.map(({ id, data }) => ({
      id,
      type: data.label,
      params: Object.fromEntries(
        data.params.map((p) => [p.name, p.default])
      ),
    }));

    const graph = {
      input_image: path.basename(inputImagePath),
      input_dir: path.dirname(inputImagePath),
      nodes: nodesForBackend,
      edges: filteredEdges,
    };

    try {
      await window.electronAPI.runPipeline(JSON.stringify(graph, null, 2));
      setNodes((prev) =>
        prev.map((node, i) => ({
          ...node,
          data: {
            ...node.data,
            image: `/run/output_node_${i}.jpg?${Date.now()}`,
          },
        }))
      );
    } catch (err) {
      console.error("[PIPELINE ERROR]", err);
    }
  };

  return (
    <div className="app">
      <aside className="sidebar">
        <button
          onClick={async () => {
            const filePath = await window.electronAPI.selectImage();
            if (filePath) setInputImagePath(filePath);
          }}
          style={{
            marginBottom: "10px",
            width: "100%",
            padding: "6px",
            backgroundColor: "#007acc",
            color: "white",
            border: "none",
            borderRadius: "6px",
            cursor: "pointer",
            fontWeight: "bold",
          }}
        >
          📂 Select Input Image
        </button>

        <h3>Blocks</h3>
        {availableBlocks.map((block) => (
          <div
            key={block.label}
            className="block-item"
            onDragStart={(e) => onDragStart(e, block)}
            draggable
          >
            {block.label}
          </div>
        ))}
      </aside>

      <div
        style={{ position: "absolute", top: "10px", right: "20px", zIndex: 10 }}
      >
        <button
          onClick={runPipeline}
          style={{
            padding: "8px 16px",
            backgroundColor: "#28a745",
            color: "white",
            border: "none",
            borderRadius: "6px",
            cursor: "pointer",
            fontWeight: "bold",
            boxShadow: "0 2px 6px rgba(0,0,0,0.2)",
          }}
        >
          ▶ Run Pipeline
        </button>
      </div>

      <div className="flow-wrapper" ref={reactFlowWrapper}>
        <ReactFlow
          nodeTypes={nodeTypes}
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          onConnect={(params) =>
            setEdges((eds) =>
              addEdge(
                {
                  ...params,
                  markerEnd: { type: MarkerType.ArrowClosed },
                  animated: true,
                  style: { strokeWidth: 2, stroke: "#444" },
                },
                eds
              )
            )
          }
          onDrop={onDrop}
          onDragOver={(e) => e.preventDefault()}
          onInit={setReactFlowInstance}
          onSelectionChange={({ nodes: selected }) => setSelectedNodes(selected)}
          snapToGrid
          snapGrid={[24, 24]}
        >
          <MiniMap />
          <Controls />
          <Background variant="lines" gap={24} color="#ddd" />
        </ReactFlow>
      </div>
    </div>
  );
}

export default function App() {
  return (
    <ReactFlowProvider>
      <FlowEditor />
    </ReactFlowProvider>
  );
}
