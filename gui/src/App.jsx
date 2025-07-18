import path from "path-browserify"; 

import React, { useCallback, useRef, useState, useEffect } from 'react';
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
  Position
} from 'reactflow';
import 'reactflow/dist/style.css';
import './App.css';
import { v4 as uuidv4 } from 'uuid';

const blockTypes = ['Image Input', 'Grayscale', 'Canny Edge'];
const CVBlock = ({ data }) => {
  return (
    <div style={{
      padding: '10px',
      background: 'white',
      border: '1px solid #ccc',
      borderRadius: '6px',
      textAlign: 'center',
      minWidth: 120,
      fontSize: 14,
      position: 'relative',
    }}>
      <Handle
        type="target"
        position={Position.Left}
        style={{
          background: '#007acc',
          width: 12,
          height: 12,
          borderRadius: '50%',
        }}
      />
      <div style={{ fontWeight: 'bold', marginBottom: 6 }}>{data.label}</div>
      {data.image && (
        <img
          src={data.image}
          alt="node-preview"
          style={{
            maxWidth: '100%',
            maxHeight: '80px',
            borderRadius: '4px',
            marginTop: '4px',
            objectFit: 'contain',
          }}
        />
      )}
      <Handle
        type="source"
        position={Position.Right}
        style={{
          background: '#2e7d32',
          width: 12,
          height: 12,
          borderRadius: '50%',
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

  const reactFlowWrapper = useRef(null);
  const [nodes, setNodes, onNodesChange] = useNodesState(initialNodes);
  const [selectedNodes, setSelectedNodes] = useState([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState(initialEdges);
  const [reactFlowInstance, setReactFlowInstance] = useState(null);
  const [inputImagePath, setInputImagePath] = useState(null);
  

  const exec = window.electronAPI.exec;
  const fs = window.electronAPI.fs;

  const onConnect = useCallback(
    (params) =>
      setEdges((eds) =>
        addEdge(
          {
            ...params,
            markerEnd: { type: MarkerType.ArrowClosed },
            animated: true,
            style: { strokeWidth: 2, stroke: '#444' },
          },
          eds
        )
      ),
    [setEdges]
  );

  useEffect(() => {
    const handleKeyDown = (e) => {
      if ((e.key === 'Delete' || e.key === 'Backspace') && selectedNodes.length > 0) {
        setNodes((nds) => nds.filter((n) => !selectedNodes.some((sn) => sn.id === n.id)));
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [selectedNodes, setNodes]);

  const onDragStart = (event, blockType) => {
    event.dataTransfer.setData('application/reactflow', blockType);
    event.dataTransfer.effectAllowed = 'move';
  };

  const onDrop = useCallback((event) => {
    event.preventDefault();
    const type = event.dataTransfer.getData('application/reactflow');
    if (!type || !reactFlowInstance) return;
    const bounds = reactFlowWrapper.current.getBoundingClientRect();
    const dropX = event.clientX - bounds.left;
    const dropY = event.clientY - bounds.top;
    const estimatedBlockSize = { width: 100, height: 40 };
    const position = reactFlowInstance.project({
      x: dropX - estimatedBlockSize.width / 2,
      y: dropY - estimatedBlockSize.height / 2,
    });
    const newNode = {
      id: uuidv4(),
      type: 'cvblock',
      position,
      data: { label: type, image: undefined },
    };
    setNodes((nds) => nds.concat(newNode));
  }, [reactFlowInstance]);

  const onDragOver = useCallback((event) => {
    event.preventDefault();
    event.dataTransfer.dropEffect = 'move';
  }, []);

  const runPipeline = async () => {
  if (!inputImagePath) {
    alert("Please select an input image before running the pipeline.");
    return;
  }

  const graph = {
    input_image: path.basename(inputImagePath),
    input_dir: path.dirname(inputImagePath),
    nodes: nodes.map(({ id, data }) => ({ id, type: data.label })),
    edges: edges.map(({ source, target }) => ({ source, target })),
  };

  try {
    const result = await window.electronAPI.runPipeline(JSON.stringify(graph, null, 2));
    console.log("[PIPELINE OUTPUT]", result);

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
    if (filePath) {
      console.log("[INFO] Full path:", filePath);
      setInputImagePath(filePath);
    } else {
      alert("No file selected.");
    }
  }}
  style={{
    marginBottom: '10px',
    width: '100%',
    padding: '6px',
    backgroundColor: '#007acc',
    color: 'white',
    border: 'none',
    borderRadius: '6px',
    cursor: 'pointer',
    fontWeight: 'bold',
  }}
>
  📂 Select Input Image
</button>


        <h3>Blocks</h3>
        {blockTypes.map((block) => (
          <div
            key={block}
            className="block-item"
            onDragStart={(e) => onDragStart(e, block)}
            draggable
          >
            {block}
          </div>
        ))}
      </aside>
      <div style={{ position: 'absolute', top: '10px', right: '20px', zIndex: 10 }}>
        <button
          onClick={runPipeline}
          style={{
            padding: '8px 16px',
            backgroundColor: '#28a745',
            color: 'white',
            border: 'none',
            borderRadius: '6px',
            cursor: 'pointer',
            fontWeight: 'bold',
            boxShadow: '0 2px 6px rgba(0,0,0,0.2)',
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
          onConnect={onConnect}
          onDrop={onDrop}
          onDragOver={onDragOver}
          onInit={setReactFlowInstance}
          onSelectionChange={({ nodes: selected }) => setSelectedNodes(selected)}
          snapToGrid={true}
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
