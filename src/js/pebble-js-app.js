

var stufen = ["5","6","7","8","9","10","ABI "];
var klassen = ["A","B","C","D","G","Y","2015","2016","2017"];

//message to send

//function for http get request
function HTTPGET(url) {
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);
    return req.responseText;
}



//function for parsing plan
function getPlan() {
  var data = {0:"Keine Vertretung",50:"Keine Vertretung"};
  //get Plan
  var response = HTTPGET("http://goethe-gym-ger.de/vertretungsplan/api.php?access=36f3b27");                    
  //convert to json
  var json = JSON.parse(response);
  
  var Filter = window.localStorage.getItem("Filter") ? window.localStorage.getItem("Filter") : "ABI 2016";
  //console.log(Filter);
  
  data.TAG1 = json[0].tag;
  data.TAG2 = json[1].tag;
  //data.LENGTH_TAG1 = json[0]["ABI 2015"].length;
  //data.LENGTH_TAG2 = json[1]["ABI 2015"].length;
  
  for(t in json){
    if(json[t][Filter]){
      for(i in json[t][Filter]){
        if(json[t][Filter][i].Fach=="---"){
          var tmp = json[t][Filter][i].Stunde+" "+json[t][Filter][i]["(Fach)"]+" "+json[t][Filter][i].Art;
      }else{
          var tmp = json[t][Filter][i].Stunde+" "+json[t][Filter][i]["Fach"]+" "+json[t][Filter][i].Art;
      }
        var index = 50*parseInt(t)+parseInt(i);
        //console.log(tmp + index);
        data[index] = tmp; 
      }
    }
  }
  
  Pebble.sendAppMessage(data);
}



// Called when JS is ready
Pebble.addEventListener("ready",
							function(e) {
                getPlan();
							});
												
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",
							function(e) {
								//console.log("Received Status: " + e.payload.UPDATE);
								//sendMessage();
							});

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("http://olrikheuser.de/pebble/plan-einstellungen.html");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    //console.log("Configuration window returned: " + JSON.stringify(configuration));
    var filterOne = encodeURIComponent(configuration.filterOne);
    var filterTwo = encodeURIComponent(configuration.filterTwo);
    
    if(filterOne == "undefined"){
      filterOne = 6;
    }
    if(filterTwo == "undefined"){
      filterOne = 7;
    }
    
    window.localStorage.setItem("Filter",stufen[filterOne]+klassen[filterTwo]);
 
    getPlan();
  }
);
