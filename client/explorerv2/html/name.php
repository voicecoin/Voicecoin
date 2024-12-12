<?php
if (isset($_SERVER['REQUEST_URI'])) {
	$URI=explode('/',$_SERVER['REQUEST_URI']);
	if ($URI[1]=="name") {
		if (isset($URI[2])) {
			$aname=$URI[2];
		}
	}
}

$request = array(
	"jsonrpc" => "1.0",
	"id" => 1,
	"method" => "name_history",
	"params" => array("a://".$aname)
	);
$request = json_encode($request);
//echo $request;

$opts = array("http" => array(
	"method" => "POST",
	"header" => "Content-type: application/json",
	"content" => $request
	));

$context  = stream_context_create($opts);
if ($response = file_get_contents('http://zhao:gang@127.0.0.1:9877', false, $context)) {
	$obj = json_decode($response);

	$data = $obj->{'result'};
	//echo $data[0]->value;
} else {
	//echo '{ "code": -1, "message": "failed" }';
}


echo '<div class="container">';

		echo '
		<div class="panel panel-default">
			<div class="panel-heading">
				<h3 class="panel-title">'.lang("NAME_DETAILS").' - '.$aname.'</h3>
			</div>
			<div class="panel-body">

				<table class="table">';

echo '<tr><td>'.lang("VALUE_VALUENVS").'</td><td>'.$data[0]->value.'</td></tr>';

?>
	
