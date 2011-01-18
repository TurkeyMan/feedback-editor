<html>
  <head>
    <title>FeedBack - The Official Website</title>
    <link rel="stylesheet" type="text/css" href="style.css" />
    <style type="text/css">
      <!-- 
        @import('style.css');
        .menulink { text-decoration: none; color: #FFFF00; }
        TABLE.pagecontent { background-color: #B22222; }
        TD.infotitle { background-color: #B22222; color: #FFFF00; text-align: center; }
        TD.infotext { background-color: #000000; color: #B0B0B0; text-align: left; }
        TD.infofooter { background-color: #400000; color: #FFFFFF; text-align: center; }
        .style3 { font-size: 24px; font-weight: bold; }
        .style4 {font-size: xx-large}
      -->
    </style>
    <script language="Javascript" src="http://www.dotblip.com/feedback/ccount/display.php">
      <!--
      //-->
    </script>
  </head>
  <body bgcolor="#000000" leftmargin="0" topmargin="0" marginwidth="0" marginheight="0">
    <table border="0" cellspacing="0" cellpadding="0" bgcolor="#000000" width="1000">
      <tr>
        <td><font size="20" color="#FFFFFF">&nbsp;&nbsp;FeedBack</font></td>
        <td align="right" width="*"><font size="20" color="#FFFFFF">Developer Home&nbsp;&nbsp;</font></td>
      </tr>
      <tr>
        <td height="1" bgcolor="#FFFFFF" colspan="2"><img src='blank.gif' width='1' height='1'></td>
      </tr>
      <tr>
        <td><font color="#FFFFFF"><b>&nbsp; <a href="index.php" class="menulink">Developer Home</a> | <a href="index.php?page=contribute" class="menulink">Contribute</a> | <a href="index.php?page=translation" class="menulink">Translate FeedBack</a> | <a href="index.php?page=bugs" class="menulink">Report Bugs</a></b></font></td>
        <td align="right"><font color="#FFFFFF"><b><?
        if(!$user)
        {
			echo("<a href=\"index.php?page=login\" class=\"menulink\">Log In</a> | ");
        }
        ?><a href="index.php?page=contact" class="menulink">Contact Us</a> &nbsp;</b></font></td>
      </tr>
      <tr>
        <td height="1" bgcolor="#FFFFFF" colspan="2"><img src='blank.gif' width='1' height='1'></td>
      </tr>
    </table>
    <table class="pagecontent" border="0" cellspacing="0" cellpadding="0" width="1000">
      <tr>
        <td>
<?
	$page = $_GET['page'];

	switch($page)
	{
		case "translation":
			include("translation.php");
			break;			
		case "contribute":
			include("contribute.php");
			break;			
		default:
			include("home.php");
			break;
	}
?>
        </td>
      </tr>
    </table>
  </body>
</html>