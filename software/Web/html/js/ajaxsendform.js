function createXMLOBJ()
 {
 var xmlHttp;
 
 try
    {
   // Firefox, Opera 8.0+, Safari
    xmlHttp=new XMLHttpRequest();
    return xmlHttp;
    }
 catch (e)
    {

  // Internet Explorer
   try
      {
      xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
      return xmlHttp;
      }
   catch (e)
      {

      try
         {
         xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
         return xmlHttp;
         }
      catch (e)
         {
         alert("�����������֧��AJAX��");
         return false;
         }
      }
    }
 }


function getObjectvalue ( elementID )
{
	

				return document.getElementById( elementID ).value;

	

}

function getObject ( elementID )
{
				return document.getElementById( elementID );

}