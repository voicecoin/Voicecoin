<?php
session_start();
require_once __DIR__ . '/../tools/include.php';

if (!empty($_COOKIE["lang"])) {
	$lang=$_COOKIE["lang"];
	require("lang/".$lang.".php");
} else {
	$lang="en";
	setcookie("lang","en",time()+(3600*24*14), "/");
	require("lang/en.php");
}

date_default_timezone_set('UTC');
$include_file="home";
if (isset($_SERVER['REQUEST_URI'])) {
	$URI=explode('/',$_SERVER['REQUEST_URI']);
	if ($URI[1]=="api") {
		$include_file="api";
	}
	if ($URI[1]=="openapi") {
		$include_file="openapi";
	}
	if ($URI[1]=="chain") {
		$include_file="chain";
	}
	if ($URI[1]=="stats") {
		$include_file="stats";
	}
	if ($URI[1]=="chart") {
		$include_file="chart";
	}
	if ($URI[1]=="block") {
		$include_file="block";
	}
	if ($URI[1]=="name") {
		$include_file="name";
	}
	if ($URI[1]=="tx") {
		$include_file="tx";
	}
	if ($URI[1]=="address") {
		$include_file="address";
	}
	if ($URI[1]=="account") {
		$include_file="account";
	}
	if ($URI[1]=="top") {
		$include_file="top";
	}
	if ($URI[1]=="nvs") {
		$include_file="nvs";
	}
	if ($URI[1]=="vcboard") {
		$include_file="emerboard";
	}
	if ($URI[1]=="dpo") {
		$include_file="dpo";
	}
	if ($URI[1]=="cointrace") {
		$include_file="cointrace";
	}
	if ($URI[1]=="tos") {
		$include_file="tos";
	}
	if ($URI[1]=="poscal") {
		$include_file="poscal";
	}
	$searchinput="";
	if ($URI[1]=="search") {
		$include_file="search";
		if (isset($URI[2])) {
			$searchinput=$URI[2];
		}
	}
}

?>

<?php
if ($include_file=="api" || $include_file=="openapi") {
	include ($include_file.".php");
	exit;
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
	<link rel="stylesheet" type="text/css" href="/css/jquery.dataTables.css">
	<link rel="stylesheet" type="text/css" href="/css/dataTables.colVis.css">
	<link rel="stylesheet" href="/font-awesome/css/font-awesome.min.css">
	<link rel="stylesheet" href="/css/bootstrap-select.min.css">
	<link rel="stylesheet" href="/css/style.css">
	<script src="/js/jquery.min.js"></script>
	<script src="/js/jquery-ui.min.js"></script>
	<script src="/js/jquery.tablesorter.min.js"></script>
	<script src="/js/jquery.metadata.js"></script>
	<script src="/js/bootstrap-select.js"></script>
	<script src="/bootstrap/js/bootstrap.min.js"></script>
	<script src="/js/notify-custom.js" type="text/javascript"></script>
	<script src="/js/qrcode.min.js" type="text/javascript"></script>
	<script src="/js/highstock.js" type="text/javascript"></script>
	<script src="/js/highcharts.js" type="text/javascript"></script>
	<script src="/js/exporting.js" type="text/javascript"></script>
	<script type="text/javascript" language="javascript" src="/js/jquery.dataTables.min.js"></script>
	<script type="text/javascript" language="javascript" src="/js/dataTables.colVis.min.js"></script>
	<script type="text/javascript" language="javascript" src="/js/bootstrap.datatable.js"></script>
	<script type="text/javascript" language="javascript" src="/js/d3.js"></script>
	<script type="text/javascript" language="javascript" src="/js/d3.layout.js"></script>
	<style type="text/css">
		a:hover {
			cursor:pointer;
		}
	</style>
	<title>voicechain Blockchain</title>
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

    <!-- Collect the nav links, forms, and other content for toggling -->
    <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
      <ul class="nav navbar-nav">
				<li <?php if ($include_file=='chain') { echo 'class="active"'; } ?>><a href="/chain"><?php echo lang('CHAIN_CHAIN'); ?></a></li>
				<li <?php if ($include_file=='stats') { echo 'class="active"'; } ?>><a href="/stats"><?php echo lang('STATS_STATS'); ?></a></li>
				<li <?php if ($include_file=='chart') { echo 'class="active"'; } ?>><a href="/chart"><?php echo lang('CHARTS_CHARTS'); ?></a></li>
				<li <?php if ($include_file=='top') { echo 'class="active"'; } ?>><a href="/top"><?php echo lang('TOP_TOP'); ?></a></li>
				<li <?php if ($include_file=='nvs') { echo 'class="active"'; } ?>><a href="/nvs"><?php echo lang('NVS_NVS'); ?></a></li>
		<!--		<li <?php if ($include_file=='dpo') { echo 'class="active"'; } ?>><a href="/dpo"><?php echo 'DPO'; ?></a></li>
				<li <?php if ($include_file=='poscal') { echo 'class="active"'; } ?>><a href="/poscal"><?php echo 'Mining-Calculator'; ?></a></li>
				<li <?php if ($include_file=='vcboard') { echo 'class="active"'; } ?>><a href="/vcboard"><?php echo lang('EMERBOARD_EMERBOARD'); ?></a></li> -->
			</ul>
		<form class="navbar-form navbar-left" role="search" action="javascript:search();">
			<div class="form-group ">
				<input type="text" id="search" class="form-control" placeholder="<?php echo lang('ADDRESS_TX'); ?>" value="<?php echo $searchinput; ?>">
			</div>
			<button type="submit" class="btn btn-default"><?php echo lang('SHOW_SHOW'); ?></button>
		</form>
		<ul class="nav navbar-nav navbar-right">
			<li class="dropdown">
				<a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">
				<span class="label label-primary">Testnet</span>
				<span class="caret"></span></a>
							  <ul class="dropdown-menu">
							    <!-- <li><a href="https://emercoin.mintr.org"> Mainnet</a></li> -->
							    <li><a href="http://alpha.voicecoin.com"> Testnet</a></li>
					</ul>
			</li>
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
    </div><!-- /.navbar-collapse -->
  </div><!-- /.container-fluid -->
</nav>

	<script type="text/javascript">

	$("#logout").click(function() {
		var request = $.ajax({
			type: "GET",
			url: "/usrmgmt/logout.php"
		});
		request.done(function( response ) {
			window.location.href = '/wallet';
		});
	});

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

	function search() {
		window.location.href = '/search/'+$("#search").val();
	};
	</script>

	<!-- Beginn include -->
	<?php
		include ($include_file.".php");
	?>
	<!-- Ende include-->
<!-- <footer class="footer">
      <div class="container" >
		<button class="btn btn-xs btn-primary" type="button" data-toggle="collapse" data-target="#collapseDonations" aria-expanded="false" aria-controls="collapseDonations">
			<?php echo lang('DO_SERVICE'); ?>
		</button>
		<br>
		<sub><a href="https://emercoin.mintr.org/api/help">API</a> | <a href="https://hashflare.io/r/AD872628" target="_blank">Cloud Mining</a> | <a href="https://livecoin.net?from=Livecoin-20e00c47" target="_blank">voicecoin Exchange</a></sub>
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
		<sub><?php echo lang('INFO_FEEDBACK'); ?>: <a href="https://github.com/Valermos/emercoin-blockchain-explorer" target="_blank">GitHub</a> | <a href="mailto:admin@mintr.org">E-Mail</a></sub><br>
		<sub>v.1.8.2</sub></p>
      </div>
</footer> -->
</body>

</html>
