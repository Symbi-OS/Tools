import React, { useState } from 'react';
import pageTableData from './data/sample_dump.json';
import RadialTree from './RadialTree';

const PageTableVisualizer = () => {
    // Initialize state with imported JSON data
    const [data, setData] = useState(pageTableData);
    const [data2, setData2] = useState(null); // New state for the second dataset
    const [diffData, setDiffData] = useState(null);
    const [showLabels, setShowLabels] = useState(false);
    const [showHeatmap, setShowHeatmap] = useState(true);
    const [heatmapProperty, setHeatmapProperty] = useState('read_write');
    const [fileName, setFileName] = useState("No file chosen");
    const [fileName2, setFileName2] = useState("No file chosen");

    const toggleShowLabels = () => setShowLabels(!showLabels);
    const toggleShowHeatmap = () => setShowHeatmap(!showHeatmap);

    const handleFileChange = (e) => {
        const file = e.target.files[0];
        if (!file) {
            return;
        }

        setFileName(file.name);

        // Read the file content
        const reader = new FileReader();
        reader.onload = (event) => {
            const fileContent = event.target.result;
            // Assuming the file contains JSON that's compatible with your existing state
            setData(JSON.parse(fileContent));
        };
        reader.readAsText(file);
    };

    const handleFileChange2 = (e) => {
        const file = e.target.files[0];
        if (!file) {
            return;
        }

        setFileName2(file.name);
    
        // Read the file content
        const reader = new FileReader();
        reader.onload = (event) => {
            const fileContent = event.target.result;
            // Assuming the file contains JSON that's compatible with your existing state
            setData2(JSON.parse(fileContent));
        };
        reader.readAsText(file);
    };

    const generateDiffDataset = (data1, data2) => {
        // Step 1: Deep clone the second dataset to start creating the diff
        let diffData = JSON.parse(JSON.stringify(data2));
    
        const attributesEqual = (node1, node2) => {
            for (const key of Object.keys(node1)) {
                if (key !== 'pfn' && key !== 'children') {
                    if (node1[key] !== node2[key]) {
                        return false;
                    }
                }
            }
            return true;
        };
    
        // Recursive function to mark new nodes that are present in data2 but not in data1
        const markNodes = (node1Array, node2Array) => {
            const node1Map = new Map();
            node1Array.forEach(n1 => {
                const key = `${n1.pfn}_${n1.level}`;
                node1Map.set(key, n1);
            });
    
            node2Array.forEach(node2 => {
                const key = `${node2.pfn}_${node2.level}`;
                const existingNode1 = node1Map.get(key);
    
                // If the node exists in data2 but not in data1, mark the link as 'new'
                if (!existingNode1) {
                    node2.linkStatus = 'new';
                } else {
                    // Check if attributes of the node have changed
                    if (!attributesEqual(existingNode1, node2)) {
                        node2.linkStatus = 'modified';
                    }
    
                    // Remove from map to keep track of nodes that were not matched
                    node1Map.delete(key);
                }
    
                // Recur for each child in data2
                const correspondingNode1Children = existingNode1 ? existingNode1.children || [] : [];
                const node2Children = node2.children || [];
                markNodes(correspondingNode1Children, node2Children);
            });
    
            // Nodes remaining in node1Map were not found in node2Array, mark them as removed
            node1Map.forEach((value) => {
                value.linkStatus = 'removed';
                node2Array.push(value);
            });
        };
    
        // Start the process from the root node
        markNodes([data1], [diffData]);
    
        return diffData;
    };

    const handleGenerateDiff = () => {
        if (data && data2) {
            const diffResult = generateDiffDataset(data, data2);
            setDiffData(diffResult);
        }
    };

    return (
        <div className="App">
            <header className="App-header">
                <div className="header-buttons" style={{ display: 'flex', justifyContent: 'space-between' }}>
                    <div>
                        <button onClick={toggleShowLabels}>
                            {showLabels ? 'Hide Labels' : 'Show Labels'}
                        </button>
                        <button onClick={toggleShowHeatmap}>
                            {showHeatmap ? 'Hide Heatmap' : 'Show Heatmap'}
                        </button>
                        <label>Heatmap Property: </label>
                        <select value={heatmapProperty} onChange={(e) => setHeatmapProperty(e.target.value)}>
                            <option value="read_write">Read/Write</option>
                            <option value="user_supervisor">User/Supervisor</option>
                            <option value="accessed">Accessed</option>
                            <option value="dirty">Dirty</option>
                            <option value="page_access_type">Page Access Type</option>
                            <option value="global">Global</option>
                            <option value="protection_key">Protection Key</option>
                            <option value="execute_disable">Execute Disable</option>
                        </select>
                    </div>
                    <div className="upload-section" style={{ alignSelf: 'flex-end', marginLeft: 40 }}>
                        <input type="file" id="fileInput" onChange={handleFileChange} style={{ display: 'none' }} />
                        <label htmlFor="fileInput">
                            <button type="button" onClick={() => document.getElementById('fileInput').click()}>
                                Upload PTE Data
                            </button>
                        </label>
                        <span>{fileName}</span>
                    </div>
                    <div className="upload-section" style={{alignSelf: 'flex-end'}}>
                        <input type="file" id="fileInput2" onChange={handleFileChange2} style={{display: 'none'}} />
                        <label htmlFor="fileInput2">
                            <button type="button" onClick={() => document.getElementById('fileInput2').click()}>
                                Upload Second PTE Data
                            </button>
                        </label>
                        <span>{fileName2}</span>
                    </div>
                    <button onClick={handleGenerateDiff} disabled={!data2}>
                        Generate Diff
                    </button>
                </div>
            </header>
            <main className="App-main">
            {diffData ? 
                <RadialTree data={diffData} showLabels={showLabels} showHeatmap={showHeatmap} heatmapProperty={heatmapProperty} /> :
                <RadialTree data={data} showLabels={showLabels} showHeatmap={showHeatmap} heatmapProperty={heatmapProperty} />
            }
            </main>
        </div>
    );
};

export default PageTableVisualizer;
