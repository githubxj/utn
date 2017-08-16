function isFuncKey(object,string)
{ 
	var str = object.value;
	var len = str.length;
	var name = string;
	var myReg = /^[*]/;
	if (len != 2) {
		alert("\nERROR:The length of \""+name+"\" must be 2.\n\nPlease enter again.");
		object.focus();
		return false;
	}
	if(!myReg.test(str)) {
		alert("\nERROR:¡°"+name+"¡±must be started with \'*\' .\n\nPlease enter again.");
		object.focus();
		return false;
	}
	return true;
}

function isMultiName(object,string)
{
	var str = object.value;
	var name = string;
	var myReg = /^[a-zA-Z]/; 
	if (str == "") {
		alert("\nERROR:The \""+name+"\" field is blank.\n\nPlease enter again.");
		object.focus();
		return false;
	}
	if(!myReg.test(str)) {
		alert("\nERROR:¡°"+name+"¡±must be started with letter¡£\n\nPlease enter again.");
		object.focus();
		return false;
	}
	return true;
}



function isnotEmpty(object,string)
{
	var str = object.value;
	var name = string;
	if (str == "") {
		alert("\nERROR:The \""+name+"\" field is blank.\n\nPlease enter again.");
		object.focus();
		return false;
	}
	return true;
}

function isName(object,string) {
	var str = object.value;
	var name = string;
	var myReg = /^[ | ]+$/
	if (myReg.test(str)) {
		alert("\nERROR:The \""+name+"\" field is blank.\n\nPlease enter again.");
		object.focus();
		return false;
	}
	return true;
}

function isEmail(object) { 
	var str = object.value;
  	var myReg = /^\S+@(\S+\.)+\S{2,3}$/; 
  	if(myReg.test(str)) return true; 
  	else
  		alert("\nERROR:The E-mail field is incorrect.\n\nPlease re-enter your E-mail.");
	object.select();
	object.focus();
  	return false; 
} 

function isIpAddress(object) { 
	/*var str = object.value;
	if (str == "") {
		alert("\nERROR:The IP address field is incorrect.\n\nPlease re-enter your IP address.");
		object.focus();
		return false;
	}
	return true;
	*/
	var str = object.value;
  	var myReg = /^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$/; 
  	if(myReg.test(str)) {
  		var ipaddress=str.split(".");
  		for(i=0;i<ipaddress.length;i++)
  		 {
  		 	if (ipaddress[i]<0||ipaddress[i]>255) {
  		 		   break;
  		      }
  		 } 
  		 if (i==ipaddress.length)   		
  		return true;
  	   else
  	  	 alert("\nERROR:The IP address field is incorrect.\n\nPlease re-enter your IP address.");
  	}	   
  	else
  		alert("\nERROR:The IP address field is incorrect.\n\nPlease re-enter your IP address.");
	object.select();
	object.focus();
  	return false; 
	
} 

function isNumber(object,string) { 
	var str = object.value;
	var name = string;
  	var myReg = /^[0-9]+$/; 
  	if(myReg.test(str)) return true; 
  	else
  		alert("\nERROR:The \"" + name +"\" field is incorrect.\n\nPlease re-enter.");
	object.select();
	object.focus();
  	return false; 
}


function isPpcParaOK(object,string,string1,len) { 
	var str = object.value;
	var name = string;
  if(isNumber(object,string))
  {
  	if(string1=='int')
  	{
  		if(len==0)
  		{
  			if(str>2147483647)
  			{
  				alert("\nERROR:¡°"+name+"¡±is too big.\n\nPlease re-enter.");
  				object.select();
					object.focus();
  				return false; 
  			}
  			else 
  				return true;
  		}
  		else
  		{
  			if(str>len)
  			{
  				alert("\nERROR:¡°"+name+"¡±length can't be bigger than "+len+".\n\nPlease re-enter.");
  				object.select();
					object.focus();
  				return false; 
  			}
  			else
  				return true;
  		}
  	}
  	else
  	{
  		if(str.length>len)
  		{
  			alert("\nERROR:¡°"+name+"¡±length can't be bigger than "+len+".\n\nPlease re-enter.");
  			object.select();
				object.focus();
  			return false;
  		}
  		return true;
  	}
  }
  else
  {
		 object.select();
		 object.focus();
  	 return false;
	}
} 

