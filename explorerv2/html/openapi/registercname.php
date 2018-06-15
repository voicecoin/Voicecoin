<?php

$request = array(
	"jsonrpc" => "1.0",
	"id" => (int)$_REQUEST["requestid"],
	"method" => "name_new",
	"params" => array("c://".$_REQUEST["name"], $_REQUEST["value"], (int)$_REQUEST["days_added"])
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
	//echo $response;
	$ret = array(
		"code" => 0,
		"message" => "success",
		"data" => array('txid' => $obj->{'result'}));
		echo json_encode($ret);
	} else {
		echo '{ "code": -1, "message": "failed" }';
	}
?>
