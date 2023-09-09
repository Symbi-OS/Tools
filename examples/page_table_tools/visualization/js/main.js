// Load the datasets
d3.json("/data/pte_dump_before.json").then(beforeData => {
    render(beforeData);
});

const baseLabelRadiusOffset = 15;
const tooltip = d3.select("#tooltip");

function transformData(data) {
    return {
        name: "root",
        children: data.pgds.map(pgd => ({
            ...pgd,
            children: pgd.p4ds.map(p4d => ({
                ...p4d,
                children: p4d.puds.map(pud => ({
                    ...pud,
                    children: pud.pmds.map(pmd => ({
                        ...pmd,
                        children: pmd.ptes
                    }))
                }))
            }))
        }))
    };
}

function render(data) {
    const svgWidth = window.innerWidth;
    const svgHeight = window.innerHeight;
    const svg = d3.select("#visualization").append("svg")
        .attr("width", svgWidth)
        .attr("height", svgHeight);


    const g = svg.append("g").attr("transform", "translate(" + svgWidth / 2 + "," + svgHeight / 2 + ")");

    // Define the zoom behavior
    const zoom = d3.zoom()
        .scaleExtent([0.1, 60]) 
        .on("zoom", zoomed);

    // Apply the zoom behavior to the SVG
    svg.call(zoom);

    svg.on("click", () => {
        d3.event.stopPropagation();  // Stop any click events
    });  

    // Transform the data to fit D3 hierarchy
    const transformedData = transformData(data);

    let nodeLookup = {};
    function buildNodeLookup(node) {
        nodeLookup[node.pfn] = node;
        if (node.children) {
            node.children.forEach(buildNodeLookup);
        }
    }

    // Call this function after loading your JSON data
    buildNodeLookup(transformedData);

    // Create the visualization hierarchy with radial layout
    const root = d3.hierarchy(transformedData);
    const layout = d3.tree()
    .size([2 * Math.PI, (Math.min(svgWidth, svgHeight) / 2) - 120])  // Using 120 as a margin from the edge
    .separation(function(a, b) { return (a.parent == b.parent ? 1 : 2) / a.depth; });
    layout(root);

    const link = g.selectAll(".link")
        .data(root.links())
        .enter().append("path")
        .attr("class", "link")
        .attr("fill", "none")         
        .attr("stroke", "#555")       
        .attr("stroke-opacity", 0.4)  
        .attr("stroke-width", 1.5)    
        .attr("d", d3.linkRadial()
            .angle(d => d.x)
            .radius(d => d.y)
        );

    const node = g.selectAll(".node")
        .data(root.descendants())
        .enter().append("circle")
        .attr("class", "node")
        .attr("r", 3)
        .attr("transform", d => `translate(${getXYFromPolar(d.x, d.y)})`)
        .on("mouseover", function(event, d) {
            tooltip.style("display", "block")
                .html(getTooltipHtml(d.data))
                .style("left", (event.pageX + 5) + "px")
                .style("top", (event.pageY + 5) + "px");
        })
        .on("mouseout", function() {
            tooltip.style("display", "none");
        });

    const label = g.selectAll(".label")
        .data(root.descendants())
        .enter().append("text")
        .attr("class", "label")
        .attr("transform", d => `translate(${getXYFromPolar(d.x, d.y + baseLabelRadiusOffset)})`)
        .attr("text-anchor", "middle")
        .attr("dy", ".31em")
        .text(d => {
            switch (d.depth) {
                case 0: return "root";
                case 1: return "pgd:0x" + d.data.pfn;
                case 2: return "p4d:0x" + d.data.pfn;
                case 3: return "pud:0x" + d.data.pfn;
                case 4: return "pmd:0x" + d.data.pfn;
                case 5: return "pte:0x" + d.data.pfn;
                default: return d.data.name || "";
            }
        });

    const initialTransform = d3.zoomIdentity.translate(svgWidth / 2, svgHeight / 2);
    svg.call(zoom.transform, initialTransform);

    function zoomed(event) {
        // Adjust the translation and scale using the event transform
        const [translateX, translateY] = [event.transform.x, event.transform.y];
        const scale = event.transform.k;

        g.attr("transform", `translate(${translateX}, ${translateY}) scale(${scale})`);

        const effectiveOffset = baseLabelRadiusOffset / scale;
        label.attr("transform", d => {
            const [x, y] = getXYFromPolar(d.x, d.y);
            return `translate(${x}, ${y - effectiveOffset})`;
        }).attr("font-size", 16 / scale + "px");

        node.attr("r", 3 / scale);
        link.attr("stroke-width", 1.5 / scale);
    }

    function getXYFromPolar(angle, radius) {
        return [radius * Math.sin(angle), -radius * Math.cos(angle)];
    }

    function getTooltipHtml(data) {
        const accessed = (data.accessed !== undefined) ? data.accessed : 'N/A';
        const dirty = (data.dirty !== undefined) ? data.dirty : 'N/A';
        const executeDisable = (data.execute_disable !== undefined) ? data.execute_disable : 'N/A';
        const globalFlag = (data.global !== undefined) ? data.global : 'N/A'; // 'global' is a reserved keyword
        const pageAccessType = (data.page_access_type !== undefined) ? data.page_access_type : 'N/A';
        const pageCacheDisabled = (data.page_cache_disabled !== undefined) ? data.page_cache_disabled : 'N/A';
        const pageWriteThrough = (data.page_write_through !== undefined) ? data.page_write_through : 'N/A';
        const pfn = data.pfn || 'N/A';
        const present = (data.present !== undefined) ? data.present : 'N/A';
        const protectionKey = (data.protection_key !== undefined) ? data.protection_key : 'N/A';
        const readWrite = (data.read_write !== undefined) ? data.read_write : 'N/A';
        const userSupervisor = (data.user_supervisor !== undefined) ? data.user_supervisor : 'N/A';
    
        return `
            <div><strong>Accessed:</strong> ${accessed}</div>
            <div><strong>Dirty:</strong> ${dirty}</div>
            <div><strong>Execute Disable:</strong> ${executeDisable}</div>
            <div><strong>Global:</strong> ${globalFlag}</div>
            <div><strong>Page Access Type:</strong> ${pageAccessType}</div>
            <div><strong>Page Cache Disabled:</strong> ${pageCacheDisabled}</div>
            <div><strong>Page Write Through:</strong> ${pageWriteThrough}</div>
            <div><strong>PFN:</strong> ${pfn}</div>
            <div><strong>Present:</strong> ${present}</div>
            <div><strong>Protection Key:</strong> ${protectionKey}</div>
            <div><strong>Read/Write:</strong> ${readWrite}</div>
            <div><strong>User Supervisor:</strong> ${userSupervisor}</div>
        `;
    }    
}
