const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("electronAPI", {
  selectImage: () => ipcRenderer.invoke("select-image"),
  runPipeline: (graphJson) => ipcRenderer.invoke("run-pipeline", graphJson),
});
