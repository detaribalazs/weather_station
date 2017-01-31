function periodicData()
            {
              respObj;
              var req = false;

              function periodicComplete()
              {
                if(req.readyState == 4)
                {
                  if(req.status == 200)
                  {
                    respObj = JSON.parse(req.responseText);
                    document.getElementById("periodic").innerHTML = "<div>" 
                                            + "Temperature: " + respObj.temperature + " "
                                            + "Humidity: " + respObj.humidity + " "
                                            + "Pressure: " + respObj.pressure + " "
                                            + "Light: " + respObj.light + " "
                                            + "</div>";
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
                  req.onreadystatechange = periodicComplete;
                  req.send(null);
              }
            }

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
                document.getElementById("data").innerHTML = "<div>" 
                	+ "Temperature: " + respObj.temperature + " "
                	+ "Humidity: " + respObj.humidity + " "
                	+ "Pressure: " + respObj.pressure + " "
                	+ "Light: " + respObj.light + " "
                    + "</div>";
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
      period = 5000;
    }
    else{
      period = Number.parseInt(param);
      console.log(period);
    }
    //if(initialized === false){
        tempGraph = initGraph(80, "temp", period);
        humidityGraph  = initGraph(80, "humidity", period);
        pressureGraph = initGraph(80, "pressure", period);
        lightGraph = initGraph(80, "light", period);
    //    initialized = true;
    //}
    //else{
    //    tempGraph = initGraph(80, "temp", period, [tempGraph.minY, tempGraph.maxY]);
    //    humidityGraph  = initGraph(80, "humidity", period, [humidityGraph.minY, humidityGraph.maxY]);
    //    pressureGraph = initGraph(80, "pressure", period, [pressureGraph.minY, pressureGraph.maxY]);
    //    lightGraph = initGraph(80, "light", period, [lightGraph.minY, lightGraph.maxY]);
    //}
}

function deInitAnim(){
    d3.select("#temp_graph").remove();
    d3.select("#humidity_graph").remove();
    d3.select("#pressure_graph").remove();
    d3.select("#light_graph").remove();
}

function refreshPeriod()
{
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
        random = d3.randomNormal(0, .9);
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

            // Push a new data point onto the beginning.
            var newData = random();
            data.unshift(newData);
            function checkScale(measurementGraph){
                var changed = false;
                if(newData > measurementGraph.maxY)
                {
                    measurementGraph.maxY = newData;
                    changed = true;   
                }
                if(newData < measurementGraph.minY)
                {
                    measurementGraph.minY = newData;
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
            }
            switch(id){
            case "#temp":
                document.getElementById("temp_value").innerHTML = "Temperature: " + newData;
                checkScale(tempGraph);
                break;
            case "#humidity":
                document.getElementById("humidity_value").innerHTML = "Humidity: " + newData;
                checkScale(humidityGraph);
                break;
            case "#pressure":
                document.getElementById("pressure_value").innerHTML = "Pressure: " + newData;
                checkScale(pressureGraph);
                break;
            case "#light":
                document.getElementById("light_value").innerHTML = "Light: " + newData;
                checkScale(lightGraph);
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
            avg: 0 
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