function isEqual(object1,object2) { 
	var str1 = object1.value;
	var str2 = object2.value;
  	if(str1.length!=0&&str1==str2) return true; 
  	else  
  		alert("\nERROR:The Password is incorrect.\n\nPlease re-enter your password.");

  	return false; 
} 

function isSipPort(object,string) { 
	var str = object.value;
	var name = string;
  	var myReg = /^[_0-9]+$/; 
  	if(myReg.test(str)&&str<=65535&&str>=1000)   
  		return true;	
  	else
  		alert("\nERROR:The \"" + name +"\" field is invalid.\n\nThe value is only between 1000 and 65535.");
	object.select();
	object.focus();
  	return false; 
} 
 
function isPort(object1,object2) { 
	var start = object1.value;
	var end = object2.value;
  	if(start>65535||start<1000){
  		alert("\nERROR:The Start RTP Port is invalid.\n\nThe value is only between 1000 and 65535.");
  		return false;
  	}
  	if(end>65535||end<1000) {
  		alert("\nERROR:The End RTP Port is invalid.\n\nThe value is only between 1000 and 65535.");
  		return false;
  	}
        if(start-end>=0) {
                alert("\nERROR:The End RTP Port must be larger than Start RTP Port.");
  		return false; 
  	}
  	return true;
} 

function isLarge(object1,object2) {
	var extstart = object1.value;
	var extend   = object2.value;
	
	if (extend-extstart < 0) {
		alert("\nERROR:The End Value must be larger than or equal to Start Value.");
  		return false; 
  	}
  	
  	return true;
}
	
function isOld(object1,object2,object3,object4,object5,object6,object7,object8,object9,object10){
	var styear = object1.value;
	var stmon  = object2.value;
	var stday  = object3.value;
	var sthour = object4.value;
	var stmin  = object5.value;
	var edyear = object6.value;
	var edmon  = object7.value;
	var edday  = object8.value;
	var edhour = object9.value;
	var edmin  = object10.value;
	
	if(edyear-styear<0){
		alert("\nERROR:The End Date must be larger than Start Date.");
  	return false; 
	}
	else if(edyear-styear == 0){
		if(edmon-stmon<0){
			alert("\nERROR:The End Date must be larger than Start Date.");
  		return false; 
		}
		else if(edmon-stmon == 0){
			if(edday-stday<0){
				alert("\nERROR:The End Date must be larger than Start Date.");
  			return false; 
			}
			else if(edday-stday == 0){
				if(edhour-sthour<0){
					alert("\nERROR:The End Date must be larger than Start Date.");
  				return false; 
				}
				else if(edhour-sthour == 0){
					if(edmin-stmin<0){
						alert("\nERROR:The End Date must be larger than Start Date.");
  					return false; 
					}
					else if((edyear == styear)&&(edmon == stmon)&&(edday == stday)&&(edhour == sthour)&&(edmin == stmin)){
						alert("\nERROR:The End Date must be larger than Start Date.");
  					return false; 
					}
				}
			}
		}
	}
	return true;
}

