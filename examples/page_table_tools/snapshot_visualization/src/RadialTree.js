import React, { useEffect, useRef } from 'react';
import * as d3 from 'd3';

const RadialTree = ({ data, showLabels, showHeatmap, heatmapProperty }) => {
    const containerRef = useRef();
    const zoomStateRef = useRef({ x: 0, y: 0, k: 1 });

    useEffect(() => {
        const tooltip = d3.select("body")
            .append("div")
            .attr("class", "tooltip")
            .style("opacity", 0);

        const container = d3.select(containerRef.current);
        let width = container.node().getBoundingClientRect().width;
        let height = container.node().getBoundingClientRect().height;

        width = width === 0 ? 800 : width;
        height = height === 0 ? 800 : height;

        container.select("svg").remove();

        const svg = container.append("svg")
            .attr("width", width)
            .attr("height", height);
        
        const zoomG = svg.append("g");

        const initialZoomState = zoomStateRef.current;
        const initialScale = initialZoomState.k;

        const g = zoomG.append("g")
            .attr("transform", `translate(${width / 2}, ${height / 2})`);

        const zoomBehavior = d3.zoom().on("zoom", (event) => {
            zoomG.attr("transform", event.transform);
            zoomStateRef.current = event.transform;

            const k = event.transform.k;
            g.selectAll('.node circle').attr('r', 4 / k);
            g.selectAll('.link').style('stroke-width', 1 / k);
            g.selectAll('text').style('font-size', `${12 / k}px`);
        });

        svg.call(zoomBehavior)
            .call(zoomBehavior.transform, d3.zoomIdentity.translate(initialZoomState.x, initialZoomState.y).scale(initialZoomState.k));

        const root = d3.hierarchy(data);
        const treeLayout = d3.tree()
            .size([2 * Math.PI, height / 2 - 100]);

        treeLayout(root);

        const linkGenerator = d3.linkRadial()
            .angle(d => d.x)
            .radius(d => d.y);

        g.selectAll(".link")
            .data(root.links())
            .enter()
            .append("path")
            .attr("class", "link")
            .attr("d", linkGenerator)
            .style("fill", "none")
            .style("stroke", "#ccc")
            .style('stroke-width', 1 / initialScale);

        const nodeGroup = g.selectAll(".node")
            .data(root.descendants())
            .enter()
            .append("g")
            .attr("class", "node")
            .attr("transform", d => `
                rotate(${d.x * 180 / Math.PI - 90})
                translate(${d.y})
            `);

        nodeGroup.append("circle")
            .attr("r", 4 / initialScale)
            .style("fill", d => {
                if (showHeatmap) {
                    return (d.data[heatmapProperty] === 1 ? 'green' : 'red');
                } else {
                    return "#445";
                }
            })
            .on("mouseover", (event, d) => {
                const entryData = d.data; // Assume entryData contains your page_table_entry struct
    
                let tooltipHTML = Object.keys(entryData).map(key => {
                    if (key === 'children') {
                        return '';
                    }
                
                    return `<div><strong>${key}</strong><span class="value">${entryData[key]}</span></div>`;
                }).join('');

                tooltip.transition()
                    .duration(100)
                    .style("opacity", 0.9);

                tooltip.html(tooltipHTML)
                    .style("opacity", 1)
                    .style("left", (event.pageX + 10) + "px")
                    .style("top", (event.pageY - 30) + "px");
            })
            .on("mouseout", () => {
                tooltip.transition()
                    .duration(200)
                    .style("opacity", 0);
            });

        const labelGroup = nodeGroup.append("g")
            .attr("transform", d => `rotate(${-1 * (d.x * 180 / Math.PI - 90)})`);

        labelGroup.append("text")
            .attr("dy", "-1em")
            .style("text-anchor", "middle")
            .style("fill", "white")
            .style("display", showLabels ? "block" : "none")
            .style('font-size', `${12 / initialScale}px`)
            .text(d => {
                const level = ["root", "pgd", "p4d", "pud", "pmd", "pte"][d.depth];
                if (level === "root") {
                    return 'Process Root';
                }

                return `${level || ""}:0x${d.data.pfn}`;
            });
    }, [data, showLabels, showHeatmap, heatmapProperty]);

    return (
        <div ref={containerRef} id="radial-tree-container"></div>
    );
};

export default RadialTree;
