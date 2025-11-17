
var deviceID    = "<TODO:deviceID>";
var accessToken = "<TODO:accessToken>";
var strJsonPrev = "";
var cdeEventsList = [];

    window.setInterval(function() {
        //$("#debug").html("start");
        requestURL = "https://api.spark.io/v1/devices/" + deviceID + "/get_cde/?access_token=" + accessToken;
        $.getJSON(requestURL, function(json) {
        
        
        var inputJson = json.result;

        if(strJsonPrev == inputJson) {
            console.log("no change!");
            return;
        }
        console.log("json="+json.result);

        strJsonPrev = inputJson;
        inputJson = inputJson.trim(); // Remove escape char

        if(inputJson.startsWith("{") && inputJson.endsWith("")){
            inputJson = inputJson.substring(1,inputJson.length -1 ); // Remove brace {}


            var strArray = inputJson.split("&");


            if(strArray.length > 1 ) { // If not, nothinf to display !
                
                var strCurrentId = strArray[0];
                var strHtmlList = "";
                cdeEventsList = [];                 // Clear event list

                for(var i=1; i < strArray.length - 1; i++) { // Foreach event
                    console.log("event["+i+"]="+strArray[i]);
                    
                    var cdeEvent = new CdeEvent();
                    
                    cdeEvent.parseJson(strArray[i]);
                    strHtmlList += cdeEvent.html( strCurrentId );
                    
                    if(cdeEvent.getId() != ""){
                        cdeEventsList.push(cdeEvent);
                    }

                }
                $("#accordion").html( strHtmlList ) ;
            }

        } else {
            console.error("invalid Json string !");
        }

        });
    }, 1000);

      function setValue(obj) {
        var newValue = document.getElementById('degBoxId').value;
        sparkSetPos(newValue);
      }
      
      function fineAdjust(value) {
        var currentValue = parseInt(document.getElementById('curPos').innerHTML);
        var setValue = value + currentValue;
        sparkSetPos(setValue);
        document.getElementById("degBoxId").value = setValue;
      }

      function sparkSetPos(newValue) {
  var requestURL = "https://api.spark.io/v1/devices/" +deviceID + "/" + setFunc + "/";
        $.post( requestURL, { params: newValue, access_token: accessToken });
      }

        function sparkSendCmd(actionStr, jsonStr) {
            
            var requestURL = "https://api.spark.io/v1/devices/" + deviceID + "/"+ actionStr  +"/";
            console.log(requestURL);
            $.post( requestURL, { params: jsonStr, access_token: accessToken }); 

        }



