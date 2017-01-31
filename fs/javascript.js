var respObj = {
    temperature: 0,
    humidity: 0,
    pressure: 100000,
    light: 0
};

function sendData()
{
    var req = false;

    function sendingComplete()
    {
        if(req.readyState == 4)
        {
            if(req.status == 200)
            {
            	respObj = JSON.parse(req.responseText);
            }
        }
    }

    if(window.XMLHttpRequest)
    {
        req = new XMLHttpRequest();
    }
    else if(window.ActiveXObject)
    {
        req = new ActiveXObject("Microsoft.XMLHTTP");
    }
    if(req)
    {
        req.open("GET", "/cgi-bin/send_data?id" + Math.random(), true);
        req.onreadystatechange = sendingComplete;
        req.send(null);
    }
}

function initAnim(param){
    console.log(param);
    if(param == undefined){
      /* set default refresh period to 1000 ms */
      period = 1000;
    }
    else{
      period = Number.parseInt(param);
      console.log(period);

      tempGraph = initGraph(80, "temp", period);
      humidityGraph  = initGraph(80, "humidity", period);
      pressureGraph = initGraph(80, "pressure", period);
      lightGraph = initGraph(80, "light", period);
    }
}

function deInitAnim(){
    d3.select("#temp_graph").remove();
    d3.select("#humidity_graph").remove();
    d3.select("#pressure_graph").remove();
    d3.select("#light_graph").remove();
}

function refreshPeriod(){
    var newPeriod = document.getElementById("period").value;
    console.log(newPeriod);
    deInitAnim();
    initAnim(newPeriod);

}

/* arguments: 
    - sampleNum - number of samples showed at once
    - data - array of length 'sampleNum' containing the measurement data
    - random - temporal variable for testing
    - id - contains the ID of 
*/

function initGraph(sampleNum, id, period, yScale, data){
    var n = sampleNum,
        random = d3.randomNormal(0, 0);
    if(data == undefined){
        var data = d3.range(n).map(random);
    }
    if(yScale == undefined){
        var yScale = [-1, 1];
    }

    var graphId = id + "_graph";;
    var id = "#" + id;

    var svg = d3.select(id),
        margin = {top: 20, right: 20, bottom: 20, left: 40},
        width = +svg.attr("width") - margin.left - margin.right,
        height = +svg.attr("height") - margin.top - margin.bottom,
        g = svg.append("g").attr("transform", "translate(" + margin.left + "," + margin.top + ")")
                           .attr("id", graphId);

    var x = d3.scaleLinear()
        .domain([0, n - 1])
        .range([0, width]);

    var y = d3.scaleLinear()
        .domain(yScale)
        .range([height, 0]);

    var line = d3.line()
        .x(function(d, i) { return x(i); })
        .y(function(d, i) { return y(d); });

    g.append("defs").append("clipPath")
        .attr("id", "clip")
        .append("rect")
        .attr("width", width)
        .attr("height", height);

    g.append("g")
        .attr("class", "axis axis--x")
        .attr("transform", "translate(0," + y(0) + ")")
        .call(d3.axisBottom(x));

    g.append("g")
        .attr("class", "axis axis--y")
        .call(d3.axisLeft(y));

    g.append("g")
        .attr("clip-path", "url(#clip)")
        .append("path")
        .datum(data)
    
        .attr("class", "line")
        .transition()
        .duration(period)
        .ease(d3.easeLinear)
        .on("start", tick);

        function tick() {

            console.log("tick'd");
            // Push a new data point onto the beginning.
            function checkScale(measurementGraph){
                var changed = false;
                var actualMax = d3.max(data);
                var actualMin = d3.min(data);
                if( (actualMax != measurementGraph.maxY) || (actualMin != measurementGraph.minY)){
                    measurementGraph.maxY = actualMax;
                    measurementGraph.minY = actualMin;
                    changed = true;   
                }
                if(changed === true)
                {
                    function redrawGraph(measurementGraph, id, data){
                        function clearGraph(id){   
                            var graphId = id + "_graph";
                            d3.select(graphId).remove();
                        };

                        var clearId = id.replace(/#/g,"");
                        var newScale = [measurementGraph.minY, measurementGraph.maxY];
                        clearGraph(id);
                        initGraph(80, clearId, period, newScale, data);
                    };
                    redrawGraph(measurementGraph, id, data);
                }
                return changed;
            }
            switch(id){
            case "#temp":
                var newData = respObj.temperature;
                data.unshift(newData);
                document.getElementById("temp_value").innerHTML = "Temperature: " + newData;
                if(checkScale(tempGraph)){
                    return;
                }
                break;
            case "#humidity":
                var newData = respObj.humidity;
                data.unshift(newData);
                document.getElementById("humidity_value").innerHTML = "Humidity: " + newData;
                if(checkScale(humidityGraph)){
                    return;
                }
                break;
            case "#pressure":
                var newData = respObj.pressure;
                data.unshift(newData);
                document.getElementById("pressure_value").innerHTML = "Pressure: " + newData;
                if(checkScale(pressureGraph)){
                    return;
                }
                break;
            case "#light":
                var newData = respObj.light;
                data.unshift(newData);
                document.getElementById("light_value").innerHTML = "Light: " + newData;
                if(checkScale(lightGraph)){
                    return;
                }
                break;
            };
            // Redraw the line.
            d3.select(this)
            .attr("d", line)
            .attr("transform", null);

            // Slide it to the right.
            d3.active(this)
              .attr("transform", "translate(" + x(+1) + ",0)")
              .transition()
              .on("start", tick);

            // Pop the old data point off the back.
            data.pop();
        };
        var retVal = {
            minY: -1,
            maxY: 1,
        };
        return retVal;
}

function redrawGraph(measGraph, id, data){
    
    function clearGraph(id)
    {   
        var graphId = id + "_graph";
        d3.select(graphId).remove();
    }

    var clearId = id.replace(/#/g,"");
    var newScale = [measGraph.minY, measGraph.maxY];
    clearGraph(id);
    initGraph(80, clearId, period, newScale, data);
}