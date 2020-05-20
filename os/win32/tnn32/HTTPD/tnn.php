<html>
  <head>
    <meta http-equiv="content-type" content="text/html;charset=UTF-8" />

    <title>TheNetNode Knoten Glaubitz</title>
    <link rel=stylesheet type=text/css href=sv.css>
	
    <script type="text/javascript">
      var framefenster = document.getElementsByTagName("iFrame");
      var auto_resize_timer = window.setInterval("autoresize_frames()", 400);

      function autoresize_frames()
      {
        for (var i = 0; i < framefenster.length; ++i)
	{
          if(framefenster[i].contentWindow.document.body)
	  {
            var framefenster_size = framefenster[i].contentWindow.document.body.offsetHeight;
		  
            if(document.all && !window.opera)
	    {
              framefenster_size = framefenster[i].contentWindow.document.body.scrollHeight;
            }

            framefenster[i].style.height = framefenster_size + 'px';
          }
        }
      }
    </script>
  </head>

  <body>
    <div id="bgoverlay">
      <div id="wrapper">
        <div id="login">


         <div id="navigation">
            <a href="start.php" class="navi" style="border-right: 1px solid #ffffff;">Home</a><a href="cmd?cmd=u" class="navi" style="border-right: 1px solid #ffffff;">Login</a><a href="tnn.php" class="navi" id="nav_active" style="border-right: 1px solid #ffffff;">TheNetNode</a><a href="faq.php" class="navi" style="border-right: 1px solid #ffffff;">FAQ</a>

        </div>



        <div id="content">
          <!-- CONTENT ///////////////////////////////////////////////////////////////////////////////// -->
            <iFrame src="http://127.0.0.1/TheNetNode" style="width:100%; height:100%; border:none;" frameborder="2" name="irgendwas" scrolling="no"></iFrame>
          <!-- \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ -->
          <span style="clear:both;"></span>
        </div>
    </div>
  
    </div>

    <script type="text/javascript">
      var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");

      document.write(unescape("%3Cscript src='" + gaJsHost + "google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E"));
    </script>

    <script type="text/javascript">
      var pageTracker = _gat._getTracker("UA-5648074-1");

      pageTracker._initData();
      pageTracker._trackPageview();
    </script>

  </body>
</html>
