
// Conscturctor
var CdeEvent = function () {

}

// 
CdeEvent.prototype.parseJson = function( jsonStr ) {

    //console.log("jsonStr=" + jsonStr);

    var jsonStrArray = jsonStr.split('\\');

    if(jsonStrArray.length == 6) {
        
        this.id             = jsonStrArray[0];

        this.endingDate     = new Date(0);
        this.endingDate.setUTCSeconds( parseInt( "0x" + jsonStrArray[1] ) );

        this.alarm1         = parseInt("0x" + jsonStrArray[2]);
        this.alarm2         = parseInt("0x" + jsonStrArray[3]);
        this.alarm3         = parseInt("0x" + jsonStrArray[4]);
        this.description    = jsonStrArray[5];

    } else {
        console.error("CdeEvent.parseJson > invalid jsonStr");
        this.id = "";
    }
}

CdeEvent.prototype.getId = function() {
    return this.id;
}

sendForm = function(strAction, strId) {
    
    var description     = $( "#description" + strId ).val();
    var endDateInput    = $( "#datetimepicker" + strId ).val();
    var alarm1          = parseInt($( "#alarm1_" + strId ).val());
    var alarm2          = parseInt($( "#alarm2_" + strId ).val());
    var alarm3          = parseInt($( "#alarm3_" + strId ).val());

    alarm1 = isNaN(alarm1) ? 0xFFFF : alarm1;
    alarm2 = isNaN(alarm2) ? 0xFFFF : alarm2;
    alarm3 = isNaN(alarm3) ? 0xFFFF : alarm3;

    var endDateStrTmp   = moment(endDateInput, "DD/MM/YYYY hh:mm").format();
    console.log("endDateStr=", endDateStrTmp);
    var endDatet = new Date(endDateStrTmp);
    endDatet.setHours(23);
    endDatet.setMinutes(59);
    var endDate         = (endDatet.getTime() / 1000);

    endDate = isNaN(endDate) ? 0 : endDate;


    console.log("strAction=" + strAction + " strId=" + strId);
    console.log("desc=" + $( "#description" + strId ).val() );

    if(strAction == "update"){
        var strJsonValue =        strId + '\\' 
                                + endDate.toString(16).toUpperCase() + '\\' 
                                + alarm1.toString(16).toUpperCase() + '\\'
                                + alarm2.toString(16).toUpperCase() + '\\'
                                + alarm3.toString(16).toUpperCase() + '\\'  
                                + description + "&";

        sparkSendCmd("update_cde", strJsonValue); // Send data to spark cloud !
        console.log("json=" + strJsonValue);

    
    }
    else if (strAction == "delete"){
        sparkSendCmd("delete_cde",strId);
    }


}

