import React, { useState } from 'react';
import pageTableData from './data/sample_dump.json';
import RadialTree from './RadialTree';

const PageTableVisualizer = () => {
    // Initialize state with imported JSON data
    const [data, setData] = useState(pageTableData);
    const [showLabels, setShowLabels] = useState(false);
    const [showHeatmap, setShowHeatmap] = useState(true);
    const [heatmapProperty, setHeatmapProperty] = useState('read_write');
    const [fileName, setFileName] = useState("No file chosen");

    const toggleShowLabels = () => setShowLabels(!showLabels);
    const toggleShowHeatmap = () => setShowHeatmap(!showHeatmap);

    const handleFileChange = (e) => {
        const file = e.target.files[0];
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
                    <div className="upload-section" style={{ alignSelf: 'flex-end' }}>
                        <input type="file" id="fileInput" onChange={handleFileChange} style={{ display: 'none' }} />
                        <label htmlFor="fileInput">
                            <button type="button" onClick={() => document.getElementById('fileInput').click()}>
                                Upload PTE Data
                            </button>
                        </label>
                        <span>{fileName}</span>
                    </div>
                </div>
            </header>
            <main className="App-main">
                <RadialTree data={data} showLabels={showLabels} showHeatmap={showHeatmap} heatmapProperty={heatmapProperty} />
            </main>
        </div>
    );
};

export default PageTableVisualizer;
