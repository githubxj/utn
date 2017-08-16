function trim(str)
{
	while(str.charAt(str.length-1)==' ')
  	str=str.substring(0,str.length-1);
  	return str;
}

function isnull(obj)
{
	if(trim(obj.value) == null ||trim(obj.value) == "")
	{
		alert("\n错误:该区域不允许为空。\n\n请重新输入。");
		obj.focus();
		return true;
	}
	else
		return false;

}


function getConfirm()
{
	return(confirm('您确定要修改吗？'));
}

function onlyEngAndNum()
{
	if((window.event.keyCode>=48 && window.event.keyCode<=57) 
		|| window.event.keyCode==8 || window.event.keyCode==9 
		|| window.event.keyCode==16 || window.event.keyCode==20 || window.event.keyCode==116 
		|| (window.event.keyCode>=96 && window.event.keyCode<=105)
		|| (window.event.keyCode>=65&&window.event.keyCode<=90) ) {
		return true;
 	}
 	else
 	{
  		alert("\n错误:只能输入英文或数字.");
  		return false;
	}
}
function isEnAndNum(object,string) {
	var str = object.value;
  var myReg = /^[a-zA-Z0-9]{0,32}$/; 
  if(myReg.test(str)) {
  		return true;
  }	   
  else {
		alert("\n错误:“"+ string +"”只能输入英文或数字。\n\n请重新输入。");
		object.select();
		object.focus();
  	return false; 
  }
}

function isnotEmpty(object,string)
{
	var str = object.value;
	var name = string;
	if (str == "") {
		alert("\n错误:“"+name+"”区域为空。\n\n请重新输入。");
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
		alert("\n错误:“"+name+"”输入非法。\n\n请重新输入。");
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
  		alert("\n错误:邮箱地址不正确。\n\n请检查后重新输入。");
	object.select();
	object.focus();
  	return false; 
} 

function isIpAddress(object,string) { 
	var str = object.value;
	var name = string;
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
  	  	 alert("\n错误:“"+name+"”不正确。\n\n请检查后重新输入。");
  	}	   
  	else
  		alert("\n错误:“"+name+"”不正确。\n\n请检查后重新输入。");
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
  		alert("\n错误:“"+name+"”的值不正确。\n\n请检查后重新输入。");
	object.select();
	object.focus();
  	return false; 
} 



function isEqual(object1,object2) { 
	var str1 = object1.value;
	var str2 = object2.value;
  	if(str1.length!=0&&str1==str2) return true; 
  	else  
  		alert("\n错误:密码不一致。\n\n请检查后重新输入密码。");

  	return false; 
} 


function isLarge(object1,object2) {
	var extstart = object1.value;
	var extend   = object2.value;
	
	if (extend-extstart < 0) {
		alert("\n 错误:终止值必须不小于其起始值。");
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
	if(edyear-styear < 0){
		alert("\n 错误:终止日期必须大于其起始日期。");
  	return false; 
	}
	else if(edyear-styear == 0){
		if(edmon-stmon<0){
			alert("\n 错误:终止日期必须大于其起始日期。");
  		return false; 
		}
		else if(edmon-stmon == 0){
			if(edday-stday<0){
				alert("\n 错误:终止日期必须大于其起始日期。");
  			return false; 
			}
			else if(edday-stday == 0){
				if(edhour-sthour<0){
					alert("\n 错误:终止日期必须大于其起始日期。");
  				return false; 
				}
				else if(edhour-sthour == 0){
					if(edmin-stmin<0){
						alert("\n 错误:终止日期必须大于其起始日期。");
  					return false; 
					}
					else if((edyear == styear)&&(edmon == stmon)&&(edday == stday)&&(edhour == sthour)&&(edmin == stmin)){
						alert("\n 错误:终止日期必须大于其起始日期。");
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
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '2':
			if(year%4==0){
				if(day>29||day<1){
					alert("\n 错误:请正确填写日期。");
					object3.select();
					object3.focus();
  				return false; 
				}
			}
			else{
				if(day>28||day<1){
					alert("\n 错误:请正确填写日期。");
					object3.select();
					object3.focus();
  				return false; 
				}
			}
			break;
		case '3':
			if(day>31||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '4':
			if(day>30||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '5':
			if(day>31||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '6':
			if(day>30||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '7':
			if(day>31||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '8':
			if(day>31||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '9':
			if(day>30||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '10':
			if(day>31||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '11':
			if(day>30||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		case '12':
			if(day>31||day<1){
				alert("\n 错误:请正确填写日期。");
				object3.select();
				object3.focus();
  			return false; 
			}
			break;
		default:{
			alert("\n 错误:请正确填写月份。");
			object2.select();
			object2.focus();
  		return false;
  	} 
	}
	
	if(hour>23||hour<0){
		alert("\n 错误:请正确填写时间。");
		object4.select();
		object4.focus();
  	return false; 
	}
	if(min>59||min<0){
		alert("\n 错误:请正确填写时间。");
		object5.select();
		object5.focus();
  	return false; 
	}
	return true;
}

   

function isRange(object1,var1,var2,string1) {
	var str = object1.value;
	var name = string1;

	if (str<var1||str>var2) {
		alert("\提示: \""+name+"\" 字段的范围是("+var1+","+var2+").\n\n请重新输入！.");
		object1.focus();
		return false;
	}
	return true;
}

function isMAC(object1) {
	var str = object1.value;
	var myReg = /[^0123456789ABCDEFabcdef:]/;
  var pattern=/^\w{2}(:\w{2}){5}$/;
  if ( pattern.test(str) == false )
  {
  	alert("\n提示:MAC地址格式不正确！\n\n请重新输入.");
  	object1.focus();
		return false;
  }
	if(myReg.exec(str) || str.length != 17) {
		alert("\n提示:MAC地址含有非法字符！\n\n请重新输入.");
		object1.focus();
		return false;
	}
	
	return true;
}

function checkLength(object1,maxlen) {
	var str = object1.value;
	
	if(str.length > maxlen) {
		alert("\n错误: 该字段长度必须小于" + maxlen + "。\n\请重新输入.");
		object1.focus();
		return false;
	}
	return true;
}

function isInRange(Object,value1,value2,String) {
	var value = Object.value;
	if((value< value1)||(value > value2)){
		alert("\n错误:\""+String+"\"的值超出范围("+value1+"-"+value2+")。\n\n请重新输入。")
		Object.select();
		Object.focus();
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
  		alert("\n错误:\"" + name +"\" 的值无效.\n\n请输入0与30之间的数字。");
	object.select();
	object.focus();
  	return false; 
}

function isLengthRange(object,value1,value2,string) {
	var str = object.value;
	var name = string;  
  if(str.length >= value1&&str.length <= value2)
      return true;
  else 
    alert("\n错误:\"" + name +"\" 的值无效.\n\n请输入"+value1+"与"+value2+"之间的数字。");
  object.focus();
     return false;
}   
      