CdeEvent.prototype.html = function( strCurrentId ) {

    if(this.id == ""){
        return "";
    }
    var now = new Date();
    var strRemainingDay = "End";
    var dateZero = new Date(0);

    // TO DO : probably don't work for other countrys...
    var strEndDate = this.endingDate.toLocaleDateString("fr-FR");   // display date in FR standard
    strEndDate += (" 0" + this.endingDate.getHours()).slice(-2);    // 2 digit hours
    strEndDate += (":0" + this.endingDate.getMinutes()).slice(-2);  // 2 digit minutes


    if(this.endingDate.getTime() == (new Date(0)).getTime()) {
        strRemainingDay = "not defined";
        strEndDate = ""; // Reset also endDate (to display placeHolder text)
    }
    else if(this.endingDate.getTime() > now.getTime()) { 
        var remainingDays =  ((this.endingDate.getTime() - now.getTime() ) / (1000 * 60 * 60 * 24)).toFixed(0) ;
        strRemainingDay =  remainingDays.toString() + " day" + ((remainingDays > 1) ? "s" : "");
    } 

    var alarm1 = (this.alarm1 != 0 && this.alarm1 < 1000 ) ? this.alarm1 : "";
    var alarm2 = (this.alarm2 != 0 && this.alarm2 < 1000 ) ? this.alarm2 : "";
    var alarm3 = (this.alarm3 != 0 && this.alarm3 < 1000 ) ? this.alarm3 : "";

    var description = (this.description == "") ? '#'+ this.id : this.description;

    var tagIsSelected = ( (this.id == strCurrentId) && (this.id != "") );

    var html = "";

    if(tagIsSelected == true) {
        html += '<div class="panel  panel-info">';
    } else {
        html += '<div class="panel panel-primary">';        
    }

    html += '  <div class="panel-heading">';
    html += '    <div class="row">';
    html += '      <div class="col-sm-4">';
    html += '        <h4 class="panel-title">';
    html += '          <a data-toggle="collapse" data-parent="#accordion" href="#collapse'+this.id+'">' + description + '</a>';
    html += '      </h4>';
    html += '      </div>';
    html += '      <div class="col-sm-2 col-sm-offset-6">';
    html += '        <h4 class="panel-title">'+strRemainingDay+'</h4>';
    html += '      </div>';
    html += '    </div>';    
    html += '  </div>';
    html += '  <div id="collapse'+this.id+'" class="panel-collapse collapse">';
    html += '    <div class="panel-body">';
    html += '    <div class="form-horizontal" role="form">';
    html += '      <div class="form-group">';
    html += '        <label class="control-label col-sm-2" for="description">Description:</label>';
    html += '        <div class="col-sm-10">';
    html += '          <input type="text" class="form-control" id="description'+this.id+'" placeholder="Enter description" value="'+this.description+'"/>';
    html += '        </div>';
    html += '      </div>';
    html += '      <div class="form-group">';
    html += '        <label class="control-label col-sm-2" for="notification">Notification:</label>';
    html += '        <div class="col-sm-3">';
    html += '          <input type="text" class="form-control" id="alarm1_'+this.id+'" placeholder="Enter notif 1" value="'+alarm1+'"/>';
    html += '        </div>';
    html += '        <div class="col-sm-3">';
    html += '          <input type="text" class="form-control" id="alarm2_'+this.id+'" placeholder="Enter notif 2" value="'+alarm2+'"/>';
    html += '        </div>';
    html += '        <div class="col-sm-3">';
    html += '          <input type="text" class="form-control" id="alarm3_'+this.id+'" placeholder="Enter notif 3" value="'+alarm3+'"/>';
    html += '        </div>';
    html += '      </div>';
    html += '      <div class="form-group"> ';
    html += '        <label class="control-label col-sm-2" for="endDate">End date:</label>';
    html += '        <div class="col-sm-10">';
    html += '          <input type="text" class="form-control" id="datetimepicker'+this.id+'" placeholder="Enter end date" value="'+strEndDate+'"/>';
    html += '          <script type="text/javascript">';
    html += '              $(function () {';
    html += '                  $("#datetimepicker'+this.id+'").datetimepicker({';
    html += '                locale: "fr",';
    html += '                format: "L"';
    html += '                  });';
    html += '              });';
    html += '          </script>';
    html += '        </div> ';
    html += '      </div>';
    html += '      <div class="form-group">';
    html += '        <div class="col-sm-offset-2 col-sm-10">';
    html += '          <button onclick="sendForm(\'update\',\''+this.id+'\')" class="btn btn-warning">Save</button>';
    if(tagIsSelected == true) {
        html += '      <button disabled onclick="sendForm(\'delete\',\''+this.id+'\')" class="btn btn-danger disabled">Delete</button>';
    } else {
        html += '      <button onclick="sendForm(\'delete\',\''+this.id+'\')" class="btn btn-danger">Delete</button>';

    }


    
    html += '        </div>';
    html += '      </div>';

    html += '    </div>';
    html += '    </div>';
    html += '  </div>';
    html += '</div>';


    return html;
} 