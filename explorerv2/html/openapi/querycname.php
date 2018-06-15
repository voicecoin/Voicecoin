<?php

$request = array(
	"jsonrpc" => "1.0",
	"id" => (int)$_REQUEST["requestid"],
	"method" => "name_history",
	"params" => array("c://".$_REQUEST["name"])
	);
$request = json_encode($request);
// echo $request;

$opts = array("http" => array(
	"method" => "POST",
	"header" => "Content-type: application/json",
	"content" => $request
	));

$context  = stream_context_create($opts);
if ($response = file_get_contents('http://zhao:gang@127.0.0.1:9877', false, $context)) {
	$obj = json_decode($response);
	$ret = array(
		"code" => 0,
		"message" => "success",
		"data" => $obj->{'result'}[0]);
	echo json_encode($ret);
} else {
	echo '{ "code": -1, "message": "failed" }';
}

?>
