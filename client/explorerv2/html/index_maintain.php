<?php
session_start();
error_reporting(E_ALL);
ini_set("display_errors", 1);
date_default_timezone_set('UTC');

if (!empty($_COOKIE["lang"])) {
	$lang=$_COOKIE["lang"];
	require("lang/".$lang.".php");
} else {
	$lang="en";
	setcookie("lang","en",time()+(3600*24*14), "/");
	require("lang/en.php");
}
?>


<!DOCTYPE html>
<html>

<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<link rel="shortcut icon" href="favicon.ico" type="image/x-icon" />
	<link rel="stylesheet" href="/bootstrap/css/bootstrap.min.css">
	<link rel="stylesheet" href="/bootstrap/css/bootstrap-theme.min.css">
	<link rel="stylesheet" href="/font-awesome/css/font-awesome.min.css">
	<link rel="stylesheet" href="/css/style.css">
	<script src="/js/jquery.min.js"></script>
	<script src="/js/jquery-ui.min.js"></script>
	<script src="/js/jquery.tablesorter.min.js"></script>
	<script src="/js/jquery.metadata.js"></script>
	<script src="/bootstrap/js/bootstrap.min.js"></script>
	<script src="/js/notify-custom.js" type="text/javascript"></script>
	<script src="/js/highstock.js" type="text/javascript"></script>
	<script src="/js/exporting.js" type="text/javascript"></script>
	<title>voicecoin Blockchain</title>
</head>

<body>
<!-- Fixed navbar -->
<nav class="navbar navbar-default">
  <div class="container-fluid">
    <!-- Brand and toggle get grouped for better mobile display -->
    <div class="navbar-header">
      <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#bs-example-navbar-collapse-1">
        <span class="sr-only">Toggle navigation</span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
		<a class="navbar-brand" style="padding:11px" href="/"><img src="/img/emercoin_header.png" height="28" ></a>
    </div>

	<ul class="nav navbar-nav navbar-right">
		<li class="dropdown">
			<a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">
			<?php
			switch ($lang){
				case "cn":
					echo '<img src="/lang/cn.png"> CN ';
					break;
				case "de":
					echo '<img src="/lang/de.png"> DE ';
					break;
				case "en":
					echo '<img src="/lang/en.png"> EN ';
					break;
				default:
					echo '<img src="/lang/en.png"> EN ';
					break;
			}
			?>
			<span class="caret"></span></a>
						  <ul class="dropdown-menu">
						    <li><a href="javascript:setLanguage('cn');"><img src="/lang/cn.png"> CN</a></li>
						    <!-- <li><a href="javascript:setLanguage('de');"><img src="/lang/de.png"> DE</a></li> -->
							<li><a href="javascript:setLanguage('en');"><img src="/lang/en.png"> EN</a></li>
						  </ul>
						</li>

      </ul>
  </div><!-- /.container-fluid -->
</nav>

	<script type="text/javascript">
	function setLanguage(lang) {
		var request = $.ajax({
			type: "POST",
			url: "/lang/setlanguage.php",
			data: { lang: lang }
		});
		request.done(function( response ) {
			location.reload();
		});
	};
	</script>

	<!-- Beginn include -->
	<div class="container">
	<div class="jumbotron">
		<h2><strong><?php echo lang('MAINTENANCE_MAINTENANCE'); ?></strong></h2>
	</div>
	<p>
</div>
	<!-- Ende include-->
<!-- <footer class="footer">
      <div class="container" >
		<button class="btn btn-xs btn-primary" type="button" data-toggle="collapse" data-target="#collapseDonations" aria-expanded="false" aria-controls="collapseDonations">
			<?php echo lang('DO_SERVICE'); ?>
		</button>
		<br>
		<sub><a href="https://hashflare.io/r/AD872628" target="_blank">Cloud Mining</a> | <a href="https://livecoin.net?from=Livecoin-20e00c47" target="_blank">voicecoin Exchange</a></sub>
		<font size="1%"><a href="emclnx/robotrap.php">&#21;</a></font>
		<div class="collapse" id="collapseDonations">
		<p class="text-muted"><sub>
		<b><?php echo lang('DONATION_DONATION'); ?></b><br>
			[VC] <a href="emercoin:EZRcD6ZfGJLaYSkKwZYRkrhVGxk1mimG93?amount=0&label=Donation - emercoin.mintr.org">EZRcD6ZfGJLaYSkKwZYRkrhVGxk1mimG93</a><br>
			[PPC] <a href="peercoin:PTDnVfgcDVk417rLXrokC3nLj44tFZ3Hyw?amount=0&label=Donation - emercoin.mintr.org">PTDnVfgcDVk417rLXrokC3nLj44tFZ3Hyw</a><br>
			[BTC] <a href="bitcoin:1N1G4Ac8vkvMpMKDksrxuHpkRiC86KsBvX ?amount=0&label=Donation - emercoin.mintr.org">1N1G4Ac8vkvMpMKDksrxuHpkRiC86KsBvX </a><br>

		<b><?php echo lang('REFERRAL_REFERRAL'); ?></b><br>
			Start mining Bitcoins from $1.50 per 10GH/s here: <a href="https://hashflare.io/r/AD872628" target="_blank">Hashflare.io</a><br>
			Sell your Bitcoins for voicecoins here: <a href="https://livecoin.net?from=Livecoin-20e00c47" target="_blank">Livecoin.net</a><br>
		</sub></p>
		</div>
		<p class="text-muted">
		<sub><?php echo lang('INFO_FEEDBACK'); ?>: <a href="https://bitcointalk.org/index.php?topic=958008.0" target="_blank">Bitcointalk</a> | <a href="https://github.com/Valermos/emercoin-blockchain-explorer" target="_blank">GitHub</a> | <a href="mailto:admin@mintr.org">E-Mail</a></sub><br>
		<sub>v.1.8.2</sub></p>
      </div>
</footer> -->
</body>

</html>