function isDate(object1,object2,object3,object4,object5){
	var year = object1.value;
	var mon  = object2.value;
	var day  = object3.value;
	var hour = object4.value;
	var min  = object5.value;

	switch(mon){
		case '1':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '2':
			if(year%4==0){
				if(day>29||day<1){
					alert("\n Error: Please input date correctly");
					object3.select();
					object3.focus();
  				return false; 
				}
			}
			else{
				if(day>28||day<1){
					alert("\n Error: Please input date correctly");
					object3.select();
					object3.focus();
  				return false; 
				}
			}
			break;
		case '3':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '4':
			if(day>30||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '5':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '6':
			if(day>30||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '7':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '8':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '9':
			if(day>30||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '10':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '11':
			if(day>30||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '12':
			if(day>31||day<1){
				alert("\n Error: Please input date correctly");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		default:{
			alert("\n Error: Please input date correctly");
			object2.select();
			object2.focus();
  		return false; 
  	} 
	}
	
	if(hour>23||hour<0){
		alert("\n Error: Please input time correctly");
		object4.select();
		object4.focus();
  	return false; 
	}
	if(min>59||min<0){
		alert("\n Error: Please input time correctly");
		object5.select();
		object5.focus();
  	return false; 
	}
	return true;
}
function isPktTimeOK(object,object2) {
	var str = object.value;
	var name = object2.value;
	if (((str == "g711ulaw" || str == "g711alaw") && name!="20") || (str == "g723" && name!="30" && name!="60") || (str=="g729" && name!="20" && name!="40")) {
		alert("\nERROR:The \"pktTime0\" field is not right.\n\nPlease check it!");
		object.focus();
		return false;
	}
	return true;
}

function changelanguage() {
	alert("Please refresh the page after submission if language modification is needed");
}

function resetChan(){
	alert("Warning: After modify Physical Medium, Please reconfigure channel, and cold reboot your server.");	
}

function promptreboot(){
  alert("Prompt:Please cold reboot your server, after configure ports, and hot reload.");
}

function promptreset()
{
	var sel=confirm("Warning:To press 'OK' button will clean all channels configureation ,and reset ports configuration.Please update configuration and reboot your server machine after press 'OK' button");
	if (sel==true)
	{
		return true;
	}
	else
	{
		return false;
	}

}     

function ChanChanged(){
	  var card = document.getElementById("id_card");
    var port = document.getElementById('id_port');
    var st = document.getElementById("id_stchan");
    var ed = document.getElementById('id_edchan');
    var signal = document.getElementById('id_signal');
    var chantype = document.getElementById('id_chantype');
		var cardindex = card.options.selectedIndex;
		var portidx = port.options.selectedIndex;
	    	
	  if(isFirst==true){
			setMenu();
			isFirst=false;
		}
		
			
		if(media[cardindex]=='T1'){
				if(portidx==0){
					if(st.value<1 || st.value>23 || ed.value<1 || ed.value>23){
				    alert("\nPrompt: Channel range of this port is 1-23, please reset it.\n");
				    return false;
				  }else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
				}else if(portidx==1){
					if(st.value<25 || st.value>47 || ed.value<25 || ed.value>47){
	    	    alert("\nPrompt: Channel range of this port is 25-47, please reset it.\n");
	    	    return false;
	    	  }else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
	      }else if(portidx==2){
	      	if(st.value<49 || st.value>71 || ed.value<49 || ed.value>71){
	      	  alert("\nPrompt: Channel range of this port is 49-71, please reset it.\n");
	      	  return false;
	      	}else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
	      }else if(portidx==3){
	      	if(st.value<73 || st.value>95 || ed.value<73 || ed.value>95){
	      	  alert("\nPrompt: Channel range of this port is 73-95, please reset it.\n");
	      	  return false;
	        }else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
	      }
    }else if(media[cardindex]=='E1'){
    		if(portidx==0){
    			if((st.value<1 || st.value==16 || st.value>31 )||(ed.value<1 || ed.value==16 || ed.value>31 )||(st.value<16 && ed.value>16)){
				    alert("\nPrompt: Channel range of this port is 1-15 or 17-31, please reset it.\n");
				    return false;
				  }else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
				}else if(portidx==1){
					if((st.value<32 || st.value==47 || st.value>62 )||(ed.value<32 || ed.value==47 || ed.value>62 )||(st.value<47 && ed.value>47)){
	    	    alert("\nPrompt: Channel range of this port is 32-46 or 48-62, please reset it.\n");
	    	    return false;
	    	  }else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
	      }else if(portidx==2){
	      	if((st.value<63 || st.value==78 || st.value>93 )||(ed.value<63 || ed.value==78 || ed.value>93 )||(st.value<78 && ed.value>78)){
	      	  alert("\nPrompt: Channel range of this port is 63-77 or 79-93, please reset it.\n");
	      	  return false;
	      	}else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
	      }else if(portidx==3){
	      	if((st.value<94 || st.value==109 || st.value>124 )||(ed.value<94 || ed.value==109 || ed.value>124 )||(st.value<109 && ed.value>109)){
	      	  alert("\nPrompt: Channel range of this port is 94-108 or 110-124, please reset it.\n");
	      	  return false;
	      	}else if(ed.value-st.value<0){
				    alert("\nEnd channel number must be larger than start channel number, please reset it.\n");
				    return false;
				  }
	      }
   }else if(media[cardindex]=='FXO/FXS'){
    		if(portidx==0){
				  if(signal.value=='fxs_ks'){
    				if(chantype.value=='2FXO/2FXS'){
    			     if(st.value<3 || st.value>4 ){
				           alert("\nPrompt: Channel range of this port is 3-4, please reset it.\n");
				           return false;
				       }
				    }else if(chantype.value=='4FXO/4FXS'){
    			     if(st.value<3 || (st.value>4 && st.value<7) || st.value >8){
				           alert("\nPrompt: Channel range of this port is 3-4¡¢7-8, please reset it.\n");
				           return false;
				       }
				    }else if(chantype.value=='6FXO/6FXS'){
    			     if(st.value<3 || (st.value>4 && st.value<7) || (st.value >8 && st.value < 11) || st.value > 12){
				           alert("\nPrompt: Channel range of this port is 3-4¡¢7-8¡¢11-12, please reset it.\n");
				           return false;
				       }
				    }else if(chantype.value=='8FXO/8FXS'){
    			     if(st.value<3 || (st.value>4 && st.value<7) || (st.value >8 && st.value < 11) || (st.value > 12 && st.value < 15) || st.value > 16){
				           alert("\nPrompt: Channel range of this port is 3-4¡¢7-8¡¢11-12¡¢15-16, please reset it.\n");
				           return false;
				       }
				    }else if(chantype.value=='4FXO'){
				    	  if(st.value<1 || st.value>4 ){
				            alert("\nPrompt: Channel range of this port is 1-4, please reset it.\n");
				            return false;
				         }
				    }
				  }else if(signal.value=='fxo_ks'){
				  	if(chantype.value=='2FXO/2FXS'){
				  	   if(st.value<1 || st.value>2 ){
				          alert("\nPrompt: Channel range of this port is 1-2, please reset it.\n");
				          return false;
				       }
				    }else if(chantype.value=='4FXO/4FXS'){
				  	   if(st.value<1 || (st.value>2 && st.value<5) || st.value>6 ){
				          alert("\nPrompt: Channel range of this port is 1-2¡¢5-6, please reset it.\n");
				          return false;
				       }
				    }else if(chantype.value=='6FXO/6FXS'){
				  	   if(st.value<1 || (st.value>2 && st.value<5) || (st.value>6 && st.value<9) || st.value>10){
				          alert("\nPrompt: Channel range of this port is 1-2¡¢5-6¡¢9-10, please reset it.\n");
				          return false;
				       }
				    }else if(chantype.value=='8FXO/8FXS'){
				  	   if(st.value<1 || (st.value>2 && st.value<5) || (st.value>6 && st.value<9) || (st.value>10 && st.value<13) || st.value>14){
				          alert("\nPrompt: Channel range of this port is 1-2¡¢5-6¡¢9-10¡¢13-14, please reset it.\n");
				          return false;
				       }
				    }
				  }
				}
   }
   return true
}


function portChanged() {

    var card = document.getElementById("id_card");
    var port = document.getElementById('id_port');
		var cardindex = card.options.selectedIndex;
		var portidx = port.options.selectedIndex;
	    	
	  if(isFirst==true){
			setMenu();
			isFirst=false;
		}
		
		if(media[cardindex]=='T1'){
				if(portidx==0){
				   alert("\nPrompt: Channel range of this port is 1-23.\n");
				}else if(portidx==1){
	    	   alert("\nPrompt: Channel range of this port is 25-47.\n");
	      }else if(portidx==2){
	      	 alert("\nPrompt: Channel range of this port is 49-71.\n");
	      }else if(portidx==3){
	      	 alert("\nPrompt: Channel range of this port is 73-95.\n");
	      }
    }else if(media[cardindex]=='E1'){
    		if(portidx==0){
				   alert("\nPrompt: Channel range of this port is 1-15 and 17-31.\n");
				}else if(portidx==1){
	    	   alert("\nPrompt: Channel range of this port is 32-46 and 48-62.\n");
	      }else if(portidx==2){
	      	 alert("\nPrompt: Channel range of this port is 63-77 and 79-93.\n");
	      }else if(portidx==3){
	      	 alert("\nPrompt: Channel range of this port is 94-108 and 110-124.\n");
	      }
   }else if(media[cardindex]=='FXO/FXS'){
    		if(portidx==0){
				   alert("\nPrompt: Channel range of this port is 1-4.\n");
				}
   }
}

function isRange(object1,var1,var2,string) {
	var str = object1.value;
	var name = string;
	if (str<var1 || str>var2) {
		alert("\nERROR:The \""+name+"\" field is in the range of ("+var1+","+var2+").\n\nPlease enter again.");
		object1.focus();
		return false;
	}
	return true;
}

function isMAC(object1,string1) {
	var str = object1.value;
	var myReg = /[^0123456789ABCDEFabcdef]/;
	var name = string1;
	
	if(myReg.exec(str) || str.length != 17) {
		alert("\nERROR:¡°"+name+"¡±must be a MAC address¡£\n\nPlease enter again.");
		object1.focus();
		return false;
	}
	return true;
}

function checkLength(object1,string1) {
	var str = object1.value;
	var name = string1;
	
	if(str.length > 10) {
		alert("\nERROR:¡°The length of "+name+"¡±should be less than 10¡£\n\nPlease enter again.");
		object1.focus();
		return false;
	}
	return true;
}

function isInRange(Object,value1,value2,String) {
	var value = Object.value;
	if((value< value1)||(value > value2)){
		alert("\nERROR:The \""+String+"\" field is over range ("+value1+"-"+value2+").\n\nPlease enter again.");
		object.focus();
		return false;
	}
	return true;
}

function isLargeThanValue(object,string) { 
	var str = object.value;
	var name = string;
  	var myReg = /^[_0-9]+$/; 
  	if(myReg.test(str)&&str<=30&&str>=0)   
  		return true;	
  	else
  		alert("\nERROR:The \"" + name +"\" field is invalid.\n\nThe value is only between 0 and 30.");
	object.select();
	object.focus();
  	return false; 
} 

function isLengthRange(object,value1,value2,string) {
	var str = object.value;
	var name =string;
  if(str.length>=value1&&str.length<=value2)
      return true;
  else 
    alert("\nERROR:The\"" + name +"\" field is invalid.\n\nThe value is only between"+value1+"and"+value2+".");
  object.focus();
     return false;
}

function isMobile(object,string) { 
	var str = object.value;
	var name = string;
  	var myReg = /^[_0-9]+$/; 
  	if(myReg.test(str)&&str.length==11) return true; 
  	else
  		alert("\nError:The\""+name+"\"field is invalid.\n\nIt's length must be 11.");
	object.select();
	object.focus();
  	return false; 
}         

function checkLength16(object1,string1) {
	var str = object1.value;
	var name = string1;
	
	if(str.length !=16) {
		alert("\nERROR:¡°The length of "+name+"¡±should be 16.\n\nPlease enter again.");
		object1.focus();
		return false;
	}
	return true;
}

function checkLength128(object1,string1) {
	var str = object1.value;
	var name = string1;
	
	if(str.length >128) {
		alert("\nERROR:¡°The length of "+name+"¡±should less than 128.\n\nPlease enter again.");
		object1.focus();
		return false;
	}
	return true;
}