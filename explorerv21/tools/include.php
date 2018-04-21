<?php
error_reporting(0);
$dbconn = new mysqli("localhost", "root","zhaogang", "emcchain");
// Check connection
if ($dbconn->connect_error) {
    die("Connection failed: " . $dbconn->connect_error);
}

$dbconn2 = new mysqli("localhost", "root","zhaogang", "emcchain");
// Check connection
if ($dbconn2->connect_error) {
    die("Connection failed: " . $dbconn2->connect_error);
}

require_once '/var/www/tools/include/jsonRPCClient.php';
$emercoin = new jsonRPCClient('http://zhao:gang@192.168.111.132:6662/');
?>
