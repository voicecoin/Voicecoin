<?php
error_reporting(0);
ini_set("display_errors", 0);

$params = $_REQUEST;

/*
$appkey="";
foreach ($params as $k => $v){
	if ($k != "sign")
		$arr[$k]=urldecode($v);
	if ($k == "appkey")
		$appkey = urldecode($v);
}
ksort($arr);
foreach ($arr as $k => $v){
	$str = $str.$k.'='.$v;
}

if ($appkey != "voicegroup")
{
	echo '{ "code": -1, "message": "invalid appkey" }';
	exit;
}

$sigsrc = $str . 'voicegroup123';

if (md5($sigsrc) != $params['sign'])
{
	echo '{ "code": -1, "message": "sign check failed. '.md5($sigsrc).' }';
	exit;
}
 */

$uripath = $_SERVER['REQUEST_URI'];
if (($pos=strpos($uripath, '?')))
	$uripath = substr($uripath, 0, $pos);

$method_name="";
$URI=explode('/',$uripath);
if ($URI[1]=="openapi") {
	if (isset($URI[2])) {
		$method_name=urldecode($URI[2]);
	}
}

if ($method_name!="") {
	include ("openapi/".$method_name.".php");
	exit;
}

?>
